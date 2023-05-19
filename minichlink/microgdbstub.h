/* 
 * Micro GDBStub Driver, for implementing a gdbserver.
 * Copyright (C) Charles Lohr 2023
 *  You may freely license this file under the MIT-x11, or the 2- or 3- or New BSD Licenses.
 *  You may also use this as though it is public domain.
 *
 * This project is based off of picorvd. https://github.com/aappleby/PicoRVD/
 *
 * Simply:
 *   1: define the RV_ Functions
 *   2: Call the MicroGDB* functions needed.
 *   3: Define MICROGDBSTUB_IMPLEMENTATION at least in one place this is included in your program.
 *   4: If you want to let this manage the server as a network device, simply #define MICROGDBSTUB_SOCKETS
 *
 * To connect to your GDBStub running, you can:
 *  gdb-multiarch -ex 'target remote :2000' ./blink.elf 
 *
 */

#ifndef _MICROGDBSTUB_H
#define _MICROGDBSTUB_H

// You must write these for your processor.
void RVNetPoll(void * dev );
int RVSendGDBHaltReason( void * dev );
void RVNetConnect( void * dev );
int RVReadCPURegister( void * dev, int regno, uint32_t * regret );
void RVDebugExec( void * dev, int halt_reset_or_resume );
int RVReadMem( void * dev, uint32_t memaddy, uint8_t * payload, int len );
int RVHandleBreakpoint( void * dev, int set, uint32_t address );
int RVWriteRAM(void * dev, uint32_t memaddy, uint32_t length, uint8_t * payload );
void RVHandleDisconnect( void * dev );
void RVHandleGDBBreakRequest( void * dev );
void RVHandleKillRequest( void * dev );

#ifdef MICROGDBSTUB_SOCKETS
int MicroGDBPollServer( void * dev );
int MicroGDBStubStartup( void * dev );
void MicroGDBExitServer( void * dev );
#endif

// If you are not a network socket, you can pass in this data.
void MicroGDBStubSendReply( const void * data, int len, int docs );
void MicroGDBStubHandleClientData( void * dev, const uint8_t * rxdata, int len );


#ifdef MICROGDBSTUB_IMPLEMENTATION

///////////////////////////////////////////////////////////////////////////////
// Protocol Stuff

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef MICROGDBSTUB_SOCKETS
#if defined( WIN32 ) || defined( _WIN32 )
#include <winsock2.h>
#if !defined( POLLIN ) 
typedef struct pollfd { SOCKET fd; SHORT  events; SHORT  revents; };
#define POLLIN 0x0001
#define POLLERR 0x008
#define POLLHUP 0x010
int WSAAPI WSAPoll(struct pollfd * fdArray, ULONG       fds, INT         timeout );
#endif
#define poll WSAPoll
#define socklen_t uint32_t
#define SHUT_RDWR SD_BOTH
#define MSG_NOSIGNAL 0
#else
#define closesocket close
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#endif
#endif

#ifdef __linux__
#include <linux/in.h>
#endif

char gdbbuffer[65536];
uint8_t gdbchecksum = 0;
int gdbbufferplace = 0;
int gdbbufferstate = 0;


static inline char ToHEXNibble( int i )
{
	i &= 0xf;
	return ( i < 10 )?('0' + i):('a' - 10 + i);
}

static int fromhex( char c )
{
	if( c >= '0' && c <= '9' ) c = c - '0';
	else if( c >= 'A' && c <= 'F' ) c = c - 'A' + 10;
	else if( c >= 'a' && c <= 'f' ) c = c - 'a' + 10;
	else return -1;
	return c;
}

// if (numhex < 0) 
static int ReadHex( char ** instr, int numhex, uint32_t * outwrite )
{
	if( !instr ) return -1;
	char * str = *instr;
	// If negative - error.
	// If positive - number of bytes read.

	*outwrite = 0;

	int scanhex = numhex;
	if( scanhex < 0 )
		scanhex = strlen( str );

	int i;
	for( i = 0; i < scanhex; i++ )
	{
		int v = fromhex( *(str++) );
		if( v < 0 )
		{
			if( numhex < 0 )
			{
				str--;
				*instr = str;
				return i;
			}
			else
			{
				*instr = str;
				return - i - 1;
			}
		}
		(*outwrite) = ((*outwrite) << 4) | v;
	}
	*instr = str;
	return i;
}

static int StringMatch( const char * haystack, const char * mat )
{
	int i;
	for( i = 0; mat[i] && haystack[i]; i++ )
		if( mat[i] != haystack[i] || haystack[i] == 0 ) break;
	return mat[i] == 0;
}

