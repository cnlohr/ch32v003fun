#ifdef _WIN32

/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009, All Rights Reserved.
 
 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU General Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        http://github.com/signal11/hidapi .
********************************************************/


/* Copy of LICENSE-orig.txt (compatible with MIT/x11 license)

 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Copyright 2009, Alan Ott, Signal 11 Software.
 All Rights Reserved.
 
 This software may be used by anyone for any reason so
 long as the copyright notice in the source files
 remains intact.
*/

#include <windows.h>

#ifndef _NTDEF_
typedef LONG NTSTATUS;
#endif

#ifdef __MINGW32__
#include <ntdef.h>
#include <winbase.h>
#endif

#ifdef __CYGWIN__
#include <ntdef.h>
#define _wcsdup wcsdup
#endif

/* The maximum number of characters that can be passed into the
   HidD_Get*String() functions without it failing.*/
#define MAX_STRING_WCHARS 0xFFF

/*#define HIDAPI_USE_DDK*/

#if defined(WINDOWS) || defined(WIN32)  || defined(WIN64) \
                     || defined(_WIN32) || defined(_WIN64)
#ifndef strdup
#define strdup _strdup
#endif
#endif 

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TCC
	typedef struct _SP_DEVINFO_DATA {
		DWORD     cbSize;
		GUID      ClassGuid;
		DWORD     DevInst;
		ULONG_PTR Reserved;
	} SP_DEVINFO_DATA, *PSP_DEVINFO_DATA;
	typedef struct _SP_DEVICE_INTERFACE_DATA {
		DWORD     cbSize;
		GUID      InterfaceClassGuid;
		DWORD     Flags;
		ULONG_PTR Reserved;
	} SP_DEVICE_INTERFACE_DATA, *PSP_DEVICE_INTERFACE_DATA;
	typedef struct _SP_DEVICE_INTERFACE_DETAIL_DATA {
		DWORD cbSize;
		CHAR DevicePath[ANYSIZE_ARRAY];
	} SP_DEVICE_INTERFACE_DETAIL_DATA_A, *PSP_DEVICE_INTERFACE_DETAIL_DATA_A;
	typedef PVOID HDEVINFO;
	
	HDEVINFO WINAPI SetupDiGetClassDevsA(CONST GUID*,PCSTR,HWND,DWORD);
	
	#define DIGCF_PRESENT	0x00000002
	#define DIGCF_DEVICEINTERFACE	0x00000010
	#define SPDRP_CLASS	7
	#define SPDRP_DRIVER	9
	#define FILE_DEVICE_KEYBOARD            0x0000000b
	#define METHOD_OUT_DIRECT                 2
	enum
	{ FILE_ANY_ACCESS			= 0x00000000UL,
		FILE_SPECIAL_ACCESS			= FILE_ANY_ACCESS,
		FILE_READ_ACCESS			= 0x00000001UL,
		FILE_WRITE_ACCESS			= 0x00000002UL
	};
	#define CTL_CODE(t,f,m,a) (((t)<<16)|((a)<<14)|((f)<<2)|(m))

#define WINSETUPAPI __declspec(dllimport) //XXX Should these be CDECL?
WINSETUPAPI BOOL SetupDiEnumDeviceInterfaces(
  HDEVINFO                  DeviceInfoSet,
  PSP_DEVINFO_DATA          DeviceInfoData,
  const GUID                *InterfaceClassGuid,
  DWORD                     MemberIndex,
  PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
);
WINSETUPAPI BOOL SetupDiGetDeviceInterfaceDetailA(
  HDEVINFO                           DeviceInfoSet,
  PSP_DEVICE_INTERFACE_DATA          DeviceInterfaceData,
  PSP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData,
  DWORD                              DeviceInterfaceDetailDataSize,
  PDWORD                             RequiredSize,
  PSP_DEVINFO_DATA                   DeviceInfoData
);
WINSETUPAPI BOOL SetupDiEnumDeviceInfo(
  HDEVINFO         DeviceInfoSet,
  DWORD            MemberIndex,
  PSP_DEVINFO_DATA DeviceInfoData
);
WINSETUPAPI BOOL SetupDiGetDeviceRegistryPropertyA(
  HDEVINFO         DeviceInfoSet,
  PSP_DEVINFO_DATA DeviceInfoData,
  DWORD            Property,
  PDWORD           PropertyRegDataType,
  PBYTE            PropertyBuffer,
  DWORD            PropertyBufferSize,
  PDWORD           RequiredSize
);
WINSETUPAPI BOOL SetupDiDestroyDeviceInfoList(
  HDEVINFO DeviceInfoSet
);
#else
	#include <setupapi.h>
	#include <winioctl.h>
#endif
	#ifdef HIDAPI_USE_DDK
		#include <hidsdi.h>
	#endif

	/* Copied from inc/ddk/hidclass.h, part of the Windows DDK. */
	#define HID_OUT_CTL_CODE(id)  \
		CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
	#define IOCTL_HID_GET_FEATURE                   HID_OUT_CTL_CODE(100)

#ifdef __cplusplus
} /* extern "C" */
#endif

#include <stdio.h>
#include <stdlib.h>


#include "hidapi.h"

#undef MIN
#define MIN(x,y) ((x) < (y)? (x): (y))

#ifdef _MSC_VER
	/* Thanks Microsoft, but I know how to use strncpy(). */
	#pragma warning(disable:4996)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HIDAPI_USE_DDK
	/* Since we're not building with the DDK, and the HID header
	   files aren't part of the SDK, we have to define all this
	   stuff here. In lookup_functions(), the function pointers
	   defined below are set. */
	typedef struct _HIDD_ATTRIBUTES{
		ULONG Size;
		USHORT VendorID;
		USHORT ProductID;
		USHORT VersionNumber;
	} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

	typedef USHORT USAGE;
	typedef struct _HIDP_CAPS {
		USAGE Usage;
		USAGE UsagePage;
		USHORT InputReportByteLength;
		USHORT OutputReportByteLength;
		USHORT FeatureReportByteLength;
		USHORT Reserved[17];
		USHORT fields_not_used_by_hidapi[10];
	} HIDP_CAPS, *PHIDP_CAPS;
	typedef void* PHIDP_PREPARSED_DATA;
	#define HIDP_STATUS_SUCCESS 0x110000

	typedef BOOLEAN (__stdcall *HidD_GetAttributes_)(HANDLE device, PHIDD_ATTRIBUTES attrib);
	typedef BOOLEAN (__stdcall *HidD_GetSerialNumberString_)(HANDLE device, PVOID buffer, ULONG buffer_len);
	typedef BOOLEAN (__stdcall *HidD_GetManufacturerString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
	typedef BOOLEAN (__stdcall *HidD_GetProductString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
	typedef BOOLEAN (__stdcall *HidD_SetFeature_)(HANDLE handle, PVOID data, ULONG length);
	typedef BOOLEAN (__stdcall *HidD_GetFeature_)(HANDLE handle, PVOID data, ULONG length);
	typedef BOOLEAN (__stdcall *HidD_GetIndexedString_)(HANDLE handle, ULONG string_index, PVOID buffer, ULONG buffer_len);
	typedef BOOLEAN (__stdcall *HidD_GetPreparsedData_)(HANDLE handle, PHIDP_PREPARSED_DATA *preparsed_data);
	typedef BOOLEAN (__stdcall *HidD_FreePreparsedData_)(PHIDP_PREPARSED_DATA preparsed_data);
	typedef NTSTATUS (__stdcall *HidP_GetCaps_)(PHIDP_PREPARSED_DATA preparsed_data, HIDP_CAPS *caps);
	typedef BOOLEAN (__stdcall *HidD_SetNumInputBuffers_)(HANDLE handle, ULONG number_buffers);

	static HidD_GetAttributes_ HidD_GetAttributes;
	static HidD_GetSerialNumberString_ HidD_GetSerialNumberString;
	static HidD_GetManufacturerString_ HidD_GetManufacturerString;
	static HidD_GetProductString_ HidD_GetProductString;
	static HidD_SetFeature_ HidD_SetFeature;
	static HidD_GetFeature_ HidD_GetFeature;
	static HidD_GetIndexedString_ HidD_GetIndexedString;
	static HidD_GetPreparsedData_ HidD_GetPreparsedData;
	static HidD_FreePreparsedData_ HidD_FreePreparsedData;
	static HidP_GetCaps_ HidP_GetCaps;
	static HidD_SetNumInputBuffers_ HidD_SetNumInputBuffers;

	static HMODULE lib_handle = NULL;
	static BOOLEAN initialized = FALSE;
#endif /* HIDAPI_USE_DDK */

struct hid_device_ {
		HANDLE device_handle;
		BOOL blocking;
		USHORT output_report_length;
		size_t input_report_length;
		void *last_error_str;
		DWORD last_error_num;
		BOOL read_pending;
		char *read_buf;
		OVERLAPPED ol;
};

static hid_device *new_hid_device()
{
	hid_device *dev = (hid_device*) calloc(1, sizeof(hid_device));
	dev->device_handle = INVALID_HANDLE_VALUE;
	dev->blocking = TRUE;
	dev->output_report_length = 0;
	dev->input_report_length = 0;
	dev->last_error_str = NULL;
	dev->last_error_num = 0;
	dev->read_pending = FALSE;
	dev->read_buf = NULL;
	memset(&dev->ol, 0, sizeof(dev->ol));
	dev->ol.hEvent = CreateEvent(NULL, FALSE, FALSE /*initial state f=nonsignaled*/, NULL);

	return dev;
}

static void free_hid_device(hid_device *dev)
{
	CloseHandle(dev->ol.hEvent);
	CloseHandle(dev->device_handle);
	LocalFree(dev->last_error_str);
	free(dev->read_buf);
	free(dev);
}

static void register_error(hid_device *device, const char *op)
{
	WCHAR *ptr, *msg;

	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPVOID)&msg, 0/*sz*/,
		NULL);
	
	/* Get rid of the CR and LF that FormatMessage() sticks at the
	   end of the message. Thanks Microsoft! */
	ptr = msg;
	while (*ptr) {
		if (*ptr == '\r') {
			*ptr = 0x0000;
			break;
		}
		ptr++;
	}

	/* Store the message off in the Device entry so that
	   the hid_error() function can pick it up. */
	LocalFree(device->last_error_str);
	device->last_error_str = msg;
}

#ifndef HIDAPI_USE_DDK
static int lookup_functions()
{
	lib_handle = LoadLibraryA("hid.dll");
	if (lib_handle) {
#define RESOLVE(x) x = (x##_)GetProcAddress(lib_handle, #x); if (!x) return -1;
		RESOLVE(HidD_GetAttributes);
		RESOLVE(HidD_GetSerialNumberString);
		RESOLVE(HidD_GetManufacturerString);
		RESOLVE(HidD_GetProductString);
		RESOLVE(HidD_SetFeature);
		RESOLVE(HidD_GetFeature);
		RESOLVE(HidD_GetIndexedString);
		RESOLVE(HidD_GetPreparsedData);
		RESOLVE(HidD_FreePreparsedData);
		RESOLVE(HidP_GetCaps);
		RESOLVE(HidD_SetNumInputBuffers);
#undef RESOLVE
	}
	else
		return -1;

	return 0;
}
#endif

static HANDLE open_device(const char *path, BOOL enumerate)
{
	HANDLE handle;
	DWORD desired_access = (enumerate)? 0: (GENERIC_WRITE | GENERIC_READ);
	DWORD share_mode = FILE_SHARE_READ|FILE_SHARE_WRITE;

	handle = CreateFileA(path,
		desired_access,
		share_mode,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,/*FILE_ATTRIBUTE_NORMAL,*/
		0);

	return handle;
}

int HID_API_EXPORT hid_init(void)
{
#ifndef HIDAPI_USE_DDK
	if (!initialized) {
		if (lookup_functions() < 0) {
			hid_exit();
			return -1;
		}
		initialized = TRUE;
	}
#endif
	return 0;
}

int HID_API_EXPORT hid_exit(void)
{
#ifndef HIDAPI_USE_DDK
	if (lib_handle)
		FreeLibrary(lib_handle);
	lib_handle = NULL;
	initialized = FALSE;
#endif
	return 0;
}

struct hid_device_info HID_API_EXPORT * HID_API_CALL hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	BOOL res;
	struct hid_device_info *root = NULL; /* return object */
	struct hid_device_info *cur_dev = NULL;

	/* Windows objects for interacting with the driver. */
	GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };
	SP_DEVINFO_DATA devinfo_data;
	SP_DEVICE_INTERFACE_DATA device_interface_data;
	SP_DEVICE_INTERFACE_DETAIL_DATA_A *device_interface_detail_data = NULL;
	HDEVINFO device_info_set = INVALID_HANDLE_VALUE;
	int device_index = 0;
	int i;

	if (hid_init() < 0)
		return NULL;

	/* Initialize the Windows objects. */
	memset(&devinfo_data, 0x0, sizeof(devinfo_data));
	devinfo_data.cbSize = sizeof(SP_DEVINFO_DATA);
	device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	/* Get information for all the devices belonging to the HID class. */
	device_info_set = SetupDiGetClassDevsA(&InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	
	/* Iterate over each device in the HID class, looking for the right one. */
	
	for (;;) {
		HANDLE write_handle = INVALID_HANDLE_VALUE;
		DWORD required_size = 0;
		HIDD_ATTRIBUTES attrib;

		res = SetupDiEnumDeviceInterfaces(device_info_set,
			NULL,
			&InterfaceClassGuid,
			device_index,
			&device_interface_data);
		
		if (!res) {
			/* A return of FALSE from this function means that
			   there are no more devices. */
			break;
		}

		/* Call with 0-sized detail size, and let the function
		   tell us how long the detail struct needs to be. The
		   size is put in &required_size. */
		res = SetupDiGetDeviceInterfaceDetailA(device_info_set,
			&device_interface_data,
			NULL,
			0,
			&required_size,
			NULL);

		/* Allocate a long enough structure for device_interface_detail_data. */
		device_interface_detail_data = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*) malloc(required_size);
		device_interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

		/* Get the detailed data for this device. The detail data gives us
		   the device path for this device, which is then passed into
		   CreateFile() to get a handle to the device. */
		res = SetupDiGetDeviceInterfaceDetailA(device_info_set,
			&device_interface_data,
			device_interface_detail_data,
			required_size,
			NULL,
			NULL);

		if (!res) {
			/* register_error(dev, "Unable to call SetupDiGetDeviceInterfaceDetail");
			   Continue to the next device. */
			goto cont;
		}

		/* Make sure this device is of Setup Class "HIDClass" and has a
		   driver bound to it. */
		for (i = 0; ; i++) {
			char driver_name[256];

			/* Populate devinfo_data. This function will return failure
			   when there are no more interfaces left. */
			res = SetupDiEnumDeviceInfo(device_info_set, i, &devinfo_data);
			if (!res)
				goto cont;

			res = SetupDiGetDeviceRegistryPropertyA(device_info_set, &devinfo_data,
			               SPDRP_CLASS, NULL, (PBYTE)driver_name, sizeof(driver_name), NULL);
			if (!res)
				goto cont;

			if (strcmp(driver_name, "HIDClass") == 0) {
				/* See if there's a driver bound. */
				res = SetupDiGetDeviceRegistryPropertyA(device_info_set, &devinfo_data,
				           SPDRP_DRIVER, NULL, (PBYTE)driver_name, sizeof(driver_name), NULL);
				if (res)
					break;
			}
		}

		//wprintf(L"HandleName: %s\n", device_interface_detail_data->DevicePath);

		/* Open a handle to the device */
		write_handle = open_device(device_interface_detail_data->DevicePath, TRUE);

		/* Check validity of write_handle. */
		if (write_handle == INVALID_HANDLE_VALUE) {
			/* Unable to open the device. */
			//register_error(dev, "CreateFile");
			goto cont_close;
		}		


		/* Get the Vendor ID and Product ID for this device. */
		attrib.Size = sizeof(HIDD_ATTRIBUTES);
		HidD_GetAttributes(write_handle, &attrib);
		//wprintf(L"Product/Vendor: %x %x\n", attrib.ProductID, attrib.VendorID);

		/* Check the VID/PID to see if we should add this
		   device to the enumeration list. */
		if ((vendor_id == 0x0 || attrib.VendorID == vendor_id) &&
		    (product_id == 0x0 || attrib.ProductID == product_id)) {

			#define WSTR_LEN 512
			const char *str;
			struct hid_device_info *tmp;
			PHIDP_PREPARSED_DATA pp_data = NULL;
			HIDP_CAPS caps;
			BOOLEAN res;
			NTSTATUS nt_res;
			wchar_t wstr[WSTR_LEN]; /* TODO: Determine Size */
			size_t len;

			/* VID/PID match. Create the record. */
			tmp = (struct hid_device_info*) calloc(1, sizeof(struct hid_device_info));
			if (cur_dev) {
				cur_dev->next = tmp;
			}
			else {
				root = tmp;
			}
			cur_dev = tmp;

			/* Get the Usage Page and Usage for this device. */
			res = HidD_GetPreparsedData(write_handle, &pp_data);
			if (res) {
				nt_res = HidP_GetCaps(pp_data, &caps);
				if (nt_res == HIDP_STATUS_SUCCESS) {
					cur_dev->usage_page = caps.UsagePage;
					cur_dev->usage = caps.Usage;
				}

				HidD_FreePreparsedData(pp_data);
			}
			
			/* Fill out the record */
			cur_dev->next = NULL;
			str = device_interface_detail_data->DevicePath;
			if (str) {
				len = strlen(str);
				cur_dev->path = (char*) calloc(len+1, sizeof(char));
				strncpy(cur_dev->path, str, len+1);
				cur_dev->path[len] = '\0';
			}
			else
				cur_dev->path = NULL;

			/* Serial Number */
			res = HidD_GetSerialNumberString(write_handle, wstr, sizeof(wstr));
			wstr[WSTR_LEN-1] = 0x0000;
			if (res) {
				cur_dev->serial_number = _wcsdup(wstr);
			}

			/* Manufacturer String */
			res = HidD_GetManufacturerString(write_handle, wstr, sizeof(wstr));
			wstr[WSTR_LEN-1] = 0x0000;
			if (res) {
				cur_dev->manufacturer_string = _wcsdup(wstr);
			}

			/* Product String */
			res = HidD_GetProductString(write_handle, wstr, sizeof(wstr));
			wstr[WSTR_LEN-1] = 0x0000;
			if (res) {
				cur_dev->product_string = _wcsdup(wstr);
			}

			/* VID/PID */
			cur_dev->vendor_id = attrib.VendorID;
			cur_dev->product_id = attrib.ProductID;

			/* Release Number */
			cur_dev->release_number = attrib.VersionNumber;

			/* Interface Number. It can sometimes be parsed out of the path
			   on Windows if a device has multiple interfaces. See
			   http://msdn.microsoft.com/en-us/windows/hardware/gg487473 or
			   search for "Hardware IDs for HID Devices" at MSDN. If it's not
			   in the path, it's set to -1. */
			cur_dev->interface_number = -1;
			if (cur_dev->path) {
				char *interface_component = strstr(cur_dev->path, "&mi_");
				if (interface_component) {
					char *hex_str = interface_component + 4;
					char *endptr = NULL;
					cur_dev->interface_number = strtol(hex_str, &endptr, 16);
					if (endptr == hex_str) {
						/* The parsing failed. Set interface_number to -1. */
						cur_dev->interface_number = -1;
					}
				}
			}
		}

cont_close:
		CloseHandle(write_handle);
cont:
		/* We no longer need the detail data. It can be freed */
		free(device_interface_detail_data);

		device_index++;

	}

	/* Close the device information handle. */
	SetupDiDestroyDeviceInfoList(device_info_set);

	return root;

}

