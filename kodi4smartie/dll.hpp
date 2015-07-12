


extern "C" {
	
	__declspec(dllexport)  void __stdcall  SmartieInit();
	__declspec(dllexport)  void __stdcall  SmartieFini();
	__declspec(dllexport)  int __stdcall  GetMinRefreshInterval();
	__declspec(dllexport)  char * __stdcall  function1(char *param1, char *param2);
	__declspec(dllexport)  char * __stdcall  function2(char *param1, char *param2);
	__declspec(dllexport)  char * __stdcall  function3(char *param1, char *param2);
	__declspec(dllexport)  char * __stdcall  function4(char *param1, char *param2);
	__declspec(dllexport)  char * __stdcall  function5(char *param1, char *param2);
	__declspec(dllexport)  bool __stdcall is_kodi_running();

}