void SendReplyFull( const char * replyMessage )
{
	MicroGDBStubSendReply( replyMessage, -1, '$' );
}

///////////////////////////////////////////////////////////////////////////////
// General Protocol

void HandleGDBPacket( void * dev, char * data, int len )
{
	int i;

	//printf( ":::%s:::\n", data );
	// Got a packet?
	if( data[0] != '$' ) return;

	data++;

	char cmd = *(data++);
	switch( cmd )
	{
	case 'q':
		if( StringMatch( data, "Attached" ) )
		    SendReplyFull( "1" ); //Attached to an existing process.
		else if( StringMatch( data, "Supported" ) )
		    SendReplyFull( "PacketSize=f000;qXfer:memory-map:read+" );
		else if( StringMatch( data, "C") ) // Get Current Thread ID. (Can't be -1 or 0.  Those are special)
		    SendReplyFull( "QC1" );
		else if( StringMatch( data, "fThreadInfo" ) )  // Query all active thread IDs (Can't be 0 or 1)
			SendReplyFull( "m1" );
		else if( StringMatch( data, "sThreadInfo" ) )  // Query all active thread IDs, continued
		    SendReplyFull( "l" );
		else if( StringMatch( data, "Xfer:memory-map" ) )
		    SendReplyFull( MICROGDBSTUB_MEMORY_MAP );
		else
			SendReplyFull( "" );
		break;
	case 'c':
	case 'C':
	case 's':
		RVDebugExec( dev, (cmd == 'C')?4:2 );
		SendReplyFull( "OK" );
		break;
	case 'D':
		// Handle disconnect.
		RVHandleDisconnect( dev );
		break;
	case 'k':
		RVHandleKillRequest( dev ); // no reply.
		break;
	case 'Z':
	case 'z':
	{
		uint32_t type = 0;
		uint32_t addr = 0;
		uint32_t time = 0;
		if( ReadHex( &data, -1, &type ) < 0 ) goto err;
		if( *(data++) != ',' ) goto err;
		if( ReadHex( &data, -1, &addr ) < 0 ) goto err;
		if( *(data++) != ',' ) goto err;
		if( ReadHex( &data, -1, &time ) < 0 ) goto err;
		if( RVHandleBreakpoint( dev, cmd == 'Z', addr ) == 0 )
		{
			SendReplyFull( "OK" );
		}
		else
			goto err;
		break;
	}
	case 'm':
	{
		// Read memory (Binary)
		uint32_t address_to_read = 0;
		uint32_t length_to_read = 0;
		if( ReadHex( &data, -1, &address_to_read ) < 0 ) goto err;
		if( *(data++) != ',' ) goto err;
		if( ReadHex( &data, -1, &length_to_read ) < 0 ) goto err;

		uint8_t * pl = alloca( length_to_read * 2 );
		if( RVReadMem( dev, address_to_read, pl, length_to_read ) < 0 )
			goto err;
		char * repl = alloca( length_to_read * 2 );
		int i;
		for( i = 0; i < length_to_read; i++ )
		{
			sprintf( repl + i * 2, "%02x", pl[i] );
		}
		SendReplyFull( repl );
		break;
	}
	case 'M':
	{
		uint32_t address_to_write = 0;
		uint32_t length_to_write = 0;
		if( ReadHex( &data, -1, &address_to_write ) < 0 ) goto err;
		if( *(data++) != ',' ) goto err;
		if( ReadHex( &data, -1, &length_to_write ) < 0 ) goto err;
		if( *(data++) != ':' ) goto err;

		uint8_t * meml = alloca( length_to_write );
		int i;
		for( i = 0; i < length_to_write; i++ )
		{
			uint32_t rv;
			if( ReadHex( &data, 2, &rv ) < 0 ) goto err;
			meml[i] = rv;
		}
		if( RVWriteRAM( dev, address_to_write, length_to_write, meml ) < 0 ) goto err;
		SendReplyFull( "OK" );
		break;
	}
	case 'X':
	{
		// Write memory, binary.
		uint32_t address_to_write = 0;
		uint32_t length_to_write = 0;
		if( ReadHex( &data, -1, &address_to_write ) < 0 ) goto err;
		if( *(data++) != ',' ) goto err;
		if( ReadHex( &data, -1, &length_to_write ) < 0 ) goto err;
		if( *(data++) != ':' ) goto err;
		if( RVWriteRAM( dev, address_to_write, length_to_write, (uint8_t*)data ) < 0 ) goto err;
		SendReplyFull( "OK" );
		break;
	}
	case 'v':
		if( StringMatch( data, "Cont?" ) )
		{
			// Request a list of actions supported by the ‘vCont’ packet. 
			// We don't support vCont
			SendReplyFull( "vCont;;c;;" ); //no ;s or ;t because we don't implement them.
		}
		else
		{
			SendReplyFull( "" ); //"vMustReplyEmpty"
		}
		break;
	case 'g':
	{
		// Register Read (All regs)
		char cts[17*8+1];
		for( i = 0; i < 17; i++ )
		{
			uint32_t regret;
			if( RVReadCPURegister( dev, i, &regret ) ) goto err;
			sprintf( cts + i * 8, "%08x", (uint32_t)htonl( regret ) );
		}
		SendReplyFull( cts );
		break;
	}
	case 'p':
	{
		uint32_t regno;
		// Register Read (Specific Reg)
		if( ReadHex( &data, 2, &regno ) < 0 )
			SendReplyFull( "E 10" );
		else
		{
			char cts[9];
			uint32_t regret;
			if( RVReadCPURegister( dev, regno, &regret ) ) goto err;
			sprintf( cts, "%08x", (uint32_t)htonl( regret ) );
			SendReplyFull( cts );
		}
		break;
	}
	case '?': // Query reason for target halt.
		RVSendGDBHaltReason( dev );
		break;
	case 'H':
		// This is for things like selecting threads.
		// I am going to leave this stubbed out.
		SendReplyFull( "" );
		break;
	default:
		printf( "UNKNOWN PACKET: %d (%s)\n", len, data-1 );
		for( i = 0; i < len; i++ )
		{
			printf( "%02x ", data[i] );
		}
		printf( "\n" );
		goto err;
		break;
	}
	return;
err:
	SendReplyFull( "E 00" );
}

