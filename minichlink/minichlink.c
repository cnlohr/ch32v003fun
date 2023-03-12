// The "bootloader" blob is (C) WCH.
// The rest of the code, Copyright 2023 Charles Lohr
// Freely licensable under the MIT/x11, NewBSD Licenses, or
// public domain where applicable. 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "minichlink.h"
#include "../ch32v003fun/ch32v003fun.h"

static int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber );
static int64_t StringToMemoryAddress( const char * number );

void TestFunction(void * v );
struct MiniChlinkFunctions MCF;

int main( int argc, char ** argv )
{
	void * dev = 0;
	if( (dev = TryInit_WCHLinkE()) )
	{
		fprintf( stderr, "Found WCH LinkE\n" );
	}
	else if( (dev = TryInit_ESP32S2CHFUN()) )
	{
		fprintf( stderr, "Found ESP32S2 Programmer\n" );
	}
	else
	{
		fprintf( stderr, "Error: Could not initialize any supported programmers\n" );
		return -32;
	}
	
	SetupAutomaticHighLevelFunctions( dev );

	int status;
	int must_be_end = 0;
	uint8_t rbuff[1024];

	if( MCF.SetupInterface )
	{
		if( MCF.SetupInterface( dev ) < 0 )
		{
			fprintf( stderr, "Could not setup interface.\n" );
			return -33;
		}
		printf( "Interface Setup\n" );
	}

//	TestFunction( dev );

	int iarg = 1;
	const char * lastcommand = 0;
	for( ; iarg < argc; iarg++ )
	{
		char * argchar = argv[iarg];

		lastcommand = argchar;
		if( argchar[0] != '-' )
		{
			fprintf( stderr, "Error: Need prefixing - before commands\n" );
			goto help;
		}
		if( must_be_end )
		{
			fprintf( stderr, "Error: the command '%c' cannot be followed by other commands.\n", must_be_end );
			return -1;
		}
		
keep_going:
		switch( argchar[1] )
		{
			default:
				fprintf( stderr, "Error: Unknown command %c\n", argchar[1] );
				goto help;
			case '3':
				if( MCF.Control3v3 )
					MCF.Control3v3( dev, 1 );
				else
					goto unimplemented;
				break;
			case '5':
				if( MCF.Control5v )
					MCF.Control5v( dev, 1 );
				else
					goto unimplemented;
				break;
			case 't':
				if( MCF.Control3v3 )
					MCF.Control3v3( dev, 0 );
				else
					goto unimplemented;
				break;
			case 'f':
				if( MCF.Control5v )
					MCF.Control5v( dev, 0 );
				else
					goto unimplemented;
				break;
			case 'u':
				if( MCF.Unbrick )
					MCF.Unbrick( dev );
				else
					goto unimplemented;
				break;
			case 'b': 
				if( !MCF.HaltMode || MCF.HaltMode( dev, 1 ) )
					goto unimplemented;
				break;
			case 'e':  //rEsume
				if( !MCF.HaltMode || MCF.HaltMode( dev, 2 ) )
					goto unimplemented;
				break;
			case 'E':  //Erase whole chip.
				if( !MCF.Erase || MCF.Erase( dev, 0, 0, 1 ) )
					goto unimplemented;
				break;
			case 'h':
				if( !MCF.HaltMode || MCF.HaltMode( dev, 0 ) )
				break;

			// disable NRST pin (turn it into a GPIO)
			case 'd':  // see "RSTMODE" in datasheet
				if( MCF.ConfigureNRSTAsGPIO )
					MCF.ConfigureNRSTAsGPIO( dev, 0 );
				else
					goto unimplemented;
				break;
			case 'D': // see "RSTMODE" in datasheet
				if( MCF.ConfigureNRSTAsGPIO )
					MCF.ConfigureNRSTAsGPIO( dev, 1 );
				else
					goto unimplemented;
				break;
			case 'T':
			{
				if( !MCF.PollTerminal )
					goto unimplemented;
				do
				{
					uint8_t buffer[128];
					int r = MCF.PollTerminal( dev, buffer, sizeof( buffer ) );
					if( r < 0 )
					{
						fprintf( stderr, "Terminal dead.  code %d\n", r );
						return -32;
					}
					if( r > 0 )
					{
						fwrite( buffer, r, 1, stdout ); 
					}
				} while( 1 );
			}
			// PROTECTION UNTESTED!
			/*
			case 'p':
				wch_link_multicommands( devh, 8,
					11, "\x81\x06\x08\x02\xf7\xff\xff\xff\xff\xff\xff",
					4, "\x81\x0b\x01\x01",
					4, "\x81\x0d\x01\xff",
					4, "\x81\x0d\x01\x01",
					5, "\x81\x0c\x02\x09\x01",
					4, "\x81\x0d\x01\x02",
					4, "\x81\x06\x01\x01",
					4, "\x81\x0d\x01\xff" );
				break;
			case 'P':
				wch_link_multicommands( devh, 7,
					11, "\x81\x06\x08\x03\xf7\xff\xff\xff\xff\xff\xff",
					4, "\x81\x0b\x01\x01",
					4, "\x81\x0d\x01\xff",
					4, "\x81\x0d\x01\x01",
					5, "\x81\x0c\x02\x09\x01",
					4, "\x81\x0d\x01\x02",
					4, "\x81\x06\x01\x01" );
				break;
			*/
			case 'r':
			{
				int i;
				int transferred;
				if( argchar[2] != 0 )
				{
					fprintf( stderr, "Error: can't have char after paramter field\n" ); 
					goto help;
				}
				iarg++;
				argchar = 0; // Stop advancing
				if( iarg + 2 >= argc )
				{
					fprintf( stderr, "Error: missing file for -o.\n" ); 
					goto help;
				}
				const char * fname = argv[iarg++];
				uint64_t offset = StringToMemoryAddress( argv[iarg++] );

				uint64_t amount = SimpleReadNumberInt( argv[iarg], -1 );
				if( offset > 0xffffffff || amount > 0xffffffff )
				{
					fprintf( stderr, "Error: memory value request out of range\n" );
					return -9;
				}

				// Round up amount.
				amount = ( amount + 3 ) & 0xfffffffc;
				FILE * f = 0;
				int hex = 0;
				if( strcmp( fname, "-" ) == 0 )
					f = stdout;
				else if( strcmp( fname, "+" ) == 0 )
					f = stdout, hex = 1;
				else
					f = fopen( fname, "wb" );
				if( !f )
				{
					fprintf( stderr, "Error: can't open write file \"%s\"\n", fname );
					return -9;
				}
				uint8_t * readbuff = malloc( amount );
				int readbuffplace = 0;

				if( MCF.ReadBinaryBlob )
				{
					if( MCF.ReadBinaryBlob( dev, offset, amount, readbuff ) < 0 )
					{
						fprintf( stderr, "Fault reading device\n" );
						return -12;
					}
				}				
				else
				{
					goto unimplemented;
				}

				if( hex )
				{
					int i;
					for( i = 0; i < amount; i++ )
					{
						if( ( i & 0xf ) == 0 )
						{
							if( i != 0 ) printf( "\n" );
							printf( "%08x: ", (uint32_t)(offset + i) );
						}
						printf( "%02x ", readbuff[i] );
					}
					printf( "\n" );
				}
				else
					fwrite( readbuff, amount, 1, f );

				free( readbuff );

				if( f != stdout ) fclose( f );
				break;
			}
			case 'w':
			{
				if( argchar[2] != 0 ) goto help;
				iarg++;
				argchar = 0; // Stop advancing
				if( iarg + 1 >= argc ) goto help;
				// Write binary.
				int i;
				int len = 0;
				char * image = 0;
				const char * fname = argv[iarg++];

				if( fname[0] == '-' )
				{
					len = strlen( fname + 1 );
					image = strdup( fname + 1 );
					status = 1;
				}
				else if( fname[0] == '+' )
				{
					int hl = strlen( fname+1 );
					if( hl & 1 )
					{
						fprintf( stderr, "Error: hex input doesn't align to chars correctly.\n" );
						return -32;
					}
					len = hl/2;
					image = malloc( len );
					int i;
					for( i = 0; i < len; i ++ )
					{
						char c1 = fname[i*2+1];
						char c2 = fname[i*2+2];
						int v1, v2;
						if( c1 >= '0' && c1 <= '9' ) v1 = c1 - '0';
						else if( c1 >= 'a' && c1 <= 'f' ) v1 = c1 - 'a' + 10;
						else if( c1 >= 'A' && c1 <= 'F' ) v1 = c1 - 'A' + 10;
						else
						{
							fprintf( stderr, "Error: Bad hex\n" );
							return -32;
						}

						if( c2 >= '0' && c2 <= '9' ) v2 = c2 - '0';
						else if( c2 >= 'a' && c2 <= 'f' ) v2 = c2 - 'a' + 10;
						else if( c2 >= 'A' && c2 <= 'F' ) v2 = c2 - 'A' + 10;
						else
						{
							fprintf( stderr, "Error: Bad hex\n" );
							return -32;
						}
						image[i] = (v1<<4) | v2;
					}
					status = 1;
				}
				else
				{
					FILE * f = fopen( fname, "rb" );
					fseek( f, 0, SEEK_END );
					len = ftell( f );
					fseek( f, 0, SEEK_SET );
					image = malloc( len );
					status = fread( image, len, 1, f );
					fclose( f );
				}


				uint64_t offset = StringToMemoryAddress( argv[iarg] );
				if( offset > 0xffffffff )
				{
					fprintf( stderr, "Error: Invalid offset (%s)\n", argv[iarg] );
					exit( -44 );
				}
				if( status != 1 )
				{
					fprintf( stderr, "Error: File I/O Fault.\n" );
					exit( -10 );
				}
				if( len > 16384 )
				{
					fprintf( stderr, "Error: Image for CH32V003 too large (%d)\n", len );
					exit( -9 );
				}

				if( MCF.WriteBinaryBlob )
				{
					if( MCF.WriteBinaryBlob( dev, offset, len, image ) )
					{
						fprintf( stderr, "Error: Fault writing image.\n" );
						return -13;
					}
				}
				else
				{
					goto unimplemented;
				}

				free( image );
				break;
			}
			
		}
		if( argchar && argchar[2] != 0 ) { argchar++; goto keep_going; }
	}

	if( MCF.FlushLLCommands )
		MCF.FlushLLCommands( dev );

	if( MCF.Exit )
		MCF.Exit( dev );

	return 0;

help:
	fprintf( stderr, "Usage: minichlink [args]\n" );
	fprintf( stderr, " single-letter args may be combined, i.e. -3r\n" );
	fprintf( stderr, " multi-part args cannot.\n" );
	fprintf( stderr, " -3 Enable 3.3V\n" );
	fprintf( stderr, " -5 Enable 5V\n" );
	fprintf( stderr, " -t Disable 3.3V\n" );
	fprintf( stderr, " -f Disable 5V\n" );
	fprintf( stderr, " -u Clear all code flash - by power off (also can unbrick)\n" );
	fprintf( stderr, " -b Reboot out of Halt\n" );
	fprintf( stderr, " -e Resume from halt\n" );
	fprintf( stderr, " -h Place into Halt\n" );
	fprintf( stderr, " -D Configure NRST as GPIO **WARNING** If you do this and you reconfig\n" );
	fprintf( stderr, "      the SWIO pin (PD1) on boot, your part can never again be programmed!\n" );
	fprintf( stderr, " -d Configure NRST as NRST\n" );
//	fprintf( stderr, " -P Enable Read Protection (UNTESTED)\n" );
//	fprintf( stderr, " -p Disable Read Protection (UNTESTED)\n" );
	fprintf( stderr, " -w [binary image to write] [address, decimal or 0x, try0x08000000]\n" );
	fprintf( stderr, " -r [memory address, decimal or 0x, try 0x08000000] [size, decimal or 0x, try 16384] [output binary image]\n" );
	fprintf( stderr, "   Note: for memory addresses, you can use 'flash' 'launcher' 'bootloader' 'option' 'ram' and say \"ram+0x10\" for instance\n" );
	fprintf( stderr, "   For filename, you can use - for raw or + for hex.\n" );
	fprintf( stderr, " -T is a terminal. This MUST be the last argument.  You MUST have resumed or \n" );

	return -1;	

unimplemented:
	fprintf( stderr, "Error: Command '%s' unimplemented on this programmer.\n", lastcommand );
	return -1;
}


