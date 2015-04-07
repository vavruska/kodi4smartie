/*
Kodi4Smartie - A DLL to display information from Kodi on a LCD character display

Copyright (C) 2015  Chris Vavruska

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "config.hpp"

FILE	*log_fd = NULL;
bool	log_init = false;

#define LOG_FILE "plugins/kodi4smartie.log"

void log(char *format, ...)
{
	va_list arguments;
	if (log_init && fopen_s(&log_fd, LOG_FILE, "a") == 0)
	{
		time_t rawtime;
		struct tm timeinfo;
		char timestr[30];

		time(&rawtime);
		localtime_s(&timeinfo, &rawtime);
		strftime(timestr, sizeof(timestr), "%m/%d/%y %H:%M", &timeinfo);

		fprintf(log_fd, "%s: ", timestr);
		va_start(arguments, format);
		vfprintf(log_fd, format, arguments);
		va_end(arguments);
		fprintf(log_fd, "\n");
		fclose(log_fd);
	}
}

void end_logging()
{
	if (log_init && fopen_s(&log_fd, LOG_FILE, "a") == 0)
	{
		log("Close");
		fclose(log_fd);
		log_init = false;
	}
}

__declspec(dllexport) void init_logging()
{
	if (get_config(cLOGGING))
	{
		char access[10] = { "a" };
		log_init = true;

		if (get_config(cNEWFILE))
		{
			strncpy_s(access, "w", sizeof(access));
		}
	
		//if (fopen_s(&log_fd, "plugins/kodi_smartie.log", "w") == 0)
		if (fopen_s(&log_fd, LOG_FILE, access) == 0)
		{
			fclose(log_fd);
		}
			log("Init");
	}

}

