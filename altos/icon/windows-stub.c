/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
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

/* A windows stub program to launch a java program with suitable parameters
 *
 * Given that the name of this exe is altusmetrum-foo.exe living in directory bar, and
 * that it was run with 'args' extra command line parameters, run:
 *
 *	javaw.exe -Djava.library.path="bar" -jar "bar/foo-fat.jar" args
 */

#define _UNICODE
#define UNICODE
#include <stdlib.h>
#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdarg.h>
#include <shlwapi.h>

/* Concatenate a list of strings together
 */
static LPTSTR
wcsbuild(LPTSTR first, ...)
{
	va_list	args;
	int	len;
	LPTSTR	buf;
	LPTSTR	arg;

	buf = wcsdup(first);
	va_start(args, first);
	while ((arg = va_arg(args, LPTSTR)) != NULL) {
		len = wcslen(buf) + wcslen(arg) + 1;
		buf = realloc(buf, len * sizeof (wchar_t));
		wcscat(buf, arg);
	}
	va_end(args);
	return buf;
}

/* Quote a single string, taking care to escape embedded quote and
 * backslashes within
 */
static LPTSTR
quote_arg(LPTSTR arg)
{
	LPTSTR	result;
	LPTSTR	in, out;
	int	out_len = 3;	/* quotes and terminating null */

	/* Find quote and backslashes */
	for (in = arg; *in; in++) {
		switch (*in) {
		case '"':
		case '\\':
			out_len += 2;
			break;
		default:
			out_len++;
			break;
		}
	}

	result = malloc ((out_len + 1) * sizeof (wchar_t));
	out = result;
	*out++ = '"';
	for (in = arg; *in; in++) {
		switch (*in) {
		case '"':
		case '\\':
			*out++ = '\\';
			break;
		}
		*out++ = *in;
	}
	*out++ = '"';
	*out++ = '\0';
	return result;
}

/* Construct a single string from a list of arguments
 */
static LPTSTR
quote_args(LPTSTR *argv, int argc)
{
	LPTSTR	result = NULL, arg;
	int	i;

	result = malloc(1 * sizeof (wchar_t));
	result[0] = '\0';
	for (i = 0; i < argc; i++) {
		arg = quote_arg(argv[i]);
		result = realloc(result, (wcslen(result) + 1 + wcslen(arg) + 1) * sizeof (wchar_t));
		wcscat(result, L" ");
		wcscat(result, arg);
		free(arg);
	}
	return result;
}

/* Return the directory portion of the provided file
 */
static LPTSTR
get_dir(LPTSTR file)
{
	DWORD	len = GetFullPathName(file, 0, NULL, NULL);
	LPTSTR	full = malloc (len * sizeof (wchar_t));
	GetFullPathName(file, len, full, NULL);
	PathRemoveFileSpec(full);
	return full;
}

/* Convert a .exe name into a -fat.jar name, starting
 * by computing the complete path name of the source filename
 */
static LPTSTR
make_jar(LPTSTR file)
{
	DWORD	len = GetFullPathName(file, 0, NULL, NULL);
	LPTSTR	full = malloc (len * sizeof (wchar_t));
	LPTSTR	base_part;
	LPTSTR	jar;
	LPTSTR	dot;
	GetFullPathName(file, len, full, &base_part);
	static const wchar_t head[] = L"altusmetrum-";

	if (wcsncmp(base_part, head, wcslen(head)) == 0)
		base_part += wcslen(head);
	dot = wcsrchr(base_part, '.');
	if (dot)
		*dot = '\0';
	jar = wcsdup(base_part);
	PathRemoveFileSpec(full);
	return wcsbuild(full, L"\\", jar, L"-fat.jar", NULL);
}

/* Build the complete command line from the pieces
 */
static LPTSTR
make_cmd(LPTSTR dir, LPTSTR jar, LPTSTR quote_args)
{
	LPTSTR	quote_dir = quote_arg(dir);
	LPTSTR	quote_jar = quote_arg(jar);
	LPTSTR	cmd;

	cmd = wcsbuild(L"javaw.exe -Djava.library.path=", quote_dir, L" -jar ", quote_jar, quote_args, NULL);
	free(quote_jar);
	free(jar);
	free(quote_dir);
	return cmd;
}

int WINAPI
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line_a, int cmd_show)
{
	STARTUPINFO		startup_info;
	PROCESS_INFORMATION	process_information;
	BOOL			result;
	wchar_t			*command_line;
	int			argc;
	LPTSTR			*argv = CommandLineToArgvW(GetCommandLine(), &argc);
	LPTSTR			my_dir;
	LPTSTR			my_jar;
	LPTSTR			args = quote_args(argv + 1, argc - 1);

	my_dir = get_dir(argv[0]);
	my_jar = make_jar(argv[0]);
	command_line = make_cmd(my_dir, my_jar, args);
	memset(&startup_info, '\0', sizeof startup_info);
	startup_info.cb = sizeof startup_info;
	result = CreateProcess(NULL,
			       command_line,
			       NULL,
			       NULL,
			       FALSE,
			       CREATE_NO_WINDOW,
			       NULL,
			       NULL,
			       &startup_info,
			       &process_information);
	if (result) {
		CloseHandle(process_information.hProcess);
		CloseHandle(process_information.hThread);
	}
	exit(0);
}