#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
#define strtoll _strtoi64
#endif

static int StaticUnlockFlash( void * dev, struct InternalState * iss );
static int StaticWaitForFlash( void * dev );
static int StaticEnterResetMode( void * dev, struct InternalState * iss, int halt );

static int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber )
{
	if( !number || !number[0] ) return defaultNumber;
	int radix = 10;
	if( number[0] == '0' )
	{
		char nc = number[1];
		number+=2;
		if( nc == 0 ) return 0;
		else if( nc == 'x' ) radix = 16;
		else if( nc == 'b' ) radix = 2;
		else { number--; radix = 8; }
	}
	char * endptr;
	uint64_t ret = strtoll( number, &endptr, radix );
	if( endptr == number )
	{
		return defaultNumber;
	}
	else
	{
		return ret;
	}
}

static int64_t StringToMemoryAddress( const char * number )
{
	uint32_t base = 0;

	if( strncmp( number, "flash", 5 ) == 0 )       base = 0x08000000, number += 5;
	if( strncmp( number, "launcher", 8 ) == 0 )    base = 0x1FFFF000, number += 8;
	if( strncmp( number, "bootloader", 10 ) == 0 ) base = 0x1FFFF000, number += 10;
	if( strncmp( number, "option", 6 ) == 0 )      base = 0x1FFFF800, number += 6;
	if( strncmp( number, "user", 4 ) == 0 )        base = 0x1FFFF800, number += 4;
	if( strncmp( number, "ram", 3 ) == 0 )         base = 0x20000000, number += 3;

	if( base )
	{
		if( *number != '+' )
			return base;
		number++;
		return base + SimpleReadNumberInt( number, 0 );
	}
	return SimpleReadNumberInt( number, -1 );
}

