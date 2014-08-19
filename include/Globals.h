#ifndef GLOBALS_H
#define GLOBALS_H

#define RESET_COLOR "\e[m"
#define BLUE "\e[1;34m"
#define YELLOW "\e[1;33m"
#define GREEN "\e[1;32m"
#define RED "\e[1;31m"
#define BLACK "\e[1;30m"
#define MAGENTA "\e[1;35m"
#define CYAN "\e[1;36m"
#define WHITE "\e[1;37m"

#define DBLUE "\e[0;34m"
#define DYELLOW "\e[0;33m"
#define DGREEN "\e[0;32m"
#define DRED "\e[0;31m"
#define DBLACK "\e[0;30m"
#define DMAGENTA "\e[0;35m"
#define DCYAN "\e[0;36m"
#define DWHITE "\e[0;37m"

#define BG_WHITE   "\e[47m"
#define BG_RED     "\e[41m"
#define BG_GREEN   "\e[42m"
#define BG_YELLOW  "\e[43m"
#define BG_BLUE    "\e[44m"
#define BG_MAGENTA "\e[45m"
#define BG_CYAN    "\e[46m"

#define HIDE_CURSOR "\e[?25l"
#define SHOW_CURSOR "\e[?25h"

#define ALERTTEXT "\e[47m\e[0;31m"

#define NUM_SIS_CHAN 8

#define MAXSAMPLESIZE 8192 

#define FRAGMENTBUFFERSIZE 1000

#define BUILDINGTIMECONDITION 2
#define BUILDINGTRIGGERCONDITION 999

#include <string>

#if __APPLE__
#ifdef __CINT__
#undef __GNUC__
typedef char __signed;
typedef char int8_t;
#endif
#endif

struct FSPC	{
	unsigned char type;
	int			integration;
	int 		number;
	int 		nbins;
	int 		nsamp;
	int 		MaxCharge;
	float 	maxE;
	float		gain;
	float		offset;
	std::string name;
	std::string	type_title;
	std::string type_name;
	std::string type_units;
};

const std::string &ProgramName(void);

#endif
