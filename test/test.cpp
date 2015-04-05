//
// test.cpp : Defines the entry point for the console application.
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

