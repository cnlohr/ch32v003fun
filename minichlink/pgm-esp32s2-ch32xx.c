#include <stdint.h>
#include "hidapi.c"
#include "minichlink.h"

struct ESP32ProgrammerStruct
{
	void * internal;

	hid_device * hd;
	uint32_t state;
	uint8_t commandbuffer[256];
	int commandplace;
	uint8_t reply[256];
	int replylen;

	int dev_version;
};

int ESPFlushLLCommands( void * dev );

static inline int SRemain( struct ESP32ProgrammerStruct * e )
{
	return sizeof( e->commandbuffer ) - e->commandplace - 2; //Need room for EOF.
}

static inline void Write4LE( struct ESP32ProgrammerStruct * e, uint32_t val )
{
	if( SRemain( e ) < 4 ) return;
	uint8_t * d = e->commandbuffer + e->commandplace;
	d[0] = val & 0xff;
	d[1] = (val>>8) & 0xff;
	d[2] = (val>>16) & 0xff;
	d[3] = (val>>24) & 0xff;
	e->commandplace += 4;
}

static inline void Write2LE( struct ESP32ProgrammerStruct * e, uint16_t val )
{
	if( SRemain( e ) < 2 ) return;
	uint8_t * d = e->commandbuffer + e->commandplace;
	d[0] = val & 0xff;
	d[1] = (val>>8) & 0xff;
	e->commandplace += 2;
}

static inline void Write1( struct ESP32ProgrammerStruct * e, uint8_t val )
{
	if( SRemain( e ) < 1 ) return;
	uint8_t * d = e->commandbuffer + e->commandplace;
	d[0] = val & 0xff;
	e->commandplace ++;
}


static int ESPWriteReg32( void * dev, uint8_t reg_7_bit, uint32_t value )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
//	printf( "WriteReg: %02x -> %08x\n", reg_7_bit, value );


	if( SRemain( eps ) < 5 ) ESPFlushLLCommands( eps );

	Write1( eps, (reg_7_bit<<1) | 1 );
	Write4LE( eps, value );
	return 0;
}

int ESPReadReg32( void * dev, uint8_t reg_7_bit, uint32_t * commandresp )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	ESPFlushLLCommands( eps );
	Write1( eps, (reg_7_bit<<1) | 0 );

	ESPFlushLLCommands( eps );

//	printf( "ReadReg: %02x -> %d\n", reg_7_bit,eps->replylen );

	if( eps->replylen < 6 )
	{
		return -9;
	}
	else
	{
		memcpy( commandresp, eps->reply+2, 4 );
		return 0;
	}
}

int ESPFlushLLCommands( void * dev )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;

	if( eps->commandplace >= sizeof( eps->commandbuffer ) )
	{
		fprintf( stderr, "Error: Command buffer overflow\n" );
		return -5; 
	}

	if( eps->commandplace == 1 ) return 0;

	int r;

	eps->commandbuffer[0] = 0xad; // Key report ID
	eps->commandbuffer[eps->commandplace] = 0xff;

#if 0
	int i;
	for( i = 0; i < eps->commandplace; i++ )
	{
		if( ( i & 0xff ) == 0 ) printf( "\n" );
		printf( "%02x ", eps->commandbuffer[i] );
	}
	printf("\n" );
#endif

	r = hid_send_feature_report( eps->hd, eps->commandbuffer, 255 );
	eps->commandplace = 1;
	if( r < 0 )
	{
		fprintf( stderr, "Error: Got error %d when sending hid feature report.\n", r );
		exit( -9 );
	}
retry:
	eps->reply[0] = 0xad; // Key report ID
	r = hid_get_feature_report( eps->hd, eps->reply, sizeof( eps->reply ) );
/*
	int i;
	printf( "RESP: %d\n",eps->reply[0] );

	for( i = 0; i < eps->reply[0]; i++ )
	{
		printf( "%02x ", eps->reply[i+1] );
		if( (i % 16) == 15 ) printf( "\n" );
	}
	printf( "\n" );*/

	if( eps->reply[0] == 0xff ) goto retry;
//printf( ">:::%d: %02x %02x %02x %02x %02x %02x\n", eps->replylen, eps->reply[0], eps->reply[1], eps->reply[2], eps->reply[3], eps->reply[4], eps->reply[5] );
	if( r < 0 )
	{
		fprintf( stderr, "Error: Got error %d when sending hid feature report.\n", r );
		return r;
	}
	eps->replylen = eps->reply[0] + 1; // Include the header byte.
	return r;
}
	

