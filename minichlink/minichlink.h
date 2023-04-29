#ifndef _MINICHLINK_H
#define _MINICHLINK_H

#include <stdint.h>

struct MiniChlinkFunctions
{
	// All functions return 0 if OK, negative number if fault, positive number as status code.

	// Low-level functions, if they exist.
	int (*WriteReg32)( void * dev, uint8_t reg_7_bit, uint32_t command );
	int (*ReadReg32)( void * dev, uint8_t reg_7_bit, uint32_t * commandresp );
	int (*FlushLLCommands)( void * dev );
	int (*DelayUS)( void * dev, int microseconds );

	// Higher-level functions can be generated automatically.
	int (*SetupInterface)( void * dev );
	int (*Control3v3)( void * dev, int bOn );
	int (*Control5v)( void * dev, int bOn );
	int (*Unbrick)( void * dev ); // Turns on chip, erases everything, powers off.

	int (*Exit)( void * dev );

	int (*HaltMode)( void * dev, int mode ); //0 for halt, 1 for reset, 2 for resume
	int (*ConfigureNRSTAsGPIO)( void * dev, int one_if_yes_gpio );

	// WARNING: Reading/writing must be at 32-bit boundaries for 32-bit sizes.
	// WARNING: Writing binary blobs may write groups of 64-bytes.
	int (*WriteBinaryBlob)( void * dev, uint32_t address_to_write, uint32_t blob_size, uint8_t * blob );
	int (*ReadBinaryBlob)( void * dev, uint32_t address_to_read_from, uint32_t read_size, uint8_t * blob );
	int (*Erase)( void * dev, uint32_t address, uint32_t length, int type ); //type = 0 for fast, 1 for whole-chip

	// MUST be 4-byte-aligned.
	int (*VoidHighLevelState)( void * dev );
	int (*WriteWord)( void * dev, uint32_t address_to_write, uint32_t data );
	int (*ReadWord)( void * dev, uint32_t address_to_read, uint32_t * data );

	int (*WaitForFlash)( void * dev );
	int (*WaitForDoneOp)( void * dev );

	int (*PrintChipInfo)( void * dev );

	// Geared for flash, but could be anything.
	int (*BlockWrite64)( void * dev, uint32_t address_to_write, uint8_t * data );

	// TODO: What about 64-byte block-reads?
	// TODO: What about byte read/write?
	// TODO: What about half read/write?

	// Returns positive if received text.
	// Returns negative if error.
	// Returns 0 if no text waiting.
	// Note: YOU CANNOT make lsb of leaveflagA bit in place 0x80 be high!!!
	int (*PollTerminal)( void * dev, uint8_t * buffer, int maxlen, uint32_t leaveflagA, int leaveflagB );

	int (*PerformSongAndDance)( void * dev );

	int (*VendorCommand)( void * dev, const char * command );

	// Do Not override these.  they are cursed.
	int (*WriteHalfWord)( void * dev, uint32_t address_to_write, uint32_t data );
	int (*ReadHalfWord)( void * dev, uint32_t address_to_read, uint32_t * data );
};

/** If you are writing a driver, the minimal number of functions you can implement are:
	WriteReg32
	ReadReg32
	FlushLLCommands
*/

// Convert a 4-character string to an int.
#define STTAG( x ) (*((uint32_t*)(x)))

struct InternalState;

struct ProgrammerStructBase
{
	struct InternalState * internal;
	// You can put other things here.
};

struct InternalState
{
	uint32_t statetag;
	uint32_t currentstateval;
	uint32_t flash_unlocked;
	int lastwriteflags;
	int processor_in_mode;
};


#define DMDATA0        0x04
#define DMDATA1        0x05
#define DMCONTROL      0x10
#define DMSTATUS       0x11
#define DMHARTINFO     0x12
#define DMABSTRACTCS   0x16
#define DMCOMMAND      0x17
#define DMABSTRACTAUTO 0x18
#define DMPROGBUF0     0x20
#define DMPROGBUF1     0x21
#define DMPROGBUF2     0x22
#define DMPROGBUF3     0x23
#define DMPROGBUF4     0x24
#define DMPROGBUF5     0x25
#define DMPROGBUF6     0x26
#define DMPROGBUF7     0x27

#define DMCPBR       0x7C
#define DMCFGR       0x7D
#define DMSHDWCFGR   0x7E

extern struct MiniChlinkFunctions MCF;

// Returns 'dev' on success, else 0.
void * TryInit_WCHLinkE();
void * TryInit_ESP32S2CHFUN();
void * TryInit_NHCLink042(void);

// Returns 0 if ok, populated, 1 if not populated.
int SetupAutomaticHighLevelFunctions( void * dev );

// Useful for converting numbers like 0x, etc.
int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber );


int SetupGDBServer();
int PollGDBServer();
void ExitGDBServer();

#endif