static int StaticWaitForFlash( void * dev )
{
	uint32_t rw, timeout = 0;
	do
	{
		rw = 0;
		MCF.ReadWord( dev, (intptr_t)&FLASH->STATR, &rw ); // FLASH_STATR => 0x4002200C
		if( timeout++ > 100 ) return -1;
	} while(rw & 1);  // BSY flag.

	if( rw & FLASH_STATR_WRPRTERR )
	{
		fprintf( stderr, "Memory Protection Error\n" );
		return -44;
	}

	return 0;
}

static int StaticEnterResetMode( void * dev, struct InternalState * iss, int mode )
{
	switch ( mode )
	{
	case 0:
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Initiate a halt request.
		MCF.WriteReg32( dev, DMCONTROL, 0x00000001 ); // Clear Halt Request.
		MCF.FlushLLCommands( dev );
		break;
	case 1:
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Initiate a halt request.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000003 ); // Reboot.
		MCF.WriteReg32( dev, DMCONTROL, 0x40000001 ); // resumereq
		MCF.FlushLLCommands( dev );
		printf( "Reboot\n" );
		break;
	case 2:
		MCF.WriteReg32( dev, DMCONTROL, 0x40000001 ); // resumereq
		MCF.FlushLLCommands( dev );
		break;
	}
	iss->processor_in_mode = mode;
}