int ESPControl3v3( void * dev, int bOn )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;

	if( SRemain( eps ) < 2 )
		ESPFlushLLCommands( eps );

	if( bOn )
		Write2LE( eps, 0x03fe );
	else
		Write2LE( eps, 0x02fe );
	return 0;
}


int ESPReadWord( void * dev, uint32_t address_to_read, uint32_t * data )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
//printf( "READ: %08x\n", address_to_read );
	if( SRemain( eps ) < 6 )
		ESPFlushLLCommands( eps );

	Write2LE( eps, 0x09fe );
	Write4LE( eps, address_to_read );
	ESPFlushLLCommands( eps );

//	printf( "Got: %d\n", eps->replylen );
	if( eps->replylen < 5 )
	{
		return -9;
	}
	int tail = eps->replylen-5;
	memcpy( data, eps->reply + tail + 1, 4 );
//	printf( "Read Mem: %08x => %08x\n", address_to_read, *data );
	return eps->reply[tail];
}

int ESPWriteWord( void * dev, uint32_t address_to_write, uint32_t data )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;

//printf( "WRITE: %08x\n", address_to_write );

	if( SRemain( eps ) < 10 )
		ESPFlushLLCommands( eps );

	Write2LE( eps, 0x08fe );
	Write4LE( eps, address_to_write );	
	Write4LE( eps, data );	
	return 0;
}

static int ESPDelayUS( void * dev, int microseconds )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	if( SRemain( eps ) < 6 )
		ESPFlushLLCommands( eps );

	Write2LE( eps, 0x04fe );
	Write2LE( eps, microseconds );
	return 0;
}


static int ESPWaitForFlash( void * dev )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	if( SRemain( eps ) < 2 )
		ESPFlushLLCommands( eps );
	Write2LE( eps, 0x06fe );
	return 0;
}

static int ESPWaitForDoneOp( void * dev, int ignore )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	if( SRemain( eps ) < 2 )
		ESPFlushLLCommands( dev );
	Write2LE( eps, 0x07fe );
	return 0;
}


int ESPExit( void * dev )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	hid_close( eps->hd );
	free( eps );
	return 0;
}

int ESPBlockWrite64( void * dev, uint32_t address_to_write, uint8_t * data )
{
	int writeretry = 0;
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	ESPFlushLLCommands( dev );

retry:

	if( eps->dev_version >= 2 && InternalIsMemoryErased( (struct InternalState*)eps->internal, address_to_write ) )
		Write2LE( eps, 0x0efe );
	else
		Write2LE( eps, 0x0bfe );
	Write4LE( eps, address_to_write );

	int i;
	int timeout = 0;
	for( i = 0; i < 64; i++ ) Write1( eps, data[i] );

	InternalMarkMemoryNotErased( (struct InternalState*)eps->internal, address_to_write );

	do
	{
		ESPFlushLLCommands( dev );
		timeout++;
		if( timeout > 1000 )
		{
			fprintf( stderr, "Error: Timed out block-writing 64\n" );
			return -49;
		}
	} while( eps->replylen < 2 );

	if( eps->reply[1] )
	{
		fprintf( stderr, "Error: Got code %d from ESP write algo. %d [%02x %02x %02x]\n", (char)eps->reply[1], eps->replylen, eps->reply[0], eps->reply[1], eps->reply[2] );
		if( writeretry < 10 )
		{
			writeretry++;
			goto retry;
		}
	}

	return (char)eps->reply[1];
}

int ESPPerformSongAndDance( void * dev )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	Write2LE( eps, 0x01fe );
	ESPFlushLLCommands( dev );	
	return 0;
}

int ESPVoidHighLevelState( void * dev )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	Write2LE( eps, 0x05fe );
	ESPFlushLLCommands( dev );	
	DefaultVoidHighLevelState( dev );
	return 0;
}

