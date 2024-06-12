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
#include <iomanip>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <execinfo.h>
#include <cxxabi.h>
#include <sstream>
#include <array>
#include <memory>
#include <unistd.h>

#include "TEnv.h"

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

void SetGRSIEnv();

//-------------------- three function templates that print all arguments into a string
// this template uses existing stream and appends the last argument to it
template <typename T>
void Append(std::stringstream& stream, const T& tail)
{
   // append last argument
   stream << tail;
}

// this template uses existing stream and appends to it
template <typename T, typename... U>
void Append(std::stringstream& stream, const T& head, const U&... tail)
{
   // append first argument
   stream << head;

   // reversely call this template (or the one with the last argument)
   Append(stream, tail...);
}

// this function typically gets called by user
template <typename T, typename... U>
std::string Stringify(const T& head, const U&... tail)
{
   // print first arguments to string
   std::stringstream stream;
   stream << head;

   // call the second template (or the third if tail is just one argument)
   Append(stream, tail...);

   // append a newline
   stream << std::endl;

   // return resulting string
   return stream.str();
}

}   // end of namespace grsi

template <typename T>
inline std::string hex(T val, int width = -1)
{
   std::ostringstream str;
   str << "0x" << std::hex;
   if(width > 0) {
      str << std::setfill('0') << std::setw(width);
   }
   str << val;
   if(width > 0) {
      str << std::setfill(' ');
   }
   return str.str();
}

static inline std::string getexepath()
{
   char    result[1024];
   ssize_t count = readlink("/proc/self/exe", result, sizeof(result) - 1);
   return std::string(result, (count > 0) ? count : 0);
}

static inline std::string sh(std::string cmd)
{
   std::array<char, 128> buffer;
   std::string           result;
   std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
   if(!pipe) throw std::runtime_error("popen() failed!");
   while(!feof(pipe.get())) {
      if(fgets(buffer.data(), 128, pipe.get()) != nullptr) {
         result += buffer.data();
      }
   }
   return result;
}

// print a demangled stack backtrace of the caller function (copied from https://panthema.net/2008/0901-stacktrace-demangled/)
static inline void PrintStacktrace(std::ostream& out = std::cout, unsigned int maxFrames = 63)
{
   std::stringstream str;
   str << "stack trace:" << std::endl;

   // storage array for stack trace address data
   void** addrlist = new void*[maxFrames + 1];

   // retrieve current stack addresses
   int addrlen = backtrace(addrlist, maxFrames + 1);

   if(addrlen == 0) {
      str << "  <empty, possibly corrupt>" << std::endl;
      out << str.str();
      return;
   }

   // resolve addresses into strings containing "filename(function+address)",
   // this array must be free()-ed
   char** symbollist = backtrace_symbols(addrlist, addrlen);

   // allocate string which will be filled with the demangled function name
   size_t funcnamesize = 256;
   char*  funcname     = new char[funcnamesize];

   // iterate over the returned symbol lines. skip the first, it is the
   // address of this function.
   for(int i = 2; i < addrlen; i++) {
      char* begin_name   = nullptr;
      char* begin_offset = nullptr;
      char* end_offset   = nullptr;

      // find parentheses and +address offset surrounding the mangled name:
      // ./module(function+0x15c) [0x8048a6d]
      for(char* p = symbollist[i]; *p; ++p) {
         if(*p == '(') {
            begin_name = p;
         } else if(*p == '+') {
            begin_offset = p;
         } else if(*p == ')' && begin_offset) {
            end_offset = p;
            break;
         }
      }

      // try and decode file and line number (only if we have an absolute path)
      // std::string line;
      // if(symbollist[i][0] == '/') {
      //	std::stringstream command;
      //	std::string filename = symbollist[i];
      //	command<<"addr2line "<<addrlist[i]<<" -e "<<filename.substr(0, filename.find_first_of('('));
      //	//std::cout<<symbollist[i]<<": executing command "<<command.str()<<std::endl;
      //	line = sh(command.str());
      //}

      if(begin_name && begin_offset && end_offset && begin_name < begin_offset) {
         *begin_name++   = '\0';
         *begin_offset++ = '\0';
         *end_offset     = '\0';

         // mangled name is now in [begin_name, begin_offset) and caller
         // offset in [begin_offset, end_offset). now apply
         // __cxa_demangle():

         int   status;
         char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
         if(status == 0) {
            funcname = ret;   // use possibly realloc()-ed string
            str << "  " << symbollist[i] << ": " << funcname << "+" << begin_offset << std::endl;
         } else {
            // demangling failed. Output function name as a C function with
            // no arguments.
            str << "  " << symbollist[i] << ": " << begin_name << "()+" << begin_offset << std::endl;
         }
      } else {
         // couldn't parse the line? print the whole line.
         str << "  " << symbollist[i] << std::endl;
      }
      // str<<line;
   }

   free(funcname);
   free(symbollist);
   out << str.str();
}

#if !__APPLE__
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/prctl.h>
static inline void PrintGdbStacktrace()
{
   char pid_buf[30];
   sprintf(pid_buf, "%d", getpid());
   char name_buf[512];
   name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
   prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);
   int child_pid = fork();
   if(!child_pid) {
      dup2(2, 1);   // redirect output to stderr - edit: unnecessary?
      execl("/usr/bin/gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, NULL);
      abort(); /* If gdb failed to start */
   } else {
      waitpid(child_pid, NULL, 0);
   }
}
#endif

#endif