int DefaultSetupInterface( void * dev )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);

	if( MCF.Control3v3 ) MCF.Control3v3( dev, 1 );
	if( MCF.DelayUS ) MCF.DelayUS( dev, 16000 );
	MCF.WriteReg32( dev, DMSHDWCFGR, 0x5aa50000 | (1<<10) ); // Shadow Config Reg
	MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // CFGR (1<<10 == Allow output from slave)
	MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // Bug in silicon?  If coming out of cold boot, and we don't do our little "song and dance" this has to be called.

	// Read back chip status.  This is really baskc.
	uint32_t reg;
	int r = MCF.ReadReg32( dev, DMSTATUS, &reg );
	if( r >= 0 )
	{
		// Valid R.
		if( reg == 0x00000000 || reg == 0xffffffff )
		{
			fprintf( stderr, "Error: Setup chip failed. Got code %08x\n", reg );
			return -9;
		}
	}
	else
	{
		fprintf( stderr, "Error: Could not read chip code.\n" );
		return r;
	}

	if( MCF.ReadWord( dev, 0x1FFFF800, &reg ) ) goto fail;	
	printf( "USER/RDPR: %08x\n", reg );
/*	if( MCF.ReadWord( dev, 0x1FFFF804, &reg ) ) goto fail;	
	printf( "NDATA: %08x\n", reg );
	if( MCF.ReadWord( dev, 0x1FFFF808, &reg ) ) goto fail;	
	printf( "WRPR01: %08x\n", reg );
	if( MCF.ReadWord( dev, 0x1FFFF80c, &reg ) ) goto fail;	
	printf( "WRPR23: %08x\n", reg );*/
	if( MCF.ReadWord( dev, 0x1FFFF7E0, &reg ) ) goto fail;
	printf( "Flash Size: %d kB\n", (reg&0xffff) );
	if( MCF.ReadWord( dev, 0x1FFFF7E8, &reg ) ) goto fail;	
	printf( "R32_ESIG_UNIID1: %08x\n", reg );
	if( MCF.ReadWord( dev, 0x1FFFF7EC, &reg ) ) goto fail;	
	printf( "R32_ESIG_UNIID2: %08x\n", reg );
	if( MCF.ReadWord( dev, 0x1FFFF7F0, &reg ) ) goto fail;	
	printf( "R32_ESIG_UNIID3: %08x\n", reg );

	iss->statetag = STTAG( "HALT" );
	return 0;
fail:
	fprintf( stderr, "Error: Failed to get chip details\n" );
	return -11;
}

