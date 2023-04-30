// This file is loosely based on aappleby's GDBServer.

#include "minichlink.h"

#ifdef WIN32
#include <winsock2.h>
#define socklen_t uint32_t
#define SHUT_RDWR SD_BOTH
#define MSG_NOSIGNAL 0
#else
#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/in.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <poll.h>
uint16_t htons(uint16_t hostshort);
uint32_t htonl(uint32_t hostlong);

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define GDBSERVER_PORT 2000
void SendReplyFull( const char * replyMessage );

///////////////////////////////////////////////////////////////////////////////
// Actual Chip Operations

// TODO: Breakpoints
// TODO: Unaligned access?

int shadow_running_state = 1;

void RVNetConnect( void * dev )
{
	// ???
	MCF.HaltMode( dev, 0 );
	shadow_running_state = 0;
}

void RVNetPoll(void * dev )
{
	// ???
}

int RVReadCPURegister( void * dev, int regno, uint32_t * regret )
{
	if( shadow_running_state )
	{
		printf( "HALTTTTTTTTTTTTTTTTTTTTTT\n" );
		MCF.HaltMode( dev, 0 );
		shadow_running_state = 0;
	}

	if( regno > 16 ) return 0; // Invalid register.

	if( !MCF.ReadCPURegister )
	{
		fprintf( stderr, "Error: Programmer does not support register reading\n" );
	}

	return MCF.ReadCPURegister( dev, regno, regret );
}

void RVContinueExec( void * dev )
{
	if( !MCF.HaltMode )
	{
		fprintf( stderr, "Error: Can't alter halt mode with this programmer.\n" );
		exit( -6 );
	}

	if( !shadow_running_state )
		MCF.HaltMode( dev, 2 );

	shadow_running_state = 1;

	//SendReplyFull( "T05" );
	printf( "Cont\n" );
}

void RVStepExec( void * dev )
{
	if( !MCF.HaltMode )
	{
		fprintf( stderr, "Error: Can't alter halt mode with this programmer.\n" );
		exit( -6 );
	}

	if( !shadow_running_state )
		MCF.HaltMode( dev, 2 );

	shadow_running_state = 1;

	printf( "Step\n" );
}

void RVStopExec( void * dev )
{
	if( !MCF.HaltMode )
	{
		fprintf( stderr, "Error: Can't alter halt mode with this programmer.\n" );
		exit( -6 );
	}

	if( shadow_running_state )
		MCF.HaltMode( dev, 0 );

	shadow_running_state = 0;

	printf( "Exec\n" );
}

int RVReadMem( void * dev, uint32_t memaddy, uint8_t * payload, int len )
{
	if( !MCF.ReadBinaryBlob )
	{
		fprintf( stderr, "Error: Can't alter halt mode with this programmer.\n" );
		exit( -6 );
	}

	MCF.ReadBinaryBlob( dev, memaddy, len, payload );

	return len;
}

int RVHandleBreakpoint( void * dev, int set, uint32_t address )
{
	
	return 0;
}

int RVWriteRAM(void * dev, uint32_t memaddy, uint32_t length, uint8_t * payload )
{
	if( !MCF.WriteBinaryBlob )
	{
		fprintf( stderr, "Error: Can't alter halt mode with this programmer.\n" );
		exit( -6 );
	}

	MCF.WriteBinaryBlob( dev, memaddy, length, payload );

	return 0;
}

void RVHandleDisconnect( void * dev )
{
	// Do nothing?
}


///////////////////////////////////////////////////////////////////////////////
// Protocol Stuff

int listenMode; // 0 for uninit.  1 for server, 2 for client.
int serverSocket;

char gdbbuffer[65536];
uint8_t gdbchecksum = 0;
int gdbbufferplace = 0;
int gdbbufferstate = 0;

static inline char ToHEXNibble( int i )
{
	i &= 0xf;
	return ( i < 10 )?('0' + i):('a' - 10 + i);
}

int fromhex( char c )
{
	if( c >= '0' && c <= '9' ) c = c - '0';
	else if( c >= 'A' && c <= 'F' ) c = c - 'A' + 10;
	else if( c >= 'a' && c <= 'f' ) c = c - 'a' + 10;
	else return -1;
	return c;
}

// if (numhex < 0) 
int ReadHex( char ** instr, int numhex, uint32_t * outwrite )
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

