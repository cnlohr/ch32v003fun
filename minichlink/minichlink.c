#include <stdio.h>
#include <stdlib.h>
#include "wch_link_base.h"


const uint8_t * bootloader = (const uint8_t*)
"\x21\x11\x22\xca\x26\xc8\x93\x77\x15\x00\x99\xcf\xb7\x06\x67\x45" \
"\xb7\x27\x02\x40\x93\x86\x36\x12\x37\x97\xef\xcd\xd4\xc3\x13\x07" \
"\xb7\x9a\xd8\xc3\xd4\xd3\xd8\xd3\x93\x77\x25\x00\x9d\xc7\xb7\x27" \
"\x02\x40\x98\x4b\xad\x66\x37\x33\x00\x40\x13\x67\x47\x00\x98\xcb" \
"\x98\x4b\x93\x86\xa6\xaa\x13\x67\x07\x04\x98\xcb\xd8\x47\x05\x8b" \
"\x63\x16\x07\x10\x98\x4b\x6d\x9b\x98\xcb\x93\x77\x45\x00\xa9\xcb" \
"\x93\x07\xf6\x03\x99\x83\x2e\xc0\x2d\x63\x81\x76\x3e\xc4\xb7\x32" \
"\x00\x40\xb7\x27\x02\x40\x13\x03\xa3\xaa\xfd\x16\x98\x4b\xb7\x03" \
"\x02\x00\x33\x67\x77\x00\x98\xcb\x02\x47\xd8\xcb\x98\x4b\x13\x67" \
"\x07\x04\x98\xcb\xd8\x47\x05\x8b\x69\xe7\x98\x4b\x75\x8f\x98\xcb" \
"\x02\x47\x13\x07\x07\x04\x3a\xc0\x22\x47\x7d\x17\x3a\xc4\x79\xf7" \
"\x93\x77\x85\x00\xf1\xcf\x93\x07\xf6\x03\x2e\xc0\x99\x83\x37\x27" \
"\x02\x40\x3e\xc4\x1c\x4b\xc1\x66\x2d\x63\xd5\x8f\x1c\xcb\x37\x07" \
"\x00\x20\x13\x07\x07\x20\xb7\x27\x02\x40\xb7\x03\x08\x00\xb7\x32" \
"\x00\x40\x13\x03\xa3\xaa\x94\x4b\xb3\xe6\x76\x00\x94\xcb\xd4\x47" \
"\x85\x8a\xf5\xfe\x82\x46\xba\x84\x37\x04\x04\x00\x36\xc2\xc1\x46" \
"\x36\xc6\x92\x46\x84\x40\x11\x07\x84\xc2\x94\x4b\xc1\x8e\x94\xcb" \
"\xd4\x47\x85\x8a\xb1\xea\x92\x46\xba\x84\x91\x06\x36\xc2\xb2\x46" \
"\xfd\x16\x36\xc6\xf9\xfe\x82\x46\xd4\xcb\x94\x4b\x93\xe6\x06\x04" \
"\x94\xcb\xd4\x47\x85\x8a\x85\xee\xd4\x47\xc1\x8a\x85\xce\xd8\x47" \
"\xb7\x06\xf3\xff\xfd\x16\x13\x67\x07\x01\xd8\xc7\x98\x4b\x21\x45" \
"\x75\x8f\x98\xcb\x52\x44\xc2\x44\x61\x01\x02\x90\x23\x20\xd3\x00" \
"\xf5\xb5\x23\xa0\x62\x00\x3d\xb7\x23\xa0\x62\x00\x55\xb7\x23\xa0" \
"\x62\x00\xc1\xb7\x82\x46\x93\x86\x06\x04\x36\xc0\xa2\x46\xfd\x16" \
"\x36\xc4\xb5\xf2\x98\x4b\xb7\x06\xf3\xff\xfd\x16\x75\x8f\x98\xcb" \
"\x41\x89\x05\xcd\x2e\xc0\x0d\x06\x02\xc4\x09\x82\xb7\x07\x00\x20" \
"\x32\xc6\x93\x87\x07\x20\x98\x43\x13\x86\x47\x00\xa2\x47\x82\x46" \
"\x8a\x07\xb6\x97\x9c\x43\x63\x1c\xf7\x00\xa2\x47\x85\x07\x3e\xc4" \
"\xa2\x46\x32\x47\xb2\x87\xe3\xe0\xe6\xfe\x01\x45\x61\xb7\x41\x45" \
"\x51\xb7\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" \
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" \
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";