static int DefaultWriteWord( void * dev, uint32_t address_to_write, uint32_t data )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	uint32_t rrv;
	int r;
	int first = 0;

	if( iss->processor_in_mode )
	{
		StaticEnterResetMode( dev, iss, 0 );
	}

	int is_flash = 0;
	if( ( address_to_write & 0xff000000 ) == 0x08000000 || ( address_to_write & 0x1FFFF800 ) == 0x1FFFF000 )
	{
		// Is flash.
		is_flash = 1;
	}

	if( iss->statetag != STTAG( "WRSQ" ) || is_flash != iss->lastwriteflags )
	{
		MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.

		// Different address, so we don't need to re-write all the program regs.
		MCF.WriteReg32( dev, DMPROGBUF0, 0x00032283 ); // lw x5,0(x6)
		MCF.WriteReg32( dev, DMPROGBUF1, 0x0072a023 ); // sw x7,0(x5)
		MCF.WriteReg32( dev, DMPROGBUF2, 0x00428293 ); // addi x5, x5, 4
		MCF.WriteReg32( dev, DMPROGBUF3, 0x00532023 ); // sw x5,0(x6)
		if( is_flash )
		{
			// After writing to memory, also hit up page load flag.
			MCF.WriteReg32( dev, DMPROGBUF4, 0x00942023 ); // sw x9,0(x8)
			MCF.WriteReg32( dev, DMPROGBUF5, 0x00100073 ); // ebreak

			MCF.WriteReg32( dev, DMDATA0, (intptr_t)&FLASH->CTLR );
			MCF.WriteReg32( dev, DMCOMMAND, 0x00231008 ); // Copy data to x8
			MCF.WriteReg32( dev, DMDATA0, CR_PAGE_PG|CR_BUF_LOAD);
			MCF.WriteReg32( dev, DMCOMMAND, 0x00231009 ); // Copy data to x9
		}
		else
		{
			MCF.WriteReg32( dev, DMPROGBUF4, 0x00100073 ); // ebreak
		}


		MCF.WriteReg32( dev, DMDATA0, 0xe00000f8); // Address of DATA1.
		MCF.WriteReg32( dev, DMCOMMAND, 0x00231006 ); // Location of DATA1 to x6

		iss->lastwriteflags = is_flash;

		MCF.WriteReg32( dev, DMDATA1, address_to_write );

		iss->statetag = STTAG( "WRSQ" );
		iss->currentstateval = address_to_write;

		MCF.WriteReg32( dev, DMDATA0, data );
		MCF.WriteReg32( dev, DMCOMMAND, 0x00271007 ); // Copy data to x7, and execute program.
		MCF.WriteReg32( dev, DMABSTRACTAUTO, 1 ); // Enable Autoexec.

		do
		{
			r = MCF.ReadReg32( dev, DMABSTRACTCS, &rrv );
			if( r ) return r;
		}
		while( rrv & (1<<12) );
		if( (rrv >> 8 ) & 7 )
		{
			fprintf( stderr, "Fault writing memory (DMABSTRACTS = %08x)\n", rrv );
			MCF.WriteReg32( dev, DMABSTRACTCS, 0x00000700 );
		}
	}
	else
	{
		if( address_to_write != iss->currentstateval )
		{
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 0 ); // Disable Autoexec.
			MCF.WriteReg32( dev, DMDATA1, address_to_write );
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 1 ); // Enable Autoexec.
		}
		MCF.WriteReg32( dev, DMDATA0, data );
		if( is_flash )
		{
			MCF.DelayUS( dev, 100 );
		}
		else
		{
			do
			{
				r = MCF.ReadReg32( dev, DMABSTRACTCS, &rrv );
				if( r ) return r;
			}
			while( rrv & (1<<12) );
			if( (rrv >> 8 ) & 7 )
			{
				fprintf( stderr, "Fault writing memory (DMABSTRACTS = %08x)\n", rrv );
				rrv &= 0xfffff8ff;
				MCF.WriteReg32( dev, DMABSTRACTCS, rrv );
			}
		}
	}


	iss->currentstateval += 4;

	return 0;
}

