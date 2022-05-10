/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
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

#include <jansson.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#define ALTOS_MAP_PORT	16717

#define MISSING		0x7fffffff

#define ALTOS_MAP_PROTOCOL_VERSION	"1.0.0"

static char *
reason_string(int code)
{
	switch (code) {
	case 200:
		return "OK";
	case 400:
		return "Bad Request";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 408:
		return "Request Timeout";
	default:
		return "Failure";
	}
}

static void
write_status(int status)
{
	printf("Status: %d %s\n", status, reason_string(status));
}

static void
write_type(char * type)
{
	printf("Content-Type: %s\n", type);
}

static void
fail(int status, char *format, ...)
{
	va_list ap;

	write_status(status);
	write_type("text/html");
	printf("\n");
	printf("<html>\n");
	printf("<head><title>Map Fetch Failure</title></head>\n");
	printf("<body>\n");
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("</body>\n");
	printf("</html>\n");
	exit(1);
}

static char *
getenv_copy(const char *name)
{
	const char *value = getenv(name);

	if (!value)
		return NULL;

	return strdup(value);
}

static double
parse_double(char *string)
{
	char *end;
	double value;

	value = strtod(string, &end);
	if (*end)
		fail(400, "Invalid double %s", string);
	return value;
}

static int
parse_int(char *string)
{
	char *end;
	long int value;

	value = strtol(string, &end, 10);
	if (*end)
		fail(400, "Invalid int %s", string);
	if (value < INT_MIN || INT_MAX < value)
		fail(400, "Int value out of range %ld", value);
	return (int) value;
}


static int
connect_service(void)
{
	struct sockaddr_in	altos_map_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(ALTOS_MAP_PORT),
		.sin_addr = {
			.s_addr = htonl(INADDR_LOOPBACK),
		},
	};

	int	s = socket(AF_INET, SOCK_STREAM, 0);

	if (s < 0)
		return -1;

	if (connect (s, (const struct sockaddr *) &altos_map_addr, sizeof (altos_map_addr)) < 0) {
		close (s);
		return -1;
	}

	return s;
}

int main(int argc, char **argv)
{

	char *query_string = getenv_copy("QUERY_STRING");

	if (query_string == NULL)
		fail(400, "%s", "Missing query string");

	char *remote_addr = getenv_copy("REMOTE_ADDR");

	if (remote_addr == NULL)
		fail(400, "%s", "Missing remote address");

	double	lon = MISSING;
	double	lat = MISSING;
	int	zoom = MISSING;
	char	*version = NULL;

	char	*query, *query_save = NULL;
	char	*query_start = query_string;

	while ((query = strtok_r(query_start, "&", &query_save)) != NULL) {
		query_start = NULL;

		char	*token, *token_save = NULL;
		char	*token_start = query;

		char	*name = NULL;
		char	*value = NULL;

		while ((token = strtok_r(token_start, "=", &token_save)) != NULL) {
			token_start = NULL;
			if (name == NULL)
				name = token;
			else if (value == NULL)
				value = token;
			else
				break;
		}

		if (name && value) {
			if (!strcmp(name, "lon"))
				lon = parse_double(value);
			else if (!strcmp(name, "lat"))
				lat = parse_double(value);
			else if (!strcmp(name, "zoom"))
				zoom = parse_int(value);
			else if (!strcmp(name, "version"))
				version = value;
			else
				fail(400, "Extra query param \"%s\"", query);
		}
	}

	if (version != NULL) {
		printf("Content-Type: text/plain\n");
		printf("\n");
		printf("%s\n", ALTOS_MAP_PROTOCOL_VERSION);
		return 0;
	}
	if (lon == MISSING)
		fail(400, "Missing longitude");
	if (lat == MISSING)
		fail(400, "Missing latitude");
	if (zoom == MISSING)
		fail(400, "Missing zoom");

	int	s = -1;
	int	tries = 0;

	while (tries < 10 && s < 0) {
		s = connect_service();
		if (s < 0) {
			usleep(100 * 1000);
			tries++;
		}
	}

	if (s < 0)
		fail(408, "Cannot connect AltOS map daemon");

	FILE	*sf = fdopen(s, "r+");

	if (sf == NULL)
		fail(400, "allocation failure");

	json_t *request = json_pack("{s:f s:f s:i s:s}", "lat", lat, "lon", lon, "zoom", zoom, "remote_addr", remote_addr);

	if (request == NULL)
		fail(400, "Cannot create JSON request");

	if (json_dumpf(request, sf, 0) < 0)
		fail(400, "Cannot write JSON request");

	fflush(sf);

	json_error_t	error;
	json_t 		*reply = json_loadf(sf, 0, &error);

	if (!reply)
		fail(400, "Cannot read JSON reply");

	int	status;

	if (json_unpack(reply, "{s:i}", "status", &status) < 0)
		fail(400, "No status returned");

	if (status != 200)
		fail(status, "Bad cache status");

	char	*filename, *content_type;

	if (json_unpack(reply, "{s:s s:s}", "filename", &filename, "content_type", &content_type) < 0)
		fail(400, "JSON reply parse failure");

	int	fd = open(filename, O_RDONLY);

	if (fd < 0)
		fail(400, "%s: %s", filename, strerror(errno));

	struct stat	statb;

	if (fstat(fd, &statb) < 0)
		fail(400, "%s: %s", filename, strerror(errno));

	printf("Content-Type: %s\n", content_type);
	printf("Content-Length: %lu\n", (unsigned long) statb.st_size);
	printf("\n");
	fflush(stdout);

	char	buf[4096];
	ssize_t	bytes_read;

	while ((bytes_read = read(fd, buf, sizeof (buf))) > 0) {
		ssize_t total_write = 0;
		while (total_write < bytes_read) {
			ssize_t bytes_write = write(1, buf + total_write, bytes_read - total_write);
			if (bytes_write <= 0)
				return 1;
			total_write += bytes_write;
		}
	}
	if (bytes_read < 0)
		return 1;
	return 0;
}
