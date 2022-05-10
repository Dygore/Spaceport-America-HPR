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

#include <winsock2.h>
#include <windows.h>
#include <setupapi.h>

struct altos_list {
	HDEVINFO	dev_info;
	int		index;
	int		ftdi;
};

#define USB_BUF_SIZE	64

struct altos_file_windows {
	struct altos_file		file;

	BOOL				is_winsock;
	/* Data used by the regular I/O */
	HANDLE				handle;
	OVERLAPPED			ov_read;
	BOOL				pend_read;
	OVERLAPPED			ov_write;

	/* Data used by winsock */
	SOCKET				socket;
};

#include <stdarg.h>

static void
log_message(char *fmt, ...)
{
	static FILE *log = NULL;
	va_list	a;

	if (!log)
		log = fopen("\\temp\\altos.txt", "w");
	if (log) {
		SYSTEMTIME time;
		char	buffer[4096];

		GetLocalTime(&time);
		__ms_sprintf (buffer, "%4d-%02d-%02d %2d:%02d:%02d. ",
			 time.wYear, time.wMonth, time.wDay,
			 time.wHour, time.wMinute, time.wSecond);
		va_start(a, fmt);

		__ms_vsprintf(buffer + strlen(buffer), fmt, a);
		va_end(a);

		fputs(buffer, log);
		fflush(log);
		fputs(buffer, stdout);
		fflush(stdout);
	}
}

static void
_altos_set_last_windows_error(char *file, int line, DWORD error)
{
	TCHAR	message[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
		      0,
		      error,
		      0,
		      message,
		      sizeof (message) / sizeof (TCHAR),
		      NULL);
	if (error != ERROR_SUCCESS)
		log_message ("%s:%d (%d) %s\n", file, line, error, message);
	altos_set_last_error(error, message);
}

#define altos_set_last_windows_error() _altos_set_last_windows_error(__FILE__, __LINE__, GetLastError())
#define altos_set_last_winsock_error() _altos_set_last_windows_error(__FILE__, __LINE__, WSAGetLastError())

PUBLIC struct altos_list *
altos_list_start(void)
{
	struct altos_list	*list = calloc(1, sizeof (struct altos_list));

	if (!list)
		return NULL;
	list->dev_info = SetupDiGetClassDevs(NULL, "USB", NULL,
					     DIGCF_ALLCLASSES|DIGCF_PRESENT);
	if (list->dev_info == INVALID_HANDLE_VALUE) {
		altos_set_last_windows_error();
		free(list);
		return NULL;
	}
	list->index = 0;
	list->ftdi = 0;
	return list;
}

PUBLIC struct altos_list *
altos_ftdi_list_start(void)
{
	struct altos_list	*list = calloc(1, sizeof (struct altos_list));

	if (!list)
		return NULL;
	list->dev_info = SetupDiGetClassDevs(NULL, "FTDIBUS", NULL,
					     DIGCF_ALLCLASSES|DIGCF_PRESENT);
	if (list->dev_info == INVALID_HANDLE_VALUE) {
		altos_set_last_windows_error();
		free(list);
		return NULL;
	}
	list->index = 0;
	list->ftdi = 1;
	return list;
}

static struct {
	unsigned int	vid, pid;
	char	*name;
} name_map[] = {
	{ .vid = 0xfffe, .pid = 0x000d, .name = "EasyTimer" },
	{ .vid = 0xfffe, .pid = 0x0028, .name = "EasyMega" },
	{ .vid = 0xfffe, .pid = 0x002c, .name = "EasyMotor" },
	{ .name = NULL },
};

