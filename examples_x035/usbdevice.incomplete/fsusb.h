#ifndef _FSUSB_H
#define _FSUSB_H

#include <stdint.h>

#include "descriptor.h"

extern uint32_t USBDEBUG0, USBDEBUG1, USBDEBUG2;

int FSUSBSetup();
void USBFS_Poll();
uint8_t USBFS_Endp_DataUp(uint8_t endp, uint8_t *pbuf, uint16_t len, uint8_t mod);

// XXX TODO: Put the below in an object.

/*******************************************************************************/
/* Variable Definition */
/* Global */
extern const    uint8_t  *pUSBFS_Descr;


/* USB Device Status */
extern volatile uint8_t  USBFS_DevConfig;
extern volatile uint8_t  USBFS_DevAddr;
extern volatile uint8_t  USBFS_DevSleepStatus;
extern volatile uint8_t  USBFS_DevEnumStatus;

/* Endpoint Buffer */
extern __attribute__ ((aligned(4))) uint8_t USBFS_EP0_Buf[];
extern __attribute__ ((aligned(4))) uint8_t USBFS_EP2_Buf[];


/* USB IN Endpoint Busy Flag */
extern volatile uint8_t  USBFS_Endp_Busy[ ];


// ABOVE: TOOD: REWRITE!

// Copied from x035 documentation.
#define USBFSD_UEP_MOD_BASE         0x4002340C
#define USBFSD_UEP_DMA_BASE         0x40023410
#define USBFSD_UEP_LEN_BASE         0x40023420
#define USBFSD_UEP_CTL_BASE         0x40023422
#define USBFSD_UEP_RX_EN            0x08
#define USBFSD_UEP_TX_EN            0x04
#define USBFSD_UEP_BUF_MOD          0x01
#define DEF_UEP_DMA_LOAD            0 /* Direct the DMA address to the data to be processed */
#define DEF_UEP_CPY_LOAD            1 /* Use memcpy to move data to a buffer */
#define USBFSD_UEP_MOD( N )         (*((volatile uint8_t *)( USBFSD_UEP_MOD_BASE + N )))
#define USBFSD_UEP_TX_CTRL( N )     (*((volatile uint8_t *)( USBFSD_UEP_CTL_BASE + N * 0x04 )))
#define USBFSD_UEP_RX_CTRL( N )     (*((volatile uint8_t *)( USBFSD_UEP_CTL_BASE + N * 0x04 )))
#define USBFSD_UEP_DMA( N )         (*((volatile uint32_t *)( USBFSD_UEP_DMA_BASE + N * 0x04 )))
#define USBFSD_UEP_BUF( N )         ((uint8_t *)(*((volatile uint32_t *)( USBFSD_UEP_DMA_BASE + N * 0x04 ))) + 0x20000000)
#define USBFSD_UEP_TLEN( N )        (*((volatile uint16_t *)( USBFSD_UEP_LEN_BASE + N * 0x04 )))

/* R8_UEPn_TX_CTRL */
#define USBFS_UEP_T_AUTO_TOG        (1<<4)      // enable automatic toggle after successful transfer completion on endpoint 1/2/3: 0=manual toggle, 1=automatic toggle
#define USBFS_UEP_T_TOG             (1<<6)      // prepared data toggle flag of USB endpoint X transmittal (IN): 0=DATA0, 1=DATA1
#define USBFS_UEP_T_RES_MASK        (3<<0)      // bit mask of handshake response type for USB endpoint X transmittal (IN)
#define USBFS_UEP_T_RES_ACK         (0<<1)
#define USBFS_UEP_T_RES_NONE        (1<<0)
#define USBFS_UEP_T_RES_NAK         (1<<1)
#define USBFS_UEP_T_RES_STALL       (3<<0)
// bUEP_T_RES1 & bUEP_T_RES0: handshake response type for USB endpoint X transmittal (IN)
//   00: DATA0 or DATA1 then expecting ACK (ready)
//   01: DATA0 or DATA1 then expecting no response, time out from host, for non-zero endpoint isochronous transactions
//   10: NAK (busy)
//   11: STALL (error)
// host aux setup

/* R8_UEPn_RX_CTRL, n=0-7 */
#define USBFS_UEP_R_AUTO_TOG        (1<<4)      // enable automatic toggle after successful transfer completion on endpoint 1/2/3: 0=manual toggle, 1=automatic toggle
#define USBFS_UEP_R_TOG             (1<<7)      // expected data toggle flag of USB endpoint X receiving (OUT): 0=DATA0, 1=DATA1
#define USBFS_UEP_R_RES_MASK        (3<<2)      // bit mask of handshake response type for USB endpoint X receiving (OUT)
#define USBFS_UEP_R_RES_ACK         (0<<3)
#define USBFS_UEP_R_RES_NONE        (1<<2)
#define USBFS_UEP_R_RES_NAK         (1<<3)
#define USBFS_UEP_R_RES_STALL       (3<<2)