void MicroGDBStubHandleClientData( void * dev, const uint8_t * rxdata, int len )
{
	int pl = 0;
	for( pl = 0; pl < len; pl++ )
	{
		int c = rxdata[pl];
		if( c == '$' && gdbbufferstate == 0 )
		{
			gdbbufferplace = 0;
			gdbbufferstate = 1;
		}
		if( c == 3 && gdbbufferstate == 0 )
		{
			RVHandleGDBBreakRequest( dev );
			continue;
		}

		switch( gdbbufferstate )
		{
		default:
			break;
		case 1:
			if( gdbbufferplace < sizeof( gdbbuffer ) - 2 )
			{
				if( c == '}' ) { gdbbufferstate = 9; break; }
				gdbbuffer[gdbbufferplace++] = c;
			}
			if( c == '#' ) gdbbufferstate = 2;
			break;
		case 9: // escape
			gdbbuffer[gdbbufferplace++] = c ^ 0x20;
			break;
		case 2:
		case 3:
		{
			int i;
			c = fromhex( c );
			if( gdbbufferstate == 2 )
			{
				gdbchecksum = c << 4;
				gdbbufferstate++;
				break; 
			}
			if( gdbbufferstate == 3 ) gdbchecksum |= c;
			
			gdbbuffer[gdbbufferplace] = 0;

			uint8_t checkcsum = 0;
			for( i = 1; i < gdbbufferplace - 1; i++ )
			{
				checkcsum += ((uint8_t*)gdbbuffer)[i];
			}
			if( checkcsum == gdbchecksum )
			{
				MicroGDBStubSendReply( "+", -1, 0 );
				HandleGDBPacket( dev, (char*)gdbbuffer, gdbbufferplace );
			}
			else
			{
				MicroGDBStubSendReply( "-", -1, 0 );
			}

			gdbbufferplace = 0;
			gdbbufferstate = 0;
			break;
		}
		}
	}
}




#ifdef MICROGDBSTUB_SOCKETS

#include <fcntl.h>
#include <sys/time.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


int listenMode; // 0 for uninit.  1 for server, 2 for client.
int serverSocket;

///////////////////////////////////////////////////////////////////////////////
// Network layer.


void MicroGDBStubHandleDisconnect( void * dev )
{
	RVHandleDisconnect( dev );
}

static int GDBListen( void * dev )
{
	struct	sockaddr_in sin;
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	//Make sure the socket worked.
	if( serverSocket == -1 )
	{
		fprintf( stderr, "Error: Cannot create socket.\n" );
		return -1;
	}

	//Disable SO_LINGER (Well, enable it but turn it way down)
#if defined( WIN32 ) || defined( _WIN32 )
	struct linger lx;
	lx.l_onoff = 1;
	lx.l_linger = 0;
	setsockopt( serverSocket, SOL_SOCKET, SO_LINGER, (const char *)&lx, sizeof( lx ) );

	//Enable SO_REUSEADDR
	int reusevar = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reusevar, sizeof(reusevar));