PUBLIC int
altos_list_next(struct altos_list *list, struct altos_device *device)
{
	SP_DEVINFO_DATA dev_info_data;
	BYTE		port[128];
	DWORD		port_len;
	char		friendlyname[256];
	BYTE		symbolic[256];
	DWORD		symbolic_len;
	HKEY		dev_key;
	unsigned int	vid, pid;
	int		serial;
	HRESULT 	result;
	DWORD		friendlyname_type;
	DWORD		friendlyname_len;
	char		instanceid[1024];
	DWORD		instanceid_len;
	int		i;

	dev_info_data.cbSize = sizeof (SP_DEVINFO_DATA);
	while(SetupDiEnumDeviceInfo(list->dev_info, list->index,
				    &dev_info_data))
	{
		list->index++;

		dev_key = SetupDiOpenDevRegKey(list->dev_info, &dev_info_data,
					       DICS_FLAG_GLOBAL, 0, DIREG_DEV,
					       KEY_READ);
		if (dev_key == INVALID_HANDLE_VALUE) {
			altos_set_last_windows_error();
			continue;
		}

		if (list->ftdi) {
			vid = 0x0403;
			pid = 0x6015;
			serial = 0;
		} else {
			vid = pid = serial = 0;
			/* Fetch symbolic name for this device and parse out
			 * the vid/pid/serial info */
			symbolic_len = sizeof(symbolic);
			result = RegQueryValueEx(dev_key, "SymbolicName", NULL, NULL,
						 symbolic, &symbolic_len);
			if (result != 0) {
				altos_set_last_windows_error();
			} else {
				__ms_sscanf((char *) symbolic + sizeof("\\??\\USB#VID_") - 1,
				       "%04X", &vid);
				__ms_sscanf((char *) symbolic + sizeof("\\??\\USB#VID_XXXX&PID_") - 1,
				       "%04X", &pid);
				__ms_sscanf((char *) symbolic + sizeof("\\??\\USB#VID_XXXX&PID_XXXX#") - 1,
				       "%d", &serial);
			}
			if (vid == 0 || pid == 0 || serial == 0) {
				if (SetupDiGetDeviceInstanceId(list->dev_info,
							       &dev_info_data,
							       instanceid,
							       sizeof (instanceid),
							       &instanceid_len)) {
					__ms_sscanf((char *) instanceid + sizeof("USB\\VID_") - 1,
					       "%04X", &vid);
					__ms_sscanf((char *) instanceid + sizeof("USB\\VID_XXXX&PID_") - 1,
					       "%04X", &pid);
					__ms_sscanf((char *) instanceid + sizeof("USB\\VID_XXXX&PID_XXXX\\") - 1,
					       "%d", &serial);
				} else {
					altos_set_last_windows_error();
				}
			}
			if (vid == 0 || pid == 0 || serial == 0) {
				RegCloseKey(dev_key);
				continue;
			}
		}

		/* Fetch the com port name */
		port_len = sizeof (port);
		result = RegQueryValueEx(dev_key, "PortName", NULL, NULL,
					 port, &port_len);
		RegCloseKey(dev_key);
		if (result != 0) {
			altos_set_last_windows_error();
			continue;
		}

		/* Fetch the device description which is the device name,
		 * with firmware that has unique USB ids */
		friendlyname_len = sizeof (friendlyname);
		if(!SetupDiGetDeviceRegistryProperty(list->dev_info,
						     &dev_info_data,
						     SPDRP_FRIENDLYNAME,
						     &friendlyname_type,
						     (BYTE *)friendlyname,
						     sizeof(friendlyname),
						     &friendlyname_len))
		{
			altos_set_last_windows_error();
			continue;
		}

		char *space = friendlyname;
		while (*space) {
			if (*space == ' ') {
				*space = '\0';
				break;
			}
			space++;
		}

		for (i = 0; name_map[i].name; i++) {
			if (name_map[i].vid == vid && name_map[i].pid == pid) {
				strcpy(friendlyname, name_map[i].name);
				break;
			}
		}

		device->vendor = vid;
		device->product = pid;
		device->serial = serial;
		strcpy(device->name, friendlyname);

		strcpy(device->path, (char *) port);
		return 1;
	}
	result = GetLastError();
	if (result != ERROR_NO_MORE_ITEMS)
		altos_set_last_windows_error();
	return 0;
}

PUBLIC void
altos_list_finish(struct altos_list *list)
{
	SetupDiDestroyDeviceInfoList(list->dev_info);
	free(list);
}

static int
altos_queue_read(struct altos_file_windows *file)
{
	DWORD	got;
	if (file->pend_read)
		return LIBALTOS_SUCCESS;

	if (!ReadFile(file->handle, file->file.in_data, USB_BUF_SIZE, &got, &file->ov_read)) {
		if (GetLastError() != ERROR_IO_PENDING) {
			altos_set_last_windows_error();
			return LIBALTOS_ERROR;
		}
		file->pend_read = TRUE;
	} else {
		file->pend_read = FALSE;
		file->file.in_read = 0;
		file->file.in_used = got;
	}
	return LIBALTOS_SUCCESS;
}