/* R8_USB_DEV_AD */
#define USBFS_UDA_GP_BIT            0x80
#define USBFS_USB_ADDR_MASK         0x7F



#define DEF_USBD_UEP0_SIZE		   64	 /* usb hs/fs device end-point 0 size */
#define EP_SIZE 64

#define DEF_UEP_IN                  0x80
#define DEF_UEP_OUT                 0x00
#define DEF_UEP_BUSY                0x01
#define DEF_UEP_FREE                0x00

#define DEF_UEP0 0
#define DEF_UEP1 1
#define DEF_UEP2 2
#define DEF_UEP3 3
#define DEF_UEP4 4
#define DEF_UEP5 5
#define DEF_UEP6 6
#define DEF_UEP7 7
#define NUM_EP 8

// Hardware specific

#define EP1_T_EN					(1<<6)
#define EP2_T_EN					(1<<2)
#define EP3_T_EN					(1<<6)
#define EP4_T_EN					(1<<2)
#define EP1_R_EN					(1<<7)
#define EP2_R_EN					(1<<3)
#define EP3_R_EN					(1<<7)
#define EP4_R_EN					(1<<3)


/* R8_USB_CTRL */
#define USBFS_UC_HOST_MODE          0x80
#define USBFS_UC_LOW_SPEED          0x40
#define USBFS_UC_DEV_PU_EN          0x20
#define USBFS_UC_SYS_CTRL_MASK      0x30
#define USBFS_UC_SYS_CTRL0          0x00
#define USBFS_UC_SYS_CTRL1          0x10
#define USBFS_UC_SYS_CTRL2          0x20
#define USBFS_UC_SYS_CTRL3          0x30
#define USBFS_UC_INT_BUSY           0x08
#define USBFS_UC_RESET_SIE          0x04
#define USBFS_UC_CLR_ALL            0x02
#define USBFS_UC_DMA_EN             0x01

/* R8_USB_INT_EN */
#define USBFS_UIE_DEV_SOF           0x80
#define USBFS_UIE_DEV_NAK           0x40
#define USBFS_UIE_FIFO_OV           0x10
#define USBFS_UIE_HST_SOF           0x08
#define USBFS_UIE_SUSPEND           0x04
#define USBFS_UIE_TRANSFER          0x02
#define USBFS_UIE_DETECT            0x01
#define USBFS_UIE_BUS_RST           0x01

/* R8_USB_DEV_AD */
#define USBFS_UDA_GP_BIT            0x80
#define USBFS_USB_ADDR_MASK         0x7F

/* R8_USB_MIS_ST */
#define USBFS_UMS_SOF_PRES          0x80
#define USBFS_UMS_SOF_ACT           0x40
#define USBFS_UMS_SIE_FREE          0x20
#define USBFS_UMS_R_FIFO_RDY        0x10
#define USBFS_UMS_BUS_RESET         0x08
#define USBFS_UMS_SUSPEND           0x04
#define USBFS_UMS_DM_LEVEL          0x02
#define USBFS_UMS_DEV_ATTACH        0x01

/* USB Setup Request */
typedef struct __attribute__((packed)) _USB_SETUP_REQ
{
    uint8_t  bRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} USB_SETUP_REQ, *PUSB_SETUP_REQ;

/* USB Device Descriptor */
typedef struct __attribute__((packed)) _USB_DEVICE_DESCR
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} USB_DEV_DESCR, *PUSB_DEV_DESCR;

/* USB Configuration Descriptor */
typedef struct __attribute__((packed)) _USB_CONFIG_DESCR
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  MaxPower;
} USB_CFG_DESCR, *PUSB_CFG_DESCR;

/* USB Interface Descriptor */
typedef struct __attribute__((packed)) _USB_INTERF_DESCR
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bInterfaceNumber;
    uint8_t  bAlternateSetting;
    uint8_t  bNumEndpoints;
    uint8_t  bInterfaceClass;
    uint8_t  bInterfaceSubClass;
    uint8_t  bInterfaceProtocol;
    uint8_t  iInterface;
} USB_ITF_DESCR, *PUSB_ITF_DESCR;

/* USB Endpoint Descriptor */
typedef struct __attribute__((packed)) _USB_ENDPOINT_DESCR
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint8_t  wMaxPacketSizeL;
    uint8_t  wMaxPacketSizeH;
    uint8_t  bInterval;
} USB_ENDP_DESCR, *PUSB_ENDP_DESCR;

