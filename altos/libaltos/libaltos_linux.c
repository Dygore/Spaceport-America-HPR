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

#define _GNU_SOURCE
#include "libaltos_private.h"
#include "libaltos_posix.h"

#include <ctype.h>
#include <dirent.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

static char *
cc_fullname (char *dir, char *file)
{
	char	*new;
	int	dlen = strlen (dir);
	int	flen = strlen (file);
	int	slen = 0;

	if (dir[dlen-1] != '/')
		slen = 1;
	new = malloc (dlen + slen + flen + 1);
	if (!new)
		return 0;
	strcpy(new, dir);
	if (slen)
		strcat (new, "/");
	strcat(new, file);
	return new;
}

static char *
cc_basename(char *file)
{
	char *b;

	b = strrchr(file, '/');
	if (!b)
		return file;
	return b + 1;
}

static char *
load_string(char *dir, char *file)
{
	char	*full = cc_fullname(dir, file);
	char	line[4096];
	char	*r;
	FILE	*f;
	int	rlen;

	f = fopen(full, "r");
	free(full);
	if (!f)
		return NULL;
	r = fgets(line, sizeof (line), f);
	fclose(f);
	if (!r)
		return NULL;
	rlen = strlen(r);
	if (r[rlen-1] == '\n')
		r[rlen-1] = '\0';
	return strdup(r);
}

static int
load_hex(char *dir, char *file)
{
	char	*line;
	char	*end;
	long	i;

	line = load_string(dir, file);
	if (!line)
		return -1;
	i = strtol(line, &end, 16);
	free(line);
	if (end == line)
		return -1;
	return i;
}

static int
load_dec(char *dir, char *file)
{
	char	*line;
	char	*end;
	long	i;

	line = load_string(dir, file);
	if (!line)
		return -1;
	i = strtol(line, &end, 10);
	free(line);
	if (end == line)
		return -1;
	return i;
}

static int
dir_filter_tty_colon(const struct dirent *d)
{
	return strncmp(d->d_name, "tty:", 4) == 0;
}

static int
dir_filter_tty(const struct dirent *d)
{
	return strncmp(d->d_name, "tty", 3) == 0;
}

struct altos_usbdev {
	char	*sys;
	char	*tty;
	char	*manufacturer;
	char	*product_name;
	int	serial;	/* AltOS always uses simple integer serial numbers */
	int	idProduct;
	int	idVendor;
};

static char *
usb_tty(char *sys)
{
	char *base;
	int num_configs;
	int config;
	struct dirent **namelist;
	int interface;
	int num_interfaces;
	char endpoint_base[20];
	char *endpoint_full;
	char *tty_dir;
	int ntty;
	char *tty;

	base = cc_basename(sys);
	num_configs = load_hex(sys, "bNumConfigurations");
	num_interfaces = load_hex(sys, "bNumInterfaces");
	for (config = 1; config <= num_configs; config++) {
		for (interface = 0; interface < num_interfaces; interface++) {
			sprintf(endpoint_base, "%s:%d.%d",
				base, config, interface);
			endpoint_full = cc_fullname(sys, endpoint_base);


			/* Check for tty:ttyACMx style names
			 */
			ntty = scandir(endpoint_full, &namelist,
				       dir_filter_tty_colon,
				       alphasort);
			if (ntty > 0) {
				free(endpoint_full);
				tty = cc_fullname("/dev", namelist[0]->d_name + 4);
				free(namelist);
				return tty;
			}

			/* Check for tty/ttyACMx style names
			 */
			tty_dir = cc_fullname(endpoint_full, "tty");
			ntty = scandir(tty_dir, &namelist,
				       dir_filter_tty,
				       alphasort);
			free (tty_dir);
			if (ntty > 0) {
				tty = cc_fullname("/dev", namelist[0]->d_name);
				free(endpoint_full);
				free(namelist);
				return tty;
			}

			/* Check for ttyACMx style names
			 */
			ntty = scandir(endpoint_full, &namelist,
				       dir_filter_tty,
				       alphasort);
			free(endpoint_full);
			if (ntty > 0) {
				tty = cc_fullname("/dev", namelist[0]->d_name);
				free(namelist);
				return tty;
			}

		}
	}
	return NULL;
}

