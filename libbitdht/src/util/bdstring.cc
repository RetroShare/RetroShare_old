/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (c) 2012, RetroShare Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include "bdstring.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <stdarg.h>
#include <stdlib.h>
#endif
#include <stdio.h>

#ifdef _WIN32
// asprintf() and vasprintf() are missing in Win32
static int vasprintf(char **sptr, const char *fmt, va_list argv)
{
	int wanted = __mingw_vsnprintf(*sptr = NULL, 0, fmt, argv);
	if ((wanted > 0) && ((*sptr = (char*) malloc(wanted + 1)) != NULL)) {
		return __mingw_vsprintf(*sptr, fmt, argv);
	}

	return wanted;
}

//static int asprintf(char **sptr, const char *fmt, ...)
//{
//	int retval;
//	va_list argv;
//	va_start( argv, fmt );
//	retval = vasprintf(sptr, fmt, argv);
//	va_end(argv);
//	return retval;
//}
#endif

int bd_sprintf(std::string &str, const char *fmt, ...)
{
	char *buffer = NULL;
	va_list ap;

	va_start(ap, fmt);
	int retval = vasprintf(&buffer, fmt, ap);
	va_end(ap);

	if (retval >= 0) {
		if (buffer) {
			str = buffer;
			free(buffer);
		} else {
			str.clear();
		}
	} else {
		str.clear();
	}

	return retval;
}

int bd_sprintf_append(std::string &str, const char *fmt, ...)
{
	va_list ap;
	char *buffer = NULL;

	va_start(ap, fmt);
	int retval = vasprintf(&buffer, fmt, ap);
	va_end(ap);

	if (retval >= 0) {
		if (buffer) {
			str.append(buffer);
			free(buffer);
		}
	}

	return retval;
}
