#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "KT_BinIO.h"
#include "libusb-1.0/libusb.h"

uint32_t u8tou32(uint8_t *p)
{
    uint32_t u32Tmp;

    u32Tmp = p[3];
    u32Tmp <<= 8;
    u32Tmp += p[2];
    u32Tmp <<= 8;
    u32Tmp += p[1];
    u32Tmp <<= 8;
    u32Tmp += p[0];

    return u32Tmp;
}

void u32tou8(uint32_t u32Data, uint8_t *p)
{

    p[0] = u32Data;
    u32Data >>= 8;
    p[1] = u32Data;
    u32Data >>= 8;
    p[2] = u32Data;
    u32Data >>= 8;
    p[3] = u32Data;
}

uint32_t nhc_usb_open(void);
void nhc_usb_close(void);
uint32_t nhc_usb_write(uint8_t u8Ep, uint8_t *pu8Data, uint32_t u32Len);
uint32_t nhc_usb_read(uint8_t u8Ep, uint8_t *pu8Data, uint32_t u32Len);

libusb_device_handle *h;

uint32_t nhc_usb_open(void)
{

    libusb_init(NULL);
    h = libusb_open_device_with_vid_pid(NULL, 0x1986, 0x0034);
    if (h == NULL)
    {
        return 0;
    }

    libusb_claim_interface(h, 0);

    return 1;
}

void nhc_usb_close(void)
{

    libusb_close(h);
    libusb_exit(NULL);
}

uint32_t nhc_usb_write(uint8_t u8Ep, uint8_t *pu8Data, uint32_t u32Len)
{
    uint32_t u32Tmp;

    u32Tmp = u32Len;
    if (libusb_bulk_transfer(h, u8Ep, pu8Data, u32Len, (int *)&u32Tmp, 5000) != 0)
    {
        return 0;
    }

    return 1;
}

uint32_t nhc_usb_read(uint8_t u8Ep, uint8_t *pu8Data, uint32_t u32Len)
{
    uint32_t u32Tmp;

    u32Tmp = u32Len;
    if (libusb_bulk_transfer(h, u8Ep | 0x80, pu8Data, u32Len, (int *)&u32Tmp, 5000) != 0)
    {
        return 0;
    }

    return 1;
}

uint32_t Init(void)
{
    uint8_t u8Buff[64];

    u8Buff[0] = 0xA0;

    if (!nhc_usb_write(0x01, u8Buff, 64))
    {
        return 0;
    }

    return 1;
}

uint32_t Exit(void)
{
    uint8_t u8Buff[64];

    u8Buff[0] = 0xA1;

    if (!nhc_usb_write(0x01, u8Buff, 64))
    {
        return 0;
    }

    return 1;
}

uint32_t ReadReg(uint8_t u8Address, uint32_t *pu32Data)
{
    uint8_t u8Buff[64];

    u8Buff[0] = 0xA2;
    u8Buff[1] = u8Address;

    if (!nhc_usb_write(0x01, u8Buff, 64))
    {
        return 0;
    }

    if (!nhc_usb_read(0x81, u8Buff, 64))
    {
        return 0;
    }

    if (!u8Buff[0])
    {
        return 0;
    }

    *pu32Data = u8tou32(u8Buff + 1);

    return 1;
}

uint32_t WriteReg(uint8_t u8Address, uint32_t u32Data)
{
    uint8_t u8Buff[64];

    u8Buff[0] = 0xA3;
    u8Buff[1] = u8Address;
    u32tou8(u32Data, u8Buff + 2);

    if (!nhc_usb_write(0x01, u8Buff, 64))
    {
        return 0;
    }

    return 1;
}

uint32_t ReadMem(uint32_t u32Address, uint32_t *pu32Data, uint8_t u8Width)
{
    uint8_t u8Buff[64];

    u8Buff[0] = 0xA4;
    u32tou8(u32Address, u8Buff + 1);
    u8Buff[5] = u8Width;

    if (!nhc_usb_write(0x01, u8Buff, 64))
    {
        return 0;
    }

    if (!nhc_usb_read(0x81, u8Buff, 64))
    {
        return 0;
    }

    if (!u8Buff[0])
    {
        *pu32Data = u8tou32(u8Buff + 1);
        return 0;
    }

    *pu32Data = u8tou32(u8Buff + 1);

    return 1;
}