void  HID_API_EXPORT HID_API_CALL hid_free_enumeration(struct hid_device_info *devs)
{
	/* TODO: Merge this with the Linux version. This function is platform-independent. */
	struct hid_device_info *d = devs;
	while (d) {
		struct hid_device_info *next = d->next;
		free(d->path);
		free(d->serial_number);
		free(d->manufacturer_string);
		free(d->product_string);
		free(d);
		d = next;
	}
}


HID_API_EXPORT hid_device * HID_API_CALL hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
	/* TODO: Merge this functions with the Linux version. This function should be platform independent. */
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device *handle = NULL;
	
	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev) {
		if (cur_dev->vendor_id == vendor_id &&
		    cur_dev->product_id == product_id) {
			if (serial_number) {
				if (wcscmp(serial_number, cur_dev->serial_number) == 0) {
					path_to_open = cur_dev->path;
					break;
				}
			}
			else {
				path_to_open = cur_dev->path;
				break;
			}
		}
		cur_dev = cur_dev->next;
	}

	if (path_to_open) {
		/* Open the device */
		handle = hid_open_path(path_to_open);
	}

	hid_free_enumeration(devs);
	
	return handle;
}

HID_API_EXPORT hid_device * HID_API_CALL hid_open_path(const char *path)
{
	hid_device *dev;
	HIDP_CAPS caps;
	PHIDP_PREPARSED_DATA pp_data = NULL;
	BOOLEAN res;
	NTSTATUS nt_res;

	if (hid_init() < 0) {
		return NULL;
	}

	dev = new_hid_device();

	/* Open a handle to the device */
	dev->device_handle = open_device(path, FALSE);

	/* Check validity of write_handle. */
	if (dev->device_handle == INVALID_HANDLE_VALUE) {
		/* Unable to open the device. */
		register_error(dev, "CreateFile");
		goto err;
	}

	/* Set the Input Report buffer size to 64 reports. */
	res = HidD_SetNumInputBuffers(dev->device_handle, 64);
	if (!res) {
		register_error(dev, "HidD_SetNumInputBuffers");
		goto err;
	}

	/* Get the Input Report length for the device. */
	res = HidD_GetPreparsedData(dev->device_handle, &pp_data);
	if (!res) {
		register_error(dev, "HidD_GetPreparsedData");
		goto err;
	}
	nt_res = HidP_GetCaps(pp_data, &caps);
	if (nt_res != HIDP_STATUS_SUCCESS) {
		register_error(dev, "HidP_GetCaps");	
		goto err_pp_data;
	}
	dev->output_report_length = caps.OutputReportByteLength;
	dev->input_report_length = caps.InputReportByteLength;
	HidD_FreePreparsedData(pp_data);

	dev->read_buf = (char*) malloc(dev->input_report_length);

	return dev;

err_pp_data:
		HidD_FreePreparsedData(pp_data);
err:	
		free_hid_device(dev);
		return NULL;
}

int HID_API_EXPORT HID_API_CALL hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
	DWORD bytes_written;
	BOOL res;

	OVERLAPPED ol;
	unsigned char *buf;
	memset(&ol, 0, sizeof(ol));

	/* Make sure the right number of bytes are passed to WriteFile. Windows
	   expects the number of bytes which are in the _longest_ report (plus
	   one for the report number) bytes even if the data is a report
	   which is shorter than that. Windows gives us this value in
	   caps.OutputReportByteLength. If a user passes in fewer bytes than this,
	   create a temporary buffer which is the proper size. */
	if (length >= dev->output_report_length) {
		/* The user passed the right number of bytes. Use the buffer as-is. */
		buf = (unsigned char *) data;
	} else {
		/* Create a temporary buffer and copy the user's data
		   into it, padding the rest with zeros. */
		buf = (unsigned char *) malloc(dev->output_report_length);
		memcpy(buf, data, length);
		memset(buf + length, 0, dev->output_report_length - length);
		length = dev->output_report_length;
	}

	res = WriteFile(dev->device_handle, buf, (DWORD)length, NULL, &ol);
	int err = GetLastError();
	
	if (!res) {
		if (err != ERROR_IO_PENDING) {
			/* WriteFile() failed. Return error. */
			register_error(dev, "WriteFile");
			bytes_written = -1;
			goto end_of_function;
		}
	}

	/* Wait here until the write is done. This makes
	   hid_write() synchronous. */
	res = GetOverlappedResult(dev->device_handle, &ol, &bytes_written, TRUE/*wait*/);
	if (!res) {
		/* The Write operation failed. */
		register_error(dev, "WriteFile");
		bytes_written = -1;
		goto end_of_function;
	}

end_of_function:
	if (buf != data)
		free(buf);

	return bytes_written;
}


int HID_API_EXPORT HID_API_CALL hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds)
{
	DWORD bytes_read = 0;
	size_t copy_len = 0;
	BOOL res;

	/* Copy the handle for convenience. */
	HANDLE ev = dev->ol.hEvent;

	if (!dev->read_pending) {
		/* Start an Overlapped I/O read. */
		dev->read_pending = TRUE;
		memset(dev->read_buf, 0, dev->input_report_length);
		ResetEvent(ev);
		res = ReadFile(dev->device_handle, dev->read_buf, (DWORD)dev->input_report_length, &bytes_read, &dev->ol);
		
		if (!res) {
			if (GetLastError() != ERROR_IO_PENDING) {
				/* ReadFile() has failed.
				   Clean up and return error. */
				CancelIo(dev->device_handle);
				dev->read_pending = FALSE;
				goto end_of_function;
			}
		}
	}

	if (milliseconds >= 0) {
		/* See if there is any data yet. */
		res = WaitForSingleObject(ev, milliseconds);
		if (res != WAIT_OBJECT_0) {
			/* There was no data this time. Return zero bytes available,
			   but leave the Overlapped I/O running. */
			return 0;
		}
	}

	/* Either WaitForSingleObject() told us that ReadFile has completed, or
	   we are in non-blocking mode. Get the number of bytes read. The actual
	   data has been copied to the data[] array which was passed to ReadFile(). */
	res = GetOverlappedResult(dev->device_handle, &dev->ol, &bytes_read, TRUE/*wait*/);
	
	/* Set pending back to false, even if GetOverlappedResult() returned error. */
	dev->read_pending = FALSE;

	if (res && bytes_read > 0) {
		if (dev->read_buf[0] == 0x0) {
			/* If report numbers aren't being used, but Windows sticks a report
			   number (0x0) on the beginning of the report anyway. To make this
			   work like the other platforms, and to make it work more like the
			   HID spec, we'll skip over this byte. */
			bytes_read--;
			copy_len = length > bytes_read ? bytes_read : length;
			memcpy(data, dev->read_buf+1, copy_len);
		}
		else {
			/* Copy the whole buffer, report number and all. */
			copy_len = length > bytes_read ? bytes_read : length;
			memcpy(data, dev->read_buf, copy_len);
		}
	}
	
end_of_function:
	if (!res) {
		register_error(dev, "GetOverlappedResult");
		return -1;
	}
	
	return (int)copy_len;
}

int HID_API_EXPORT HID_API_CALL hid_read(hid_device *dev, unsigned char *data, size_t length)
{
	return hid_read_timeout(dev, data, length, (dev->blocking)? -1: 0);
}

int HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device *dev, int nonblock)
{
	dev->blocking = !nonblock;
	return 0; /* Success */
}

int HID_API_EXPORT HID_API_CALL hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
	BOOL res = HidD_SetFeature(dev->device_handle, (PVOID)data, (ULONG)length);
	if (!res) {
		register_error(dev, "HidD_SetFeature");
		return -1;
	}

	return (int)length;
}


int HID_API_EXPORT HID_API_CALL hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
	BOOL res;
#if 0
	res = HidD_GetFeature(dev->device_handle, data, length);
	if (!res) {
		register_error(dev, "HidD_GetFeature");
		return -1;
	}
	return 0; /* HidD_GetFeature() doesn't give us an actual length, unfortunately */
#else
	DWORD bytes_returned;

	OVERLAPPED ol;
	memset(&ol, 0, sizeof(ol));

	res = DeviceIoControl(dev->device_handle,
		IOCTL_HID_GET_FEATURE,
		data, (DWORD)length,
		data, (DWORD)length,
		&bytes_returned, &ol);

	if (!res) {
		if (GetLastError() != ERROR_IO_PENDING) {
			/* DeviceIoControl() failed. Return error. */
			register_error(dev, "Send Feature Report DeviceIoControl");
			return -1;
		}
	}

	/* Wait here until the write is done. This makes
	   hid_get_feature_report() synchronous. */
	res = GetOverlappedResult(dev->device_handle, &ol, &bytes_returned, TRUE/*wait*/);
	if (!res) {
		/* The operation failed. */
		register_error(dev, "Send Feature Report GetOverLappedResult");
		return -1;
	}

	/* bytes_returned does not include the first byte which contains the
	   report ID. The data buffer actually contains one more byte than
	   bytes_returned. */
	bytes_returned++;

	return bytes_returned;
#endif
}

void HID_API_EXPORT HID_API_CALL hid_close(hid_device *dev)
{
	if (!dev)
		return;
	CancelIo(dev->device_handle);
	free_hid_device(dev);
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_manufacturer_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetManufacturerString(dev->device_handle, string, (ULONG)(sizeof(wchar_t) * MIN(maxlen, MAX_STRING_WCHARS)));
	if (!res) {
		register_error(dev, "HidD_GetManufacturerString");
		return -1;
	}

	return 0;
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_product_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetProductString(dev->device_handle, string, (ULONG)(sizeof(wchar_t) * MIN(maxlen, MAX_STRING_WCHARS)));
	if (!res) {
		register_error(dev, "HidD_GetProductString");
		return -1;
	}

	return 0;
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_serial_number_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetSerialNumberString(dev->device_handle, string, (ULONG)(sizeof(wchar_t) * MIN(maxlen, MAX_STRING_WCHARS)));
	if (!res) {
		register_error(dev, "HidD_GetSerialNumberString");
		return -1;
	}

	return 0;
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_indexed_string(hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetIndexedString(dev->device_handle, string_index, string, (ULONG)(sizeof(wchar_t) * MIN(maxlen, MAX_STRING_WCHARS)));
	if (!res) {
		register_error(dev, "HidD_GetIndexedString");
		return -1;
	}

	return 0;
}


HID_API_EXPORT const wchar_t * HID_API_CALL  hid_error(hid_device *dev)
{
	return (wchar_t*)dev->last_error_str;
}


/*#define PICPGM*/
/*#define S11*/
#define P32
#ifdef S11 
  unsigned short VendorID = 0xa0a0;
	unsigned short ProductID = 0x0001;
#endif

#ifdef P32
  unsigned short VendorID = 0x04d8;
	unsigned short ProductID = 0x3f;
#endif


#ifdef PICPGM
  unsigned short VendorID = 0x04d8;
  unsigned short ProductID = 0x0033;
#endif


#if 0
int __cdecl main(int argc, char* argv[])
{
	int res;
	unsigned char buf[65];

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	/* Set up the command buffer. */
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0;
	buf[1] = 0x81;
	

	/* Open the device. */
	int handle = open(VendorID, ProductID, L"12345");
	if (handle < 0)
		printf("unable to open device\n");


	/* Toggle LED (cmd 0x80) */
	buf[1] = 0x80;
	res = write(handle, buf, 65);
	if (res < 0)
		printf("Unable to write()\n");

	/* Request state (cmd 0x81) */
	buf[1] = 0x81;
	write(handle, buf, 65);
	if (res < 0)
		printf("Unable to write() (2)\n");

	/* Read requested state */
	read(handle, buf, 65);
	if (res < 0)
		printf("Unable to read()\n");

	/* Print out the returned buffer. */
	for (int i = 0; i < 4; i++)
		printf("buf[%d]: %d\n", i, buf[i]);

	return 0;
}
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif


#elif defined( __MACOSX__ )

/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 2010-07-03

 Copyright 2010, All Rights Reserved.

 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU General Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        http://github.com/signal11/hidapi .
********************************************************/

/* See Apple Technical Note TN2187 for details on IOHidManager. */

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <wchar.h>
#include <locale.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <dlfcn.h>

#include "hidapi.h"

/* Barrier implementation because Mac OSX doesn't have pthread_barrier.
   It also doesn't have clock_gettime(). So much for POSIX and SUSv2.
   This implementation came from Brent Priddy and was posted on
   StackOverflow. It is used with his permission. */
typedef int pthread_barrierattr_t;
typedef struct pthread_barrier {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int trip_count;
} pthread_barrier_t;

static int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)
{
	if(count == 0) {
		errno = EINVAL;
		return -1;
	}

	if(pthread_mutex_init(&barrier->mutex, 0) < 0) {
		return -1;
	}
	if(pthread_cond_init(&barrier->cond, 0) < 0) {
		pthread_mutex_destroy(&barrier->mutex);
		return -1;
	}
	barrier->trip_count = count;
	barrier->count = 0;

	return 0;
}

