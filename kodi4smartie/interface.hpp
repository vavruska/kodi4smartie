#include <cpprest/json.h>
#include <pplx/pplxtasks.h>

using namespace web;

__declspec(dllexport) void ws_connect(void);
__declspec(dllexport) void start_listen();
__declspec(dllexport) void ws_close();
__declspec(dllexport) json::value ws_send_wait(json::value input);
__declspec(dllexport) json::value ws_send_wait(utility::string_t method, json::value input);
__declspec(dllexport) void display(json::value val, utility::string_t offset = utility::string_t());
__declspec(dllexport) json::value find_val(json::value val, wchar_t *key);



void parse(json::value incoming);
json::value get_item_from_player(int id);
void get_time_properties_from_player(int id);
utility::string_t format_time(json::value time_val);
void handle_on_play(json::value in);
void handle_on_pause();
void handle_on_stop();
void handle_speed_change(json::value incoming);
void handle_volume_change(json::value incoming);
std::string get_custom_data(char *method, char *item);