/* USB Configuration Descriptor Set */
typedef struct __attribute__((packed)) _USB_CONFIG_DESCR_LONG
{
    USB_CFG_DESCR  cfg_descr;
    USB_ITF_DESCR  itf_descr;
    USB_ENDP_DESCR endp_descr[ 1 ];
} USB_CFG_DESCR_LONG, *PUSB_CFG_DESCR_LONG;

/* USB HUB Descriptor */
typedef struct __attribute__((packed)) _USB_HUB_DESCR
{
    uint8_t  bDescLength;
    uint8_t  bDescriptorType;
    uint8_t  bNbrPorts;
    uint8_t  wHubCharacteristicsL;
    uint8_t  wHubCharacteristicsH;
    uint8_t  bPwrOn2PwrGood;
    uint8_t  bHubContrCurrent;
    uint8_t  DeviceRemovable;
    uint8_t  PortPwrCtrlMask;
} USB_HUB_DESCR, *PUSB_HUB_DESCR;

/* USB HID Descriptor */
typedef struct __attribute__((packed)) _USB_HID_DESCR
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdHID;
    uint8_t  bCountryCode;
    uint8_t  bNumDescriptors;
    uint8_t  bDescriptorTypeX;
    uint8_t  wDescriptorLengthL;
    uint8_t  wDescriptorLengthH;
} USB_HID_DESCR, *PUSB_HID_DESCR;


/*******************************************************************************/
/* USB Communication Related Macro Definition */

/* USB PID */
#ifndef USB_PID_SETUP
#define USB_PID_NULL                0x00
#define USB_PID_SOF                 0x05
#define USB_PID_SETUP               0x0D
#define USB_PID_IN                  0x09
#define USB_PID_OUT                 0x01
#define USB_PID_NYET                0x06
#define USB_PID_ACK                 0x02
#define USB_PID_NAK                 0x0A
#define USB_PID_STALL               0x0E
#define USB_PID_DATA0               0x03
#define USB_PID_DATA1               0x0B
#define USB_PID_PRE                 0x0C
#endif

/* USB standard device request code */
#ifndef USB_GET_DESCRIPTOR
#define USB_GET_STATUS              0x00
#define USB_CLEAR_FEATURE           0x01
#define USB_SET_FEATURE             0x03
#define USB_SET_ADDRESS             0x05
#define USB_GET_DESCRIPTOR          0x06
#define USB_SET_DESCRIPTOR          0x07
#define USB_GET_CONFIGURATION       0x08
#define USB_SET_CONFIGURATION       0x09
#define USB_GET_INTERFACE           0x0A
#define USB_SET_INTERFACE           0x0B
#define USB_SYNCH_FRAME             0x0C
#endif

#define DEF_STRING_DESC_LANG        0x00
#define DEF_STRING_DESC_MANU        0x01
#define DEF_STRING_DESC_PROD        0x02
#define DEF_STRING_DESC_SERN        0x03

/* USB hub class request code */
#ifndef HUB_GET_DESCRIPTOR
#define HUB_GET_STATUS              0x00
#define HUB_CLEAR_FEATURE           0x01
#define HUB_GET_STATE               0x02
#define HUB_SET_FEATURE             0x03
#define HUB_GET_DESCRIPTOR          0x06
#define HUB_SET_DESCRIPTOR          0x07
#endif

/* USB HID class request code */
#ifndef HID_GET_REPORT
#define HID_GET_REPORT              0x01
#define HID_GET_IDLE                0x02
#define HID_GET_PROTOCOL            0x03
#define HID_SET_REPORT              0x09
#define HID_SET_IDLE                0x0A
#define HID_SET_PROTOCOL            0x0B
#endif

/* Bit Define for USB Request Type */
#ifndef USB_REQ_TYP_MASK
#define USB_REQ_TYP_IN              0x80
#define USB_REQ_TYP_OUT             0x00
#define USB_REQ_TYP_READ            0x80
#define USB_REQ_TYP_WRITE           0x00
#define USB_REQ_TYP_MASK            0x60
#define USB_REQ_TYP_STANDARD        0x00
#define USB_REQ_TYP_CLASS           0x20
#define USB_REQ_TYP_VENDOR          0x40
#define USB_REQ_TYP_RESERVED        0x60
#define USB_REQ_RECIP_MASK          0x1F
#define USB_REQ_RECIP_DEVICE        0x00
#define USB_REQ_RECIP_INTERF        0x01
#define USB_REQ_RECIP_ENDP          0x02
#define USB_REQ_RECIP_OTHER         0x03
#define USB_REQ_FEAT_REMOTE_WAKEUP  0x01
#define USB_REQ_FEAT_ENDP_HALT      0x00
#endif