static int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
	pthread_cond_destroy(&barrier->cond);
	pthread_mutex_destroy(&barrier->mutex);
	return 0;
}

static int pthread_barrier_wait(pthread_barrier_t *barrier)
{
	pthread_mutex_lock(&barrier->mutex);
	++(barrier->count);
	if(barrier->count >= barrier->trip_count)
	{
		barrier->count = 0;
		pthread_cond_broadcast(&barrier->cond);
		pthread_mutex_unlock(&barrier->mutex);
		return 1;
	}
	else
	{
		pthread_cond_wait(&barrier->cond, &(barrier->mutex));
		pthread_mutex_unlock(&barrier->mutex);
		return 0;
	}
}

static int return_data(hid_device *dev, unsigned char *data, size_t length);

/* Linked List of input reports received from the device. */
struct input_report {
	uint8_t *data;
	size_t len;
	struct input_report *next;
};

struct hid_device_ {
	IOHIDDeviceRef device_handle;
	int blocking;
	int uses_numbered_reports;
	int disconnected;
	CFStringRef run_loop_mode;
	CFRunLoopRef run_loop;
	CFRunLoopSourceRef source;
	uint8_t *input_report_buf;
	CFIndex max_input_report_len;
	struct input_report *input_reports;

	pthread_t thread;
	pthread_mutex_t mutex; /* Protects input_reports */
	pthread_cond_t condition;
	pthread_barrier_t barrier; /* Ensures correct startup sequence */
	pthread_barrier_t shutdown_barrier; /* Ensures correct shutdown sequence */
	int shutdown_thread;
};

static hid_device *new_hid_device(void)
{
	hid_device *dev = calloc(1, sizeof(hid_device));
	dev->device_handle = NULL;
	dev->blocking = 1;
	dev->uses_numbered_reports = 0;
	dev->disconnected = 0;
	dev->run_loop_mode = NULL;
	dev->run_loop = NULL;
	dev->source = NULL;
	dev->input_report_buf = NULL;
	dev->input_reports = NULL;
	dev->shutdown_thread = 0;

	/* Thread objects */
	pthread_mutex_init(&dev->mutex, NULL);
	pthread_cond_init(&dev->condition, NULL);
	pthread_barrier_init(&dev->barrier, NULL, 2);
	pthread_barrier_init(&dev->shutdown_barrier, NULL, 2);

	return dev;
}

static void free_hid_device(hid_device *dev)
{
	if (!dev)
		return;

	/* Delete any input reports still left over. */
	struct input_report *rpt = dev->input_reports;
	while (rpt) {
		struct input_report *next = rpt->next;
		free(rpt->data);
		free(rpt);
		rpt = next;
	}

	/* Free the string and the report buffer. The check for NULL
	   is necessary here as CFRelease() doesn't handle NULL like
	   free() and others do. */
	if (dev->run_loop_mode)
		CFRelease(dev->run_loop_mode);
	if (dev->source)
		CFRelease(dev->source);
	free(dev->input_report_buf);

	/* Clean up the thread objects */
	pthread_barrier_destroy(&dev->shutdown_barrier);
	pthread_barrier_destroy(&dev->barrier);
	pthread_cond_destroy(&dev->condition);
	pthread_mutex_destroy(&dev->mutex);

	/* Free the structure itself. */
	free(dev);
}

static	IOHIDManagerRef hid_mgr = 0x0;


#if 0
static void register_error(hid_device *device, const char *op)
{

}
#endif


static int32_t get_int_property(IOHIDDeviceRef device, CFStringRef key)
{
	CFTypeRef ref;
	int32_t value;

	ref = IOHIDDeviceGetProperty(device, key);
	if (ref) {
		if (CFGetTypeID(ref) == CFNumberGetTypeID()) {
			CFNumberGetValue((CFNumberRef) ref, kCFNumberSInt32Type, &value);
			return value;
		}
	}
	return 0;
}

static unsigned short get_vendor_id(IOHIDDeviceRef device)
{
	return get_int_property(device, CFSTR(kIOHIDVendorIDKey));
}

static unsigned short get_product_id(IOHIDDeviceRef device)
{
	return get_int_property(device, CFSTR(kIOHIDProductIDKey));
}

static int32_t get_max_report_length(IOHIDDeviceRef device)
{
	return get_int_property(device, CFSTR(kIOHIDMaxInputReportSizeKey));
}

static int get_string_property(IOHIDDeviceRef device, CFStringRef prop, wchar_t *buf, size_t len)
{
	CFStringRef str;

	if (!len)
		return 0;

	str = IOHIDDeviceGetProperty(device, prop);

	buf[0] = 0;

	if (str) {
		CFIndex str_len = CFStringGetLength(str);
		CFRange range;
		CFIndex used_buf_len;
		CFIndex chars_copied;

		len --;

		range.location = 0;
		range.length = ((size_t)str_len > len)? len: (size_t)str_len;
		chars_copied = CFStringGetBytes(str,
			range,
			kCFStringEncodingUTF32LE,
			(char)'?',
			FALSE,
			(UInt8*)buf,
			len * sizeof(wchar_t),
			&used_buf_len);

		if (chars_copied == len)
			buf[len] = 0; /* len is decremented above */
		else
			buf[chars_copied] = 0;

		return 0;
	}
	else
		return -1;

}

static int get_serial_number(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDSerialNumberKey), buf, len);
}

static int get_manufacturer_string(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDManufacturerKey), buf, len);
}

static int get_product_string(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDProductKey), buf, len);
}


/* Implementation of wcsdup() for Mac. */
static wchar_t *dup_wcs(const wchar_t *s)
{
	size_t len = wcslen(s);
	wchar_t *ret = malloc((len+1)*sizeof(wchar_t));
	wcscpy(ret, s);

	return ret;
}

/* hidapi_IOHIDDeviceGetService()
 *
 * Return the io_service_t corresponding to a given IOHIDDeviceRef, either by:
 * - on OS X 10.6 and above, calling IOHIDDeviceGetService()
 * - on OS X 10.5, extract it from the IOHIDDevice struct
 */
static io_service_t hidapi_IOHIDDeviceGetService(IOHIDDeviceRef device)
{
	static void *iokit_framework = NULL;
	static io_service_t (*dynamic_IOHIDDeviceGetService)(IOHIDDeviceRef device) = NULL;

	/* Use dlopen()/dlsym() to get a pointer to IOHIDDeviceGetService() if it exists.
	 * If any of these steps fail, dynamic_IOHIDDeviceGetService will be left NULL
	 * and the fallback method will be used.
	 */
	if (iokit_framework == NULL) {
		iokit_framework = dlopen("/System/Library/IOKit.framework/IOKit", RTLD_LAZY);

		if (iokit_framework != NULL)
			dynamic_IOHIDDeviceGetService = dlsym(iokit_framework, "IOHIDDeviceGetService");
	}

	if (dynamic_IOHIDDeviceGetService != NULL) {
		/* Running on OS X 10.6 and above: IOHIDDeviceGetService() exists */
		return dynamic_IOHIDDeviceGetService(device);
	}
	else
	{
		/* Running on OS X 10.5: IOHIDDeviceGetService() doesn't exist.
		 *
		 * Be naughty and pull the service out of the IOHIDDevice.
		 * IOHIDDevice is an opaque struct not exposed to applications, but its
		 * layout is stable through all available versions of OS X.
		 * Tested and working on OS X 10.5.8 i386, x86_64, and ppc.
		 */
		struct IOHIDDevice_internal {
			/* The first field of the IOHIDDevice struct is a
			 * CFRuntimeBase (which is a private CF struct).
			 *
			 * a, b, and c are the 3 fields that make up a CFRuntimeBase.
			 * See http://opensource.apple.com/source/CF/CF-476.18/CFRuntime.h
			 *
			 * The second field of the IOHIDDevice is the io_service_t we're looking for.
			 */
			uintptr_t a;
			uint8_t b[4];
#if __LP64__
			uint32_t c;
#endif
			io_service_t service;
		};
		struct IOHIDDevice_internal *tmp = (struct IOHIDDevice_internal *)device;

		return tmp->service;
	}
}

/* Initialize the IOHIDManager. Return 0 for success and -1 for failure. */
static int init_hid_manager(void)
{
	/* Initialize all the HID Manager Objects */
	hid_mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (hid_mgr) {
		IOHIDManagerSetDeviceMatching(hid_mgr, NULL);
		IOHIDManagerScheduleWithRunLoop(hid_mgr, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		return 0;
	}

	return -1;
}

/* Initialize the IOHIDManager if necessary. This is the public function, and
   it is safe to call this function repeatedly. Return 0 for success and -1
   for failure. */
int HID_API_EXPORT hid_init(void)
{
	if (!hid_mgr) {
		return init_hid_manager();
	}

	/* Already initialized. */
	return 0;
}

int HID_API_EXPORT hid_exit(void)
{
	if (hid_mgr) {
		/* Close the HID manager. */
		IOHIDManagerClose(hid_mgr, kIOHIDOptionsTypeNone);
		CFRelease(hid_mgr);
		hid_mgr = NULL;
	}

	return 0;
}

static void process_pending_events(void) {
	SInt32 res;
	do {
		res = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, FALSE);
	} while(res != kCFRunLoopRunFinished && res != kCFRunLoopRunTimedOut);
}

struct hid_device_info  HID_API_EXPORT *hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	struct hid_device_info *root = NULL; /* return object */
	struct hid_device_info *cur_dev = NULL;
	CFIndex num_devices;
	int i;

	/* Set up the HID Manager if it hasn't been done */
	if (hid_init() < 0)
		return NULL;

	/* give the IOHIDManager a chance to update itself */
	process_pending_events();

	/* Get a list of the Devices */
	IOHIDManagerSetDeviceMatching(hid_mgr, NULL);
	CFSetRef device_set = IOHIDManagerCopyDevices(hid_mgr);

	/* Convert the list into a C array so we can iterate easily. */
	num_devices = CFSetGetCount(device_set);
	IOHIDDeviceRef *device_array = calloc(num_devices, sizeof(IOHIDDeviceRef));
	CFSetGetValues(device_set, (const void **) device_array);

	/* Iterate over each device, making an entry for it. */
	for (i = 0; i < num_devices; i++) {
		unsigned short dev_vid;
		unsigned short dev_pid;
		#define BUF_LEN 256
		wchar_t buf[BUF_LEN];

		IOHIDDeviceRef dev = device_array[i];

        if (!dev) {
            continue;
        }
		dev_vid = get_vendor_id(dev);
		dev_pid = get_product_id(dev);

		/* Check the VID/PID against the arguments */
		if ((vendor_id == 0x0 || vendor_id == dev_vid) &&
		    (product_id == 0x0 || product_id == dev_pid)) {
			struct hid_device_info *tmp;
			io_object_t iokit_dev;
			kern_return_t res;
			io_string_t path;

			/* VID/PID match. Create the record. */
			tmp = malloc(sizeof(struct hid_device_info));
			if (cur_dev) {
				cur_dev->next = tmp;
			}
			else {
				root = tmp;
			}
			cur_dev = tmp;

			/* Get the Usage Page and Usage for this device. */
			cur_dev->usage_page = get_int_property(dev, CFSTR(kIOHIDPrimaryUsagePageKey));
			cur_dev->usage = get_int_property(dev, CFSTR(kIOHIDPrimaryUsageKey));

			/* Fill out the record */
			cur_dev->next = NULL;

			/* Fill in the path (IOService plane) */
			iokit_dev = hidapi_IOHIDDeviceGetService(dev);
			res = IORegistryEntryGetPath(iokit_dev, kIOServicePlane, path);
			if (res == KERN_SUCCESS)
				cur_dev->path = strdup(path);
			else
				cur_dev->path = strdup("");

			/* Serial Number */
			get_serial_number(dev, buf, BUF_LEN);
			cur_dev->serial_number = dup_wcs(buf);

			/* Manufacturer and Product strings */
			get_manufacturer_string(dev, buf, BUF_LEN);
			cur_dev->manufacturer_string = dup_wcs(buf);
			get_product_string(dev, buf, BUF_LEN);
			cur_dev->product_string = dup_wcs(buf);

			/* VID/PID */
			cur_dev->vendor_id = dev_vid;
			cur_dev->product_id = dev_pid;

			/* Release Number */
			cur_dev->release_number = get_int_property(dev, CFSTR(kIOHIDVersionNumberKey));

			/* Interface Number (Unsupported on Mac)*/
			cur_dev->interface_number = -1;
		}
	}

	free(device_array);
	CFRelease(device_set);

	return root;
}

void  HID_API_EXPORT hid_free_enumeration(struct hid_device_info *devs)
{
	/* This function is identical to the Linux version. Platform independent. */
	struct hid_device_info *d = devs;
	while (d) {
		struct hid_device_info *next = d->next;
		free(d->path);
		free(d->serial_number);
		free(d->manufacturer_string);
		free(d->product_string);
		free(d);
		d = next;
	}
}

hid_device * HID_API_EXPORT hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
	/* This function is identical to the Linux version. Platform independent. */
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device * handle = NULL;

	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev) {
		if (cur_dev->vendor_id == vendor_id &&
		    cur_dev->product_id == product_id) {
			if (serial_number) {
				if (wcscmp(serial_number, cur_dev->serial_number) == 0) {
					path_to_open = cur_dev->path;
					break;
				}
			}
			else {
				path_to_open = cur_dev->path;
				break;
			}
		}
		cur_dev = cur_dev->next;
	}

	if (path_to_open) {
		/* Open the device */
		handle = hid_open_path(path_to_open);
	}

	hid_free_enumeration(devs);

	return handle;
}

static void hid_device_removal_callback(void *context, IOReturn result,
                                        void *sender)
{
	/* Stop the Run Loop for this device. */
	hid_device *d = context;

	d->disconnected = 1;
	CFRunLoopStop(d->run_loop);
}

/* The Run Loop calls this function for each input report received.
   This function puts the data into a linked list to be picked up by
   hid_read(). */
static void hid_report_callback(void *context, IOReturn result, void *sender,
                         IOHIDReportType report_type, uint32_t report_id,
                         uint8_t *report, CFIndex report_length)
{
	struct input_report *rpt;
	hid_device *dev = context;

	/* Make a new Input Report object */
	rpt = calloc(1, sizeof(struct input_report));
	rpt->data = calloc(1, report_length);
	memcpy(rpt->data, report, report_length);
	rpt->len = report_length;
	rpt->next = NULL;

	/* Lock this section */
	pthread_mutex_lock(&dev->mutex);

	/* Attach the new report object to the end of the list. */
	if (dev->input_reports == NULL) {
		/* The list is empty. Put it at the root. */
		dev->input_reports = rpt;
	}
	else {
		/* Find the end of the list and attach. */
		struct input_report *cur = dev->input_reports;
		int num_queued = 0;
		while (cur->next != NULL) {
			cur = cur->next;
			num_queued++;
		}
		cur->next = rpt;

		/* Pop one off if we've reached 30 in the queue. This
		   way we don't grow forever if the user never reads
		   anything from the device. */
		if (num_queued > 30) {
			return_data(dev, NULL, 0);
		}
	}

	/* Signal a waiting thread that there is data. */
	pthread_cond_signal(&dev->condition);

	/* Unlock */
	pthread_mutex_unlock(&dev->mutex);

}