static int
altos_wait_read(struct altos_file_windows *file, int timeout)
{
	DWORD	ret;
	DWORD	got;

	if (!file->pend_read)
		return LIBALTOS_SUCCESS;

	if (!timeout)
		timeout = INFINITE;

	ret = WaitForSingleObject(file->ov_read.hEvent, timeout);
	switch (ret) {
	case WAIT_OBJECT_0:
		if (!GetOverlappedResult(file->handle, &file->ov_read, &got, FALSE)) {
			if (GetLastError () != ERROR_OPERATION_ABORTED)
				altos_set_last_windows_error();
			return LIBALTOS_ERROR;
		}
		file->pend_read = FALSE;
		file->file.in_read = 0;
		file->file.in_used = got;
		break;
	case WAIT_TIMEOUT:
		return LIBALTOS_TIMEOUT;
		break;
	default:
		altos_set_last_windows_error();
		return LIBALTOS_ERROR;
	}
	return LIBALTOS_SUCCESS;
}

int
altos_fill(struct altos_file *file_common, int timeout)
{
	struct altos_file_windows	*file = (struct altos_file_windows *) file_common;

	int	ret;

	if (file->file.in_read < file->file.in_used)
		return LIBALTOS_SUCCESS;

	file->file.in_read = file->file.in_used = 0;

	if (file->is_winsock) {

		for (;;) {
			fd_set	readfds;
			TIMEVAL	timeval;
			int	thistimeout;

			/* Check to see if the socket has been closed */
			if (file->socket == INVALID_SOCKET)
				return LIBALTOS_ERROR;

#define POLL_TIMEOUT	10000

			/* Poll to see if the socket has been closed
			 * as select doesn't abort when that happens
			 */
			if (timeout) {
				thistimeout = timeout;
				if (thistimeout > POLL_TIMEOUT)
					thistimeout = POLL_TIMEOUT;
			} else {
				thistimeout = POLL_TIMEOUT;
			}

			timeval.tv_sec = thistimeout / 1000;
			timeval.tv_usec = (thistimeout % 1000) * 1000;

			FD_ZERO(&readfds);
			FD_SET(file->socket, &readfds);

			ret = select(1, &readfds, NULL, NULL, &timeval);

			if (ret == 0) {
				if (timeout) {
					timeout -= thistimeout;
					if (timeout == 0)
						return LIBALTOS_TIMEOUT;
				}
			} else {
				if (ret > 0)
					break;

				if (ret < 0) {
					altos_set_last_winsock_error();
					return LIBALTOS_ERROR;
				}
			}
		}

		if (file->socket == INVALID_SOCKET) {
			altos_set_last_winsock_error();
			return LIBALTOS_ERROR;
		}

		ret = recv(file->socket, (char *) file->file.in_data, USB_BUF_SIZE, 0);

		if (ret <= 0) {
			altos_set_last_winsock_error();
			return LIBALTOS_ERROR;
		}
		file->file.in_read = 0;
		file->file.in_used = ret;
	} else {
		if (file->handle == INVALID_HANDLE_VALUE) {
			altos_set_last_windows_error();
			return LIBALTOS_ERROR;
		}

		ret = altos_queue_read(file);
		if (ret)
			return ret;
		ret = altos_wait_read(file, timeout);
		if (ret)
			return ret;
	}

	return LIBALTOS_SUCCESS;
}

PUBLIC int
altos_flush(struct altos_file *file_common)
{
	struct altos_file_windows	*file = (struct altos_file_windows *) file_common;

	unsigned char	*data = file->file.out_data;
	int		used = file->file.out_used;

	while (used) {
		if (file->is_winsock) {
			int	put;

			put = send(file->socket, (char *) data, used, 0);
			if (put <= 0) {
				altos_set_last_winsock_error();
				return LIBALTOS_ERROR;
			}
			data += put;
			used -= put;
		} else {
			DWORD		put;
			DWORD		ret;
			if (!WriteFile(file->handle, data, used, &put, &file->ov_write)) {
				if (GetLastError() != ERROR_IO_PENDING) {
					altos_set_last_windows_error();
					return LIBALTOS_ERROR;
				}
				ret = WaitForSingleObject(file->ov_write.hEvent, INFINITE);
				switch (ret) {
				case WAIT_OBJECT_0:
					if (!GetOverlappedResult(file->handle, &file->ov_write, &put, FALSE)) {
						altos_set_last_windows_error();
						return LIBALTOS_ERROR;
					}
					break;
				default:
					altos_set_last_windows_error();
					return LIBALTOS_ERROR;
				}
			}
			data += put;
			used -= put;
		}
	}
	file->file.out_used = 0;
	return LIBALTOS_SUCCESS;
}