static int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber );

int bootloader_len = 512;

int main( int argc, char ** argv )
{
	int status;
	int must_be_end = 0;
	uint8_t rbuff[1024];

	libusb_device_handle * devh = wch_link_base_setup();

	int iarg;
	for( iarg = 1; iarg < argc; iarg++ )
	{
		char * argchar = argv[iarg];
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
			case '3': wch_link_command( devh, "\x81\x0d\x01\x09", 4, 0, 0, 0 ); break;
			case '5': wch_link_command( devh, "\x81\x0d\x01\x0b", 4, 0, 0, 0 ); break;
			case 't': wch_link_command( devh, "\x81\x0d\x01\x0a", 4, 0, 0, 0 ); break;
			case 'f': wch_link_command( devh, "\x81\x0d\x01\x0c", 4, 0, 0, 0 ); break;
			case 'r': 
				// This is clearly not the "best" method to exit reset.  I don't know why this combination works.
				wch_link_multicommands( devh, 3, 4, "\x81\x0b\x01\x01", 4, "\x81\x0d\x01\x02", 4, "\x81\x0d\x01\xff" );
				must_be_end = 'r';
				break;
			case 'R':
				// Part one "immediately" places the part into reset.  Part 2 says when we're done, leave part in reset.
				wch_link_multicommands( devh, 2, 4, "\x81\x0d\x01\x02", 4, "\x81\x0d\x01\x01" );
				must_be_end = 'R';
				break;

			// disable NRST pin (turn it into a GPIO)
			case 'd':  // see "RSTMODE" in datasheet
				wch_link_multicommands( devh, 2, 11, "\x81\x06\x08\x02\xf7\xff\xff\xff\xff\xff\xff", 4, "\x81\x0b\x01\x01" );
				break;
			case 'D': // see "RSTMODE" in datasheet
				wch_link_multicommands( devh, 2, 11, "\x81\x06\x08\x02\xff\xff\xff\xff\xff\xff\xff", 4, "\x81\x0b\x01\x01" );
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
				wch_link_command( devh, "\x81\x06\x01\x01", 4, 0, 0, 0 );

				// Flush out any pending data.
				libusb_bulk_transfer( devh, 0x82, rbuff, 1024, &transferred, 1 );

				// 3/8 = Read Memory
				// First 4 bytes are big-endian location.
				// Next 4 bytes are big-endian amount.
				uint8_t readop[11] = { 0x81, 0x03, 0x08, };
				
				readop[3] = (offset>>24)&0xff;
				readop[4] = (offset>>16)&0xff;
				readop[5] = (offset>>8)&0xff;
				readop[6] = (offset>>0)&0xff;

				readop[7] = (amount>>24)&0xff;
				readop[8] = (amount>>16)&0xff;
				readop[9] = (amount>>8)&0xff;
				readop[10] = (amount>>0)&0xff;
				
				wch_link_command( devh, readop, 11, 0, 0, 0 );

				// Perform operation
				wch_link_command( devh, "\x81\x02\x01\x0c", 4, 0, 0, 0 );

				uint32_t remain = amount;
				while( remain )
				{
					transferred = 0;
					WCHCHECK( libusb_bulk_transfer( devh, 0x82, rbuff, 1024, &transferred, WCHTIMEOUT ) );
					memcpy( ((uint8_t*)readbuff) + readbuffplace, rbuff, transferred );
					readbuffplace += transferred;
					remain -= transferred;
				}

				// Flip internal endian.  Must be done separately in case something was unaligned when
				// reading.
				for( i = 0; i < readbuffplace/4; i++ )
				{
					uint32_t r = readbuff[i];
					readbuff[i] = (r>>24) | ((r & 0xff0000) >> 8) | ((r & 0xff00)<<8) | (( r & 0xff )<<24); 
				}
				
				fwrite( readbuff, readbuffplace, 1, f );

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

				int transferred;
				wch_link_command( devh, "\x81\x06\x01\x01", 4, 0, 0, 0 );
				wch_link_command( devh, "\x81\x06\x01\x01", 4, 0, 0, 0 ); // Not sure why but it seems to work better when we request twice.

				// This contains the write data quantity, in bytes.  (The last 2 octets)
				// Then it just rollllls on in.
				char rksbuff[11] = { 0x81, 0x01, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
				rksbuff[9] = len >> 8;
				rksbuff[10] = len & 0xff;
				wch_link_command( devh, rksbuff, 11, 0, 0, 0 );
				
				wch_link_command( devh, "\x81\x02\x01\x05", 4, 0, 0, 0 );
				
				int pplace = 0;
				for( pplace = 0; pplace < bootloader_len; pplace += 64 )
				{
					WCHCHECK( libusb_bulk_transfer( devh, 0x02, (uint8_t*)(bootloader+pplace), 64, &transferred, WCHTIMEOUT ) );
				}
				
				for( i = 0; i < 10; i++ )
				{
					wch_link_command( devh, "\x81\x02\x01\x07", 4, &transferred, rbuff, 1024 );
					if( transferred == 4 && rbuff[0] == 0x82 && rbuff[1] == 0x02 && rbuff[2] == 0x01 && rbuff[3] == 0x07 )
					{
						break;
					}
				} 
				if( i == 10 )
				{
					fprintf( stderr, "Error, confusing respones to 02/01/07\n" );
					exit( -109 );
				}
				
				wch_link_command( devh, "\x81\x02\x01\x02", 4, 0, 0, 0 );

				for( pplace = 0; pplace < padlen; pplace += 64 )
				{
					WCHCHECK( libusb_bulk_transfer( devh, 0x02,image+pplace, 64, &transferred, WCHTIMEOUT ) );
				}

				// Waiting or something on 2.46.2???????  I don't know why the main system does this.
				//	WCHCHECK( libusb_bulk_transfer( devh, 0x82, rbuff, 1024, &transferred, 2000 ) ); // Ignore respone.
				free( image );
				break;
			}
			
		}
		if( argchar && argchar[2] != 0 ) { argchar++; goto keep_going; }
	}

	wch_link_command( devh, "\x81\x0d\x01\xff", 4, 0, 0, 0);

	return 0;

help:
	fprintf( stderr, "Usage: minichlink [args]\n" );
	fprintf( stderr, " single-letter args may be combined, i.e. -3r\n" );
	fprintf( stderr, " multi-part args cannot.\n" );
	fprintf( stderr, " -3 Enable 3.3V\n" );
	fprintf( stderr, " -5 Enable 5V\n" );
	fprintf( stderr, " -t Disable 3.3V\n" );
	fprintf( stderr, " -f Disable 5V\n" );
	fprintf( stderr, " -r Release from reest\n" );
	fprintf( stderr, " -R Place into Reset\n" );
	fprintf( stderr, " -D Configure NRST as GPIO **WARNING** If you do this and you reconfig\n" );
	fprintf( stderr, "      the SWIO pin (PD1) on boot, your part can never again be programmed!\n" );
	fprintf( stderr, " -d Configure NRST as NRST\n" );
//	fprintf( stderr, " -P Enable Read Protection (UNTESTED)\n" );
//	fprintf( stderr, " -p Disable Read Protection (UNTESTED)\n" );
	fprintf( stderr, " -w [binary image to write]\n" );
	fprintf( stderr, " -o [memory address, decimal or 0x, try 0x08000000] [size, decimal or 0x, try 16384] [output binary image]\n" );
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