/* This gets called when the read_thread's run loop gets signaled by
   hid_close(), and serves to stop the read_thread's run loop. */
static void perform_signal_callback(void *context)
{
	hid_device *dev = context;
	CFRunLoopStop(dev->run_loop); /*TODO: CFRunLoopGetCurrent()*/
}

static void *read_thread(void *param)
{
	hid_device *dev = param;
	SInt32 code;

	/* Move the device's run loop to this thread. */
	IOHIDDeviceScheduleWithRunLoop(dev->device_handle, CFRunLoopGetCurrent(), dev->run_loop_mode);

	/* Create the RunLoopSource which is used to signal the
	   event loop to stop when hid_close() is called. */
	CFRunLoopSourceContext ctx;
	memset(&ctx, 0, sizeof(ctx));
	ctx.version = 0;
	ctx.info = dev;
	ctx.perform = &perform_signal_callback;
	dev->source = CFRunLoopSourceCreate(kCFAllocatorDefault, 0/*order*/, &ctx);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), dev->source, dev->run_loop_mode);

	/* Store off the Run Loop so it can be stopped from hid_close()
	   and on device disconnection. */
	dev->run_loop = CFRunLoopGetCurrent();

	/* Notify the main thread that the read thread is up and running. */
	pthread_barrier_wait(&dev->barrier);

	/* Run the Event Loop. CFRunLoopRunInMode() will dispatch HID input
	   reports into the hid_report_callback(). */
	while (!dev->shutdown_thread && !dev->disconnected) {
		code = CFRunLoopRunInMode(dev->run_loop_mode, 1000/*sec*/, FALSE);
		/* Return if the device has been disconnected */
		if (code == kCFRunLoopRunFinished) {
			dev->disconnected = 1;
			break;
		}


		/* Break if The Run Loop returns Finished or Stopped. */
		if (code != kCFRunLoopRunTimedOut &&
		    code != kCFRunLoopRunHandledSource) {
			/* There was some kind of error. Setting
			   shutdown seems to make sense, but
			   there may be something else more appropriate */
			dev->shutdown_thread = 1;
			break;
		}
	}

	/* Now that the read thread is stopping, Wake any threads which are
	   waiting on data (in hid_read_timeout()). Do this under a mutex to
	   make sure that a thread which is about to go to sleep waiting on
	   the condition actually will go to sleep before the condition is
	   signaled. */
	pthread_mutex_lock(&dev->mutex);
	pthread_cond_broadcast(&dev->condition);
	pthread_mutex_unlock(&dev->mutex);

	/* Wait here until hid_close() is called and makes it past
	   the call to CFRunLoopWakeUp(). This thread still needs to
	   be valid when that function is called on the other thread. */
	pthread_barrier_wait(&dev->shutdown_barrier);

	return NULL;
}

/* hid_open_path()
 *
 * path must be a valid path to an IOHIDDevice in the IOService plane
 * Example: "IOService:/AppleACPIPlatformExpert/PCI0@0/AppleACPIPCI/EHC1@1D,7/AppleUSBEHCI/PLAYSTATION(R)3 Controller@fd120000/IOUSBInterface@0/IOUSBHIDDriver"
 */
hid_device * HID_API_EXPORT hid_open_path(const char *path)
{
	hid_device *dev = NULL;
	io_registry_entry_t entry = MACH_PORT_NULL;

	dev = new_hid_device();

	/* Set up the HID Manager if it hasn't been done */
	if (hid_init() < 0)
		return NULL;

	/* Get the IORegistry entry for the given path */
	entry = IORegistryEntryFromPath(kIOMasterPortDefault, path);
	if (entry == MACH_PORT_NULL) {
		/* Path wasn't valid (maybe device was removed?) */
		goto return_error;
	}

	/* Create an IOHIDDevice for the entry */
	dev->device_handle = IOHIDDeviceCreate(kCFAllocatorDefault, entry);
	if (dev->device_handle == NULL) {
		/* Error creating the HID device */
		goto return_error;
	}

	/* Open the IOHIDDevice */
	IOReturn ret = IOHIDDeviceOpen(dev->device_handle, kIOHIDOptionsTypeSeizeDevice);
	if (ret == kIOReturnSuccess) {
		char str[32];

		/* Create the buffers for receiving data */
		dev->max_input_report_len = (CFIndex) get_max_report_length(dev->device_handle);
		dev->input_report_buf = calloc(dev->max_input_report_len, sizeof(uint8_t));

		/* Create the Run Loop Mode for this device.
		   printing the reference seems to work. */
		sprintf(str, "HIDAPI_%p", dev->device_handle);
		dev->run_loop_mode =
			CFStringCreateWithCString(NULL, str, kCFStringEncodingASCII);

		/* Attach the device to a Run Loop */
		IOHIDDeviceRegisterInputReportCallback(
			dev->device_handle, dev->input_report_buf, dev->max_input_report_len,
			&hid_report_callback, dev);
		IOHIDDeviceRegisterRemovalCallback(dev->device_handle, hid_device_removal_callback, dev);

		/* Start the read thread */
		pthread_create(&dev->thread, NULL, read_thread, dev);

		/* Wait here for the read thread to be initialized. */
		pthread_barrier_wait(&dev->barrier);

		IOObjectRelease(entry);
		return dev;
	}
	else {
		goto return_error;
	}

return_error:
	if (dev->device_handle != NULL)
		CFRelease(dev->device_handle);

	if (entry != MACH_PORT_NULL)
		IOObjectRelease(entry);

	free_hid_device(dev);
	return NULL;
}

static int set_report(hid_device *dev, IOHIDReportType type, const unsigned char *data, size_t length)
{
	const unsigned char *data_to_send;
	size_t length_to_send;
	IOReturn res;

	/* Return if the device has been disconnected. */
	if (dev->disconnected)
		return -1;

	if (data[0] == 0x0) {
		/* Not using numbered Reports.
		   Don't send the report number. */
		data_to_send = data+1;
		length_to_send = length-1;
	}
	else {
		/* Using numbered Reports.
		   Send the Report Number */
		data_to_send = data;
		length_to_send = length;
	}

	if (!dev->disconnected) {
		res = IOHIDDeviceSetReport(dev->device_handle,
					   type,
					   data[0], /* Report ID*/
					   data_to_send, length_to_send);

		if (res == kIOReturnSuccess) {
			return length;
		}
		else
			return -1;
	}

	return -1;
}

int HID_API_EXPORT hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
	return set_report(dev, kIOHIDReportTypeOutput, data, length);
}

/* Helper function, so that this isn't duplicated in hid_read(). */
static int return_data(hid_device *dev, unsigned char *data, size_t length)
{
	/* Copy the data out of the linked list item (rpt) into the
	   return buffer (data), and delete the liked list item. */
	struct input_report *rpt = dev->input_reports;
	size_t len = (length < rpt->len)? length: rpt->len;
	memcpy(data, rpt->data, len);
	dev->input_reports = rpt->next;
	free(rpt->data);
	free(rpt);
	return len;
}

static int cond_wait(const hid_device *dev, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	while (!dev->input_reports) {
		int res = pthread_cond_wait(cond, mutex);
		if (res != 0)
			return res;

		/* A res of 0 means we may have been signaled or it may
		   be a spurious wakeup. Check to see that there's acutally
		   data in the queue before returning, and if not, go back
		   to sleep. See the pthread_cond_timedwait() man page for
		   details. */

		if (dev->shutdown_thread || dev->disconnected)
			return -1;
	}

	return 0;
}

static int cond_timedwait(const hid_device *dev, pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
	while (!dev->input_reports) {
		int res = pthread_cond_timedwait(cond, mutex, abstime);
		if (res != 0)
			return res;

		/* A res of 0 means we may have been signaled or it may
		   be a spurious wakeup. Check to see that there's acutally
		   data in the queue before returning, and if not, go back
		   to sleep. See the pthread_cond_timedwait() man page for
		   details. */

		if (dev->shutdown_thread || dev->disconnected)
			return -1;
	}

	return 0;

}

int HID_API_EXPORT hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds)
{
	int bytes_read = -1;

	/* Lock the access to the report list. */
	pthread_mutex_lock(&dev->mutex);

	/* There's an input report queued up. Return it. */
	if (dev->input_reports) {
		/* Return the first one */
		bytes_read = return_data(dev, data, length);
		goto ret;
	}

	/* Return if the device has been disconnected. */
	if (dev->disconnected) {
		bytes_read = -1;
		goto ret;
	}

	if (dev->shutdown_thread) {
		/* This means the device has been closed (or there
		   has been an error. An error code of -1 should
		   be returned. */
		bytes_read = -1;
		goto ret;
	}

	/* There is no data. Go to sleep and wait for data. */

	if (milliseconds == -1) {
		/* Blocking */
		int res;
		res = cond_wait(dev, &dev->condition, &dev->mutex);
		if (res == 0)
			bytes_read = return_data(dev, data, length);
		else {
			/* There was an error, or a device disconnection. */
			bytes_read = -1;
		}
	}
	else if (milliseconds > 0) {
		/* Non-blocking, but called with timeout. */
		int res;
		struct timespec ts;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		TIMEVAL_TO_TIMESPEC(&tv, &ts);
		ts.tv_sec += milliseconds / 1000;
		ts.tv_nsec += (milliseconds % 1000) * 1000000;
		if (ts.tv_nsec >= 1000000000L) {
			ts.tv_sec++;
			ts.tv_nsec -= 1000000000L;
		}

		res = cond_timedwait(dev, &dev->condition, &dev->mutex, &ts);
		if (res == 0)
			bytes_read = return_data(dev, data, length);
		else if (res == ETIMEDOUT)
			bytes_read = 0;
		else
			bytes_read = -1;
	}
	else {
		/* Purely non-blocking */
		bytes_read = 0;
	}

ret:
	/* Unlock */
	pthread_mutex_unlock(&dev->mutex);
	return bytes_read;
}

int HID_API_EXPORT hid_read(hid_device *dev, unsigned char *data, size_t length)
{
	return hid_read_timeout(dev, data, length, (dev->blocking)? -1: 0);
}

int HID_API_EXPORT hid_set_nonblocking(hid_device *dev, int nonblock)
{
	/* All Nonblocking operation is handled by the library. */
	dev->blocking = !nonblock;

	return 0;
}

int HID_API_EXPORT hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
	return set_report(dev, kIOHIDReportTypeFeature, data, length);
}

int HID_API_EXPORT hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
	CFIndex len = length;
	IOReturn res;

	/* Return if the device has been unplugged. */
	if (dev->disconnected)
		return -1;

	res = IOHIDDeviceGetReport(dev->device_handle,
	                           kIOHIDReportTypeFeature,
	                           data[0], /* Report ID */
	                           data, &len);
	if (res == kIOReturnSuccess)
		return len;
	else
		return -1;
}