#else
	struct linger lx;
	lx.l_onoff = 1;
	lx.l_linger = 0;
	setsockopt( serverSocket, SOL_SOCKET, SO_LINGER, &lx, sizeof( lx ) );

	//Enable SO_REUSEADDR
	int reusevar = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reusevar, sizeof(reusevar));
#endif
	//Setup socket for listening address.
	memset( &sin, 0, sizeof( sin ) );
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons( MICROGDBSTUB_PORT );

	//Actually bind to the socket
	if( bind( serverSocket, (struct sockaddr *) &sin, sizeof( sin ) ) == -1 )
	{
		fprintf( stderr, "Could not bind to socket: %d\n", MICROGDBSTUB_PORT );
		closesocket( serverSocket );
		serverSocket = 0;
		return -1;
	}

	//Finally listen.
	if( listen( serverSocket, 5 ) == -1 )
	{
		fprintf(stderr, "Could not lieten to socket.");
		closesocket( serverSocket );
		serverSocket = 0;
		return -1;
	}
	return 0;
}

int MicroGDBPollServer( void * dev )
{
	if( !serverSocket ) return -4;

	struct pollfd allpolls[2];

	int pollct = 1;
	allpolls[0].fd = serverSocket;
	allpolls[0].events = POLLIN;

	//Do something to watch all currently-waiting sockets.
	poll( allpolls, pollct, 0 );

	//If there's faults, bail.
	if( allpolls[0].revents & (POLLERR|POLLHUP) )
	{
		closesocket( serverSocket );
		if( listenMode == 1 )
		{
			// Some sort of weird fatal close?  Is this even possible?
			fprintf( stderr, "Error: serverSocke was forcibly closed\n" );
			exit( -4 );
		}
		else if( listenMode == 2 )
		{
			MicroGDBStubHandleDisconnect( dev );
			if( serverSocket ) 	close( serverSocket );
			serverSocket = 0;
			listenMode = 1;
			GDBListen( dev );
		}
	}
	if( allpolls[0].revents & POLLIN )
	{
		if( listenMode == 1 )
		{
			struct   sockaddr_in tin;
			socklen_t addrlen  = sizeof(tin);
			memset( &tin, 0, addrlen );
			int tsocket = accept( serverSocket, (struct sockaddr *)&tin, (void*)&addrlen );
			closesocket( serverSocket );
			serverSocket = tsocket;
			listenMode = 2;
			gdbbufferstate = 0;
			RVNetConnect( dev );
			// Established.
		}
		else if( listenMode == 2 )
		{
			// Got data from a peer.
			uint8_t buffer[16384];
			ssize_t rx = recv( serverSocket, (char*)buffer, sizeof( buffer ), MSG_NOSIGNAL );
			if( rx == 0 )
			{
				MicroGDBStubHandleDisconnect( dev );
				close( serverSocket );
				serverSocket = 0;
				listenMode = 1;
				GDBListen( dev );
			}
			else
				MicroGDBStubHandleClientData( dev, buffer, (int)rx );
		}
	}

	if( listenMode == 2 )
	{
		RVNetPoll( dev );
	}

	return 0;
}

void MicroGDBExitServer( void * dev )
{
	shutdown( serverSocket, SHUT_RDWR );
	if( listenMode == 2 )
	{
		MicroGDBStubHandleDisconnect( dev );
	}
}


void MicroGDBStubSendReply( const void * data, int len, int docs )
{
	if( len < 0 ) len = strlen( data );
	if( docs )
	{
		uint8_t * localbuffer = alloca( len ) + 5;
		localbuffer[0] = '$';
		uint8_t checksum = 0;
		int i;
		for( i = 0; i < len; i++ )
		{
			uint8_t v = ((const uint8_t*)data)[i];
			checksum += v;
			localbuffer[i+1] = v;
		}
		localbuffer[len+1] = '#';
		localbuffer[len+2] = ToHEXNibble( checksum >> 4 );
		localbuffer[len+3] = ToHEXNibble( checksum );
		localbuffer[len+4] = 0;
		data = (void*)localbuffer;
		len += 4;
	}
	
	if( listenMode == 2 )
	{
		//printf( ">>>>%s<<<<(%d)\n", data );
		send( serverSocket, data, len, MSG_NOSIGNAL );
	}
}


int MicroGDBStubStartup( void * dev )
{
#if defined( WIN32 ) || defined( _WIN32 )
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        fprintf( stderr, "WSAStartup failed with error: %d\n", err);
        return 1;
    }
}
#endif

	listenMode = 1;

	return GDBListen( dev );
}


#endif
#endif

#endif