int DefaultWriteBinaryBlob( void * dev, uint32_t address_to_write, uint32_t blob_size, uint8_t * blob )
{
	uint32_t rw;
	int timeout = 0;
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	int is_flash = 0;
	uint32_t lastgroup = -1;

	if( blob_size == 0 ) return 0;

	if( iss->processor_in_mode )
	{
		StaticEnterResetMode( dev, iss, 0 );
	}


	if( (address_to_write & 0xff000000) == 0x08000000 || (address_to_write & 0xff000000) == 0x00000000 ) 
	{
		// Need to unlock flash.
		// Flash reg base = 0x40022000,
		// FLASH_MODEKEYR => 0x40022024
		// FLASH_KEYR => 0x40022004

		if( !iss->flash_unlocked )
		{
			if( ( rw = StaticUnlockFlash( dev, iss ) ) )
				return rw;
		}

		is_flash = 1;

		printf( "Erasing TO %08x %08x\n", address_to_write, blob_size );
		MCF.Erase( dev, address_to_write, blob_size, 0 );
	}

	MCF.FlushLLCommands( dev );
	MCF.DelayUS( dev, 100 ); // Why do we need this?

	uint32_t wp = address_to_write;
	uint32_t ew = wp + blob_size;
	int group = -1;
	lastgroup = -1;

	while( wp < ew )
	{
		if( is_flash )
		{
			group = (wp & 0xffffffc0);
			MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_PAGE_PG ); // THIS IS REQUIRED.
			MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_BUF_RST | CR_PAGE_PG );

			int j;
			for( j = 0; j < 16; j++ )
			{
				int index = (wp-address_to_write);
				uint32_t data = 0xffffffff;
				if( index + 3 < blob_size )
					data = ((uint32_t*)blob)[index/4];
				else if( (int32_t)(blob_size - index) > 0 )
				{
					printf( "%d %d\n", blob_size, index );
					memcpy( &data, &blob[index], blob_size - index );
				}
				MCF.WriteWord( dev, wp, data );
				wp += 4;
			}
			MCF.WriteWord( dev, (intptr_t)&FLASH->ADDR, group );
			MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_PAGE_PG|CR_STRT_Set );
			StaticWaitForFlash( dev );

			lastgroup = group;
		}
		else
		{
			int index = (wp-address_to_write);
			uint32_t data = 0xffffffff;
			if( index + 3 < blob_size )
				data = ((uint32_t*)blob)[index/4];
			else if( (int32_t)(blob_size - index) > 0 )
				memcpy( &data, &blob[index], blob_size - index );
			printf( "WRITING %08x => %08x\n", data, wp );
			MCF.WriteWord( dev, wp, data );
			wp += 4;
		}
	}

	if( is_flash )
	{
		if( StaticWaitForFlash( dev ) ) goto timedout;
	}
	return 0;
timedout:
	fprintf( stderr, "Timed out\n" );
	return -5;
}

static int DefaultReadWord( void * dev, uint32_t address_to_read, uint32_t * data )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	uint32_t rrv = 0;
	int r;
	int first = 0;

	if( iss->processor_in_mode )
	{
		StaticEnterResetMode( dev, iss, 0 );
	}

	if( iss->statetag != STTAG( "RDSQ" ) || address_to_read != iss->currentstateval )
	{
		MCF.WriteReg32( dev, DMABSTRACTAUTO, 0 ); // Disable Autoexec.
		MCF.WriteReg32( dev, DMPROGBUF0, 0x0002a303 ); // lw x6,0(x5)
		MCF.WriteReg32( dev, DMPROGBUF1, 0x00428293 ); // addi x5, x5, 4
		MCF.WriteReg32( dev, DMPROGBUF2, 0x0065a023 ); // sw x6,0(x11) // Write back to DATA0
		MCF.WriteReg32( dev, DMPROGBUF3, 0x00100073 ); // ebreak

		MCF.WriteReg32( dev, DMDATA0, address_to_read );
		MCF.WriteReg32( dev, DMCOMMAND, 0x00231005 ); // Copy data to x5

		MCF.WriteReg32( dev, DMDATA0, 0xe00000f4 );   // DATA0's location in memory.
		MCF.WriteReg32( dev, DMCOMMAND, 0x0023100b ); // Copy data to x11

		MCF.WriteReg32( dev, DMCOMMAND, 0x00241000 ); // Only execute.

		MCF.WriteReg32( dev, DMABSTRACTAUTO, 1 ); // Enable Autoexec.
		do
		{
			r = MCF.ReadReg32( dev, DMABSTRACTCS, &rrv );
			if( r ) return r;
		}
		while( rrv & (1<<12) );
		first = 1;
		iss->statetag = STTAG( "RDSQ" );
		iss->currentstateval = address_to_read;

		do
		{
			r = MCF.ReadReg32( dev, DMABSTRACTCS, &rrv );
			if( r ) return r;
		}
		while( rrv & (1<<12) );

		if( (rrv >> 8 ) & 7 )
		{
			fprintf( stderr, "Fault reading memory (DMABSTRACTS = %08x)\n", rrv );
		}
	}

	iss->currentstateval += 4;

	return MCF.ReadReg32( dev, DMDATA0, data );
}