void HID_API_EXPORT hid_close(hid_device *dev)
{
	if (!dev)
		return;

	/* Disconnect the report callback before close. */
	if (!dev->disconnected) {
		IOHIDDeviceRegisterInputReportCallback(
			dev->device_handle, dev->input_report_buf, dev->max_input_report_len,
			NULL, dev);
		IOHIDDeviceRegisterRemovalCallback(dev->device_handle, NULL, dev);
		IOHIDDeviceUnscheduleFromRunLoop(dev->device_handle, dev->run_loop, dev->run_loop_mode);
		IOHIDDeviceScheduleWithRunLoop(dev->device_handle, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
	}

	/* Cause read_thread() to stop. */
	dev->shutdown_thread = 1;

	/* Wake up the run thread's event loop so that the thread can exit. */
	CFRunLoopSourceSignal(dev->source);
	CFRunLoopWakeUp(dev->run_loop);

	/* Notify the read thread that it can shut down now. */
	pthread_barrier_wait(&dev->shutdown_barrier);

	/* Wait for read_thread() to end. */
	pthread_join(dev->thread, NULL);

	/* Close the OS handle to the device, but only if it's not
	   been unplugged. If it's been unplugged, then calling
	   IOHIDDeviceClose() will crash. */
	if (!dev->disconnected) {
		IOHIDDeviceClose(dev->device_handle, kIOHIDOptionsTypeSeizeDevice);
	}

	/* Clear out the queue of received reports. */
	pthread_mutex_lock(&dev->mutex);
	while (dev->input_reports) {
		return_data(dev, NULL, 0);
	}
	pthread_mutex_unlock(&dev->mutex);
	CFRelease(dev->device_handle);

	free_hid_device(dev);
}

int HID_API_EXPORT_CALL hid_get_manufacturer_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_manufacturer_string(dev->device_handle, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_product_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_product_string(dev->device_handle, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_serial_number_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_serial_number(dev->device_handle, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_indexed_string(hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	/* TODO: */

	return 0;
}


HID_API_EXPORT const wchar_t * HID_API_CALL  hid_error(hid_device *dev)
{
	/* TODO: */

	return NULL;
}







#if 0
static int32_t get_location_id(IOHIDDeviceRef device)
{
	return get_int_property(device, CFSTR(kIOHIDLocationIDKey));
}

static int32_t get_usage(IOHIDDeviceRef device)
{
	int32_t res;
	res = get_int_property(device, CFSTR(kIOHIDDeviceUsageKey));
	if (!res)
		res = get_int_property(device, CFSTR(kIOHIDPrimaryUsageKey));
	return res;
}

static int32_t get_usage_page(IOHIDDeviceRef device)
{
	int32_t res;
	res = get_int_property(device, CFSTR(kIOHIDDeviceUsagePageKey));
	if (!res)
		res = get_int_property(device, CFSTR(kIOHIDPrimaryUsagePageKey));
	return res;
}

static int get_transport(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDTransportKey), buf, len);
}


int main(void)
{
	IOHIDManagerRef mgr;
	int i;

	mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	IOHIDManagerSetDeviceMatching(mgr, NULL);
	IOHIDManagerOpen(mgr, kIOHIDOptionsTypeNone);

	CFSetRef device_set = IOHIDManagerCopyDevices(mgr);

	CFIndex num_devices = CFSetGetCount(device_set);
	IOHIDDeviceRef *device_array = calloc(num_devices, sizeof(IOHIDDeviceRef));
	CFSetGetValues(device_set, (const void **) device_array);

	for (i = 0; i < num_devices; i++) {
		IOHIDDeviceRef dev = device_array[i];
		printf("Device: %p\n", dev);
		printf("  %04hx %04hx\n", get_vendor_id(dev), get_product_id(dev));

		wchar_t serial[256], buf[256];
		char cbuf[256];
		get_serial_number(dev, serial, 256);


		printf("  Serial: %ls\n", serial);
		printf("  Loc: %ld\n", get_location_id(dev));
		get_transport(dev, buf, 256);
		printf("  Trans: %ls\n", buf);
		make_path(dev, cbuf, 256);
		printf("  Path: %s\n", cbuf);

	}

	return 0;
}
#endif


#elif defined(__linux__)

/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 8/22/2009
 Linux Version - 6/2/2009

 Copyright 2009, All Rights Reserved.

 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU General Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        http://github.com/signal11/hidapi .
********************************************************/

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <errno.h>

/* Unix */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <poll.h>

/* Linux */
#include <linux/hidraw.h>
#include <linux/version.h>
#include <linux/input.h>
#include <libudev.h>

#include "hidapi.h"

int g_hidapiSuppress;

/* Definitions from linux/hidraw.h. Since these are new, some distros
   may not have header files which contain them. */
#ifndef HIDIOCSFEATURE
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#endif
#ifndef HIDIOCGFEATURE
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif


/* USB HID device property names */
const char *device_string_names[] = {
	"manufacturer",
	"product",
	"serial",
};

/* Symbolic names for the properties above */
enum device_string_id {
	DEVICE_STRING_MANUFACTURER,
	DEVICE_STRING_PRODUCT,
	DEVICE_STRING_SERIAL,

	DEVICE_STRING_COUNT,
};

struct hid_device_ {
	int device_handle;
	int blocking;
	int uses_numbered_reports;
};


static __u32 kernel_version = 0;

static __u32 detect_kernel_version(void)
{
	struct utsname name;
	int major, minor, release;
	int ret;

	uname(&name);
	ret = sscanf(name.release, "%d.%d.%d", &major, &minor, &release);
	if (ret == 3) {
		return KERNEL_VERSION(major, minor, release);
	}

	ret = sscanf(name.release, "%d.%d", &major, &minor);
	if (ret == 2) {
		return KERNEL_VERSION(major, minor, 0);
	}

	printf("Couldn't determine kernel version from version string \"%s\"\n", name.release);
	return 0;
}

static hid_device *new_hid_device(void)
{
	hid_device *dev = calloc(1, sizeof(hid_device));
	dev->device_handle = -1;
	dev->blocking = 1;
	dev->uses_numbered_reports = 0;

	return dev;
}


/* The caller must free the returned string with free(). */
static wchar_t *utf8_to_wchar_t(const char *utf8)
{
	wchar_t *ret = NULL;

	if (utf8) {
		size_t wlen = mbstowcs(NULL, utf8, 0);
		if ((size_t) -1 == wlen) {
			return wcsdup(L"");
		}
		ret = calloc(wlen+1, sizeof(wchar_t));
		mbstowcs(ret, utf8, wlen+1);
		ret[wlen] = 0x0000;
	}

	return ret;
}

/* Get an attribute value from a udev_device and return it as a whar_t
   string. The returned string must be freed with free() when done.*/
static wchar_t *copy_udev_string(struct udev_device *dev, const char *udev_name)
{
	return utf8_to_wchar_t(udev_device_get_sysattr_value(dev, udev_name));
}

/* uses_numbered_reports() returns 1 if report_descriptor describes a device
   which contains numbered reports. */
static int uses_numbered_reports(__u8 *report_descriptor, __u32 size) {
	unsigned int i = 0;
	int size_code;
	int data_len, key_size;

	while (i < size) {
		int key = report_descriptor[i];

		/* Check for the Report ID key */
		if (key == 0x85/*Report ID*/) {
			/* This device has a Report ID, which means it uses
			   numbered reports. */
			return 1;
		}

		//printf("key: %02hhx\n", key);

		if ((key & 0xf0) == 0xf0) {
			/* This is a Long Item. The next byte contains the
			   length of the data section (value) for this key.
			   See the HID specification, version 1.11, section
			   6.2.2.3, titled "Long Items." */
			if (i+1 < size)
				data_len = report_descriptor[i+1];
			else
				data_len = 0; /* malformed report */
			key_size = 3;
		}
		else {
			/* This is a Short Item. The bottom two bits of the
			   key contain the size code for the data section
			   (value) for this key.  Refer to the HID
			   specification, version 1.11, section 6.2.2.2,
			   titled "Short Items." */
			size_code = key & 0x3;
			switch (size_code) {
			case 0:
			case 1:
			case 2:
				data_len = size_code;
				break;
			case 3:
				data_len = 4;
				break;
			default:
				/* Can't ever happen since size_code is & 0x3 */
				data_len = 0;
				break;
			};
			key_size = 1;
		}

		/* Skip over this key and it's associated data */
		i += data_len + key_size;
	}

	/* Didn't find a Report ID key. Device doesn't use numbered reports. */
	return 0;
}

/*
 * The caller is responsible for free()ing the (newly-allocated) character
 * strings pointed to by serial_number_utf8 and product_name_utf8 after use.
 */
static int
parse_uevent_info(const char *uevent, int *bus_type,
	unsigned short *vendor_id, unsigned short *product_id,
	char **serial_number_utf8, char **product_name_utf8)
{
	char *tmp = strdup(uevent);
	char *saveptr = NULL;
	char *line;
	char *key;
	char *value;

	int found_id = 0;
	int found_serial = 0;
	int found_name = 0;

	line = strtok_r(tmp, "\n", &saveptr);
	while (line != NULL) {
		/* line: "KEY=value" */
		key = line;
		value = strchr(line, '=');
		if (!value) {
			goto next_line;
		}
		*value = '\0';
		value++;

		if (strcmp(key, "HID_ID") == 0) {
			/**
			 *        type vendor   product
			 * HID_ID=0003:000005AC:00008242
			 **/
			int ret = sscanf(value, "%x:%hx:%hx", bus_type, vendor_id, product_id);
			if (ret == 3) {
				found_id = 1;
			}
		} else if (strcmp(key, "HID_NAME") == 0) {
			/* The caller has to free the product name */
			*product_name_utf8 = strdup(value);
			found_name = 1;
		} else if (strcmp(key, "HID_UNIQ") == 0) {
			/* The caller has to free the serial number */
			*serial_number_utf8 = strdup(value);
			found_serial = 1;
		}

next_line:
		line = strtok_r(NULL, "\n", &saveptr);
	}

	free(tmp);
	return (found_id && found_name && found_serial);
}


static int get_device_string(hid_device *dev, enum device_string_id key, wchar_t *string, size_t maxlen)
{
	struct udev *udev;
	struct udev_device *udev_dev, *parent, *hid_dev;
	struct stat s;
	int ret = -1;
        char *serial_number_utf8 = NULL;
        char *product_name_utf8 = NULL;

	/* Create the udev object */
	udev = udev_new();
	if (!udev) {
		printf("Can't create udev\n");
		return -1;
	}

	/* Get the dev_t (major/minor numbers) from the file handle. */
	ret = fstat(dev->device_handle, &s);
	if (-1 == ret)
		return ret;
	/* Open a udev device from the dev_t. 'c' means character device. */
	udev_dev = udev_device_new_from_devnum(udev, 'c', s.st_rdev);
	if (udev_dev) {
		hid_dev = udev_device_get_parent_with_subsystem_devtype(
			udev_dev,
			"hid",
			NULL);
		if (hid_dev) {
			unsigned short dev_vid;
			unsigned short dev_pid;
			int bus_type;
			size_t retm;

			ret = parse_uevent_info(
			           udev_device_get_sysattr_value(hid_dev, "uevent"),
			           &bus_type,
			           &dev_vid,
			           &dev_pid,
			           &serial_number_utf8,
			           &product_name_utf8);

			if (bus_type == BUS_BLUETOOTH) {
				switch (key) {
					case DEVICE_STRING_MANUFACTURER:
						wcsncpy(string, L"", maxlen);
						ret = 0;
						break;
					case DEVICE_STRING_PRODUCT:
						retm = mbstowcs(string, product_name_utf8, maxlen);
						ret = (retm == (size_t)-1)? -1: 0;
						break;
					case DEVICE_STRING_SERIAL:
						retm = mbstowcs(string, serial_number_utf8, maxlen);
						ret = (retm == (size_t)-1)? -1: 0;
						break;
					case DEVICE_STRING_COUNT:
					default:
						ret = -1;
						break;
				}
			}
			else {
				/* This is a USB device. Find its parent USB Device node. */
				parent = udev_device_get_parent_with_subsystem_devtype(
					   udev_dev,
					   "usb",
					   "usb_device");
				if (parent) {
					const char *str;
					const char *key_str = NULL;

					if (key >= 0 && key < DEVICE_STRING_COUNT) {
						key_str = device_string_names[key];
					} else {
						ret = -1;
						goto end;
					}

					str = udev_device_get_sysattr_value(parent, key_str);
					if (str) {
						/* Convert the string from UTF-8 to wchar_t */
						retm = mbstowcs(string, str, maxlen);
						ret = (retm == (size_t)-1)? -1: 0;
						goto end;
					}
				}
			}
		}
	}

end:
        free(serial_number_utf8);
        free(product_name_utf8);

	udev_device_unref(udev_dev);
	/* parent and hid_dev don't need to be (and can't be) unref'd.
	   I'm not sure why, but they'll throw double-free() errors. */
	udev_unref(udev);

	return ret;
}

int HID_API_EXPORT hid_init(void)
{
	const char *locale;

	/* Set the locale if it's not set. */
	locale = setlocale(LC_CTYPE, NULL);
	if (!locale)
		setlocale(LC_CTYPE, "");

	kernel_version = detect_kernel_version();

	return 0;
}

int HID_API_EXPORT hid_exit(void)
{
	/* Nothing to do for this in the Linux/hidraw implementation. */
	return 0;
}


struct hid_device_info  HID_API_EXPORT *hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;

	struct hid_device_info *root = NULL; /* return object */
	struct hid_device_info *cur_dev = NULL;
	struct hid_device_info *prev_dev = NULL; /* previous device */

	hid_init();

	/* Create the udev object */
	udev = udev_new();
	if (!udev) {
		printf("Can't create udev\n");
		return NULL;
	}

	/* Create a list of the devices in the 'hidraw' subsystem. */
	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "hidraw");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
	/* For each item, see if it matches the vid/pid, and if so
	   create a udev_device record for it */
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *sysfs_path;
		const char *dev_path;
		const char *str;
		struct udev_device *raw_dev; /* The device's hidraw udev node. */
		struct udev_device *hid_dev; /* The device's HID udev node. */
		struct udev_device *usb_dev; /* The device's USB udev node. */
		struct udev_device *intf_dev; /* The device's interface (in the USB sense). */
		unsigned short dev_vid;
		unsigned short dev_pid;
		char *serial_number_utf8 = NULL;
		char *product_name_utf8 = NULL;
		int bus_type;
		int result;

		/* Get the filename of the /sys entry for the device
		   and create a udev_device object (dev) representing it */
		sysfs_path = udev_list_entry_get_name(dev_list_entry);
		raw_dev = udev_device_new_from_syspath(udev, sysfs_path);
		dev_path = udev_device_get_devnode(raw_dev);

		hid_dev = udev_device_get_parent_with_subsystem_devtype(
			raw_dev,
			"hid",
			NULL);

		if (!hid_dev) {
			/* Unable to find parent hid device. */
			goto next;
		}

		result = parse_uevent_info(
			udev_device_get_sysattr_value(hid_dev, "uevent"),
			&bus_type,
			&dev_vid,
			&dev_pid,
			&serial_number_utf8,
			&product_name_utf8);

		if (!result) {
			/* parse_uevent_info() failed for at least one field. */
			goto next;
		}

		if (bus_type != BUS_USB && bus_type != BUS_BLUETOOTH) {
			/* We only know how to handle USB and BT devices. */
			goto next;
		}

		/* Check the VID/PID against the arguments */
		if ((vendor_id == 0x0 || vendor_id == dev_vid) &&
		    (product_id == 0x0 || product_id == dev_pid)) {
			struct hid_device_info *tmp;

			/* VID/PID match. Create the record. */
			tmp = malloc(sizeof(struct hid_device_info));
			if (cur_dev) {
				cur_dev->next = tmp;
			}
			else {
				root = tmp;
			}
			prev_dev = cur_dev;
			cur_dev = tmp;

			/* Fill out the record */
			cur_dev->next = NULL;
			cur_dev->path = dev_path? strdup(dev_path): NULL;

			/* VID/PID */
			cur_dev->vendor_id = dev_vid;
			cur_dev->product_id = dev_pid;

			/* Serial Number */
			cur_dev->serial_number = utf8_to_wchar_t(serial_number_utf8);

			/* Release Number */
			cur_dev->release_number = 0x0;

			/* Interface Number */
			cur_dev->interface_number = -1;

			switch (bus_type) {
				case BUS_USB:
					/* The device pointed to by raw_dev contains information about
					   the hidraw device. In order to get information about the
					   USB device, get the parent device with the
					   subsystem/devtype pair of "usb"/"usb_device". This will
					   be several levels up the tree, but the function will find
					   it. */
					usb_dev = udev_device_get_parent_with_subsystem_devtype(
							raw_dev,
							"usb",
							"usb_device");

					if (!usb_dev) {
						/* Free this device */
						free(cur_dev->serial_number);
						free(cur_dev->path);
						free(cur_dev);

						/* Take it off the device list. */
						if (prev_dev) {
							prev_dev->next = NULL;
							cur_dev = prev_dev;
						}
						else {
							cur_dev = root = NULL;
						}

						goto next;
					}

					/* Manufacturer and Product strings */
					cur_dev->manufacturer_string = copy_udev_string(usb_dev, device_string_names[DEVICE_STRING_MANUFACTURER]);
					cur_dev->product_string = copy_udev_string(usb_dev, device_string_names[DEVICE_STRING_PRODUCT]);

					/* Release Number */
					str = udev_device_get_sysattr_value(usb_dev, "bcdDevice");
					cur_dev->release_number = (str)? strtol(str, NULL, 16): 0x0;

					/* Get a handle to the interface's udev node. */
					intf_dev = udev_device_get_parent_with_subsystem_devtype(
							raw_dev,
							"usb",
							"usb_interface");
					if (intf_dev) {
						str = udev_device_get_sysattr_value(intf_dev, "bInterfaceNumber");
						cur_dev->interface_number = (str)? strtol(str, NULL, 16): -1;
					}

					break;

				case BUS_BLUETOOTH:
					/* Manufacturer and Product strings */
					cur_dev->manufacturer_string = wcsdup(L"");
					cur_dev->product_string = utf8_to_wchar_t(product_name_utf8);

					break;

				default:
					/* Unknown device type - this should never happen, as we
					 * check for USB and Bluetooth devices above */
					break;
			}
		}

	next:
		free(serial_number_utf8);
		free(product_name_utf8);
		udev_device_unref(raw_dev);
		/* hid_dev, usb_dev and intf_dev don't need to be (and can't be)
		   unref()d.  It will cause a double-free() error.  I'm not
		   sure why.  */
	}
	/* Free the enumerator and udev objects. */
	udev_enumerate_unref(enumerate);
	udev_unref(udev);

	return root;
}

void  HID_API_EXPORT hid_free_enumeration(struct hid_device_info *devs)
{
	struct hid_device_info *d = devs;
	while (d) {
		struct hid_device_info *next = d->next;
		free(d->path);
		free(d->serial_number);
		free(d->manufacturer_string);
		free(d->product_string);
		free(d);
		d = next;
	}
}

hid_device * hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device *handle = NULL;

	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev) {
		if (cur_dev->vendor_id == vendor_id &&
		    cur_dev->product_id == product_id) {
			if (serial_number) {
				if (wcscmp(serial_number, cur_dev->serial_number) == 0) {
					path_to_open = cur_dev->path;
					break;
				}
			}
			else {
				path_to_open = cur_dev->path;
				break;
			}
		}
		cur_dev = cur_dev->next;
	}

	if (path_to_open) {
		/* Open the device */
		handle = hid_open_path(path_to_open);
	}

	hid_free_enumeration(devs);

	return handle;
}

hid_device * HID_API_EXPORT hid_open_path(const char *path)
{
	hid_device *dev = NULL;

	hid_init();

	dev = new_hid_device();

	/* OPEN HERE */
	dev->device_handle = open(path, O_RDWR);

	/* If we have a good handle, return it. */
	if (dev->device_handle > 0) {

		/* Get the report descriptor */
		int res, desc_size = 0;
		struct hidraw_report_descriptor rpt_desc;

		memset(&rpt_desc, 0x0, sizeof(rpt_desc));

		/* Get Report Descriptor Size */
		res = ioctl(dev->device_handle, HIDIOCGRDESCSIZE, &desc_size);
		if (res < 0 && !g_hidapiSuppress)
			perror("HIDIOCGRDESCSIZE");


		/* Get Report Descriptor */
		rpt_desc.size = desc_size;
		res = ioctl(dev->device_handle, HIDIOCGRDESC, &rpt_desc);
		if (res < 0 && !g_hidapiSuppress) {
			perror("HIDIOCGRDESC");
		} else {
			/* Determine if this device uses numbered reports. */
			dev->uses_numbered_reports =
				uses_numbered_reports(rpt_desc.value,
				                      rpt_desc.size);
		}

		return dev;
	}
	else {
		/* Unable to open any devices. */
		free(dev);
		return NULL;
	}
}


int HID_API_EXPORT hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
	int bytes_written;

	bytes_written = write(dev->device_handle, data, length);

	return bytes_written;
}


