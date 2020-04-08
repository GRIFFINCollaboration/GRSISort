#ifndef GLOBALS_H
#define GLOBALS_H

#define RESET_COLOR "\033[m"
#define BLUE "\033[1;34m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define RED "\033[1;31m"
#define BLACK "\033[1;30m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define WHITE "\033[1;37m"

#define DBLUE "\033[0;34m"
#define DYELLOW "\033[0;33m"
#define DGREEN "\033[0;32m"
#define DRED "\033[0;31m"
#define DBLACK "\033[0;30m"
#define DMAGENTA "\033[0;35m"
#define DCYAN "\033[0;36m"
#define DWHITE "\033[0;37m"

#define BG_WHITE "\033[47m"
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN "\033[46m"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

#define ALERTTEXT "\033[47m\033[0;31m"

#define NUM_SIS_CHAN 8

#define MAXSAMPLESIZE 8192

#define FRAGMENTBUFFERSIZE 1000

#define BUILDINGTIMECONDITION 2
#define BUILDINGTRIGGERCONDITION 999

#if __APPLE__
#ifdef __CINT__
#undef __GNUC__
typedef char __signed;
typedef char int8_t;
#endif
#endif

#if __APPLE__
//#include <_types/_uint8_t.h>
#include <_types/_uint16_t.h>
#include <_types/_uint32_t.h>
#include <_types/_uint64_t.h>
#include <sys/_types/_int16_t.h>
#else
#include <cstdint>
#endif

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <execinfo.h>
#include <cxxabi.h>

const std::string& ProgramName();

namespace grsi {
struct exit_exception : public std::exception {
public:
   exit_exception(int c, const char* msg = "") : code(c), message(msg) {}
   ~exit_exception() throw() override = default;
   /*     virtual const char* what() const throw {
           //  LOG(what); // write to log file
           return what.c_str();
        }*/

   const int   code;
   const char* message;
};
}

// print a demangled stack backtrace of the caller function (copied from https://panthema.net/2008/0901-stacktrace-demangled/)
static inline void PrintStacktrace(std::ostream& out = std::cout, unsigned int maxFrames = 63)
{
	out<<"stack trace:"<<std::endl;

	// storage array for stack trace address data
	void** addrlist = new void*[maxFrames+1];

	// retrieve current stack addresses
	int addrlen = backtrace(addrlist, maxFrames+1);

	if(addrlen == 0) {
		out<<"  <empty, possibly corrupt>"<<std::endl;
		return;
	}

	// resolve addresses into strings containing "filename(function+address)",
	// this array must be free()-ed
	char** symbollist = backtrace_symbols(addrlist, addrlen);

	// allocate string which will be filled with the demangled function name
	size_t funcnamesize = 256;
	char* funcname = new char[funcnamesize];

	// iterate over the returned symbol lines. skip the first, it is the
	// address of this function.
	for(int i = 2; i < addrlen; i++) {
		char* begin_name = nullptr;
		char* begin_offset = nullptr;
		char* end_offset = nullptr;

		// find parentheses and +address offset surrounding the mangled name:
		// ./module(function+0x15c) [0x8048a6d]
		for(char* p = symbollist[i]; *p; ++p) {
			if(*p == '(') {
				begin_name = p;
			} else if (*p == '+') {
				begin_offset = p;
			} else if(*p == ')' && begin_offset) {
				end_offset = p;
				break;
			}
		}

		if(begin_name && begin_offset && end_offset && begin_name < begin_offset) {
			*begin_name++ = '\0';
			*begin_offset++ = '\0';
			*end_offset = '\0';

			// mangled name is now in [begin_name, begin_offset) and caller
			// offset in [begin_offset, end_offset). now apply
			// __cxa_demangle():

			int status;
			char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
			if(status == 0) {
				funcname = ret; // use possibly realloc()-ed string
				std::cout<<"  "<<symbollist[i]<<" : "<<funcname<<"+"<<begin_offset<<std::endl;
			} else {
				// demangling failed. Output function name as a C function with
				// no arguments.
				std::cout<<"  "<<symbollist[i]<<" : "<<begin_name<<"()+"<<begin_offset<<std::endl;
			}
		} else {
			// couldn't parse the line? print the whole line.
			std::cout<<"  "<<symbollist[i]<<std::endl;
		}
	}

	free(funcname);
	free(symbollist);
}

#endif