static HANDLE
open_serial(char *full_name)
{
	HANDLE	handle;
	DCB	dcb;

	handle = CreateFile(full_name, GENERIC_READ|GENERIC_WRITE,
			    0, NULL, OPEN_EXISTING,
			    FILE_FLAG_OVERLAPPED, NULL);

	if (handle == INVALID_HANDLE_VALUE) {
		altos_set_last_windows_error();
		return INVALID_HANDLE_VALUE;
	}

	if (!GetCommState(handle, &dcb)) {
		altos_set_last_windows_error();
		CloseHandle(handle);
		return INVALID_HANDLE_VALUE;
	}
	dcb.BaudRate = CBR_9600;
	dcb.fBinary = TRUE;
	dcb.fParity = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fTXContinueOnXoff = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fErrorChar = FALSE;
	dcb.fNull = FALSE;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	dcb.fAbortOnError = FALSE;
	dcb.XonLim = 10;
	dcb.XoffLim = 10;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.XonChar = 17;
	dcb.XoffChar = 19;
#if 0
	dcb.ErrorChar = 0;
	dcb.EofChar = 0;
	dcb.EvtChar = 0;
#endif
	if (!SetCommState(handle, &dcb)) {
		altos_set_last_windows_error();
		CloseHandle(handle);
		return INVALID_HANDLE_VALUE;
	}
	return handle;
}

PUBLIC struct altos_file *
altos_open(struct altos_device *device)
{
	struct altos_file_windows	*file = calloc (1, sizeof (struct altos_file_windows));
	char	full_name[64];
	COMMTIMEOUTS timeouts;
	int i;

	if (!file)
		return NULL;

	strcpy(full_name, "\\\\.\\");
	strcat(full_name, device->path);

	file->handle = INVALID_HANDLE_VALUE;

	for (i = 0; i < 5; i++) {
		file->handle = open_serial(full_name);
		if (file->handle != INVALID_HANDLE_VALUE)
			break;
		altos_set_last_windows_error();
		Sleep(100);
	}

	if (file->handle == INVALID_HANDLE_VALUE) {
		free(file);
		return NULL;
	}

	/* The FTDI driver doesn't appear to work right unless you open it twice */
	if (device->vendor == 0x0403) {
		CloseHandle(file->handle);
		file->handle = open_serial(full_name);
		if (file->handle == INVALID_HANDLE_VALUE) {
			free(file);
			return NULL;
		}
	}

	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = 1 << 30;	/* almost forever */
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(file->handle, &timeouts);

	file->ov_read.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	file->ov_write.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	return &file->file;
}

PUBLIC void
altos_close(struct altos_file *file_common)
{
	struct altos_file_windows	*file = (struct altos_file_windows *) file_common;

	if (file->is_winsock) {
		SOCKET	socket = file->socket;
		if (socket != INVALID_SOCKET) {
			file->socket = INVALID_SOCKET;
			closesocket(socket);
		}
	} else {
		HANDLE	handle = file->handle;
		if (handle != INVALID_HANDLE_VALUE) {
			HANDLE	ov_read = file->ov_read.hEvent;
			HANDLE	ov_write = file->ov_write.hEvent;
			file->handle = INVALID_HANDLE_VALUE;
			file->ov_read.hEvent = INVALID_HANDLE_VALUE;
			file->ov_write.hEvent = INVALID_HANDLE_VALUE;
			PurgeComm(handle, PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR);
			Sleep(100);
			CloseHandle(handle);
			file->handle = INVALID_HANDLE_VALUE;
			CloseHandle(ov_read);
			CloseHandle(ov_write);
		}
	}
}

#include <ws2bth.h>

#define LUP_SET	(LUP_RETURN_NAME| LUP_CONTAINERS | LUP_RETURN_ADDR | LUP_FLUSHCACHE |\
		 LUP_RETURN_TYPE | LUP_RETURN_BLOB | LUP_RES_SERVICE)

struct altos_bt_list {
	WSADATA	WSAData;
	HANDLE	lookup;
};

