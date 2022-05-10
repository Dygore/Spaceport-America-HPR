/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include "libaltos_private.h"
#include "libaltos_posix.h"

#include <IOKitLib.h>
#include <IOKit/usb/USBspec.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <usb/IOUSBLib.h>
#include <usb/USBSpec.h>
#include <sys/param.h>
#include <paths.h>
#include <CFNumber.h>
#include <IOBSD.h>

/* Mac OS X don't have strndup even if _GNU_SOURCE is defined */
char *
altos_strndup (const char *s, size_t n)
{
    size_t len = strlen (s);
    char *ret;

    if (len <= n)
       return strdup (s);
    ret = malloc(n + 1);
    strncpy(ret, s, n);
    ret[n] = '\0';
    return ret;
}

struct altos_list {
	io_iterator_t iterator;
	int ftdi;
};

static char *
get_cfstring(CFTypeRef string, char result[512])
{
	Boolean		got_string;

	got_string = CFStringGetCString(string, result, 512, kCFStringEncodingASCII);
	if (!got_string)
		strcpy(result, "CFStringGetCString failed");
	return result;
}

static int
get_string(io_object_t object, CFStringRef entry, char *result, int result_len)
{
	CFTypeRef entry_as_string;
	Boolean got_string;
	char entry_string[512];

	entry_as_string = IORegistryEntrySearchCFProperty (object,
							   kIOServicePlane,
							   entry,
							   kCFAllocatorDefault,
							   kIORegistryIterateRecursively);
	if (entry_as_string) {
		got_string = CFStringGetCString(entry_as_string,
						result, result_len,
						kCFStringEncodingASCII);

		CFRelease(entry_as_string);
		if (got_string) {
			return 1;
		}
	}
	return 0;
}

static int
get_number(io_object_t object, CFStringRef entry, int *result)
{
	CFTypeRef entry_as_number;
	Boolean got_number;
	char entry_string[512];

	entry_as_number = IORegistryEntrySearchCFProperty (object,
							   kIOServicePlane,
							   entry,
							   kCFAllocatorDefault,
							   kIORegistryIterateRecursively);
	if (entry_as_number) {
		got_number = CFNumberGetValue(entry_as_number,
					      kCFNumberIntType,
					      result);
		if (got_number) {
			return 1;
		}
	}
	return 0;
}

PUBLIC struct altos_list *
altos_list_start(void)
{
	struct altos_list *list = calloc (sizeof (struct altos_list), 1);
	CFMutableDictionaryRef matching_dictionary;
	io_iterator_t tdIterator;
	io_object_t tdObject;
	kern_return_t ret;
	int i;

	matching_dictionary = IOServiceMatching(kIOSerialBSDServiceValue);
	if (matching_dictionary) {
		CFDictionarySetValue(matching_dictionary,
				     CFSTR(kIOSerialBSDTypeKey),
				     CFSTR(kIOSerialBSDAllTypes));
	}

	ret = IOServiceGetMatchingServices(kIOMasterPortDefault, matching_dictionary, &list->iterator);
	if (ret != kIOReturnSuccess) {
		free(list);
		return NULL;
	}
	list->ftdi = 0;
	return list;
}

PUBLIC struct altos_list *
altos_ftdi_list_start(void)
{
	struct altos_list *list = altos_list_start();

	if (list)
		list->ftdi = 1;
	return list;
}

static io_service_t get_usb_object(io_object_t serial_device)
{
	io_iterator_t iterator;
	io_service_t usb_device;
	io_service_t service;
	IOReturn status;

	status = IORegistryEntryCreateIterator(serial_device,
				      kIOServicePlane,
				      kIORegistryIterateParents | kIORegistryIterateRecursively,
				      &iterator);

	if (status != kIOReturnSuccess)
		return 0;

	while((service = IOIteratorNext(iterator))) {
		io_name_t servicename;
		status = IORegistryEntryGetNameInPlane(service, kIOServicePlane, servicename);

		if (status == kIOReturnSuccess && IOObjectConformsTo(service, kIOUSBDeviceClassName)) {
			IOObjectRelease(iterator);
			return service;
		}
		IOObjectRelease(service);
	}
	IOObjectRelease(iterator);
	return 0;
}

PUBLIC int
altos_list_next(struct altos_list *list, struct altos_device *device)
{

	io_object_t object;
	io_service_t usb_device;
	char serial_string[128];

	for (;;) {
		object = IOIteratorNext(list->iterator);
		if (!object) {
			return 0;
		}

		usb_device = get_usb_object(object);

		if (get_number (usb_device, CFSTR(kUSBVendorID), &device->vendor) &&
		    get_number (usb_device, CFSTR(kUSBProductID), &device->product) &&
		    get_string (object, CFSTR(kIOCalloutDeviceKey), device->path, sizeof (device->path)) &&
		    (get_string (usb_device, CFSTR("kUSBProductString"), device->name, sizeof (device->name)) ||
		     get_string (usb_device, CFSTR(kUSBProductString), device->name, sizeof (device->name))) &&
		    get_string (usb_device, CFSTR(kUSBSerialNumberString), serial_string, sizeof (serial_string))) {
			device->serial = atoi(serial_string);
			IOObjectRelease(object);
			IOObjectRelease(usb_device);
			return 1;
		}
		IOObjectRelease(object);
		IOObjectRelease(usb_device);
	}
}

PUBLIC void
altos_list_finish(struct altos_list *list)
{
	IOObjectRelease (list->iterator);
	free(list);
}

struct altos_bt_list {
	int		sock;
	int		dev_id;
	int		rsp;
	int		num_rsp;
};

#define INQUIRY_MAX_RSP	255

struct altos_bt_list *
altos_bt_list_start(int inquiry_time)
{
	return NULL;
}

int
altos_bt_list_next(struct altos_bt_list *bt_list,
		   struct altos_bt_device *device)
{
	return 0;
}

void
altos_bt_list_finish(struct altos_bt_list *bt_list)
{
}

void
altos_bt_fill_in(char *name, char *addr, struct altos_bt_device *device)
{
	strncpy(device->name, name, sizeof (device->name));
	device->name[sizeof(device->name)-1] = '\0';
	strncpy(device->addr, addr, sizeof (device->addr));
	device->addr[sizeof(device->addr)-1] = '\0';
}

struct altos_file *
altos_bt_open(struct altos_bt_device *device)
{
	return NULL;
}