int ESPVendorCommand( void * dev, const char * cmd )
{
	char command[10] = { 0 };
	char tbuf[10] = { 0 };
	int fields[10];
	char c;
	int i = 0;
	int f = 0;
	while( (c = *cmd++) )
	{
		if( c == ':' ) break;
		if( c == '\0' ) break;
		if( i + 1 >= sizeof( command )) break;
		command[i++] = c;
		command[i] = 0;
	}
	i = 0;
	f = 0;
	while( 1 )
	{
		c = *cmd++;
		if( c == ':' || c == '\0' )
		{
			fields[f++] = SimpleReadNumberInt( tbuf,  0 );
			puts( tbuf );
			if( f == 10 ) break; 
			tbuf[0] = 0;
			i = 0;
			if( c == '\0' ) break;
			continue;
		}
		if( i + 1 >= sizeof( tbuf )) break;
		tbuf[i++] = c;
		tbuf[i] = 0;
	}
	printf( "Got Vendor Command \"%s\"\n", command );
	ESPFlushLLCommands( dev );
	if( strcasecmp( command, "ECLK" ) == 0 )
	{
		printf( "Setting up external clock on pin.\n" );
		if( f < 5 )
		{
			fprintf( stderr, "Error: Need fields :use_apll:sdm0:sdm1:sdm2:odiv try 1:0:0:8:3 for 24MHz\n" );
			fprintf( stderr, "Definition:\n\
	use_apll = Configures APLL = 480 / 4 = 120\n\
	40 * (SDM2 + SDM1/(2^8) + SDM0/(2^16) + 4) / ( 2 * (ODIV+2) );\n\
	Datasheet recommends that numerator is between 300 and 500MHz.\n ");
			return -9;
		}
		Write2LE( dev, 0x0cfe );
		Write1( dev, fields[0] ); 
		Write1( dev, fields[1] ); 
		Write1( dev, fields[2] ); 
		Write1( dev, fields[3] ); 
		Write1( dev, fields[4] ); 
		Write1( dev, 0 ); 
		Write1( dev, 0 ); 
		Write1( dev, 0 ); 
		ESPFlushLLCommands( dev );
	}
	else
	{
		fprintf( stderr, "Error: Unknown vendor command %s\n", command );
	}
	return 0;
}

int ESPPollTerminal( void * dev, uint8_t * buffer, int maxlen, uint32_t leaveflagA, int leaveflagB )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	ESPFlushLLCommands( dev );	
	Write1( dev, 0xfe );
	Write1( dev, 0x0d );
	Write4LE( dev, leaveflagA );
	Write4LE( dev, leaveflagB );
	Write1( dev, 0xff );

	ESPFlushLLCommands( dev );

	int rlen = eps->reply[0];
	if( rlen < 1 ) return -8;


#if 0
	int i;

	printf( "RESP (ML %d): %d\n", maxlen,eps->reply[0] );

	for( i = 0; i < eps->reply[0]; i++ )
	{
		printf( "%02x ", eps->reply[i+1] );
		if( (i % 16) == 15 ) printf( "\n" );
	}
	printf( "\n" );
#endif

	int errc = eps->reply[1];
	if( errc > 7 ) return -7;

	if( rlen - 1 >= maxlen ) return -6; 

	memcpy( buffer, eps->reply + 2, rlen - 1 );

	return rlen - 1;
}


void * TryInit_ESP32S2CHFUN()
{
	#define VID 0x303a
	#define PID 0x4004
	hid_init();
	hid_device * hd = hid_open( VID, PID, L"s2-ch32xx-pgm-v0"); // third parameter is "serial"
	if( !hd ) return 0;

	struct ESP32ProgrammerStruct * eps = malloc( sizeof( struct ESP32ProgrammerStruct ) );
	memset( eps, 0, sizeof( *eps ) );
	eps->hd = hd;
	eps->commandplace = 1;
	eps->dev_version = 0;

	memset( &MCF, 0, sizeof( MCF ) );
	MCF.WriteReg32 = ESPWriteReg32;
	MCF.ReadReg32 = ESPReadReg32;
	MCF.FlushLLCommands = ESPFlushLLCommands;
	MCF.DelayUS = ESPDelayUS;
	MCF.Control3v3 = ESPControl3v3;
	MCF.Exit = ESPExit;
	MCF.VoidHighLevelState = ESPVoidHighLevelState;
	MCF.PollTerminal = ESPPollTerminal;

	// These are optional. Disabling these is a good mechanismto make sure the core functions still work.
	MCF.WriteWord = ESPWriteWord;
	MCF.ReadWord = ESPReadWord;

	MCF.WaitForFlash = ESPWaitForFlash;
	MCF.WaitForDoneOp = ESPWaitForDoneOp;

	MCF.PerformSongAndDance = ESPPerformSongAndDance;

	MCF.BlockWrite64 = ESPBlockWrite64;
	MCF.VendorCommand = ESPVendorCommand;

	// Reset internal programmer state.
	Write2LE( eps, 0x0afe );
	ESPFlushLLCommands( eps );
	Write2LE( eps, 0xfefe );
	ESPFlushLLCommands( eps );
	if( eps->replylen > 1 )
	{
		eps->dev_version = eps->reply[1];
	}
	return eps;
}