static int StaticUnlockFlash( void * dev, struct InternalState * iss )
{
	uint32_t rw;
	MCF.ReadWord( dev, (intptr_t)&FLASH->CTLR, &rw ); 
	if( rw & 0x8080 ) 
	{

		MCF.WriteWord( dev, (intptr_t)&FLASH->KEYR, 0x45670123 );
		MCF.WriteWord( dev, (intptr_t)&FLASH->KEYR, 0xCDEF89AB );
		MCF.WriteWord( dev, (intptr_t)&FLASH->OBKEYR, 0x45670123 );
		MCF.WriteWord( dev, (intptr_t)&FLASH->OBKEYR, 0xCDEF89AB );
		MCF.WriteWord( dev, (intptr_t)&FLASH->MODEKEYR, 0x45670123 );
		MCF.WriteWord( dev, (intptr_t)&FLASH->MODEKEYR, 0xCDEF89AB );

		MCF.ReadWord( dev, (intptr_t)&FLASH->CTLR, &rw );
		if( rw & 0x8080 ) 
		{
			fprintf( stderr, "Error: Flash is not unlocked (CTLR = %08x)\n", rw );
			return -9;
		}
	}
	iss->flash_unlocked = 1;
	return 0;
}

int DefaultErase( void * dev, uint32_t address, uint32_t length, int type )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	uint32_t rw;
	uint32_t timeout = 0;

	if( iss->processor_in_mode )
	{
		StaticEnterResetMode( dev, iss, 0 );
	}

	if( !iss->flash_unlocked )
	{
		if( ( rw = StaticUnlockFlash( dev, iss ) ) )
			return rw;
	}

	if( type == 1 )
	{
		// Whole-chip flash
		iss->statetag = STTAG( "XXXX" );
		printf( "Whole-chip erase\n" );
		MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, 0 );
		MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, FLASH_CTLR_MER  );
		MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_STRT_Set|FLASH_CTLR_MER );
		if( StaticWaitForFlash( dev ) ) return -11;		
		MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, 0 );
	}
	else
	{
		// 16.4.7, Step 3: Check the BSY bit of the FLASH_STATR register to confirm that there are no other programming operations in progress.
		// skip (we make sure at the end)

		int chunk_to_erase = address;

		while( chunk_to_erase < address + length )
		{
			// Step 4:  set PAGE_ER of FLASH_CTLR(0x40022010)
			MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_PAGE_ER ); // Actually FTER

			// Step 5: Write the first address of the fast erase page to the FLASH_ADDR register.
			MCF.WriteWord( dev, (intptr_t)&FLASH->ADDR, chunk_to_erase  );

			// Step 6: Set the STAT bit of FLASH_CTLR register to '1' to initiate a fast page erase (64 bytes) action.
			MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_STRT_Set|CR_PAGE_ER );
			if( StaticWaitForFlash( dev ) ) return -99;
			chunk_to_erase+=64;
		}
	}
	return 0;
timedout:
	return -6;
}

int DefaultReadBinaryBlob( void * dev, uint32_t address_to_read_from, uint32_t read_size, uint8_t * blob )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);

	if( iss->processor_in_mode )
	{
		StaticEnterResetMode( dev, iss, 0 );
	}

	uint32_t rpos = address_to_read_from;
	uint32_t rend = address_to_read_from + read_size;
	while( rpos < rend )
	{
		uint32_t rw;
		int r = DefaultReadWord( dev, rpos, &rw );
		if( r ) return r;
		int remain = rend - rpos;
		if( remain > 3 ) remain = 4;
		memcpy( blob, &rw, remain );
		blob += 4;
		rpos += 4;
	}
}


static int DefaultHaltMode( void * dev, int mode )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	StaticEnterResetMode( dev, iss, mode );
	return 0;
}