int StringMatch( const char * haystack, const char * mat )
{
	int i;
	for( i = 0; mat[i] && haystack[i]; i++ )
		if( mat[i] != haystack[i] || haystack[i] == 0 ) break;
	return mat[i] == 0;
}


///////////////////////////////////////////////////////////////////////////////
// General Protocol


void SendGDBReply( const void * data, int len, int docs )
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
		send( serverSocket, data, len, MSG_NOSIGNAL );
	}
}

void SendReplyFull( const char * replyMessage )
{
	SendGDBReply( replyMessage, -1, 1 );
}

void HandleGDBPacket( void * dev, char * data, int len )
{
	int i;

	// Got a packet?
	if( data[0] != '$' ) return;

	puts( data );

	data++;

	char cmd = *(data++);
	switch( cmd )
	{
	case 'q':
		if( StringMatch( data, "Attached" ) )
		    SendReplyFull( "1" );
		else if( StringMatch( data, "Supported" ) )
		    SendReplyFull( "PacketSize=f000;qXfer:memory-map:read+" );
		else if( StringMatch( data, "C") ) // Get Current Thread ID. (Can't be -1 or 0.  Those are special)
		    SendReplyFull( "QC1" );
		else if( StringMatch( data, "fThreadInfo" ) )  // Query all active thread IDs (Can't be 0 or 1)
			SendReplyFull( "m1" );
		else if( StringMatch( data, "sThreadInfo" ) )  // Query all active thread IDs, continued
		    SendReplyFull( "l" );
		else
			SendReplyFull( "" );
		break;
	case 'c':
		RVContinueExec( dev );
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
			SendReplyFull( "OK" );
		else
			goto err;
		break;
	}
	case 'm':
	{
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
			SendReplyFull( "vCont;c;s;t;" );
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
			sprintf( cts + i * 8, "%08x", htonl( regret ) );
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
			sprintf( cts, "%08x", htonl( regret ) );
			SendReplyFull( cts );
		}
		break;
	}
	case '?': // Query reason for target halt.
		SendReplyFull( "T05" ); // Ignoring - no need to stub, this is what PicoRVD does.
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

void HandleClientData( void * dev, const uint8_t * rxdata, int len )
{
	int pl = 0;
	for( pl = 0; pl < len; pl++ )
	{
		int c = rxdata[pl];
		if( c == '$' )
		{
			gdbbufferplace = 0;
			gdbbufferstate = 1;
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
				SendGDBReply( "+", -1, 0 );
				HandleGDBPacket( dev, (char*)gdbbuffer, gdbbufferplace );
			}
			else
			{
				SendGDBReply( "-", -1, 0 );
			}

			gdbbufferplace = 0;
			gdbbufferstate = 0;
			break;
		}
		}
	}
}


void HandleDisconnect( void * dev )
{
	RVHandleDisconnect( dev );
}

///////////////////////////////////////////////////////////////////////////////
// Network layer.

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
#ifdef WIN32
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
	sin.sin_port = htons( GDBSERVER_PORT );

	//Actually bind to the socket
	if( bind( serverSocket, (struct sockaddr *) &sin, sizeof( sin ) ) == -1 )
	{
		fprintf( stderr, "Could not bind to socket: %d\n", GDBSERVER_PORT );
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

int PollGDBServer( void * dev )
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
			HandleDisconnect( dev );
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
			int tsocket = accept( serverSocket, (struct sockaddr *)&tin, &addrlen );
			closesocket( serverSocket );
			serverSocket = tsocket;
			listenMode = 2;
			gdbbufferstate = 0;
			// Established.
		}
		else if( listenMode == 2 )
		{
			// Got data from a peer.
			uint8_t buffer[16384];
			ssize_t rx = recv( serverSocket, buffer, sizeof( buffer ), MSG_NOSIGNAL );
			if( rx == 0 )
			{
				HandleDisconnect( dev );
				close( serverSocket );
				serverSocket = 0;
				listenMode = 1;
				GDBListen( dev );
			}
			else
				HandleClientData( dev, buffer, (int)rx );
		}
	}

	if( listenMode == 2 )
	{
		RVNetPoll( dev );
	}

	return 0;
}

void ExitGDBServer( void * dev )
{
	shutdown( serverSocket, SHUT_RDWR );
}

int SetupGDBServer( void * dev )
{
#ifdef WIN32
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