int HID_API_EXPORT hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds)
{
	int bytes_read;

	if (milliseconds >= 0) {
		/* Milliseconds is either 0 (non-blocking) or > 0 (contains
		   a valid timeout). In both cases we want to call poll()
		   and wait for data to arrive.  Don't rely on non-blocking
		   operation (O_NONBLOCK) since some kernels don't seem to
		   properly report device disconnection through read() when
		   in non-blocking mode.  */
		int ret;
		struct pollfd fds;

		fds.fd = dev->device_handle;
		fds.events = POLLIN;
		fds.revents = 0;
		ret = poll(&fds, 1, milliseconds);
		if (ret == -1 || ret == 0) {
			/* Error or timeout */
			return ret;
		}
		else {
			/* Check for errors on the file descriptor. This will
			   indicate a device disconnection. */
			if (fds.revents & (POLLERR | POLLHUP | POLLNVAL))
				return -1;
		}
	}

	bytes_read = read(dev->device_handle, data, length);
	if (bytes_read < 0 && (errno == EAGAIN || errno == EINPROGRESS))
		bytes_read = 0;

	if (bytes_read >= 0 &&
	    kernel_version != 0 &&
	    kernel_version < KERNEL_VERSION(2,6,34) &&
	    dev->uses_numbered_reports) {
		/* Work around a kernel bug. Chop off the first byte. */
		memmove(data, data+1, bytes_read);
		bytes_read--;
	}

	return bytes_read;
}

int HID_API_EXPORT hid_read(hid_device *dev, unsigned char *data, size_t length)
{
	return hid_read_timeout(dev, data, length, (dev->blocking)? -1: 0);
}

int HID_API_EXPORT hid_set_nonblocking(hid_device *dev, int nonblock)
{
	/* Do all non-blocking in userspace using poll(), since it looks
	   like there's a bug in the kernel in some versions where
	   read() will not return -1 on disconnection of the USB device */

	dev->blocking = !nonblock;
	return 0; /* Success */
}


int HID_API_EXPORT hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
	int res;

	res = ioctl(dev->device_handle, HIDIOCSFEATURE(length), data);
	if (res < 0 && !g_hidapiSuppress)
		perror("ioctl (SFEATURE)");

	return res;
}

int HID_API_EXPORT hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
	int res;

	res = ioctl(dev->device_handle, HIDIOCGFEATURE(length), data);
	if (res < 0 && !g_hidapiSuppress)
		perror("ioctl (GFEATURE)");


	return res;
}


void HID_API_EXPORT hid_close(hid_device *dev)
{
	if (!dev)
		return;
	close(dev->device_handle);
	free(dev);
}


int HID_API_EXPORT_CALL hid_get_manufacturer_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_device_string(dev, DEVICE_STRING_MANUFACTURER, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_product_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_device_string(dev, DEVICE_STRING_PRODUCT, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_serial_number_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_device_string(dev, DEVICE_STRING_SERIAL, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_indexed_string(hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	return -1;
}


HID_API_EXPORT const wchar_t * HID_API_CALL  hid_error(hid_device *dev)
{
	return NULL;
}

#else

/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 8/22/2009
 Linux Version - 6/2/2010
 Libusb Version - 8/13/2010
 FreeBSD Version - 11/1/2011

 Copyright 2009, All Rights Reserved.

 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU General Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        http://github.com/signal11/hidapi .
********************************************************/

#define _GNU_SOURCE /* needed for wcsdup() before glibc 2.10 */

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>

/* Unix */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <pthread.h>
#include <wchar.h>

/* GNU / LibUSB */
#include <libusb.h>
#ifndef __ANDROID__
#include <iconv.h>
#endif

#include "hidapi.h"

#ifdef __ANDROID__

/* Barrier implementation because Android/Bionic don't have pthread_barrier.
   This implementation came from Brent Priddy and was posted on
   StackOverflow. It is used with his permission. */
typedef int pthread_barrierattr_t;
typedef struct pthread_barrier {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int trip_count;
} pthread_barrier_t;

static int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)
{
	if(count == 0) {
		errno = EINVAL;
		return -1;
	}

	if(pthread_mutex_init(&barrier->mutex, 0) < 0) {
		return -1;
	}
	if(pthread_cond_init(&barrier->cond, 0) < 0) {
		pthread_mutex_destroy(&barrier->mutex);
		return -1;
	}
	barrier->trip_count = count;
	barrier->count = 0;

	return 0;
}

static int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
	pthread_cond_destroy(&barrier->cond);
	pthread_mutex_destroy(&barrier->mutex);
	return 0;
}

static int pthread_barrier_wait(pthread_barrier_t *barrier)
{
	pthread_mutex_lock(&barrier->mutex);
	++(barrier->count);
	if(barrier->count >= barrier->trip_count)
	{
		barrier->count = 0;
		pthread_cond_broadcast(&barrier->cond);
		pthread_mutex_unlock(&barrier->mutex);
		return 1;
	}
	else
	{
		pthread_cond_wait(&barrier->cond, &(barrier->mutex));
		pthread_mutex_unlock(&barrier->mutex);
		return 0;
	}
}

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG_PRINTF
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG(...) do {} while (0)
#endif

#ifndef __FreeBSD__
#define DETACH_KERNEL_DRIVER
#endif

/* Uncomment to enable the retrieval of Usage and Usage Page in
hid_enumerate(). Warning, on platforms different from FreeBSD
this is very invasive as it requires the detach
and re-attach of the kernel driver. See comments inside hid_enumerate().
libusb HIDAPI programs are encouraged to use the interface number
instead to differentiate between interfaces on a composite HID device. */
/*#define INVASIVE_GET_USAGE*/

/* Linked List of input reports received from the device. */
struct input_report {
	uint8_t *data;
	size_t len;
	struct input_report *next;
};


struct hid_device_ {
	/* Handle to the actual device. */
	libusb_device_handle *device_handle;

	/* Endpoint information */
	int input_endpoint;
	int output_endpoint;
	int input_ep_max_packet_size;

	/* The interface number of the HID */
	int interface;

	/* Indexes of Strings */
	int manufacturer_index;
	int product_index;
	int serial_index;

	/* Whether blocking reads are used */
	int blocking; /* boolean */

	/* Read thread objects */
	pthread_t thread;
	pthread_mutex_t mutex; /* Protects input_reports */
	pthread_cond_t condition;
	pthread_barrier_t barrier; /* Ensures correct startup sequence */
	int shutdown_thread;
	int cancelled;
	struct libusb_transfer *transfer;

	/* List of received input reports. */
	struct input_report *input_reports;
};

static libusb_context *usb_context = NULL;

uint16_t get_usb_code_for_current_locale(void);
static int return_data(hid_device *dev, unsigned char *data, size_t length);

static hid_device *new_hid_device(void)
{
	hid_device *dev = calloc(1, sizeof(hid_device));
	dev->blocking = 1;

	pthread_mutex_init(&dev->mutex, NULL);
	pthread_cond_init(&dev->condition, NULL);
	pthread_barrier_init(&dev->barrier, NULL, 2);

	return dev;
}

static void free_hid_device(hid_device *dev)
{
	/* Clean up the thread objects */
	pthread_barrier_destroy(&dev->barrier);
	pthread_cond_destroy(&dev->condition);
	pthread_mutex_destroy(&dev->mutex);

	/* Free the device itself */
	free(dev);
}

#if 0
/*TODO: Implement this funciton on hidapi/libusb.. */
static void register_error(hid_device *device, const char *op)
{

}
#endif

#ifdef INVASIVE_GET_USAGE
/* Get bytes from a HID Report Descriptor.
   Only call with a num_bytes of 0, 1, 2, or 4. */
static uint32_t get_bytes(uint8_t *rpt, size_t len, size_t num_bytes, size_t cur)
{
	/* Return if there aren't enough bytes. */
	if (cur + num_bytes >= len)
		return 0;

	if (num_bytes == 0)
		return 0;
	else if (num_bytes == 1) {
		return rpt[cur+1];
	}
	else if (num_bytes == 2) {
		return (rpt[cur+2] * 256 + rpt[cur+1]);
	}
	else if (num_bytes == 4) {
		return (rpt[cur+4] * 0x01000000 +
		        rpt[cur+3] * 0x00010000 +
		        rpt[cur+2] * 0x00000100 +
		        rpt[cur+1] * 0x00000001);
	}
	else
		return 0;
}

/* Retrieves the device's Usage Page and Usage from the report
   descriptor. The algorithm is simple, as it just returns the first
   Usage and Usage Page that it finds in the descriptor.
   The return value is 0 on success and -1 on failure. */
static int get_usage(uint8_t *report_descriptor, size_t size,
                     unsigned short *usage_page, unsigned short *usage)
{
	unsigned int i = 0;
	int size_code;
	int data_len, key_size;
	int usage_found = 0, usage_page_found = 0;

	while (i < size) {
		int key = report_descriptor[i];
		int key_cmd = key & 0xfc;

		//printf("key: %02hhx\n", key);

		if ((key & 0xf0) == 0xf0) {
			/* This is a Long Item. The next byte contains the
			   length of the data section (value) for this key.
			   See the HID specification, version 1.11, section
			   6.2.2.3, titled "Long Items." */
			if (i+1 < size)
				data_len = report_descriptor[i+1];
			else
				data_len = 0; /* malformed report */
			key_size = 3;
		}
		else {
			/* This is a Short Item. The bottom two bits of the
			   key contain the size code for the data section
			   (value) for this key.  Refer to the HID
			   specification, version 1.11, section 6.2.2.2,
			   titled "Short Items." */
			size_code = key & 0x3;
			switch (size_code) {
			case 0:
			case 1:
			case 2:
				data_len = size_code;
				break;
			case 3:
				data_len = 4;
				break;
			default:
				/* Can't ever happen since size_code is & 0x3 */
				data_len = 0;
				break;
			};
			key_size = 1;
		}

		if (key_cmd == 0x4) {
			*usage_page  = get_bytes(report_descriptor, size, data_len, i);
			usage_page_found = 1;
			//printf("Usage Page: %x\n", (uint32_t)*usage_page);
		}
		if (key_cmd == 0x8) {
			*usage = get_bytes(report_descriptor, size, data_len, i);
			usage_found = 1;
			//printf("Usage: %x\n", (uint32_t)*usage);
		}

		if (usage_page_found && usage_found)
			return 0; /* success */

		/* Skip over this key and it's associated data */
		i += data_len + key_size;
	}

	return -1; /* failure */
}
#endif /* INVASIVE_GET_USAGE */

#if defined(__FreeBSD__) && __FreeBSD__ < 10
/* The libusb version included in FreeBSD < 10 doesn't have this function. In
   mainline libusb, it's inlined in libusb.h. This function will bear a striking
   resemblance to that one, because there's about one way to code it.

   Note that the data parameter is Unicode in UTF-16LE encoding.
   Return value is the number of bytes in data, or LIBUSB_ERROR_*.
 */
static inline int libusb_get_string_descriptor(libusb_device_handle *dev,
	uint8_t descriptor_index, uint16_t lang_id,
	unsigned char *data, int length)
{
	return libusb_control_transfer(dev,
		LIBUSB_ENDPOINT_IN | 0x0, /* Endpoint 0 IN */
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		(LIBUSB_DT_STRING << 8) | descriptor_index,
		lang_id, data, (uint16_t) length, 1000);
}

#endif


/* Get the first language the device says it reports. This comes from
   USB string #0. */
static uint16_t get_first_language(libusb_device_handle *dev)
{
	uint16_t buf[32];
	int len;

	/* Get the string from libusb. */
	len = libusb_get_string_descriptor(dev,
			0x0, /* String ID */
			0x0, /* Language */
			(unsigned char*)buf,
			sizeof(buf));
	if (len < 4)
		return 0x0;

	return buf[1]; /* First two bytes are len and descriptor type. */
}

static int is_language_supported(libusb_device_handle *dev, uint16_t lang)
{
	uint16_t buf[32];
	int len;
	int i;

	/* Get the string from libusb. */
	len = libusb_get_string_descriptor(dev,
			0x0, /* String ID */
			0x0, /* Language */
			(unsigned char*)buf,
			sizeof(buf));
	if (len < 4)
		return 0x0;


	len /= 2; /* language IDs are two-bytes each. */
	/* Start at index 1 because there are two bytes of protocol data. */
	for (i = 1; i < len; i++) {
		if (buf[i] == lang)
			return 1;
	}

	return 0;
}


/* This function returns a newly allocated wide string containing the USB
   device string numbered by the index. The returned string must be freed
   by using free(). */
static wchar_t *get_usb_string(libusb_device_handle *dev, uint8_t idx)
{
	char buf[512];
	int len;
	wchar_t *str = NULL;

#ifndef __ANDROID__ /* we don't use iconv on Android */
	wchar_t wbuf[256];
	/* iconv variables */
	iconv_t ic;
	size_t inbytes;
	size_t outbytes;
	size_t res;
#ifdef __FreeBSD__
	const char *inptr;
#else
	char *inptr;
#endif
	char *outptr;
#endif

	/* Determine which language to use. */
	uint16_t lang;
	lang = get_usb_code_for_current_locale();
	if (!is_language_supported(dev, lang))
		lang = get_first_language(dev);

	/* Get the string from libusb. */
	len = libusb_get_string_descriptor(dev,
			idx,
			lang,
			(unsigned char*)buf,
			sizeof(buf));
	if (len < 0)
		return NULL;

#ifdef __ANDROID__

	/* Bionic does not have iconv support nor wcsdup() function, so it
	   has to be done manually.  The following code will only work for
	   code points that can be represented as a single UTF-16 character,
	   and will incorrectly convert any code points which require more
	   than one UTF-16 character.

	   Skip over the first character (2-bytes).  */
	len -= 2;
	str = malloc((len / 2 + 1) * sizeof(wchar_t));
	int i;
	for (i = 0; i < len / 2; i++) {
		str[i] = buf[i * 2 + 2] | (buf[i * 2 + 3] << 8);
	}
	str[len / 2] = 0x00000000;

#else

	/* buf does not need to be explicitly NULL-terminated because
	   it is only passed into iconv() which does not need it. */

	/* Initialize iconv. */
	ic = iconv_open("WCHAR_T", "UTF-16LE");
	if (ic == (iconv_t)-1) {
		LOG("iconv_open() failed\n");
		return NULL;
	}

	/* Convert to native wchar_t (UTF-32 on glibc/BSD systems).
	   Skip the first character (2-bytes). */
	inptr = buf+2;
	inbytes = len-2;
	outptr = (char*) wbuf;
	outbytes = sizeof(wbuf);
	res = iconv(ic, &inptr, &inbytes, &outptr, &outbytes);
	if (res == (size_t)-1) {
		LOG("iconv() failed\n");
		goto err;
	}

	/* Write the terminating NULL. */
	wbuf[sizeof(wbuf)/sizeof(wbuf[0])-1] = 0x00000000;
	if (outbytes >= sizeof(wbuf[0]))
		*((wchar_t*)outptr) = 0x00000000;

	/* Allocate and copy the string. */
	str = wcsdup(wbuf);

err:
	iconv_close(ic);

#endif

	return str;
}

static char *make_path(libusb_device *dev, int interface_number)
{
	char str[64];
	snprintf(str, sizeof(str), "%04x:%04x:%02x",
		libusb_get_bus_number(dev),
		libusb_get_device_address(dev),
		interface_number);
	str[sizeof(str)-1] = '\0';

	return strdup(str);
}


int HID_API_EXPORT hid_init(void)
{
	if (!usb_context) {
		const char *locale;

		/* Init Libusb */
		if (libusb_init(&usb_context))
			return -1;

		/* Set the locale if it's not set. */
		locale = setlocale(LC_CTYPE, NULL);
		if (!locale)
			setlocale(LC_CTYPE, "");
	}

	return 0;
}

int HID_API_EXPORT hid_exit(void)
{
	if (usb_context) {
		libusb_exit(usb_context);
		usb_context = NULL;
	}

	return 0;
}

struct hid_device_info  HID_API_EXPORT *hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	libusb_device **devs;
	libusb_device *dev;
	libusb_device_handle *handle;
	ssize_t num_devs;
	int i = 0;

	struct hid_device_info *root = NULL; /* return object */
	struct hid_device_info *cur_dev = NULL;

	if(hid_init() < 0)
		return NULL;

	num_devs = libusb_get_device_list(usb_context, &devs);
	if (num_devs < 0)
		return NULL;
	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		struct libusb_config_descriptor *conf_desc = NULL;
		int j, k;
		int interface_num = 0;

		int res = libusb_get_device_descriptor(dev, &desc);
		unsigned short dev_vid = desc.idVendor;
		unsigned short dev_pid = desc.idProduct;

		res = libusb_get_active_config_descriptor(dev, &conf_desc);
		if (res < 0)
			libusb_get_config_descriptor(dev, 0, &conf_desc);
		if (conf_desc) {
			for (j = 0; j < conf_desc->bNumInterfaces; j++) {
				const struct libusb_interface *intf = &conf_desc->interface[j];
				for (k = 0; k < intf->num_altsetting; k++) {
					const struct libusb_interface_descriptor *intf_desc;
					intf_desc = &intf->altsetting[k];
					if (intf_desc->bInterfaceClass == LIBUSB_CLASS_HID) {
						interface_num = intf_desc->bInterfaceNumber;

						/* Check the VID/PID against the arguments */
						if ((vendor_id == 0x0 || vendor_id == dev_vid) &&
						    (product_id == 0x0 || product_id == dev_pid)) {
							struct hid_device_info *tmp;

							/* VID/PID match. Create the record. */
							tmp = calloc(1, sizeof(struct hid_device_info));
							if (cur_dev) {
								cur_dev->next = tmp;
							}
							else {
								root = tmp;
							}
							cur_dev = tmp;

							/* Fill out the record */
							cur_dev->next = NULL;
							cur_dev->path = make_path(dev, interface_num);

							res = libusb_open(dev, &handle);

							if (res >= 0) {
								/* Serial Number */
								if (desc.iSerialNumber > 0)
									cur_dev->serial_number =
										get_usb_string(handle, desc.iSerialNumber);

								/* Manufacturer and Product strings */
								if (desc.iManufacturer > 0)
									cur_dev->manufacturer_string =
										get_usb_string(handle, desc.iManufacturer);
								if (desc.iProduct > 0)
									cur_dev->product_string =
										get_usb_string(handle, desc.iProduct);

#ifdef INVASIVE_GET_USAGE
{
							/*
							This section is removed because it is too
							invasive on the system. Getting a Usage Page
							and Usage requires parsing the HID Report
							descriptor. Getting a HID Report descriptor
							involves claiming the interface. Claiming the
							interface involves detaching the kernel driver.
							Detaching the kernel driver is hard on the system
							because it will unclaim interfaces (if another
							app has them claimed) and the re-attachment of
							the driver will sometimes change /dev entry names.
							It is for these reasons that this section is
							#if 0. For composite devices, use the interface
							field in the hid_device_info struct to distinguish
							between interfaces. */
								unsigned char data[256];
#ifdef DETACH_KERNEL_DRIVER
								int detached = 0;
								/* Usage Page and Usage */
								res = libusb_kernel_driver_active(handle, interface_num);
								if (res == 1) {
									res = libusb_detach_kernel_driver(handle, interface_num);
									if (res < 0)
										LOG("Couldn't detach kernel driver, even though a kernel driver was attached.");
									else
										detached = 1;
								}
#endif
								res = libusb_claim_interface(handle, interface_num);
								if (res >= 0) {
									/* Get the HID Report Descriptor. */
									res = libusb_control_transfer(handle, LIBUSB_ENDPOINT_IN|LIBUSB_RECIPIENT_INTERFACE, LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_REPORT << 8)|interface_num, 0, data, sizeof(data), 5000);
									if (res >= 0) {
										unsigned short page=0, usage=0;
										/* Parse the usage and usage page
										   out of the report descriptor. */
										get_usage(data, res,  &page, &usage);
										cur_dev->usage_page = page;
										cur_dev->usage = usage;
									}
									else
										LOG("libusb_control_transfer() for getting the HID report failed with %d\n", res);

									/* Release the interface */
									res = libusb_release_interface(handle, interface_num);
									if (res < 0)
										LOG("Can't release the interface.\n");
								}
								else
									LOG("Can't claim interface %d\n", res);
#ifdef DETACH_KERNEL_DRIVER
								/* Re-attach kernel driver if necessary. */
								if (detached) {
									res = libusb_attach_kernel_driver(handle, interface_num);
									if (res < 0)
										LOG("Couldn't re-attach kernel driver.\n");
								}
#endif
}
#endif /* INVASIVE_GET_USAGE */

								libusb_close(handle);
							}
							/* VID/PID */
							cur_dev->vendor_id = dev_vid;
							cur_dev->product_id = dev_pid;

							/* Release Number */
							cur_dev->release_number = desc.bcdDevice;

							/* Interface Number */
							cur_dev->interface_number = interface_num;
						}
					}
				} /* altsettings */
			} /* interfaces */
			libusb_free_config_descriptor(conf_desc);
		}
	}

	libusb_free_device_list(devs, 1);

	return root;
}

