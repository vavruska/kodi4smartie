
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
	int percentage = result[U("percentage")].as_integer();
	json::value time = result[U("time")];
	json::value totaltime = result[U("totaltime")];

	utility::string_t timestr = format_time(time);
	utility::string_t totaltimestr = format_time(totaltime);
	set_time(percentage, timestr, totaltimestr);
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
					int timer_val = 1;
					json::value ret_val = get_item_from_player(get_playerid());
					json::value result = ret_val[U("result")];
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
		::log("caught 1");
		int volume = vol.as_number().to_uint32();
		::log("caught2");

		set_volume(mute,volume);
	}
	catch (...)
	{
		::log("caught");

	}
}



