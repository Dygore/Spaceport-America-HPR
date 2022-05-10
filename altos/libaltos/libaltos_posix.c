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

void
altos_set_last_posix_error(void)
{
	altos_set_last_error(errno, strerror(errno));
}

PUBLIC struct altos_file *
altos_open(struct altos_device *device)
{
	struct altos_file_posix	*file = calloc (sizeof (struct altos_file_posix), 1);
	int			ret;
	struct termios		term;

	if (!file) {
		altos_set_last_posix_error();
		return NULL;
	}

//	altos_set_last_error(12, "yeah yeah, failed again");
//	free(file);
//	return NULL;

	file->fd = open(device->path, O_RDWR | O_NOCTTY);
	if (file->fd < 0) {
		altos_set_last_posix_error();
		free(file);
		return NULL;
	}
#ifdef USE_POLL
	pipe(file->pipe);
#else
	file->out_fd = open(device->path, O_RDWR | O_NOCTTY);
	if (file->out_fd < 0) {
		altos_set_last_posix_error();
		close(file->fd);
		free(file);
		return NULL;
	}
#endif
	ret = tcgetattr(file->fd, &term);
	if (ret < 0) {
		altos_set_last_posix_error();
		close(file->fd);
#ifndef USE_POLL
		close(file->out_fd);
#endif
		free(file);
		return NULL;
	}
	cfmakeraw(&term);
	cfsetospeed(&term, B9600);
	cfsetispeed(&term, B9600);
#ifdef USE_POLL
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
#else
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;
#endif
	ret = tcsetattr(file->fd, TCSAFLUSH, &term);
	if (ret < 0) {
		altos_set_last_posix_error();
		close(file->fd);
#ifndef USE_POLL
		close(file->out_fd);
#endif
		free(file);
		return NULL;
	}
	return &file->file;
}

PUBLIC void
altos_close(struct altos_file *file_common)
{
	struct altos_file_posix	*file = (struct altos_file_posix *) file_common;

	if (file->fd != -1) {
		int	fd = file->fd;
		file->fd = -1;
#ifdef USE_POLL
		write(file->pipe[1], "\r", 1);
#else
		close(file->out_fd);
		file->out_fd = -1;
#endif
		close(fd);
	}
}

PUBLIC int
altos_flush(struct altos_file *file_common)
{
	struct altos_file_posix	*file = (struct altos_file_posix *) file_common;

	if (file->file.out_used && 0) {
		printf ("flush \"");
		fwrite(file->file.out_data, 1, file->file.out_used, stdout);
		printf ("\"\n");
	}
	while (file->file.out_used) {
		int	ret;

		if (file->fd < 0)
			return -EBADF;
#ifdef USE_POLL
		ret = write (file->fd, file->file.out_data, file->file.out_used);
#else
		ret = write (file->out_fd, file->file.out_data, file->file.out_used);
#endif
		if (ret < 0) {
			altos_set_last_posix_error();
			return -altos_last_error.code;
		}
		if (ret) {
			memmove(file->file.out_data, file->file.out_data + ret,
				file->file.out_used - ret);
			file->file.out_used -= ret;
		}
	}
	return 0;
}


#ifdef USE_POLL
#include <poll.h>
#endif

int
altos_fill(struct altos_file *file_common, int timeout)
{
	struct altos_file_posix	*file = (struct altos_file_posix *) file_common;

	int		ret;
#ifdef USE_POLL
	struct pollfd	fd[2];
#endif
	if (timeout == 0)
		timeout = -1;
	while (file->file.in_read == file->file.in_used) {
		if (file->fd < 0)
			return LIBALTOS_ERROR;
#ifdef USE_POLL
		fd[0].fd = file->fd;
		fd[0].events = POLLIN|POLLERR|POLLHUP|POLLNVAL;
		fd[1].fd = file->pipe[0];
		fd[1].events = POLLIN;
		ret = poll(fd, 2, timeout);
		if (ret < 0) {
			altos_set_last_posix_error();
			return LIBALTOS_ERROR;
		}
		if (ret == 0)
			return LIBALTOS_TIMEOUT;

		if (fd[0].revents & (POLLHUP|POLLERR|POLLNVAL))
			return LIBALTOS_ERROR;
		if (fd[0].revents & POLLIN)
#endif
		{
			ret = read(file->fd, file->file.in_data, USB_BUF_SIZE);
			if (ret < 0) {
				altos_set_last_posix_error();
				return LIBALTOS_ERROR;
			}
			file->file.in_read = 0;
			file->file.in_used = ret;
#ifndef USE_POLL
			if (ret == 0 && timeout > 0)
				return LIBALTOS_TIMEOUT;
#endif
		}
	}
	if (file->file.in_used && 0) {
		printf ("fill \"");
		fwrite(file->file.in_data, 1, file->file.in_used, stdout);
		printf ("\"\n");
	}
	return 0;
}

#include <time.h>

void
altos_pause_one_second(void)
{
	struct timespec delay = { .tv_sec = 1, .tv_nsec = 0 };
	nanosleep(&delay, NULL);
}