void  HID_API_EXPORT hid_free_enumeration(struct hid_device_info *devs)
{
	struct hid_device_info *d = devs;
	while (d) {
		struct hid_device_info *next = d->next;
		free(d->path);
		free(d->serial_number);
		free(d->manufacturer_string);
		free(d->product_string);
		free(d);
		d = next;
	}
}

hid_device * hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device *handle = NULL;

	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev) {
		if (cur_dev->vendor_id == vendor_id &&
		    cur_dev->product_id == product_id) {
			if (serial_number) {
				if (cur_dev->serial_number &&
				    wcscmp(serial_number, cur_dev->serial_number) == 0) {
					path_to_open = cur_dev->path;
					break;
				}
			}
			else {
				path_to_open = cur_dev->path;
				break;
			}
		}
		cur_dev = cur_dev->next;
	}

	if (path_to_open) {
		/* Open the device */
		handle = hid_open_path(path_to_open);
	}

	hid_free_enumeration(devs);

	return handle;
}

static void read_callback(struct libusb_transfer *transfer)
{
	hid_device *dev = transfer->user_data;
	int res;

	if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {

		struct input_report *rpt = malloc(sizeof(*rpt));
		rpt->data = malloc(transfer->actual_length);
		memcpy(rpt->data, transfer->buffer, transfer->actual_length);
		rpt->len = transfer->actual_length;
		rpt->next = NULL;

		pthread_mutex_lock(&dev->mutex);

		/* Attach the new report object to the end of the list. */
		if (dev->input_reports == NULL) {
			/* The list is empty. Put it at the root. */
			dev->input_reports = rpt;
			pthread_cond_signal(&dev->condition);
		}
		else {
			/* Find the end of the list and attach. */
			struct input_report *cur = dev->input_reports;
			int num_queued = 0;
			while (cur->next != NULL) {
				cur = cur->next;
				num_queued++;
			}
			cur->next = rpt;

			/* Pop one off if we've reached 30 in the queue. This
			   way we don't grow forever if the user never reads
			   anything from the device. */
			if (num_queued > 30) {
				return_data(dev, NULL, 0);
			}
		}
		pthread_mutex_unlock(&dev->mutex);
	}
	else if (transfer->status == LIBUSB_TRANSFER_CANCELLED) {
		dev->shutdown_thread = 1;
		dev->cancelled = 1;
		return;
	}
	else if (transfer->status == LIBUSB_TRANSFER_NO_DEVICE) {
		dev->shutdown_thread = 1;
		dev->cancelled = 1;
		return;
	}
	else if (transfer->status == LIBUSB_TRANSFER_TIMED_OUT) {
		//LOG("Timeout (normal)\n");
	}
	else {
		LOG("Unknown transfer code: %d\n", transfer->status);
	}

	/* Re-submit the transfer object. */
	res = libusb_submit_transfer(transfer);
	if (res != 0) {
		LOG("Unable to submit URB. libusb error code: %d\n", res);
		dev->shutdown_thread = 1;
		dev->cancelled = 1;
	}
}


static void *read_thread(void *param)
{
	hid_device *dev = param;
	unsigned char *buf;
	const size_t length = dev->input_ep_max_packet_size;

	/* Set up the transfer object. */
	buf = malloc(length);
	dev->transfer = libusb_alloc_transfer(0);
	libusb_fill_interrupt_transfer(dev->transfer,
		dev->device_handle,
		dev->input_endpoint,
		buf,
		length,
		read_callback,
		dev,
		5000/*timeout*/);

	/* Make the first submission. Further submissions are made
	   from inside read_callback() */
	libusb_submit_transfer(dev->transfer);

	/* Notify the main thread that the read thread is up and running. */
	pthread_barrier_wait(&dev->barrier);

	/* Handle all the events. */
	while (!dev->shutdown_thread) {
		int res;
		res = libusb_handle_events(usb_context);
		if (res < 0) {
			/* There was an error. */
			LOG("read_thread(): libusb reports error # %d\n", res);

			/* Break out of this loop only on fatal error.*/
			if (res != LIBUSB_ERROR_BUSY &&
			    res != LIBUSB_ERROR_TIMEOUT &&
			    res != LIBUSB_ERROR_OVERFLOW &&
			    res != LIBUSB_ERROR_INTERRUPTED) {
				break;
			}
		}
	}

	/* Cancel any transfer that may be pending. This call will fail
	   if no transfers are pending, but that's OK. */
	libusb_cancel_transfer(dev->transfer);

	while (!dev->cancelled)
		libusb_handle_events_completed(usb_context, &dev->cancelled);

	/* Now that the read thread is stopping, Wake any threads which are
	   waiting on data (in hid_read_timeout()). Do this under a mutex to
	   make sure that a thread which is about to go to sleep waiting on
	   the condition actually will go to sleep before the condition is
	   signaled. */
	pthread_mutex_lock(&dev->mutex);
	pthread_cond_broadcast(&dev->condition);
	pthread_mutex_unlock(&dev->mutex);

	/* The dev->transfer->buffer and dev->transfer objects are cleaned up
	   in hid_close(). They are not cleaned up here because this thread
	   could end either due to a disconnect or due to a user
	   call to hid_close(). In both cases the objects can be safely
	   cleaned up after the call to pthread_join() (in hid_close()), but
	   since hid_close() calls libusb_cancel_transfer(), on these objects,
	   they can not be cleaned up here. */

	return NULL;
}


hid_device * HID_API_EXPORT hid_open_path(const char *path)
{
	hid_device *dev = NULL;

	libusb_device **devs;
	libusb_device *usb_dev;
	int res;
	int d = 0;
	int good_open = 0;

	if(hid_init() < 0)
		return NULL;

	dev = new_hid_device();

	libusb_get_device_list(usb_context, &devs);
	while ((usb_dev = devs[d++]) != NULL) {
		struct libusb_device_descriptor desc;
		struct libusb_config_descriptor *conf_desc = NULL;
		int i,j,k;
		libusb_get_device_descriptor(usb_dev, &desc);

		if (libusb_get_active_config_descriptor(usb_dev, &conf_desc) < 0)
			continue;
		for (j = 0; j < conf_desc->bNumInterfaces; j++) {
			const struct libusb_interface *intf = &conf_desc->interface[j];
			for (k = 0; k < intf->num_altsetting; k++) {
				const struct libusb_interface_descriptor *intf_desc;
				intf_desc = &intf->altsetting[k];
				if (intf_desc->bInterfaceClass == LIBUSB_CLASS_HID) {
					char *dev_path = make_path(usb_dev, intf_desc->bInterfaceNumber);
					if (!strcmp(dev_path, path)) {
						/* Matched Paths. Open this device */

						/* OPEN HERE */
						res = libusb_open(usb_dev, &dev->device_handle);
						if (res < 0) {
							LOG("can't open device\n");
							free(dev_path);
							break;
						}
						good_open = 1;
#ifdef DETACH_KERNEL_DRIVER
						/* Detach the kernel driver, but only if the
						   device is managed by the kernel */
						if (libusb_kernel_driver_active(dev->device_handle, intf_desc->bInterfaceNumber) == 1) {
							res = libusb_detach_kernel_driver(dev->device_handle, intf_desc->bInterfaceNumber);
							if (res < 0) {
								libusb_close(dev->device_handle);
								LOG("Unable to detach Kernel Driver\n");
								free(dev_path);
								good_open = 0;
								break;
							}
						}
#endif
						res = libusb_claim_interface(dev->device_handle, intf_desc->bInterfaceNumber);
						if (res < 0) {
							LOG("can't claim interface %d: %d\n", intf_desc->bInterfaceNumber, res);
							free(dev_path);
							libusb_close(dev->device_handle);
							good_open = 0;
							break;
						}

						/* Store off the string descriptor indexes */
						dev->manufacturer_index = desc.iManufacturer;
						dev->product_index      = desc.iProduct;
						dev->serial_index       = desc.iSerialNumber;

						/* Store off the interface number */
						dev->interface = intf_desc->bInterfaceNumber;

						/* Find the INPUT and OUTPUT endpoints. An
						   OUTPUT endpoint is not required. */
						for (i = 0; i < intf_desc->bNumEndpoints; i++) {
							const struct libusb_endpoint_descriptor *ep
								= &intf_desc->endpoint[i];

							/* Determine the type and direction of this
							   endpoint. */
							int is_interrupt =
								(ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK)
							      == LIBUSB_TRANSFER_TYPE_INTERRUPT;
							int is_output =
								(ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
							      == LIBUSB_ENDPOINT_OUT;
							int is_input =
								(ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
							      == LIBUSB_ENDPOINT_IN;

							/* Decide whether to use it for input or output. */
							if (dev->input_endpoint == 0 &&
							    is_interrupt && is_input) {
								/* Use this endpoint for INPUT */
								dev->input_endpoint = ep->bEndpointAddress;
								dev->input_ep_max_packet_size = ep->wMaxPacketSize;
							}
							if (dev->output_endpoint == 0 &&
							    is_interrupt && is_output) {
								/* Use this endpoint for OUTPUT */
								dev->output_endpoint = ep->bEndpointAddress;
							}
						}

						pthread_create(&dev->thread, NULL, read_thread, dev);

						/* Wait here for the read thread to be initialized. */
						pthread_barrier_wait(&dev->barrier);

					}
					free(dev_path);
				}
			}
		}
		libusb_free_config_descriptor(conf_desc);

	}

	libusb_free_device_list(devs, 1);

	/* If we have a good handle, return it. */
	if (good_open) {
		return dev;
	}
	else {
		/* Unable to open any devices. */
		free_hid_device(dev);
		return NULL;
	}
}


int HID_API_EXPORT hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
	int res;
	int report_number = data[0];
	int skipped_report_id = 0;

	if (report_number == 0x0) {
		data++;
		length--;
		skipped_report_id = 1;
	}


	if (dev->output_endpoint <= 0) {
		/* No interrupt out endpoint. Use the Control Endpoint */
		res = libusb_control_transfer(dev->device_handle,
			LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT,
			0x09/*HID Set_Report*/,
			(2/*HID output*/ << 8) | report_number,
			dev->interface,
			(unsigned char *)data, length,
			1000/*timeout millis*/);

		if (res < 0)
			return -1;

		if (skipped_report_id)
			length++;

		return length;
	}
	else {
		/* Use the interrupt out endpoint */
		int actual_length;
		res = libusb_interrupt_transfer(dev->device_handle,
			dev->output_endpoint,
			(unsigned char*)data,
			length,
			&actual_length, 1000);

		if (res < 0)
			return -1;

		if (skipped_report_id)
			actual_length++;

		return actual_length;
	}
}

/* Helper function, to simplify hid_read().
   This should be called with dev->mutex locked. */
static int return_data(hid_device *dev, unsigned char *data, size_t length)
{
	/* Copy the data out of the linked list item (rpt) into the
	   return buffer (data), and delete the liked list item. */
	struct input_report *rpt = dev->input_reports;
	size_t len = (length < rpt->len)? length: rpt->len;
	if (len > 0)
		memcpy(data, rpt->data, len);
	dev->input_reports = rpt->next;
	free(rpt->data);
	free(rpt);
	return len;
}

static void cleanup_mutex(void *param)
{
	hid_device *dev = param;
	pthread_mutex_unlock(&dev->mutex);
}


int HID_API_EXPORT hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds)
{
	int bytes_read = -1;

#if 0
	int transferred;
	int res = libusb_interrupt_transfer(dev->device_handle, dev->input_endpoint, data, length, &transferred, 5000);
	LOG("transferred: %d\n", transferred);
	return transferred;
#endif

	pthread_mutex_lock(&dev->mutex);
	pthread_cleanup_push(&cleanup_mutex, dev);

	/* There's an input report queued up. Return it. */
	if (dev->input_reports) {
		/* Return the first one */
		bytes_read = return_data(dev, data, length);
		goto ret;
	}

	if (dev->shutdown_thread) {
		/* This means the device has been disconnected.
		   An error code of -1 should be returned. */
		bytes_read = -1;
		goto ret;
	}

	if (milliseconds == -1) {
		/* Blocking */
		while (!dev->input_reports && !dev->shutdown_thread) {
			pthread_cond_wait(&dev->condition, &dev->mutex);
		}
		if (dev->input_reports) {
			bytes_read = return_data(dev, data, length);
		}
	}
	else if (milliseconds > 0) {
		/* Non-blocking, but called with timeout. */
		int res;
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += milliseconds / 1000;
		ts.tv_nsec += (milliseconds % 1000) * 1000000;
		if (ts.tv_nsec >= 1000000000L) {
			ts.tv_sec++;
			ts.tv_nsec -= 1000000000L;
		}

		while (!dev->input_reports && !dev->shutdown_thread) {
			res = pthread_cond_timedwait(&dev->condition, &dev->mutex, &ts);
			if (res == 0) {
				if (dev->input_reports) {
					bytes_read = return_data(dev, data, length);
					break;
				}

				/* If we're here, there was a spurious wake up
				   or the read thread was shutdown. Run the
				   loop again (ie: don't break). */
			}
			else if (res == ETIMEDOUT) {
				/* Timed out. */
				bytes_read = 0;
				break;
			}
			else {
				/* Error. */
				bytes_read = -1;
				break;
			}
		}
	}
	else {
		/* Purely non-blocking */
		bytes_read = 0;
	}

ret:
	pthread_mutex_unlock(&dev->mutex);
	pthread_cleanup_pop(0);

	return bytes_read;
}