static struct altos_usbdev *
usb_scan_device(char *sys)
{
	struct altos_usbdev *usbdev;
	char *tty;

	tty = usb_tty(sys);
	if (!tty)
		return NULL;
	usbdev = calloc(1, sizeof (struct altos_usbdev));
	if (!usbdev)
		return NULL;
	usbdev->sys = strdup(sys);
	usbdev->manufacturer = load_string(sys, "manufacturer");
	usbdev->product_name = load_string(sys, "product");
	usbdev->serial = load_dec(sys, "serial");
	usbdev->idProduct = load_hex(sys, "idProduct");
	usbdev->idVendor = load_hex(sys, "idVendor");
	usbdev->tty = tty;
	return usbdev;
}

static void
usbdev_free(struct altos_usbdev *usbdev)
{
	free(usbdev->sys);
	free(usbdev->manufacturer);
	free(usbdev->product_name);
	/* this can get used as a return value */
	if (usbdev->tty)
		free(usbdev->tty);
	free(usbdev);
}

#define USB_DEVICES	"/sys/bus/usb/devices"

static int
dir_filter_dev(const struct dirent *d)
{
	const char	*n = d->d_name;
	char	c;

	while ((c = *n++)) {
		if (isdigit(c))
			continue;
		if (c == '-')
			continue;
		if (c == '.' && n != d->d_name + 1)
			continue;
		return 0;
	}
	return 1;
}

struct altos_list {
	struct altos_usbdev	**dev;
	int			current;
	int			ndev;
};

struct altos_list *
altos_list_start(void)
{
	int			e;
	struct dirent		**ents;
	char			*dir;
	struct altos_usbdev	*dev;
	struct altos_list	*devs;
	int			n;

	devs = calloc(1, sizeof (struct altos_list));
	if (!devs)
		return NULL;

	n = scandir (USB_DEVICES, &ents,
		     dir_filter_dev,
		     alphasort);
	if (!n)
		return 0;
	for (e = 0; e < n; e++) {
		dir = cc_fullname(USB_DEVICES, ents[e]->d_name);
		dev = usb_scan_device(dir);
		if (!dev)
			continue;
		free(dir);
		if (devs->dev)
			devs->dev = realloc(devs->dev,
					    (devs->ndev + 1) * sizeof (struct usbdev *));
		else
			devs->dev = malloc (sizeof (struct usbdev *));
		devs->dev[devs->ndev++] = dev;
	}
	free(ents);
	devs->current = 0;
	return devs;
}

PUBLIC struct altos_list *
altos_ftdi_list_start(void)
{
	return altos_list_start();
}

int
altos_list_next(struct altos_list *list, struct altos_device *device)
{
	struct altos_usbdev *dev;
	if (list->current >= list->ndev) {
		return 0;
	}
	dev = list->dev[list->current];
	strcpy(device->name, dev->product_name);
	device->vendor = dev->idVendor;
	device->product = dev->idProduct;
	strcpy(device->path, dev->tty);
	device->serial = dev->serial;
	list->current++;
	return 1;
}

void
altos_list_finish(struct altos_list *usbdevs)
{
	int	i;

	if (!usbdevs)
		return;
	for (i = 0; i < usbdevs->ndev; i++)
		usbdev_free(usbdevs->dev[i]);
	free(usbdevs);
}

#include <dlfcn.h>

static void *libbt;
static int bt_initialized;

static int init_bt(void) {
	if (!bt_initialized) {
		bt_initialized = 1;
		libbt = dlopen("libbluetooth.so.3", RTLD_LAZY);
		if (!libbt)
			printf("failed to find bluetooth library\n");
	}
	return libbt != NULL;
}

