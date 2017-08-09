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

#include <stdexcept>
#include <string>
#include <cstdio>
#include <cstdlib>
//#include <stdint.h>
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
#endif