int HID_API_EXPORT hid_read(hid_device *dev, unsigned char *data, size_t length)
{
	return hid_read_timeout(dev, data, length, dev->blocking ? -1 : 0);
}

int HID_API_EXPORT hid_set_nonblocking(hid_device *dev, int nonblock)
{
	dev->blocking = !nonblock;

	return 0;
}


int HID_API_EXPORT hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
	int res = -1;
	int skipped_report_id = 0;
	int report_number = data[0];

	if (report_number == 0x0) {
		data++;
		length--;
		skipped_report_id = 1;
	}

	res = libusb_control_transfer(dev->device_handle,
		LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT,
		0x09/*HID set_report*/,
		(3/*HID feature*/ << 8) | report_number,
		dev->interface,
		(unsigned char *)data, length,
		1000/*timeout millis*/);

	if (res < 0)
		return -1;

	/* Account for the report ID */
	if (skipped_report_id)
		length++;

	return length;
}

int HID_API_EXPORT hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
	int res = -1;
	int skipped_report_id = 0;
	int report_number = data[0];

	if (report_number == 0x0) {
		/* Offset the return buffer by 1, so that the report ID
		   will remain in byte 0. */
		data++;
		length--;
		skipped_report_id = 1;
	}
	res = libusb_control_transfer(dev->device_handle,
		LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN,
		0x01/*HID get_report*/,
		(3/*HID feature*/ << 8) | report_number,
		dev->interface,
		(unsigned char *)data, length,
		1000/*timeout millis*/);

	if (res < 0)
		return -1;

	if (skipped_report_id)
		res++;

	return res;
}


void HID_API_EXPORT hid_close(hid_device *dev)
{
	if (!dev)
		return;

	/* Cause read_thread() to stop. */
	dev->shutdown_thread = 1;
	libusb_cancel_transfer(dev->transfer);

	/* Wait for read_thread() to end. */
	pthread_join(dev->thread, NULL);

	/* Clean up the Transfer objects allocated in read_thread(). */
	free(dev->transfer->buffer);
	libusb_free_transfer(dev->transfer);

	/* release the interface */
	libusb_release_interface(dev->device_handle, dev->interface);

	/* Close the handle */
	libusb_close(dev->device_handle);

	/* Clear out the queue of received reports. */
	pthread_mutex_lock(&dev->mutex);
	while (dev->input_reports) {
		return_data(dev, NULL, 0);
	}
	pthread_mutex_unlock(&dev->mutex);

	free_hid_device(dev);
}


int HID_API_EXPORT_CALL hid_get_manufacturer_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return hid_get_indexed_string(dev, dev->manufacturer_index, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_product_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return hid_get_indexed_string(dev, dev->product_index, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_serial_number_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return hid_get_indexed_string(dev, dev->serial_index, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_indexed_string(hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	wchar_t *str;

	str = get_usb_string(dev->device_handle, string_index);
	if (str) {
		wcsncpy(string, str, maxlen);
		string[maxlen-1] = L'\0';
		free(str);
		return 0;
	}
	else
		return -1;
}


HID_API_EXPORT const wchar_t * HID_API_CALL  hid_error(hid_device *dev)
{
	return NULL;
}


struct lang_map_entry {
	const char *name;
	const char *string_code;
	uint16_t usb_code;
};

#define LANG(name,code,usb_code) { name, code, usb_code }
static struct lang_map_entry lang_map[] = {
	LANG("Afrikaans", "af", 0x0436),
	LANG("Albanian", "sq", 0x041C),
	LANG("Arabic - United Arab Emirates", "ar_ae", 0x3801),
	LANG("Arabic - Bahrain", "ar_bh", 0x3C01),
	LANG("Arabic - Algeria", "ar_dz", 0x1401),
	LANG("Arabic - Egypt", "ar_eg", 0x0C01),
	LANG("Arabic - Iraq", "ar_iq", 0x0801),
	LANG("Arabic - Jordan", "ar_jo", 0x2C01),
	LANG("Arabic - Kuwait", "ar_kw", 0x3401),
	LANG("Arabic - Lebanon", "ar_lb", 0x3001),
	LANG("Arabic - Libya", "ar_ly", 0x1001),
	LANG("Arabic - Morocco", "ar_ma", 0x1801),
	LANG("Arabic - Oman", "ar_om", 0x2001),
	LANG("Arabic - Qatar", "ar_qa", 0x4001),
	LANG("Arabic - Saudi Arabia", "ar_sa", 0x0401),
	LANG("Arabic - Syria", "ar_sy", 0x2801),
	LANG("Arabic - Tunisia", "ar_tn", 0x1C01),
	LANG("Arabic - Yemen", "ar_ye", 0x2401),
	LANG("Armenian", "hy", 0x042B),
	LANG("Azeri - Latin", "az_az", 0x042C),
	LANG("Azeri - Cyrillic", "az_az", 0x082C),
	LANG("Basque", "eu", 0x042D),
	LANG("Belarusian", "be", 0x0423),
	LANG("Bulgarian", "bg", 0x0402),
	LANG("Catalan", "ca", 0x0403),
	LANG("Chinese - China", "zh_cn", 0x0804),
	LANG("Chinese - Hong Kong SAR", "zh_hk", 0x0C04),
	LANG("Chinese - Macau SAR", "zh_mo", 0x1404),
	LANG("Chinese - Singapore", "zh_sg", 0x1004),
	LANG("Chinese - Taiwan", "zh_tw", 0x0404),
	LANG("Croatian", "hr", 0x041A),
	LANG("Czech", "cs", 0x0405),
	LANG("Danish", "da", 0x0406),
	LANG("Dutch - Netherlands", "nl_nl", 0x0413),
	LANG("Dutch - Belgium", "nl_be", 0x0813),
	LANG("English - Australia", "en_au", 0x0C09),
	LANG("English - Belize", "en_bz", 0x2809),
	LANG("English - Canada", "en_ca", 0x1009),
	LANG("English - Caribbean", "en_cb", 0x2409),
	LANG("English - Ireland", "en_ie", 0x1809),
	LANG("English - Jamaica", "en_jm", 0x2009),
	LANG("English - New Zealand", "en_nz", 0x1409),
	LANG("English - Phillippines", "en_ph", 0x3409),
	LANG("English - Southern Africa", "en_za", 0x1C09),
	LANG("English - Trinidad", "en_tt", 0x2C09),
	LANG("English - Great Britain", "en_gb", 0x0809),
	LANG("English - United States", "en_us", 0x0409),
	LANG("Estonian", "et", 0x0425),
	LANG("Farsi", "fa", 0x0429),
	LANG("Finnish", "fi", 0x040B),
	LANG("Faroese", "fo", 0x0438),
	LANG("French - France", "fr_fr", 0x040C),
	LANG("French - Belgium", "fr_be", 0x080C),
	LANG("French - Canada", "fr_ca", 0x0C0C),
	LANG("French - Luxembourg", "fr_lu", 0x140C),
	LANG("French - Switzerland", "fr_ch", 0x100C),
	LANG("Gaelic - Ireland", "gd_ie", 0x083C),
	LANG("Gaelic - Scotland", "gd", 0x043C),
	LANG("German - Germany", "de_de", 0x0407),
	LANG("German - Austria", "de_at", 0x0C07),
	LANG("German - Liechtenstein", "de_li", 0x1407),
	LANG("German - Luxembourg", "de_lu", 0x1007),
	LANG("German - Switzerland", "de_ch", 0x0807),
	LANG("Greek", "el", 0x0408),
	LANG("Hebrew", "he", 0x040D),
	LANG("Hindi", "hi", 0x0439),
	LANG("Hungarian", "hu", 0x040E),
	LANG("Icelandic", "is", 0x040F),
	LANG("Indonesian", "id", 0x0421),
	LANG("Italian - Italy", "it_it", 0x0410),
	LANG("Italian - Switzerland", "it_ch", 0x0810),
	LANG("Japanese", "ja", 0x0411),
	LANG("Korean", "ko", 0x0412),
	LANG("Latvian", "lv", 0x0426),
	LANG("Lithuanian", "lt", 0x0427),
	LANG("F.Y.R.O. Macedonia", "mk", 0x042F),
	LANG("Malay - Malaysia", "ms_my", 0x043E),
	LANG("Malay – Brunei", "ms_bn", 0x083E),
	LANG("Maltese", "mt", 0x043A),
	LANG("Marathi", "mr", 0x044E),
	LANG("Norwegian - Bokml", "no_no", 0x0414),
	LANG("Norwegian - Nynorsk", "no_no", 0x0814),
	LANG("Polish", "pl", 0x0415),
	LANG("Portuguese - Portugal", "pt_pt", 0x0816),
	LANG("Portuguese - Brazil", "pt_br", 0x0416),
	LANG("Raeto-Romance", "rm", 0x0417),
	LANG("Romanian - Romania", "ro", 0x0418),
	LANG("Romanian - Republic of Moldova", "ro_mo", 0x0818),
	LANG("Russian", "ru", 0x0419),
	LANG("Russian - Republic of Moldova", "ru_mo", 0x0819),
	LANG("Sanskrit", "sa", 0x044F),
	LANG("Serbian - Cyrillic", "sr_sp", 0x0C1A),
	LANG("Serbian - Latin", "sr_sp", 0x081A),
	LANG("Setsuana", "tn", 0x0432),
	LANG("Slovenian", "sl", 0x0424),
	LANG("Slovak", "sk", 0x041B),
	LANG("Sorbian", "sb", 0x042E),
	LANG("Spanish - Spain (Traditional)", "es_es", 0x040A),
	LANG("Spanish - Argentina", "es_ar", 0x2C0A),
	LANG("Spanish - Bolivia", "es_bo", 0x400A),
	LANG("Spanish - Chile", "es_cl", 0x340A),
	LANG("Spanish - Colombia", "es_co", 0x240A),
	LANG("Spanish - Costa Rica", "es_cr", 0x140A),
	LANG("Spanish - Dominican Republic", "es_do", 0x1C0A),
	LANG("Spanish - Ecuador", "es_ec", 0x300A),
	LANG("Spanish - Guatemala", "es_gt", 0x100A),
	LANG("Spanish - Honduras", "es_hn", 0x480A),
	LANG("Spanish - Mexico", "es_mx", 0x080A),
	LANG("Spanish - Nicaragua", "es_ni", 0x4C0A),
	LANG("Spanish - Panama", "es_pa", 0x180A),
	LANG("Spanish - Peru", "es_pe", 0x280A),
	LANG("Spanish - Puerto Rico", "es_pr", 0x500A),
	LANG("Spanish - Paraguay", "es_py", 0x3C0A),
	LANG("Spanish - El Salvador", "es_sv", 0x440A),
	LANG("Spanish - Uruguay", "es_uy", 0x380A),
	LANG("Spanish - Venezuela", "es_ve", 0x200A),
	LANG("Southern Sotho", "st", 0x0430),
	LANG("Swahili", "sw", 0x0441),
	LANG("Swedish - Sweden", "sv_se", 0x041D),
	LANG("Swedish - Finland", "sv_fi", 0x081D),
	LANG("Tamil", "ta", 0x0449),
	LANG("Tatar", "tt", 0X0444),
	LANG("Thai", "th", 0x041E),
	LANG("Turkish", "tr", 0x041F),
	LANG("Tsonga", "ts", 0x0431),
	LANG("Ukrainian", "uk", 0x0422),
	LANG("Urdu", "ur", 0x0420),
	LANG("Uzbek - Cyrillic", "uz_uz", 0x0843),
	LANG("Uzbek – Latin", "uz_uz", 0x0443),
	LANG("Vietnamese", "vi", 0x042A),
	LANG("Xhosa", "xh", 0x0434),
	LANG("Yiddish", "yi", 0x043D),
	LANG("Zulu", "zu", 0x0435),
	LANG(NULL, NULL, 0x0),
};

uint16_t get_usb_code_for_current_locale(void)
{
	char *locale;
	char search_string[64];
	char *ptr;
	struct lang_map_entry *lang;

	/* Get the current locale. */
	locale = setlocale(0, NULL);
	if (!locale)
		return 0x0;

	/* Make a copy of the current locale string. */
	strncpy(search_string, locale, sizeof(search_string));
	search_string[sizeof(search_string)-1] = '\0';

	/* Chop off the encoding part, and make it lower case. */
	ptr = search_string;
	while (*ptr) {
		*ptr = tolower(*ptr);
		if (*ptr == '.') {
			*ptr = '\0';
			break;
		}
		ptr++;
	}

	/* Find the entry which matches the string code of our locale. */
	lang = lang_map;
	while (lang->string_code) {
		if (!strcmp(lang->string_code, search_string)) {
			return lang->usb_code;
		}
		lang++;
	}

	/* There was no match. Find with just the language only. */
	/* Chop off the variant. Chop it off at the '_'. */
	ptr = search_string;
	while (*ptr) {
		*ptr = tolower(*ptr);
		if (*ptr == '_') {
			*ptr = '\0';
			break;
		}
		ptr++;
	}

#if 0 /* TODO: Do we need this? */
	/* Find the entry which matches the string code of our language. */
	lang = lang_map;
	while (lang->string_code) {
		if (!strcmp(lang->string_code, search_string)) {
			return lang->usb_code;
		}
		lang++;
	}
#endif

	/* Found nothing. */
	return 0x0;
}

#ifdef __cplusplus
}
#endif

#endif


