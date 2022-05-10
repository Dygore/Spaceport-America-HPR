/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/l2cap.h>
#include <poll.h>

#define ATT_OP_MTU_REQ		0x02
#define ATT_OP_MTU_RESP		0x03
#define ATT_OP_WRITE_CMD	0x52
#define ATT_OP_HANDLE_NOTIFY	0x1b
#define CID_ATT			0x0004
#define TX_ENDPOINT		0x003a
#define RX_ENDPOINT		0x0037
#define RX_NOTIFY		0x0038

int
main(int argc, char **argv)
{
	int sk;
	int psm;
	struct sockaddr_l2 src_addr = { 0 };
	struct sockaddr_l2 dst_addr = { 0 };
	char buf[1024];
	struct pollfd	fd[2];
	int n, i;
	char *btaddr;
	int	mtu;

	btaddr = argc > 1 ? argv[1] : "D8:80:39:F3:4E:A5";

	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sk < 0) {
		perror("socket");
		exit(1);
	}

	src_addr.l2_family = AF_BLUETOOTH;
	/* Leave src_addr.l2_bdaddr all zeros */
	src_addr.l2_cid = htobs(CID_ATT);
	src_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
	if (bind(sk, (struct sockaddr *) &src_addr, sizeof (src_addr)) < 0) {
		perror("bind");
		exit(1);
	}

	dst_addr.l2_family = AF_BLUETOOTH;
	str2ba(btaddr, &dst_addr.l2_bdaddr);
	dst_addr.l2_cid = htobs(CID_ATT);
	dst_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;

	if (connect(sk, (struct sockaddr *) &dst_addr, sizeof(dst_addr)) < 0) {
		perror("connect");
		exit(1);
	}

	buf[0] = ATT_OP_MTU_REQ;
	buf[1] = sizeof(buf) & 0xff;
	buf[2] = sizeof(buf) >> 8;
	n = write(sk, buf, 3);
	if (n != 3) {
		perror("write mtu\n");
		exit(1);
	}

	fd[0].fd = sk;
	fd[0].events = POLLIN;
	for (;;) {
		n = poll(fd, 1, 3000);
		if (n <= 0) {
			printf("timeout waiting for MTU response\n");
			exit(1);
		}
		if (fd[0].revents & POLLIN) {
			n = read(sk, buf, sizeof(buf));
			printf("read %d\n", n);
			for (i = 0; i < n; i++)
				printf("%02x\n", buf[i]);
			if (buf[0] == ATT_OP_MTU_RESP) {
				mtu = (buf[1] & 0xff) | ((buf[2] & 0xff) << 8);
				break;
			}
		}
	}
	printf("mtu %d\n", mtu);

	buf[0] = ATT_OP_WRITE_CMD;
	buf[1] = RX_NOTIFY & 0xff;
	buf[2] = RX_NOTIFY >> 8;
	buf[3] = 1;
	n = write(sk, buf, 4);
	if (n != 4) {
		perror("write notify");
		exit(1);
	}

	fd[0].fd = 0;
	fd[0].events = POLLIN;
	fd[1].fd = sk;
	fd[1].events = POLLIN;

	for (;;) {
		n = poll(fd, 2, -1);
		if (n == 0)
			continue;
		if (fd[0].revents & POLLIN) {
			char	*b;
			n = read(0, buf+3, sizeof(buf)-3);
			if (n < 0) {
				perror("read stdin");
				exit(1);
			}
			if (n == 0)
				break;

			b = buf;
			while (n > 0) {
				int this = n;
				if (this > mtu - 3)
					this = mtu - 3;

				b[0] = ATT_OP_WRITE_CMD;
				b[1] = TX_ENDPOINT;
				b[2] = TX_ENDPOINT >> 8;
				if (write(sk, b, this + 3) != this + 3) {
					perror("write sk");
					exit(1);
				}
				b += this;
				n -= this;
			}
		}
		if (fd[1].revents & POLLIN) {
			uint16_t	ch;

			n = read(sk, buf, sizeof(buf));
			if (n < 0) {
				perror("read sk");
				exit(1);
			}
			if (n == 0)
				continue;
			ch = buf[1] | (buf[2] << 8);
			switch (buf[0]) {
			case ATT_OP_HANDLE_NOTIFY:
				if (ch == RX_ENDPOINT)
					write(1, buf+3, n-3);
				break;
			}
		}
		if (fd[1].revents & (POLLERR|POLLHUP))
			break;
	}
	close(sk);

	return 0;
}
