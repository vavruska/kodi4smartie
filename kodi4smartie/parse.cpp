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
#include <iostream>
#include <cpprest/json.h>
#include <ppltasks.h>
#include <concurrent_vector.h>

#include "interface.hpp"
#include "logging.hpp"
#include "display.hpp"

using namespace std;
using namespace web;
using namespace concurrency;



// Notifications we handle
//"Player.OnPlay"
//"Player.OnPause"
//"Player.OnStop"
//"Player.OnSpeedChanged"
//"Application.OnVolumeChanged"
//
//"System.OnQuit" (not currently supported)
//"System.OnRestart" (not currently supported)
//"System.OnSleep" (not currently supported)
//"System.OnWake" (not currently supported)


void parse(json::value incoming)
{

	json::value method = incoming[U("method")];
	if (!method.is_null())
	{
		if ((method.as_string().compare(U("Player.OnPlay")) == 0))
		{
			handle_on_play(incoming);
		}
		else if ((method.as_string().compare(U("Player.OnPause")) == 0))
		{
			handle_on_pause();
		}
		else if ((method.as_string().compare(U("Player.OnStop")) == 0))
		{
			handle_on_stop();
		}
		else if (method.as_string().compare(U("Player.OnSpeedChanged")) == 0)
		{
			handle_speed_change(incoming);
		}
		else if (method.as_string().compare(U("Application.OnVolumeChanged")) == 0)
		{
			handle_volume_change(incoming);
		}
	}
}

json::value get_item_from_player(int playerid)
{
	json::value data;
	json::value params;
	json::value properties;

	properties[0] = json::value::string(U("title"));
	properties[1] = json::value::string(U("season"));
	properties[2] = json::value::string(U("episode"));
	properties[3] = json::value::string(U("showtitle"));
	properties[4] = json::value::string(U("channelnumber"));
	properties[5] = json::value::string(U("channel"));
	properties[6] = json::value::string(U("artist"));
	properties[7] = json::value::string(U("album"));
	properties[8] = json::value::string(U("track"));
	
	params[U("properties")] = properties;
	params[U("playerid")] = playerid;
	data[U("params")] = params;
	return ws_send_wait(U("Player.GetItem"), data);
}

void get_time_properties_from_player(int playerid)
{
	json::value data;
	json::value params;
	json::value properties;

	properties[0] = json::value::string(U("percentage"));
	properties[1] = json::value::string(U("time"));
	properties[2] = json::value::string(U("totaltime"));

	params[U("properties")] = properties;
	params[U("playerid")] = playerid;
	data[U("params")] = params;
	json::value ret_val = ws_send_wait(U("Player.GetProperties"), data);

	json::value result = ret_val[U("result")];
	if (!result.is_null())
	{
		int percentage = result[U("percentage")].as_integer();
		json::value time = result[U("time")];
		json::value totaltime = result[U("totaltime")];

		utility::string_t timestr = format_time(time);
		utility::string_t totaltimestr = format_time(totaltime);
		set_time(percentage, timestr, totaltimestr);
	}
}

void handle_on_play(json::value in)
{
	try 
	{
		if (get_icon_val() == pause)
		{
			set_icon(play);
		}
		else
		{
			stop_time_timer();
			json::value method = in[U("method")];
			json::value params = in[U("params")];
			json::value data = params[U("data")];
			json::value item = data[U("item")];
			json::value title = item[U("title")];
			json::value type = item[U("type")];
			set_mode(type.as_string());
			set_icon(play);
			if ((title.is_null() || 
				(type.as_string().compare(U("channel")) == 0) || 
				(type.as_string().compare(U("song")) == 0)) &&
				(type.as_string().compare(U("picture")) != 0))
			{
				json::value player = data[U("player")];
				json::value id = player[U("playerid")];
				set_playerid(id.as_integer());

				task <void>([&]()
				{
					try
					{
						int timer_val = 1;
						json::value ret_val = get_item_from_player(get_playerid());
						json::value result = ret_val[U("result")];
						if (!result.is_null())
						{
							json::value ritem = result[U("item")];
							json::value title = ritem[U("title")];
							json::value type = ritem[U("type")];
							if (type.as_string().compare(U("movie")) == 0)
							{
								set_title(title.as_string());
							}
							else if (type.as_string().compare(U("episode")) == 0)
							{
								//get additional epsiode information.
								json::value season = ritem[U("season")];
								json::value episode = ritem[U("episode")];
								json::value showtitle = ritem[U("showtitle")];
								set_title(showtitle.as_string());
								set_episode_info(season.as_integer(), episode.as_integer(), title.as_string());
								timer_val = 3;
							}
							else if (type.as_string().compare(U("channel")) == 0)
							{
								json::value channel = ritem[U("channel")];
								json::value channel_number = ritem[U("channelnumber")];
								set_tv_info(channel.as_string(), channel_number.as_integer(), title.as_string());
								timer_val = 3;
							}
							else if (type.as_string().compare(U("song")) == 0)
							{
								json::value track = ritem[U("track")];
								json::value album = ritem[U("album")];
								json::value artist = ritem[U("artist")];
								set_title(title.as_string());
								set_song_info(artist[0].as_string(), album.as_string(), track.as_integer());
								timer_val = 3;
							}
							start_time_timer(timer_val);
						}
					}
					catch (...)
					{
						::log("Malformed get item response");
					}
				});
			}
			else
			{
				if (type.as_string().compare(U("picture")) == 0)
				{
					json::value filepath = item[U("file")];
					std::vector<wchar_t> file(1024);

					_wsplitpath_s(filepath.as_string().c_str(), NULL, 0, NULL, 0, file.data(), file.size(), NULL, 0);
					set_title(string_t(file.data()));
				}
				else
				{
					//handle title (dvd or unclassified video file
					log("title %ls", title.as_string());
					json::value player = data[U("player")];
					json::value id = player[U("playerid")];
					set_playerid(id.as_integer());
					set_title(title.as_string());
					start_time_timer(1);
				}
			}
		}
	}

	catch (...)
	{
		log("Malformed json message received: %d", in.serialize());
	};
}

