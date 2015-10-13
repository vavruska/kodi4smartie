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

#include <windows.h>
#include <Psapi.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include "display.hpp"
#include "config.hpp"
#include "logging.hpp"
#include "interface.hpp"
#include "dll.hpp"
#include <mutex> 


char line1[255];
char line2[255];
char line4[255];
char line5[255];
char line6[255];
char line7[255];
char line8[255];
extern bool connected;
extern bool connecting;
HANDLE connect_timer;

#define KODI_DLL_VERSION_MAJ "1"
#define KODI_DLL_VERSION_MIN "1"
#define KODI_DLL_BUILD_DATE __DATE__

//forward declaration
void CALLBACK try_connect(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
string center(string in, unsigned int width);



/*********************************************************
*         SmartieInit                                   *
*********************************************************/
__declspec(dllexport)  void __stdcall  SmartieInit()
{
	init_config();
	init_logging();
	log("SmartieInit");
	
	ws_connect();
}

/*********************************************************
*         SmartieFini                                   *
*********************************************************/
__declspec(dllexport)  void __stdcall  SmartieFini()
{
	stop_timers();
	//ws_close();
	log("SmartieFini");

}

/*********************************************************
*         GetMinRefreshInterval                         *
*********************************************************/
__declspec(dllexport)  int __stdcall  GetMinRefreshInterval()
{
	//
	// Define the minimum interval that a screen should get fresh
	// data from our plugin.
	// The actual value used by Smartie will be the higher of this
	// value and of the "dll check interval" setting
	// on the Misc tab.  [This function is optional, Smartie will
	// assume 300ms if it is not provided.]
	// 
	return 300; // 300 ms
}


/*********************************************************
*         Function 1                                     *
*  Returns the first display line       			     *
*********************************************************/
__declspec(dllexport)  char * __stdcall  function1(char *param1, char *param2)
{
	string display;

	if (!connected && !connecting)// && is_kodi_running())
	{
		connecting = true;
		CreateTimerQueueTimer(&connect_timer, NULL, try_connect, NULL, get_config(cCONNECT_DELAY) * 1000, 0, 0);
	}

	if (get_config(cDISABLE_ICON) == 0)
	{
		display = get_icon(7) + string(" ");
	}
	if ((get_mode().compare("movie") == 0) ||
		(get_mode().compare("episode") == 0) ||
		(get_mode().compare("song") == 0) ||
		(get_mode().compare("picture") == 0))
	{
		display += get_title();
	}
	else if (get_mode().compare("channel") == 0)
	{
		display += get_tv_info();
	}
	else if (get_mode().compare("volume") == 0)
	{
		display += center(get_config_str(sVOLUME), get_config(cLCD_WIDTH) - (2 * (get_config(cDISABLE_ICON) == 0)));
	}
	else
	{
		display += center(string(get_config_str(sWELCOME)), get_config(cLCD_WIDTH) - (2 * (get_config(cDISABLE_ICON) == 0)));
	}

	strcpy_s(line1, display.c_str());

	return line1;
}



/*********************************************************
*         Function 2                                     *
*  Returns the second display line       			     *
*********************************************************/
__declspec(dllexport)  char * __stdcall  function2(char *param1, char *param2)
{
	string display;

	if (!connected && !connecting)// && is_kodi_running())
	{
		connecting = true;
		CreateTimerQueueTimer(&connect_timer, NULL, try_connect, NULL, get_config(cCONNECT_DELAY) * 1000, 0, 0);
	}
	if (get_mode().compare("movie") == 0)
	{
		display = get_time();
	}
	else if (get_mode().compare("episode") == 0)
	{
		if (get_extra_info() == 1)
		{
			display = get_episode_info();
		}
		else
		{
			display = get_time();
		}
	}
	else if (get_mode().compare("channel") == 0)
	{
		if (get_extra_info() == 1)
		{
			display = string("$Center(") + get_title() + string(")");
		}
		else
		{
			display = get_time();
		}

	}
	else if (get_mode().compare("song") == 0)
	{
		if (get_extra_info() > 0)
		{
			display = get_song_info();
		}
		else
		{
			display = get_time();
		}
	}
	else if (get_mode().compare("volume") == 0)
	{
		if (get_config(cUSE_BARS) == 1)
		{
			display = string("$Bar(") + std::to_string(get_volume()) + string(",100,") + 
				std::to_string(get_config(cLCD_WIDTH)) + string(")");
		}
		else
		{
			display = string("$Center(") + std::to_string(get_volume()) + string(" / 100)");
		}
	}
	else
	{
		display = string("$Center($Time(")+ string(get_config_str(cTIME_FORMAT)) + string("))");
	}

	strcpy_s(line2, display.c_str());

	return line2;
}

//check to see if kodi is running
//only works if we are running on the same machine as Kodi.exe
__declspec(dllexport)  char * __stdcall  function3(char *param1, char *param2)
{
	if (is_kodi_running())
	{
		return "1";
	}
	return "0";
}

//return the dll name and version
__declspec(dllexport)  char * __stdcall  function4(char *param1, char *param2)
{
	string version = string("$Center(") + string(" v") +
		string(KODI_DLL_VERSION_MAJ) + string(".") +
		string(KODI_DLL_VERSION_MIN) + " " +
		string(KODI_DLL_BUILD_DATE) + string(")");

	strcpy_s(line4, version.c_str());
	return line4;
}

//custom commands
// param1= kodi json method
// param2= kodi json item to get. If item is in the format of item#subitem then 
//		   item will be requested but the subitem will be returned.
//
// Examples: 
// function5("Player.GetProperties","percentage") returns the percentage done
//		from the current title being played.
// function5("Player.GetProperties","time#minutes") returns the minutes into the
//		current title being played.
// function5("Player.GetActivePlayers","#type") returns the current player type.
//
__declspec(dllexport)  char * __stdcall  function5(char *param1, char *param2)
{ 

	if (!connected && !connecting)// && is_kodi_running())
	{
		connecting = true;
		CreateTimerQueueTimer(&connect_timer, NULL, try_connect, NULL, get_config(cCONNECT_DELAY) * 1000, 0, 0);
	}
	if (connected)
	{
		char *method = _strdup(param1);
		char *item = _strdup(param2);
		std::string ret = get_custom_data(method, item);
		strcpy_s(line5, ret.c_str());
		free(method);
		free(item);
		return line5;
	}
	else
	{
		if (is_kodi_running())
		{
			return "";
		}
		else
		{
			strcpy_s(line5, get_config_str(sKNF));
			return line5;
		}
	}
}

//display icon
__declspec(dllexport)  char * __stdcall  function6(char *param1, char *param2)
{
	string display;

	display = get_icon(7);

	strcpy_s(line6, display.c_str());

	return line6;
}

//display labels
__declspec(dllexport)  char * __stdcall  function7(char *param1, char *param2)
{

	if (!connected && !connecting)// && is_kodi_running())
	{
		connecting = true;
		CreateTimerQueueTimer(&connect_timer, NULL, try_connect, NULL, get_config(cCONNECT_DELAY) * 1000, 0, 0);
	}
	if (connected)
	{
		char *method = _strdup(param1);
		char *item = _strdup(param2);
		std::string ret = get_custom_label(method, item);
		strcpy_s(line7, ret.c_str());
		free(method);
		free(item);
		return line7;
	}
	else
	{
		if (is_kodi_running())
		{
			return "";
		}
		else
		{
			strcpy_s(line7, get_config_str(sKNF));
			return line7;
		}
	}
}

//display labels
__declspec(dllexport)  char * __stdcall  function8(char *param1, char *param2)
{

	if (!connected && !connecting)// && is_kodi_running())
	{
		connecting = true;
		CreateTimerQueueTimer(&connect_timer, NULL, try_connect, NULL, get_config(cCONNECT_DELAY) * 1000, 0, 0);
	}
	if (connected)
	{
		char *method = _strdup(param1);
		char *item = _strdup(param2);
		std::string ret = get_custom_label(method, item);
		strcpy_s(line7, ret.c_str());
		free(method);
		free(item);
		return "";
	}
	else
	{
		return "";
	}
}

//display state
//	kodi_not_running(0),none(1), play(2), stop, pause, ff, rew, mute,
__declspec(dllexport)  char * __stdcall  function9(char *param1, char *param2)
{
	string display;
	if (is_kodi_running())
	{ 

		display = to_string((int) get_icon_val()+1);

		strcpy_s(line8, display.c_str());
	}
	else
	{
		strcpy_s(line8, "0");
	}
	return line8;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


__declspec(dllexport) bool __stdcall is_kodi_running()
{
	WCHAR szProcessName[MAX_PATH] = { 0 };
	DWORD dwProcesses[1024], cbNeeded, cProcesses, dwFlags, dw;
	unsigned int i;
	char buf[255];
	bool found = 0;

	if (!EnumProcesses(dwProcesses, sizeof(dwProcesses), &cbNeeded))
	{
		return 0;
	}
	cProcesses = cbNeeded / sizeof(DWORD);
	for (i = 0; i < cProcesses; i++)
	{
		if (dwProcesses[i] != 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcesses[i]);
			if (NULL != hProcess)
			{
				WCHAR path[MAX_PATH];
				dwFlags = 0;
				cbNeeded = MAX_PATH;
				// note that the Query function used here is the only one that works in win7 x64
				if (!QueryFullProcessImageName(hProcess, dwFlags, (LPWSTR)&path, &cbNeeded))
				{
					dw = GetLastError();
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)buf, 256, NULL);
					sprintf_s(buf, "Error %d - %s\n", dw, buf);
				}
				else
				{
					string_t kodi = utility::conversions::to_string_t(get_config_str(cKODI_EXE));
					string_t exe = string_t(path);
					exe = exe.substr(exe.length() - kodi.length());

					if (exe.compare(kodi) == 0)
					{
						found = 1;
					}
				}
				CloseHandle(hProcess);
			}
		}
	}
	return found;
}

void CALLBACK try_connect(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	
	if (!connected && is_kodi_running())
	{
		ws_connect();
	}
	else
	{
		connecting = false;
	}

}

string center(string in, unsigned int width)
{
	if (in.length() < width)
	{
		int mid = (width - in.length()) / 2;
		string spaces;
		for (int loop = 0; loop < mid; loop++)
		{
			spaces += string(" ");
		}
		return spaces + in;
	}
	return in;
}
