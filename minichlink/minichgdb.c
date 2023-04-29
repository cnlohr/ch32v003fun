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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

int listenMode; // 0 for uninit.  1 for server, 2 for client.
int serverSocket;

char gdbbuffer[65536];
uint8_t gdbchecksum = 0;
int gdbbufferplace = 0;
int gdbbufferstate = 0;
void HandleClientData( const uint8_t * rxdata, int len )
{
	int pl = 0;
	do
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
			if( gdbbufferplace < 65535 )
			{
				gdbbuffer[gdbbufferplace++] = c;
			}
			if( c == '#' ) gdbbufferstate = 2;
		case 2:
		case 3:
			if( c >= '0' && c <= '9' ) c = c - '0';
			else if( c >= 'A' && c <= 'F' ) c = c - 'A' + 10;
			else if( c >= 'a' && c <= 'f' ) c = c - 'a' + 10;
			if( gdbbufferstate == 2 ) gdbchecksum = c << 4;
			else if( gdbbufferstate == 3 ) gdbchecksum = c << 4;
			gdbbufferstate++;
			break;
		case 4:
			// Got a packet?
		{
			int i;
			for( i = 0; i < gdbbufferplace; i++ )
			{
			}
		}
		}
	if( gdbbufferstate == 0 )

			printf( "rececived: %d\n", rx );
			int i;
			for( i = 0; i < rx;i ++ )
			{
				printf( "%02x (%c) ", buffer[i], buffer[i] );
			}
			printf( "\n" );

}

#define GDBSERVER_PORT 2345

static int GDBListen()
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

int PollGDBServer()
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
			serverSocket = 0;
			listenMode = 1;
			GDBListen();
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
			HandleClientData( buffer, (int)rx );
		}
	}

	if( listenMode == 2 )
	{
		// Otherwise, is anything else interesting going on?
	}

	return 0;
}

void ExitGDBServer()
{
	shutdown( serverSocket, SHUT_RDWR );
}

int SetupGDBServer()
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

	return GDBListen();
}


