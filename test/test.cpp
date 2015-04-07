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

//
// test.cpp : Defines the entry point for the console application. 
//
// This program was used to test the JSON requests being set to Kodi 
// and to check the return values.
//

#include "stdafx.h"
#include <cpprest/ws_client.h>
#include <cpprest/json.h>
#include <cpprest/rawptrstream.h>

#include <ppltasks.h>
#include <iostream>
#include <string>	
#include <conio.h>
#include "config.hpp"
#include "interface.hpp"
#include "logging.hpp"
#include "dll.hpp"

using namespace web;
using namespace web::websockets::client;
using namespace Concurrency::streams;
using namespace concurrency;
using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	

	if (is_kodi_running())
	{
		init_config();
		init_logging();
		ws_connect();
		start_listen();

		std::string mystr;

		do {
			getline(cin, mystr);
			if (mystr.compare(string("q")))
			{
				try
				{
					json::value myval = json::value::parse(utility::conversions::to_string_t(mystr));
					json::value result = ws_send_wait(myval);
					display(result);
				}
				catch (...) {};
			}
		} while (mystr.compare(string("q")));

		ws_close();

	}
	else
	{
		cout << "kodi not running." << endl;
	}

	return 0;
}

#if 0
Player.Position.Percentage
{ "jsonrpc": "2.0", "method" : "Player.GetProperties": "id" : 1 }
{ "jsonrpc": "2.0", "method" : "Player.GetProperties", "id" : 1 }
{"jsonrpc": "2.0", "method" : "Player.GetProperties", "params" : { "properties": ["percentage", "time", "totaltime"], "playerid": 1}, "id" : 1}
#endif

