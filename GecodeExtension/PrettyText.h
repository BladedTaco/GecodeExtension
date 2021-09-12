#pragma once

#include <string>
#include <iostream>

// enum for text formatting
enum TextF {
	// modifiers
	BOLD,//= 1,
	UNDERLINED,//= 4,
	INVERTED,//= 7,

	// dark colours
	DC_GRAY,//= 30,
	DC_RED,//= 31,
	DC_GREEN,//= 32,
	DC_YELLOW,//= 33,
	DC_BLUE,//= 34,
	DC_MAGENTA,//= 35,
	DC_CYAN,//= 36,
	// dark highlights
	DH_RED,//= 41,
	DH_GREEN,//= 42,
	DH_YELLOW,//= 43,
	DH_BLUE,//= 44,
	DH_MAGENTA,//= 45,
	DH_CYAN,//= 46,
	DH_WHITE,//= 47,
	// normal colours
	C_GRAY,//= 90,
	C_RED,//= 91,
	C_GREEN,//= 92,
	C_YELLOW,//= 93,
	C_BLUE,//= 94,
	C_MAGENTA,//= 95,
	C_CYAN,//= 96,
	C_WHITE,//= 97,
	// normal highlights
	H_GRAY,//= 100,
	H_RED,//= 101,
	H_GREEN,//= 102,
	H_YELLOW,//= 103,
	H_BLUE,//= 104,
	H_MAGENTA,//= 105,
	H_CYAN,//= 106,
	H_WHITE,//= 107
};

// constant array for text formatting
static const std::string textF[] = {
	"\x1b[1m",   "\x1b[4m",   "\x1b[7m",   "\x1b[30m",  "\x1b[31m",
	"\x1b[32m",  "\x1b[33m",  "\x1b[34m",  "\x1b[35m",  "\x1b[36m",
	"\x1b[41m",  "\x1b[42m",  "\x1b[43m",  "\x1b[44m",  "\x1b[45m",
	"\x1b[46m",  "\x1b[47m",  "\x1b[90m",  "\x1b[91m",  "\x1b[92m",
	"\x1b[93m",  "\x1b[94m",  "\x1b[95m",  "\x1b[96m",  "\x1b[97m",
	"\x1b[100m", "\x1b[101m", "\x1b[102m", "\x1b[103m", "\x1b[104m",
	"\x1b[105m", "\x1b[106m", "\x1b[107m"
};



#define ESC char(0x1B)
#define FORMAT_END "\x1b[0m"
#define LINE_START "\x1b[1G"
#define CURSOR_X(x) "\x1b[" #x "G"
#define CURSOR_LEFT(x) "\x1b[" #x "D"
#define COL_1 CURSOR_X(32)

void PP(std::string s, const std::list<int> &listRef) {
	for (auto x : listRef) {
		std::cout << textF[x];
	}
	std::cout << s << FORMAT_END;
}