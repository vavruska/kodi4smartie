

enum configs_e {
	cLOGGING,
	cNEWFILE,
	cPORT,
	cLCD_WIDTH,
	cUSE_BARS,
	cBAR_MODE,
	cRESET_DELAY,
	cKODI_EXE,
	cCONNECT_DELAY,
	cTIME_FORMAT,
	cDISABLE_ICON,
	CIDLE_TIMER,

	//strings. override these in the config file to localize
	sWELCOME,
	sSTOP,
	sTRACK,
	sCHANNEL,
	sFF,
	sREWIND,
	sVOLUME,
	sKNF,
	sUNKNOWN,
	cMAX
};

enum datatype_t {
	MINT,
	MSTR,
};

typedef struct {
	char		key[32];
	datatype_t	datatype;
	int  val;
	char str[64];
} config_t;

__declspec(dllexport) void init_config();

void set_config(char *key, char *data);
unsigned int get_config(configs_e index);
const char *get_config_str(configs_e index);