#define join(a,b)	a ## b
#define bt_func(name, ret, fail, formals, actuals)			\
	static ret join(altos_, name) formals {				\
				      static ret (*name) formals;	\
				      if (!init_bt()) return fail;	\
				      name = dlsym(libbt, #name);	\
				      if (!name) return fail;		\
				      return name actuals;		\
				      }

bt_func(ba2str, int, -1, (const bdaddr_t *ba, char *str), (ba, str))
#define ba2str altos_ba2str

bt_func(str2ba, int, -1, (const char *str, bdaddr_t *ba), (str, ba))
#define str2ba altos_str2ba

bt_func(hci_read_remote_name, int, -1, (int sock, const bdaddr_t *ba, int len, char *name, int timeout), (sock, ba, len, name, timeout))
#define hci_read_remote_name altos_hci_read_remote_name

bt_func(hci_open_dev, int, -1, (int dev_id), (dev_id))
#define hci_open_dev altos_hci_open_dev

bt_func(hci_get_route, int, -1, (bdaddr_t *bdaddr), (bdaddr))
#define hci_get_route altos_hci_get_route

bt_func(hci_inquiry, int, -1, (int adapter_id, int len, int max_rsp, const uint8_t *lap, inquiry_info **devs, long flags), (adapter_id, len, max_rsp, lap, devs, flags))
#define hci_inquiry altos_hci_inquiry

bt_func(sdp_connect, sdp_session_t *, 0, (const bdaddr_t *src, const bdaddr_t *dst, uint32_t flags), (src, dst, flags))
#define sdp_connect altos_sdp_connect

bt_func(sdp_uuid16_create, uuid_t *, 0, (uuid_t *uuid, uint16_t data), (uuid, data))
#define sdp_uuid16_create altos_sdp_uuid16_create

bt_func(sdp_list_append, sdp_list_t *, 0, (sdp_list_t *list, void *d), (list, d))
#define sdp_list_append altos_sdp_list_append

bt_func(sdp_service_search_attr_req, int, -1, (sdp_session_t *session, const sdp_list_t *search, sdp_attrreq_type_t reqtype, const sdp_list_t *attrid_list, sdp_list_t **rsp_list), (session, search, reqtype, attrid_list, rsp_list))
#define sdp_service_search_attr_req altos_sdp_service_search_attr_req

bt_func(sdp_uuid_to_proto, int, 0, (uuid_t *uuid), (uuid))
#define sdp_uuid_to_proto altos_sdp_uuid_to_proto

bt_func(sdp_get_access_protos, int, 0, (const sdp_record_t *rec, sdp_list_t **protos), (rec, protos))
#define sdp_get_access_protos altos_sdp_get_access_protos

bt_func(sdp_get_proto_port, int, 0, (const sdp_list_t *list, int proto), (list, proto))
#define sdp_get_proto_port altos_sdp_get_proto_port

bt_func(sdp_close, int, 0, (sdp_session_t *session), (session))
#define sdp_close altos_sdp_close

struct altos_bt_list {
	inquiry_info	*ii;
	int		sock;
	int		dev_id;
	int		rsp;
	int		num_rsp;
};

#define INQUIRY_MAX_RSP	255

struct altos_bt_list *
altos_bt_list_start(int inquiry_time)
{
	struct altos_bt_list	*bt_list;

	bt_list = calloc(1, sizeof (struct altos_bt_list));
	if (!bt_list)
		goto no_bt_list;

	bt_list->ii = calloc(INQUIRY_MAX_RSP, sizeof (inquiry_info));
	if (!bt_list->ii)
		goto no_ii;
	bt_list->dev_id = hci_get_route(NULL);
	if (bt_list->dev_id < 0)
		goto no_dev_id;

	bt_list->sock = hci_open_dev(bt_list->dev_id);
	if (bt_list->sock < 0)
		goto no_sock;

	bt_list->num_rsp = hci_inquiry(bt_list->dev_id,
				       inquiry_time,
				       INQUIRY_MAX_RSP,
				       NULL,
				       &bt_list->ii,
				       IREQ_CACHE_FLUSH);
	if (bt_list->num_rsp < 0)
		goto no_rsp;

	bt_list->rsp = 0;
	return bt_list;

no_rsp:
	close(bt_list->sock);
no_sock:
no_dev_id:
	free(bt_list->ii);
no_ii:
	free(bt_list);
no_bt_list:
	return NULL;
}

int
altos_bt_list_next(struct altos_bt_list *bt_list,
		   struct altos_bt_device *device)
{
	inquiry_info	*ii;

	if (bt_list->rsp >= bt_list->num_rsp)
		return 0;

	ii = &bt_list->ii[bt_list->rsp];
	if (ba2str(&ii->bdaddr, device->addr) < 0)
		return 0;
	memset(&device->name, '\0', sizeof (device->name));
	if (hci_read_remote_name(bt_list->sock, &ii->bdaddr,
				 sizeof (device->name),
				 device->name, 0) < 0) {
		strcpy(device->name, "[unknown]");
	}
	bt_list->rsp++;
	return 1;
}

void
altos_bt_list_finish(struct altos_bt_list *bt_list)
{
	close(bt_list->sock);
	free(bt_list->ii);
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
	struct sockaddr_rc 	addr = { 0 };
	int			status, i;
	struct altos_file_posix	*file;
	sdp_session_t		*session = NULL;
	int			channel = 0;

	if (str2ba(device->addr, &addr.rc_bdaddr) < 0) {
		altos_set_last_posix_error();
		goto no_file;
	}

	/* Try the built-in vendor list */
	channel = altos_bt_port(device);

	/* Not present, try to discover an RFCOMM service */
	if (channel == 0) {
		/*
		 * Search for the RFCOMM service to get the right channel
		 */
		session = sdp_connect(BDADDR_ANY, &addr.rc_bdaddr, SDP_RETRY_IF_BUSY);

		if (session) {
			static const uint8_t svc_uuid_int[] = {
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0x11, 0x01
			};
			int			err;
			uuid_t			svc_uuid;
			uint32_t		range;
			sdp_list_t		*search_list, *attrid_list;
			sdp_list_t		*response_list = NULL, *r;
			sdp_uuid16_create(&svc_uuid, PUBLIC_BROWSE_GROUP);
			search_list = sdp_list_append(NULL, &svc_uuid);

			range = 0x0000ffff;
			attrid_list = sdp_list_append(NULL, &range);

			err = sdp_service_search_attr_req(session, search_list,
							  SDP_ATTR_REQ_RANGE, attrid_list, &response_list);

			if (err >= 0) {
				for (r = response_list; r; r = r->next) {
					sdp_record_t *rec = (sdp_record_t*) r->data;
					sdp_list_t *proto_list;
					sdp_list_t *access = NULL;
					int proto;

					proto = sdp_uuid_to_proto(&rec->svclass);

					if (proto == SERIAL_PORT_SVCLASS_ID) {
						sdp_get_access_protos(rec, &access);
						if (access) {
							int this_chan = sdp_get_proto_port(access, RFCOMM_UUID);
							if (this_chan) {
								printf("found service on channel %d\n", this_chan);
								channel = this_chan;
							}
						}
					}
				}
			}

			/* Leave the session open so we don't disconnect from the device before opening
			 * the RFCOMM channel
			 */
		}
	}

	/* Still nothing, try the default */
	if (channel == 0)
		channel = BT_PORT_DEFAULT;

	/* Connect to the channel */
	file = calloc(1, sizeof (struct altos_file_posix));
	if (!file) {
		errno = ENOMEM;
		altos_set_last_posix_error();
		goto no_file;
	}
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = channel;

	for (i = 0; i < 5; i++) {
		file->fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
		if (file->fd < 0) {
			altos_set_last_posix_error();
			goto no_sock;
		}

		status = connect(file->fd,
				 (struct sockaddr *)&addr,
				 sizeof(addr));
		if (status >= 0 || errno != EBUSY)
			break;
		close(file->fd);
		usleep(100 * 1000);
	}


	if (status < 0) {
		altos_set_last_posix_error();
		goto no_link;
	}

	if (session)
		sdp_close(session);

	usleep(100 * 1000);

#ifdef USE_POLL
	pipe(file->pipe);
#else
	file->out_fd = dup(file->fd);
#endif
	return &file->file;
no_link:
	close(file->fd);
no_sock:
	free(file);
no_file:
	if (session)
		sdp_close(session);
	return NULL;
}

