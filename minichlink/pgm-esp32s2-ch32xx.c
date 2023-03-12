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
	r = hid_send_feature_report( eps->hd, eps->commandbuffer, 255 );
	eps->commandplace = 1;
	if( r < 0 )
	{
		fprintf( stderr, "Error: Got error %d when sending hid feature report.\n", r );
		return r;
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
	Write4LE( eps, microseconds );
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

static int ESPWaitForDoneOp( void * dev )
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
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	ESPFlushLLCommands( dev );
	Write2LE( eps, 0x0bfe );
	Write4LE( eps, address_to_write );
	int i;
	for( i = 0; i < 64; i++ ) Write1( eps, data[i] );
	do
	{
		ESPFlushLLCommands( dev );
	} while( eps->replylen < 2 );
	return eps->reply[1];
}

int ESPPerformSongAndDance( void * dev )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	Write2LE( eps, 0x01fe );
	ESPFlushLLCommands( dev );	
	return 0;
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

	memset( &MCF, 0, sizeof( MCF ) );
	MCF.WriteReg32 = ESPWriteReg32;
	MCF.ReadReg32 = ESPReadReg32;
	MCF.FlushLLCommands = ESPFlushLLCommands;
	MCF.DelayUS = ESPDelayUS;
	MCF.Control3v3 = ESPControl3v3;
	MCF.Exit = ESPExit;

	// These are optional. Disabling these is a good mechanismto make sure the core functions still work.
	MCF.WriteWord = ESPWriteWord;
	MCF.ReadWord = ESPReadWord;

	MCF.WaitForFlash = ESPWaitForFlash;
	MCF.WaitForDoneOp = ESPWaitForDoneOp;

	MCF.PerformSongAndDance = ESPPerformSongAndDance;

	MCF.BlockWrite64 = ESPBlockWrite64;

	// Reset internal programmer state.
	Write2LE( eps, 0x0afe );

	return eps;
}


