#if 0
#include <stdint.h>
#include "hidapi.h"
#include "minichlink.h"


struct B003FunProgrammerStruct
{
	void * internal;
	hid_device * hd;
	uint8_t commandbuffer[128];
	int commandplace;

/*
	uint32_t state;
	uint8_t reply[256];
	int replylen;
*/
};


static int B003FunFlushLLCommands( void * dev )
{
	return 0;
}

static int B003FunDelayUS( void * dev, int microseconds )
{
	usleep( microseconds );
	return 0;
}

static int B003FunSetupInterface( void * dev )
{
	return 0;
}

static int B003FunExit( void * dev )
{
	return 0;
}

// MUST be 4-byte-aligned.
static int B003FunWriteWord( void * dev, uint32_t address_to_write, uint32_t data )
{
	return 0;
}

static int B003FunReadWord( void * dev, uint32_t address_to_read, uint32_t * data )
{
	return 0;
}

static int B003FunWaitForDoneOp( void * dev, int ignore )
{
	// ... Need this.
	return 0;
}


static int B003FunBlockWrite64( void * dev, uint32_t address_to_write, uint8_t * data )
{
	return 0;
}

static int B003FunWriteHalfWord( void * dev, uint32_t address_to_write, uint16_t data )
{
}

static int B003FunReadHalfWord( void * dev, uint32_t address_to_read, uint16_t * data )
{
}

static int B003FunWriteByte( void * dev, uint32_t address_to_write, uint8_t data )
{
}

static int B003FunReadByte( void * dev, uint32_t address_to_read, uint8_t * data )
{
}


static void ResetOp( struct B003FunProgrammerStruct * eps )
{
	memcpy( eps->commandbuffer, "\xaa\x00\x00\x00", 4 );
	eps->commandplace = 4;
}

static void WriteOp4( struct B003FunProgrammerStruct * eps, uint32_t opsend )
{
	int place = eps->commandplace;
	int newend = place + 4;
	if( newend < sizeof( eps->commandbuffer ) )
	{
		memcpy( eps->commandbuffer + place, &opsend, 4 );
	}
	eps->commandplace = newend;
}

static void CommitOp( struct B003FunProgrammerStruct * eps )
{
}
	CommitOp( eps );



void * TryInit_B003Fun()
{
	#define VID 0x1209
	#define PID 0xb003
	hid_init();
	hid_device * hd = hid_open( VID, PID, 0); // third parameter is "serial"
	if( !hd ) return 0;

	struct B003FunProgrammerStruct * eps = malloc( sizeof( struct B003FunProgrammerStruct ) );
	memset( eps, 0, sizeof( *eps ) );
	eps->hd = hd;
	eps->commandplace = 1;

	memset( &MCF, 0, sizeof( MCF ) );
	MCF.WriteReg32 = 0;
	MCF.ReadReg32 = 0;
	MCF.FlushLLCommands = B003FunFlushLLCommands;
	MCF.DelayUS = B003FunDelayUS;
	MCF.Control3v3 = 0;
	MCF.SetupInterface = B003FunSetupInterface;
	MCF.Exit = B003FunExit;
	MCF.HaltMode = 0;
	MCF.VoidHighLevelState = 0;
	MCF.PollTerminal = 0;

	// These are optional. Disabling these is a good mechanismto make sure the core functions still work.
	MCF.WriteWord = B003FunWriteWord;
	MCF.ReadWord = B003FunReadWord;

	MCF.WaitForDoneOp = B003FunWaitForDoneOp;

	MCF.BlockWrite64 = B003FunBlockWrite64;

	// Alert programmer.
	ResetOp( eps )
	WriteOp4( eps, 0x00b02023 );  //sw a1, 0       ; stop execution
	WriteOp4( eps, 0x00008067 );  //jalr x0, x1, 0 ; ret
	CommitOp( eps );

	return eps;
}
#else
void * TryInit_B003Fun() { return 0; }
#endif
