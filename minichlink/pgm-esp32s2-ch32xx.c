#include <stdint.h>
#include "hidapi.c"
#include "minichlink.h"

struct ESP32ProgrammerStruct
{
	hid_device * hd;
	uint8_t commandbuffer[256];
	int commandplace;
	uint8_t reply[256];
	int replylen;
};

int ESPFlushLLCommands( void * dev );

static inline int SRemain( struct ESP32ProgrammerStruct * e )
{
	return sizeof( e->commandbuffer ) - e->commandplace - 1; //Need room for EOF.
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
	if( SRemain( eps ) < 5 ) ESPFlushLLCommands( eps );

	Write1( eps, (reg_7_bit<<1) | 1 );
	Write4LE( eps, value );
}

int ESPReadReg32( void * dev, uint8_t reg_7_bit, uint32_t * commandresp )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	ESPFlushLLCommands( eps );

	Write1( eps, (reg_7_bit<<1) | 0 );

	int len = ESPFlushLLCommands( eps );

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
	if( eps->reply[0] == 0xff ) goto retry;
//printf( ">:::%d: %02x %02x %02x %02x %02x %02x\n", eps->replylen, eps->reply[0], eps->reply[1], eps->reply[2], eps->reply[3], eps->reply[4], eps->reply[5] );
	if( r < 0 )
	{
		fprintf( stderr, "Error: Got error %d when sending hid feature report.\n", r );
		return r;
	}

	eps->replylen = r;
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
}

int ESPDelayUS( void * dev, int microseconds )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	if( SRemain( eps ) < 6 )
		ESPFlushLLCommands( eps );

	Write2LE( eps, 0x04fe );
	Write4LE( eps, microseconds );
}


int ESPExit( void * dev )
{
	struct ESP32ProgrammerStruct * eps = (struct ESP32ProgrammerStruct *)dev;
	hid_close( eps->hd );
	free( eps );
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

	return eps;
}