// Returns positive if received text.
// Returns negative if error.
// Returns 0 if no text waiting.
// maxlen MUST be at least 8 characters.  We null terminate.
int DefaultPollTerminal( void * dev, uint8_t * buffer, int maxlen )
{
	int r;
	uint32_t rr;
	r = MCF.ReadReg32( dev, DMDATA0, &rr );
	if( r < 0 ) return r;

	if( maxlen < 8 ) return -9;

	// DMDATA1:
	//  bits 0..5 = # printf chars.
	//  bit  6 = host-wants-to-say-something.
	//  bit  7 = host-acknowledge.
	if( rr & 0x80 )
	{
		int ret = 0;
		int num_printf_chars = (rr & 0x3f)-4;  // Actaully can't be more than 32.
		if( num_printf_chars > 0 && num_printf_chars <= 7)
		{
			if( num_printf_chars > 3 )
			{
				uint32_t r2;
				r = MCF.ReadReg32( dev, DMDATA1, &r2 );
				memcpy( buffer+4, &r2, num_printf_chars - 3 );
			}
			int firstrem = num_printf_chars;
			if( firstrem > 3 ) firstrem = 3;
			memcpy( buffer, ((uint8_t*)&rr)+1, firstrem );
			buffer[num_printf_chars] = 0;
			ret = num_printf_chars;
		}
		MCF.WriteReg32( dev, DMDATA0, 0x00 ); // Write that we acknowledge the data.
		return ret;
	}
	else
	{
		return 0;
	}
}

int SetupAutomaticHighLevelFunctions( void * dev )
{
	// Will populate high-level functions from low-level functions.
	if( MCF.WriteReg32 == 0 || MCF.ReadReg32 == 0 ) return -5;

	// Else, TODO: Build the high level functions from low level functions.
	// If a high-level function alrady exists, don't override.
	
	if( !MCF.SetupInterface )
		MCF.SetupInterface = DefaultSetupInterface;
	if( !MCF.WriteBinaryBlob )
		MCF.WriteBinaryBlob = DefaultWriteBinaryBlob;
	if( !MCF.ReadBinaryBlob )
		MCF.ReadBinaryBlob = DefaultReadBinaryBlob;
	if( !MCF.WriteWord )
		MCF.WriteWord = DefaultWriteWord;
	if( !MCF.ReadWord )
		MCF.ReadWord = DefaultReadWord;
	if( !MCF.Erase )
		MCF.Erase = DefaultErase;
	if( !MCF.HaltMode )
		MCF.HaltMode = DefaultHaltMode;
	if( !MCF.PollTerminal )
		MCF.PollTerminal = DefaultPollTerminal;


	struct InternalState * iss = malloc( sizeof( struct InternalState ) );
	iss->statetag = 0;
	iss->currentstateval = 0;

	((struct ProgrammerStructBase*)dev)->internal = iss;
}






void TestFunction(void * dev )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);

	uint32_t rv;
	int r;
	MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
	MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Initiate a halt request.
	MCF.WriteReg32( dev, DMCONTROL, 0x00000001 ); // Clear Halt Request.

	r = MCF.WriteWord( dev, 0x20000100, 0xdeadbeef );
	r = MCF.WriteWord( dev, 0x20000104, 0xcafed0de );
	r = MCF.WriteWord( dev, 0x20000108, 0x12345678 );
	r = MCF.WriteWord( dev, 0x20000108, 0x00b00d00 );
	r = MCF.WriteWord( dev, 0x20000104, 0x33334444 );

	r = MCF.ReadWord( dev, 0x20000100, &rv );
	printf( "**>>> %d %08x\n", r, rv );
	r = MCF.ReadWord( dev, 0x20000104, &rv );
	printf( "**>>> %d %08x\n", r, rv );
	r = MCF.ReadWord( dev, 0x20000108, &rv );
	printf( "**>>> %d %08x\n", r, rv );


	r = MCF.ReadWord( dev, 0x00000300, &rv );
	printf( "F %d %08x\n", r, rv );
	r = MCF.ReadWord( dev, 0x00000304, &rv );
	printf( "F %d %08x\n", r, rv );
	r = MCF.ReadWord( dev, 0x00000308, &rv );
	printf( "F %d %08x\n", r, rv );

	uint8_t buffer[256];
	int i;
	for( i = 0; i < 256; i++ ) buffer[i] = 0;
	MCF.WriteBinaryBlob( dev, 0x08000300, 256, buffer );
	MCF.ReadBinaryBlob( dev, 0x08000300, 256, buffer );
	for( i = 0; i < 256; i++ )
	{
		printf( "%02x ", buffer[i] );
		if( (i & 0xf) == 0xf ) printf( "\n" );
	}

	for( i = 0; i < 256; i++ ) buffer[i] = i;
	MCF.WriteBinaryBlob( dev, 0x08000300, 256, buffer );
	MCF.ReadBinaryBlob( dev, 0x08000300, 256, buffer );
	for( i = 0; i < 256; i++ )
	{
		printf( "%02x ", buffer[i] );
		if( (i & 0xf) == 0xf ) printf( "\n" );
	}
}