void handle_on_pause()
{
	set_icon(pause);
}

void handle_on_stop()
{
	show_stop();
}

void handle_speed_change(json::value incoming)
{
	try
	{
		json::value params = incoming[U("params")];
		json::value data = params[U("data")];
		json::value player = data[U("player")];
		int speed = player[U("speed")].as_integer();

		set_speed(speed);
	}

	catch (...)
	{
		cout << "error";
		log("Malformed json message received: %d", incoming.serialize());
	}
}

utility::string_t format_time(json::value time_str)
{
	utility::string_t result;

	json::value val = time_str[U("hours")];
#if 0 //do not make hour 2 digits.
	if (val.as_integer() < 10)
	{
		result += utility::string_t(U("0"));
	}
#endif // 0 //do not make hour 2 digits.
	result += std::to_wstring(val.as_integer());
	result += utility::string_t(U(":"));

	val = time_str[U("minutes")];
	if (val.as_integer() < 10)
	{
		result += utility::string_t(U("0"));
	}
	result += std::to_wstring(val.as_integer());
	result += utility::string_t(U(":"));

	val = time_str[U("seconds")];
	if (val.as_integer() < 10)
	{
		result += utility::string_t(U("0"));
	}
	result += std::to_wstring(val.as_integer());

	return result;
}

void handle_volume_change(json::value incoming)
{
	try
	{
		json::value params = incoming[U("params")];
		json::value data = params[U("data")];
		json::value vol = data[U("volume")];
		json::value muted = data[U("muted")];
		bool mute = muted.as_bool();
		int volume = vol.as_number().to_uint32();

		set_volume(mute,volume);
	}
	catch (...)
	{
		::log("caught");

	}
}

std::string get_custom_data(char *method, char *item)
{

	bool needs_playerid = false;
	int playerid;
	json::value data;
	json::value params;
	json::value properties;
	json::value ret_item;
	json::value item_val;
	char *pos;
	char *retsubitem = NULL;

	try
	{
		if ((_stricmp(method, "Player.GetItem") == 0) || 
			(_stricmp(method, "Player.GetProperties") == 0))
		{
			json::value request;
			json::value response;
			json::value result;

			response = ws_send_wait(U("Player.GetActivePlayers"), request);
			result = response[U("result")];
			if (!result.is_null())
			{
				json::value play = result[0];
				if (play[U("playerid")].is_null())
				{
					return "";
				}
				playerid = play[U("playerid")].as_number().to_int32();
				needs_playerid = true;
			}
		}

		if ((pos = strchr(item, '#')) != NULL)
		{
			*pos = NULL;
			retsubitem = ++pos;

			log("item %s, retsubItem %s", item, retsubitem);
		}

		if (strlen(item))
		{
			properties[0] = json::value::string(utility::conversions::to_string_t(item));
		

			params[U("properties")] = properties;
			if (needs_playerid)
			{
				params[U("playerid")] = playerid;
			}
			data[U("params")] = params;
		}

		json::value ret_val = ws_send_wait(utility::conversions::to_string_t(method), data);

		if (ret_val.size() > 0)
		{ 
			if (!ret_val[U("error")].is_null())
			{
				item_val = ret_val[U("error")];
				item_val = item_val[U("message")];
			}
			else
			{
				json::value result = ret_val[U("result")];
				if (retsubitem == NULL)
				{
					ret_item = result[U("item")];
					if (ret_item.is_null())
					{
						ret_item = result;
					}
					item_val = ret_item[utility::conversions::to_string_t(item)];
				}
				else
				{
					json::value ret_item;
					if (strlen(item) == 0)
					{
						result = result[0];
					}
					else
					{
						result = result[utility::conversions::to_string_t(item)];
					}
					ret_item = result[utility::conversions::to_string_t(retsubitem)];
					item_val = ret_item;
				}

				if (item_val.is_null())
				{
					return string("");
				}
			}
			string_t item_str;
		
			if (item_val.is_string())
			{
				item_str = item_val.as_string();
			}
			else if (item_val.is_integer())
			{
				item_str = to_wstring(item_val.as_number().to_int32());
			}
			else if (item_val.is_double())
			{
				item_str = to_wstring(item_val.as_number().to_double());
			}
			return utility::conversions::to_utf8string(item_str);
		}
		else
		{
			return "";
		}
	}
	catch (...)
	{
		return string("Error Caught");
	}
}