uint32_t WriteMem(uint32_t u32Address, uint32_t u32Data, uint8_t u8Width)
{
    uint8_t u8Buff[64];

    u8Buff[0] = 0xA5;
    u32tou8(u32Address, u8Buff + 1);
    u32tou8(u32Data, u8Buff + 5);
    u8Buff[9] = u8Width;

    if (!nhc_usb_write(0x01, u8Buff, 64))
    {
        return 0;
    }

    return 1;
}

#define WCH_SDI_DATA0 0x04
#define WCH_SDI_DATA1 0x05
#define WCH_SDI_DMCONTROL 0x10
#define WCH_SDI_DMSTATUS 0x11
#define WCH_SDI_HARTINFO 0x12
#define WCH_SDI_ABSTRACTCS 0x16
#define WCH_SDI_COMMAND 0x17
#define WCH_SDI_ABSTRACTAUTO 0x18
#define WCH_SDI_PROGBUFF0 0x20
#define WCH_SDI_PROGBUFF1 0x21
#define WCH_SDI_PROGBUFF2 0x22
#define WCH_SDI_PROGBUFF3 0x23
#define WCH_SDI_PROGBUFF4 0x24
#define WCH_SDI_PROGBUFF5 0x25
#define WCH_SDI_PROGBUFF6 0x26
#define WCH_SDI_PROGBUFF7 0x27
#define WCH_SDI_HALTSUM0 0x40

#define WCH_SDI_CPBR 0x7C
#define WCH_SDI_CFGR 0x7D
#define WCH_SDI_SHDWCFGR 0x7E

#define FLASH_BASE 0x40022000
#define R32_FLASH_ACTLR 0x40022000
#define R32_FLASH_KEYR 0x40022004
#define R32_FLASH_OBKEYR 0x40022008
#define R32_FLASH_STATR 0x4002200C
#define R32_FLASH_CTLR 0x40022010
#define R32_FLASH_ADDR 0x40022014
#define R32_FLASH_OBR 0x4002201C
#define R32_FLASH_WPR 0x40022020
#define R32_FLASH_MODEKEYR 0x40022024
#define R32_FLASH_BOOT_MODEKEYR 0x40022028

uint32_t sdi_flash_protocol_init(void);
uint32_t sdi_flash_init(void);
void sdi_flash_exit(void);
uint32_t sdi_flash_unlock(void);
uint32_t sdi_flash_lock(void);
uint32_t sdi_flash_erase_chip(void);
uint32_t sdi_flash_erase_option_bytes(void);
uint32_t sdi_flash_write_half_word(uint32_t u32Address, uint16_t u16Data);
uint32_t sdi_flash_unlock_fast(void);
uint32_t sdi_flash_lock_fast(void);
uint32_t sdi_flash_unlock_option_bytes(void);
uint32_t sdi_flash_buff_reset(void);
uint32_t sdi_flash_buff_load(uint32_t u32Address, uint32_t u32Data);
uint32_t sdi_flash_write_page_fast(uint32_t u32Address, uint8_t *pu8Data);
uint32_t sdi_flash_system_reset(void);
uint32_t sdi_flash_write_option_bytes(uint32_t u32Value);

void NHC_Delay_Ms(uint32_t u32Delay)
{

    // usleep(u32Delay * 1000);
}

