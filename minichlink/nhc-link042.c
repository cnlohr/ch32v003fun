#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "minichlink.h"
#include "libusb.h"

void * TryInit_NHCLink042(void);

static int NHCLinkWriteReg32(void * dev, uint8_t reg_7_bit, uint32_t command);
static int NHCLinkReadReg32(void * dev, uint8_t reg_7_bit, uint32_t * commandresp);
static int NHCLinkFlushLLCommands(void * dev);
static int NHCLinkDelayUS(void * dev, int microseconds);
static int NHCLinkExit(void * dev);

static libusb_device_handle *hdev = 0;

int NHCLinkWriteReg32(void * dev, uint8_t reg_7_bit, uint32_t command)
{
    uint8_t buff[64];
    int32_t len;
    int status;

    buff[0] = 0xa3;
    buff[1] = reg_7_bit;
    buff[2] = (command >> 0);
    buff[3] = (command >> 8);
    buff[4] = (command >> 16);
    buff[5] = (command >> 24);

    status = libusb_bulk_transfer(dev, 0x01, buff, 64, &len, 5000);
    if ((status) || (len != 64))
    {
        return status;
    }

    return 0;
}

int NHCLinkReadReg32(void * dev, uint8_t reg_7_bit, uint32_t * commandresp)
{
    uint8_t buff[64];
    int32_t len;
    uint32_t tmp;
    int status;

    buff[0] = 0xa2;
    buff[1] = reg_7_bit;

    status = libusb_bulk_transfer(dev, 0x01, buff, 64, &len, 5000);
    if ((status) || (len != 64))
    {
        return status;
    }

    status = libusb_bulk_transfer(dev, 0x81, buff, 64, &len, 5000);
    if ((status) || (len != 64))
    {
        return status;
    }

    if (!buff[0])
    {
        return 1;
    }

    tmp = buff[4];
    tmp <<= 8;
    tmp += buff[3];
    tmp <<= 8;
    tmp += buff[2];
    tmp <<= 8;
    tmp += buff[1];

    *commandresp = tmp;

    return 0;
}

int NHCLinkFlushLLCommands(void * dev)
{

    return 0;
}

int NHCLinkDelayUS(void * dev, int microseconds)
{
    uint8_t buff[64];
    int32_t len;
    uint32_t tmp;
    int status;

    tmp = microseconds;
    buff[0] = 0xa6;
    buff[1] = (tmp >> 0);
    buff[2] = (tmp >> 8);
    buff[3] = (tmp >> 16);
    buff[4] = (tmp >> 24);

    status = libusb_bulk_transfer(dev, 0x01, buff, 64, &len, 5000);
    if ((status) || (len != 64))
    {
        return status;
    }

    return 0;
}

int NHCLinkExit(void * dev)
{
    uint8_t buff[64];
    int32_t len;
    int status;

    buff[0] = 0xa1;

    status = libusb_bulk_transfer(dev, 0x01, buff, 64, &len, 5000);
    if ((status) || (len != 64))
    {
        return status;
    }

    return 0;
}

void * TryInit_NHCLink042(void)
{
    libusb_context * ctx = 0;
	int status;
    uint8_t buff[64];
    int32_t len;

	status = libusb_init(&ctx);
	if (status < 0) {
		fprintf( stderr, "Error: libusb_init_context() returned %d\n", status );
		exit( status );
	}
	
	hdev = libusb_open_device_with_vid_pid(ctx, 0x1986, 0x0034);

	if( !hdev )
	{
		return 0;
	}
		
	libusb_claim_interface(hdev, 0);

    buff[0] = 0xa0;

    status = libusb_bulk_transfer(hdev, 0x01, buff, 64, &len, 5000);
    if ((status) || (len != 64))
    {
        return 0;
    }

    MCF.WriteReg32 = NHCLinkWriteReg32;
	MCF.ReadReg32 = NHCLinkReadReg32;
    MCF.DelayUS = NHCLinkDelayUS;
    MCF.FlushLLCommands = NHCLinkFlushLLCommands;
	MCF.Exit = NHCLinkExit;

	return hdev;
}