struct altos_bt_list *
altos_bt_list_start(int inquiry_time)
{
	struct altos_bt_list	*bt_list;
	WSAQUERYSET		query_set;
	int			retCode;

	/* Windows provides no way to set the time */
	(void) inquiry_time;
	bt_list = calloc(1, sizeof (struct altos_bt_list));
	if (!bt_list) {
		altos_set_last_windows_error();
		return NULL;
	}

	if ((retCode = WSAStartup(MAKEWORD(2,2),&bt_list->WSAData)) != 0) {
		altos_set_last_winsock_error();
		free(bt_list);
		return NULL;
	}

	memset(&query_set, '\0', sizeof (query_set));
	query_set.dwSize = sizeof(WSAQUERYSET);
	query_set.dwNameSpace = NS_BTH;

	retCode = WSALookupServiceBegin(&query_set, LUP_SET, &bt_list->lookup);

	if (retCode != 0) {
		altos_set_last_winsock_error();
		free(bt_list);
		return NULL;
	}
	return bt_list;
}

static unsigned char get_byte(BTH_ADDR ba, int shift)
{
	return (ba >> ((5 - shift) << 3)) & 0xff;
}

static BTH_ADDR put_byte(unsigned char c, int shift)
{
	return ((BTH_ADDR) c) << ((5 - shift) << 3);
}

static void
ba2str(BTH_ADDR ba, char *str)
{

	__ms_sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
		get_byte(ba, 0),
		get_byte(ba, 1),
		get_byte(ba, 2),
		get_byte(ba, 3),
		get_byte(ba, 4),
		get_byte(ba, 5));
}

static BTH_ADDR
str2ba(char *str)
{
	unsigned int	bytes[6];

	__ms_sscanf(str,  "%02x:%02x:%02x:%02x:%02x:%02x",
	       &bytes[0],
	       &bytes[1],
	       &bytes[2],
	       &bytes[3],
	       &bytes[4],
	       &bytes[5]);
	return (put_byte(bytes[0], 0) |
		put_byte(bytes[1], 1) |
		put_byte(bytes[2], 2) |
		put_byte(bytes[3], 3) |
		put_byte(bytes[4], 4) |
		put_byte(bytes[5], 5));
}

int
altos_bt_list_next(struct altos_bt_list *bt_list,
		   struct altos_bt_device *device)
{
	for (;;) {
		BYTE		buffer[4096];
		DWORD		length = sizeof (buffer);;
		WSAQUERYSET	*results = (WSAQUERYSET *)buffer;
		CSADDR_INFO	*addr_info;
		int		retCode;
		SOCKADDR_BTH	*sockaddr_bth;

		memset(buffer, '\0', sizeof(buffer));

		retCode = WSALookupServiceNext(bt_list->lookup, LUP_SET, &length, results);

		if (retCode != 0) {
			int error = WSAGetLastError();
			if (error != WSAENOMORE && error != WSA_E_NO_MORE)
				altos_set_last_winsock_error();
			return 0;
		}

		if (results->dwNumberOfCsAddrs > 0) {

			addr_info = results->lpcsaBuffer;

			strncpy(device->name, results->lpszServiceInstanceName, sizeof(device->name));
			device->name[sizeof(device->name)-1] = '\0';

			sockaddr_bth = (SOCKADDR_BTH *) addr_info->RemoteAddr.lpSockaddr;

			ba2str(sockaddr_bth->btAddr, device->addr);

			return 1;
		}
	}
}

void
altos_bt_list_finish(struct altos_bt_list *bt_list)
{
	WSALookupServiceEnd(bt_list->lookup);
	free(bt_list);
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
	struct altos_file_windows	*file;
	SOCKADDR_BTH		sockaddr_bth;
	int			ret;
	int			channel = 0;

	file = calloc(1, sizeof (struct altos_file_windows));
	if (!file) {
		return NULL;
	}

	file->is_winsock = TRUE;
	file->socket = WSASocket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (file->socket == INVALID_SOCKET) {
		altos_set_last_winsock_error();
		free(file);
		return NULL;
	}

	memset(&sockaddr_bth, '\0', sizeof (sockaddr_bth));
	sockaddr_bth.addressFamily = AF_BTH;
	sockaddr_bth.btAddr = str2ba(device->addr);

	channel = altos_bt_port(device);
	if (channel == 0)
		channel = BT_PORT_DEFAULT;

	sockaddr_bth.port = channel;

	ret = connect(file->socket, (SOCKADDR *) &sockaddr_bth, sizeof (sockaddr_bth));

	if (ret != 0) {
		altos_set_last_winsock_error();
		closesocket(file->socket);
		free(file);
		log_message("Connection attempted to address %s port %d\n", device->addr, sockaddr_bth.port);
		return NULL;
	}
	return &file->file;
}

void
altos_pause_one_second(void)
{
	Sleep(1000);
}
