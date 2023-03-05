// The "bootloader" blob is (C) WCH.
// The rest of the code, Copyright 2023 Charles Lohr
// Freely licensable under the MIT/x11, NewBSD Licenses, or
// public domain where applicable. 

#include <stdio.h>
#include <stdlib.h>
#include "minichlink.h"

static int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber );

extern struct MiniChlinkFunctions MCF;

int main( int argc, char ** argv )
{
	void * dev = 0;
	if( !(dev = TryInit_WCHLinkE() ) )
	else if( !(dev = TryInit_ESP32S2CHFUN() ) )
	else
	{
		fprintf( stderr, "Error: Could not initialize any supported programmers\n" );
		return -32;
	}

	int status;
	int must_be_end = 0;
	uint8_t rbuff[1024];

	if( MCF.SetupInterface )
	{
		if( MCF.SetupInterface() < 0 )
		{
			fprintf( stderr, "Could not setup interface.\n" );
			return -33;
		}
	}

	int iarg;
	char * argchar = argv[iarg];
	for( iarg = 1; iarg < argc; iarg++ )
	{
		if( argchar[0] != '-' ) goto help;
		if( must_be_end )
		{
			fprintf( stderr, "Error: the command '%c' cannot be followed by other commands.\n", must_be_end );
			return -1;
		}
		
keep_going:
		switch( argchar[1] )
		{
			default: goto help;
			case '3':
				if( MCF.Control3v3 )
					MCF.Control3v3( 1 );
				else
					goto unimplemented;
				break;
			case '5':
				if( MCF.Control5 )
					MCF.Control5( 1 );
				else
					goto unimplemented;
				break;
			case 't':
				if( MCF.Control3v3 )
					MCF.Control3v3( 0 );
				else
					goto unimplemented;
				break;
			case 'f':
				if( MCF.Control5 )
					MCF.Control5( 0 );
				else
					goto unimplemented;
				break;
			case 'u':
				if( MCF.Unbrick )
					MCF.Unbrick();
				else
					goto unimplemented;
				break;
			case 'r': 
				if( MCF.HaltMode )
					MCF.HaltMode( 0 );
				else
					goto unimplemented;
				must_be_end = 'r';
				break;
			case 'R':
				if( MCF.HaltMode )
					MCF.HaltMode( 1 );
				else
					goto unimplemented;
				must_be_end = 'R';
				break;

			// disable NRST pin (turn it into a GPIO)
			case 'd':  // see "RSTMODE" in datasheet
				if( MCF.ConfigureNRSTAsGPIO )
					MCF.ConfigureNRSTAsGPIO( 0 );
				else
					goto unimplemented;
				break;
			case 'D': // see "RSTMODE" in datasheet
				if( MCF.ConfigureNRSTAsGPIO )
					MCF.ConfigureNRSTAsGPIO( 1 );
				else
					goto unimplemented;
				break;
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
			case 'o':
			{
				int i;
				int transferred;
				if( argchar[2] != 0 ) goto help;
				iarg++;
				argchar = 0; // Stop advancing
				if( iarg + 2 >= argc ) goto help;
				uint64_t offset = SimpleReadNumberInt( argv[iarg++], -1 );
				uint64_t amount = SimpleReadNumberInt( argv[iarg++], -1 );
				if( offset > 0xffffffff || amount > 0xffffffff )
				{
					fprintf( stderr, "Error: memory value request out of range.\n" );
					return -9;
				}

				// Round up amount.
				amount = ( amount + 3 ) & 0xfffffffc;
				FILE * f = fopen( argv[iarg], "wb" );
				if( !f )
				{
					fprintf( stderr, "Error: can't open write file \"%s\"\n", argv[iarg] );
					return -9;
				}
				uint32_t * readbuff = malloc( amount );
				int readbuffplace = 0;

				if( MCF.ReadBinaryBlob )
				{
					if( MCF.ReadBinaryBlob( offset, amount, readbuff ) < 0 )
					{
						fprintf( stderr, "Fault reading device\n" );
						return -12;
					}
				}				
				else
				{
					goto unimplemented;
				}

				fwrite( readbuff, amount, 1, f );

				free( readbuff );

				fclose( f );
				break;
			}
			case 'w':
			{
				if( argchar[2] != 0 ) goto help;
				iarg++;
				argchar = 0; // Stop advancing
				if( iarg >= argc ) goto help;

				// Write binary.
				int i;
				FILE * f = fopen( argv[iarg], "rb" );
				fseek( f, 0, SEEK_END );
				int len = ftell( f );
				fseek( f, 0, SEEK_SET );
				int padlen = ((len-1) & (~0x3f)) + 0x40;
				char * image = malloc( padlen );
				memset( image, 0xff, padlen );
				status = fread( image, len, 1, f );
				fclose( f );
	
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
					if( MCF.WriteBinaryBlob( 0x08000000, len, image ) )
					{
						fprintf( stderr, "Error: Fault writing image.\n" );
						return -13;
					}
				}
				else
				{
					goto unimplemented;
				}

				// Waiting or something on 2.46.2???????  I don't know why the main system does this.
				//	WCHCHECK( libusb_bulk_transfer( devh, 0x82, rbuff, 1024, &transferred, 2000 ) ); // Ignore respone.
				free( image );
				break;
			}
			
		}
		if( argchar && argchar[2] != 0 ) { argchar++; goto keep_going; }
	}

	if( MCF.ExitMode )
		MCF.ExitMode( dev );

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
	fprintf( stderr, " -r Release from Reset\n" );
	fprintf( stderr, " -R Place into Reset\n" );
	fprintf( stderr, " -D Configure NRST as GPIO **WARNING** If you do this and you reconfig\n" );
	fprintf( stderr, "      the SWIO pin (PD1) on boot, your part can never again be programmed!\n" );
	fprintf( stderr, " -d Configure NRST as NRST\n" );
//	fprintf( stderr, " -P Enable Read Protection (UNTESTED)\n" );
//	fprintf( stderr, " -p Disable Read Protection (UNTESTED)\n" );
	fprintf( stderr, " -w [binary image to write]\n" );
	fprintf( stderr, " -o [memory address, decimal or 0x, try 0x08000000] [size, decimal or 0x, try 16384] [output binary image]\n" );
	return -1;	

unimplemented:
	fprintf( stderr, "Error: Command '%c' unimplemented on this programmer.\n", argchar[1] );
	return -1;
}


#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
#define strtoll _strtoi64
#endif

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

int SetupAutomaticHighLevelFunctions()
{
	// Will populate high-level functions from low-level functions.
	if( MCF.WriteReg32 == 0 || MCF.ReadReg32 == 0 ) return -5;

	// Else, TODO: Build the high level functions from low level functions.
	// If a high-level function alrady exists, don't override.
}