uint32_t sdi_flash_protocol_init(void)
{
    uint32_t u32Tmp;

    if (!Init())
    {
        return 0;
    }

    if (!WriteReg(WCH_SDI_SHDWCFGR, 0x5aa50400))
    {
        return 0;
    }

    if (!WriteReg(WCH_SDI_CFGR, 0x5aa50400))
    {
        return 0;
    }

    if (!ReadReg(WCH_SDI_CPBR, &u32Tmp))
    {
        return 0;
    }

    if (u32Tmp != 0x00010403)
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_init(void)
{
    uint32_t i;
    uint32_t u32Tmp;

    if (!ReadReg(WCH_SDI_DMSTATUS, &u32Tmp))
    {
        return 0;
    }

    if ((u32Tmp & (3 << 8)) != (3 << 8))
    {
        for (i = 0; i < 10; ++i)
        {
            if (!WriteReg(0x10, 0x80000001))
            {
                return 0;
            }

            if (!WriteReg(0x10, 0x80000001))
            {
                return 0;
            }

            if (!ReadReg(WCH_SDI_DMSTATUS, &u32Tmp))
            {
                return 0;
            }

            if ((u32Tmp & (3 << 8)) == (3 << 8))
            {
                break;
            }
        }

        if (i == 10)
        {
            return 0;
        }
    }

    return 1;
}

void sdi_flash_exit(void)
{

    Exit();
}

uint32_t sdi_flash_unlock(void)
{
    uint32_t u32Tmp;

    if (!WriteMem(R32_FLASH_KEYR, 0x45670123, 32))
    {
        return 0;
    }

    if (!WriteMem(R32_FLASH_KEYR, 0xCDEF89AB, 32))
    {
        return 0;
    }

    if (!ReadMem(R32_FLASH_CTLR, &u32Tmp, 32))
    {
        return 0;
    }

    if (u32Tmp & (1 << 7))
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_lock(void)
{

    return 1;
}

uint32_t sdi_flash_erase_chip(void)
{
    uint32_t u32Tmp;
    uint32_t i;

    if (!WriteMem(R32_FLASH_CTLR, 1 << 2, 32))
    {
        return 0;
    }
    if (!WriteMem(R32_FLASH_CTLR, (1 << 2) | (1 << 6), 32))
    {
        return 0;
    }

    for (i = 0; i < 10; ++i)
    {
        if (ReadMem(R32_FLASH_STATR, &u32Tmp, 32))
        {
            if (!(u32Tmp & 1))
            {
                break;
            }
        }
        NHC_Delay_Ms(1);
    }
    if (i == 10)
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_erase_option_bytes(void)
{
    uint32_t u32Tmp;
    uint32_t i;

    if (!WriteMem(R32_FLASH_CTLR, (1 << 5) | (1 << 9), 32))
    {
        return 0;
    }

    if (!WriteMem(R32_FLASH_CTLR, (1 << 5) | (1 << 6) | (1 << 9), 32))
    {
        return 0;
    }

    for (i = 0; i < 10; ++i)
    {
        if (ReadMem(R32_FLASH_STATR, &u32Tmp, 32))
        {
            if (!(u32Tmp & 1))
            {
                break;
            }
        }
        NHC_Delay_Ms(1);
    }
    if (i == 10)
    {
        return 0;
    }

    if (!WriteMem(R32_FLASH_STATR, u32Tmp, 32))
    {
        return 0;
    }

    if (!ReadMem(R32_FLASH_STATR, &u32Tmp, 32))
    {
        return 0;
    }

    if (u32Tmp & (1 << 5))
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_write_option_bytes(uint32_t u32Value)
{
    uint32_t u32Tmp;
    uint32_t i;

    if (!WriteMem(R32_FLASH_CTLR, (1 << 4) | (1 << 9), 32))
    {
        return 0;
    }

    if (!WriteMem(0x1ffff800, u32Value, 16))
    {
        return 0;
    }

    for (i = 0; i < 10; ++i)
    {
        if (ReadMem(R32_FLASH_STATR, &u32Tmp, 32))
        {
            if (!(u32Tmp & 1))
            {
                break;
            }
        }
        NHC_Delay_Ms(1);
    }
    if (i == 10)
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_write_half_word(uint32_t u32Address, uint16_t u16Data)
{

    return 1;
}

uint32_t sdi_flash_unlock_fast(void)
{
    uint32_t u32Tmp;

    if (!WriteMem(R32_FLASH_MODEKEYR, 0x45670123, 32))
    {
        return 0;
    }

    if (!WriteMem(R32_FLASH_MODEKEYR, 0xCDEF89AB, 32))
    {
        return 0;
    }

    if (!ReadMem(R32_FLASH_CTLR, &u32Tmp, 32))
    {
        return 0;
    }

    if (u32Tmp & (1 << 15))
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_unlock_option_bytes(void)
{
    uint32_t u32Tmp;

    if (!WriteMem(R32_FLASH_OBKEYR, 0x45670123, 32))
    {
        return 0;
    }

    if (!WriteMem(R32_FLASH_OBKEYR, 0xCDEF89AB, 32))
    {
        return 0;
    }

    if (!ReadMem(R32_FLASH_CTLR, &u32Tmp, 32))
    {
        return 0;
    }

    if (!(u32Tmp & (1 << 9)))
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_lock_fast(void)
{

    return 1;
}

uint32_t sdi_flash_buff_reset(void)
{
    uint32_t u32Tmp;
    uint32_t k;

    if (!WriteMem(R32_FLASH_CTLR, (1 << 16), 32))
    {
        return 0;
    }

    if (!WriteMem(R32_FLASH_CTLR, (1 << 16) | (1 << 19), 32))
    {
        return 0;
    }

    for (k = 0; k < 10; ++k)
    {
        if (ReadMem(R32_FLASH_STATR, &u32Tmp, 32))
        {
            if (!(u32Tmp & 1))
            {
                break;
            }
        }
        NHC_Delay_Ms(1);
    }
    if (k == 10)
    {
        return 0;
    }

    if (!WriteMem(R32_FLASH_STATR, u32Tmp, 32))
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_buff_load(uint32_t u32Address, uint32_t u32Data)
{
    uint32_t u32Tmp;
    uint32_t k;

    if (!WriteMem(u32Address, u32Data, 32))
    {
        return 0;
    }
    if (!WriteMem(R32_FLASH_CTLR, (1 << 16) | (1 << 18), 32))
    {
        return 0;
    }
    for (k = 0; k < 10; ++k)
    {
        if (ReadMem(R32_FLASH_STATR, &u32Tmp, 32))
        {
            if (!(u32Tmp & 1))
            {
                break;
            }
        }
        NHC_Delay_Ms(1);
    }
    if (k == 10)
    {
        return 0;
    }
    if (!WriteMem(R32_FLASH_STATR, u32Tmp, 32))
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_write_page_fast(uint32_t u32Address, uint8_t *pu8Data)
{
    uint32_t u32Tmp;
    uint32_t k;

    if (!sdi_flash_buff_reset())
    {
        return 0;
    }

    for (k = 0; k < 16; ++k)
    {
        if (!sdi_flash_buff_load(u32Address + k * 4, u8tou32(pu8Data + k * 4)))
        {
            return 0;
        }
    }

    if (!WriteMem(R32_FLASH_ADDR, u32Address, 32))
    {
        return 0;
    }

    if (!WriteMem(R32_FLASH_CTLR, (1 << 16) | (1 << 6), 32))
    {
        return 0;
    }
    for (k = 0; k < 10; ++k)
    {
        if (ReadMem(R32_FLASH_STATR, &u32Tmp, 32))
        {
            if (!(u32Tmp & 1))
            {
                break;
            }
        }
        NHC_Delay_Ms(1);
    }
    if (k == 10)
    {
        return 0;
    }

    if (!WriteMem(R32_FLASH_STATR, u32Tmp, 32))
    {
        return 0;
    }

    return 1;
}

uint32_t sdi_flash_system_reset(void)
{

    return WriteMem(0xe000ed10, (1 << 31), 32);
}

int main(int argc, char **argv)
{
    uint32_t u32Tmp;
    char szMsg[100];
    uint32_t u32Len = 16 * 1024;
    uint32_t u32Fail;
    uint32_t n;
    uint32_t i;
    KT_BinIO ktFlash;

    printf("NHC-Link042 for CH32V003\n");
    if (argc != 2)
    {
        printf("Usage: NHC-Link042 flash.bin\n");
        return 0;
    }

    if (!nhc_usb_open())
    {
        printf("Found no NHC-Link042\n");
        return 0;
    }

    ktFlash.u32Size = 16 * 1024;
    ktFlash.InitBuffer();
    if (!ktFlash.Read(argv[1]))
    {
        printf("Open flash file: FAIL\n");
        nhc_usb_close();
        return 0;
    }

    if (!sdi_flash_protocol_init())
    {
        printf("Init protocol: FAIL\n");
        nhc_usb_close();
        return 0;
    }
    if (!sdi_flash_init())
    {
        printf("Init: FAIL\n");
        nhc_usb_close();
        return 0;
    }

    if (!ReadMem(R32_FLASH_OBR, &u32Tmp, 32))
    {
        printf("Read: FAIL\n");
        nhc_usb_close();
        return 0;
    }
    if (u32Tmp & (1 << 1))
    {
        /* locked */
        if (!sdi_flash_unlock())
        {
            printf("Unlock: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_unlock_fast())
        {
            printf("Unlock fast: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_unlock_option_bytes())
        {
            printf("Unlock option bytes: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_erase_option_bytes())
        {
            printf("Erase option bytes: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_write_option_bytes(0x5aa5))
        {
            printf("Write option bytes: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_system_reset())
        {
            printf("System Reset: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        NHC_Delay_Ms(10);
        if (!sdi_flash_protocol_init())
        {
            printf("Init protocol: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_init())
        {
            printf("Init: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!ReadMem(R32_FLASH_OBR, &u32Tmp, 32))
        {
            printf("Read: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (u32Tmp & (1 << 1))
        {
            printf("Check: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_unlock())
        {
            printf("Unlock: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_unlock_fast())
        {
            printf("Unlock fast: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_unlock_option_bytes())
        {
            printf("Unlock option bytes: FAIL\n");
            nhc_usb_close();
            return 0;
        }
    }
    else
    {
        /* no lock */
        if (!sdi_flash_unlock())
        {
            printf("Unlock: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_unlock_fast())
        {
            printf("Unlock fast: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_unlock_option_bytes())
        {
            printf("Unlock option bytes: FAIL\n");
            nhc_usb_close();
            return 0;
        }
        if (!sdi_flash_erase_chip())
        {
            printf("Erase: FAIL\n");
            nhc_usb_close();
            return 0;
        }
    }

    /* write page */
    n = u32Len / 64;
    u32Fail = 0;
    printf("Write: ");
    for (i = 0; i < n; ++i)
    {
        if (!sdi_flash_write_page_fast(0x08000000 + i * 64, ktFlash.pReadBuff + i * 64))
        {
            u32Fail = 1;
            break;
        }
        printf(".");
        fflush(stdout);
    }
    if (u32Fail)
    {
        printf("Write: FAIL\n");
        nhc_usb_close();
        return 0;
    }
    printf("\n");
    fflush(stdout);

    /* read and verify */
    printf("Read ");
    n = u32Len / 4;
    u32Fail = 0;
    for (i = 0; i < n; ++i)
    {
        if (!ReadMem(0x08000000 + i * 4, &u32Tmp, 32))
        {
            u32Fail = 1;
            break;
        }
        u32tou8(u32Tmp, ktFlash.pWriteBuff + i * 4);
        if ((i % 16) == 0)
        {
            printf(".");
            fflush(stdout);
        }
    }

    printf("\n");
    fflush(stdout);

    if (u32Fail)
    {
        printf("Read: FAIL\n");
        nhc_usb_close();
        return 0;
    }
    u32Fail = 0;
    for (i = 0; i < u32Len; ++i)
    {
        if (ktFlash.pReadBuff[i] != ktFlash.pWriteBuff[i])
        {
            u32Fail = 1;
            break;
        }
    }
    if (u32Fail)
    {
        printf("Verify: FAIL\n");
        nhc_usb_close();
        return 0;
    }

    sdi_flash_system_reset();

    sdi_flash_exit();

    printf("Done\n");
    nhc_usb_close();

    return 0;
}
