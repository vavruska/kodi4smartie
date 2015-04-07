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
#include <string>
#include <iostream>
#include <regex>
#include <cpprest/json.h>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <mutex>
#include <map>
#include <regex>

#include "interface.hpp"
#include "logging.hpp"
#include "display.hpp"
#include "config.hpp"

using namespace std;
using namespace web;
using namespace utility;

icon_t icon = none;
string title = "";
int player_id = 0;
string_t mode;
string_t prev_mode;
string_t time_str;
string_t episode_info;
string_t tv_info;
string_t artist_info;
string_t album_info;
int track_info;
HANDLE reset_timer = NULL;
HANDLE time_timer = NULL;
bool continue_timer;
std::mutex mtx;
int extra_info = 0;
int volume = 0;
bool muted;

typedef map<std::regex *, string> regex_map_t;
extern regex_map_t regex_map;

void CALLBACK reset_fired(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
void stop_reset_timer();

void set_title(string_t newtitle)
{
	regex_map_t::iterator it;

	mtx.lock();
	title = utility::conversions::utf16_to_utf8(newtitle);

	//execute each regex 
	for (it = regex_map.begin(); it != regex_map.end(); it++)
	{
		title = regex_replace(title, *it->first,  it->second);
	}

	//remove leading whitespace/cr/lf
	int pos = title.find_first_not_of("\n\r\t ");
	title = title.substr(pos);

	::log("title=%s", title.c_str());

	mtx.unlock();
}

string get_title()
{
	mtx.lock();
	string convert = title;
	mtx.unlock();

	return convert;
}


void set_icon(icon_t icon_val)
{
	mtx.lock();
	switch (icon_val)
	{
	case play:
	case stop:
	case pause:
	case ff:
	case rew:
		icon = icon_val;
		break;
	default:
		icon = none;
		break;
	}
	mtx.unlock();
}

icon_t get_icon_val()
{
	icon_t ret_val;

	mtx.lock();
	ret_val = icon;
	mtx.unlock();
	return ret_val;
}

string get_icon(int custom)
{
	static int cust[] = { 0, 176, 158, 131, 132, 133, 134, 135, 136 };
	string display = string("$CustomChar(") + std::to_string(custom);
	mtx.lock();

	icon_t icon_to_display = icon;
	if (muted == true)
	{
		icon_to_display = mute;
	}

	switch (icon_to_display)
	{
	case none:
		display = " ";
		break;
	case play:
		display += string(", 16, 24, 28, 30, 28, 24, 16, 0");
		break;
	case stop:
		display += string(", 14, 14, 14, 14, 14, 14, 14, 0");
		break;
	case pause:
		display += string(", 27, 27, 27, 27, 27, 27, 27, 0");
		break;
	case ff:
		display += string(", 0, 20, 26, 29, 29, 26, 20, 0");
		break;
	case rew:
		display += string(", 0, 5, 11, 23, 23, 11, 5, 0");
		break;
	case mute:
		display += string(", 8, 13, 10, 15, 11, 26, 12, 8");
		break;
	default:
		display += string(", 63, 0, 0, 0, 0, 0, 0, 63");
		break;
	}
	if (icon_to_display != none)
	{
		display += string(")$Chr(") + std::to_string(cust[custom]) + string(")");
	}
	mtx.unlock();

	return display;
}

void set_mode(string_t new_mode)
{
	mtx.lock();

	if (mode != new_mode)
	{
		mode = new_mode;
		log("Mode set to %ls", new_mode.c_str());
	}
	mtx.unlock();
}

string get_mode()
{
	mtx.lock();
	string convert = utility::conversions::utf16_to_utf8(mode);
	mtx.unlock();

	return convert;
}

void set_playerid(int id)
{
	mtx.lock();
	player_id = id;
	mtx.unlock();
}

int get_playerid()
{
	mtx.lock();
	int ret_val = player_id;
	mtx.unlock();

	return ret_val;
}

string_t center(string_t in)
{
	if (in.length() < get_config(cLCD_WIDTH))
	{
		int mid = (get_config(cLCD_WIDTH) - in.length()) / 2;
		string_t spaces;
		for (int loop = 0; loop < mid; loop++)
		{
			spaces += string_t(U(" "));
		}
		return spaces + in;
	}
	return in;
}

void set_time(int percentage, string_t time, string_t totaltime)
{
	mtx.lock();

	if (get_config(cUSE_BARS))
	{
		string display;

		time_str = string_t(U("$Bar(")) + std::to_wstring(percentage) + string_t(U(",100,")) +
			std::to_wstring(get_config(cLCD_WIDTH)) + string_t(U(")"));
	}
	else
	{
		time_str =  center(time + string_t(U("/")) + totaltime);
	}
	mtx.unlock();
}

string get_time()
{
	mtx.lock();
	string convert = utility::conversions::utf16_to_utf8(time_str);
	mtx.unlock();
	return convert;
}

void set_speed(int speedval)
{
	mtx.lock();
	icon_t speed = rew;
	string speedtext = string(get_config_str(sREWIND));
	static string prev_title;
	
	
	if (speedval != 1)
	{
		if (speedval > 0)
		{
			speed = ff;
			speedtext = string(get_config_str(sFF));
		}

		speedval = abs(speedval);

		if (speedval > 1)
		{
			speedtext += string(" ") + std::to_string(speedval) + string("X");
		}

		icon = speed;
		if (prev_title.compare("") == 0)
		{
			prev_title = title;
		}
		title = speedtext;
	}
	else
	{
		icon = play;
		title = prev_title;
		prev_title = "";
	}
	mtx.unlock();
}

void set_volume(bool mute, int vol)
{
	mtx.lock();
	if (muted != mute)
	{ 
		muted = mute;
		volume = vol;
		mtx.unlock();
	}
	else
	{
		volume = vol;
		stop_reset_timer();
		if (mode.compare(U("volume")) != 0)
		{
			prev_mode = mode;
		}
		mtx.unlock();
		set_mode(U("volume"));
		CreateTimerQueueTimer(&reset_timer, NULL, reset_fired, NULL, get_config(cRESET_DELAY) * 1000, 0, 0);
	}
}

int get_volume()
{
	mtx.lock();
	int vol = volume;
	mtx.unlock();

	return vol;
}

void set_episode_info(int season, int episode, string_t showtitle)
{
	mtx.lock();
	extra_info = 1;
	string_t season_str = string_t(U("0") + std::to_wstring(season)).substr(1, 2);
	string_t episode_str = string_t(U("0") + std::to_wstring(episode)).substr(1, 2);
	episode_info = U("S") + season_str + U("E") + episode_str + U(": ") + showtitle;
	mtx.unlock();
}

string get_episode_info()
{
	string convert;
	mtx.lock();
	convert = utility::conversions::utf16_to_utf8(episode_info);
	mtx.unlock();

	return convert;
}

int get_extra_info()
{
	mtx.lock();
	int ret_val = extra_info;
	mtx.unlock();
	return extra_info;
}

void set_tv_info(string_t channel, int channel_number, string_t show)
{
	set_title(show);
	mtx.lock();
	extra_info = 1;
	tv_info = string_t(utility::conversions::to_string_t(get_config_str(sCHANNEL))) + std::to_wstring(channel_number) + string_t(U(" ")) + channel;
	mtx.unlock();
}

string get_tv_info()
{
	string convert;
	mtx.lock();
	convert = utility::conversions::utf16_to_utf8(tv_info);
	mtx.unlock();

	return convert;
}

void set_song_info(string_t artist, string_t album, int track)
{
	mtx.lock();
	extra_info = 7;
	artist_info = artist;
	album_info = album;
	track_info = track;
	mtx.unlock();
}

string get_song_info()
{
	//return different information based on extra_info count
	string convert;
	mtx.lock();
	if (extra_info > 6)
	{
		convert = utility::conversions::utf16_to_utf8(artist_info);
	}
	else if (extra_info > 3)
	{
		convert = utility::conversions::utf16_to_utf8(album_info);
	}
	else
	{
		if (track_info)
		{
			convert = string(get_config_str(sTRACK)) + std::to_string(track_info);
		}
		else
		{
			//if no track then display the album for an extra cycle
			convert = utility::conversions::utf16_to_utf8(album_info);
			extra_info = 0;
		}
	}
	mtx.unlock();

	return string("$Center(") + convert + string(")");
}

void CALLBACK reset_fired(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	mtx.lock();
	if (icon == stop)
	{
		reset_timer = NULL;
		icon = none;
		title = "";
		mode = U("");
	}
	if (mode.compare(U("volume")) == 0)
	{
		reset_timer = NULL;
		mode = prev_mode;
	}
	mtx.unlock();
}

void stop_reset_timer()
{
	if (reset_timer)
	{
		DeleteTimerQueueTimer(NULL, reset_timer, NULL);
		reset_timer = NULL;
	}
}

void show_stop()
{
	stop_time_timer();
	set_icon(stop);
	set_title(string_t(utility::conversions::to_string_t(get_config_str(sSTOP))));
	CreateTimerQueueTimer(&reset_timer, NULL, reset_fired, NULL, get_config(cRESET_DELAY)*1000, 0, 0);
}

void CALLBACK update_time(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	try 
	{
		get_time_properties_from_player(get_playerid());
		mtx.lock();
		if (extra_info > 0)
		{
			extra_info--;
		}
		mtx.unlock();
	}
	catch (...)
	{

	}
}

void start_time_timer(int start_interval_secs)
{
	stop_time_timer();
	CreateTimerQueueTimer(&time_timer, NULL, update_time, NULL, start_interval_secs*1000, 1000, 0);
}

void stop_time_timer()
{
	if (time_timer)
	{
		DeleteTimerQueueTimer(NULL, time_timer, INVALID_HANDLE_VALUE);
		time_timer = NULL;
	}
}

void stop_timers()
{
	stop_reset_timer();
	stop_time_timer();
}