/* USB Descriptor Type */
#ifndef USB_DESCR_TYP_DEVICE
#define USB_DESCR_TYP_DEVICE        0x01
#define USB_DESCR_TYP_CONFIG        0x02
#define USB_DESCR_TYP_STRING        0x03
#define USB_DESCR_TYP_INTERF        0x04
#define USB_DESCR_TYP_ENDP          0x05
#define USB_DESCR_TYP_QUALIF        0x06
#define USB_DESCR_TYP_SPEED         0x07
#define USB_DESCR_TYP_OTG           0x09
#define USB_DESCR_TYP_BOS           0X0F
#define USB_DESCR_TYP_HID           0x21
#define USB_DESCR_TYP_REPORT        0x22
#define USB_DESCR_TYP_PHYSIC        0x23
#define USB_DESCR_TYP_CS_INTF       0x24
#define USB_DESCR_TYP_CS_ENDP       0x25
#define USB_DESCR_TYP_HUB           0x29
#endif

/* USB Device Class */
#ifndef USB_DEV_CLASS_HUB
#define USB_DEV_CLASS_RESERVED      0x00
#define USB_DEV_CLASS_AUDIO         0x01
#define USB_DEV_CLASS_COMMUNIC      0x02
#define USB_DEV_CLASS_HID           0x03
#define USB_DEV_CLASS_MONITOR       0x04
#define USB_DEV_CLASS_PHYSIC_IF     0x05
#define USB_DEV_CLASS_POWER         0x06
#define USB_DEV_CLASS_PRINTER       0x07
#define USB_DEV_CLASS_STORAGE       0x08
#define USB_DEV_CLASS_HUB           0x09
#define USB_DEV_CLASS_VEN_SPEC      0xFF
#endif

/* USB Hub Class Request */
#ifndef HUB_GET_HUB_DESCRIPTOR
#define HUB_CLEAR_HUB_FEATURE       0x20
#define HUB_CLEAR_PORT_FEATURE      0x23
#define HUB_GET_BUS_STATE           0xA3
#define HUB_GET_HUB_DESCRIPTOR      0xA0
#define HUB_GET_HUB_STATUS          0xA0
#define HUB_GET_PORT_STATUS         0xA3
#define HUB_SET_HUB_DESCRIPTOR      0x20
#define HUB_SET_HUB_FEATURE         0x20
#define HUB_SET_PORT_FEATURE        0x23
#endif

/* Hub Class Feature Selectors */
#ifndef HUB_PORT_RESET
#define HUB_C_HUB_LOCAL_POWER       0
#define HUB_C_HUB_OVER_CURRENT      1
#define HUB_PORT_CONNECTION         0
#define HUB_PORT_ENABLE             1
#define HUB_PORT_SUSPEND            2
#define HUB_PORT_OVER_CURRENT       3
#define HUB_PORT_RESET              4
#define HUB_PORT_POWER              8
#define HUB_PORT_LOW_SPEED          9
#define HUB_C_PORT_CONNECTION       16
#define HUB_C_PORT_ENABLE           17
#define HUB_C_PORT_SUSPEND          18
#define HUB_C_PORT_OVER_CURRENT     19
#define HUB_C_PORT_RESET            20
#endif

/* USB HID Class Request Code */
#ifndef HID_GET_REPORT
#define HID_GET_REPORT              0x01
#define HID_GET_IDLE                0x02
#define HID_GET_PROTOCOL            0x03
#define HID_SET_REPORT              0x09
#define HID_SET_IDLE                0x0A
#define HID_SET_PROTOCOL            0x0B
#endif

/* USB UDisk */
#ifndef USB_BO_CBW_SIZE
#define USB_BO_CBW_SIZE             0x1F
#define USB_BO_CSW_SIZE             0x0D
#endif
#ifndef USB_BO_CBW_SIG0
#define USB_BO_CBW_SIG0             0x55
#define USB_BO_CBW_SIG1             0x53
#define USB_BO_CBW_SIG2             0x42
#define USB_BO_CBW_SIG3             0x43
#define USB_BO_CSW_SIG0             0x55
#define USB_BO_CSW_SIG1             0x53
#define USB_BO_CSW_SIG2             0x42
#define USB_BO_CSW_SIG3             0x53
#endif

/* USB CDC Class request code */
#ifndef CDC_GET_LINE_CODING
#define CDC_GET_LINE_CODING         0X21                                      /* This request allows the host to find out the currently configured line coding */
#define CDC_SET_LINE_CODING         0x20                                      /* Configures DTE rate, stop-bits, parity, and number-of-character */
#define CDC_SET_LINE_CTLSTE         0X22                                      /* This request generates RS-232/V.24 style control signals */
#define CDC_SEND_BREAK              0X23                                      /* Sends special carrier modulation used to specify RS-232 style break */
#endif
/*******************************************************************************/


#endif

