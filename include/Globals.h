#ifndef GLOBALS_H
#define GLOBALS_H

//#define GRSI_RELEASE "2.3.22 stable"

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

#define BG_WHITE   "\033[47m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"

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
#include <stdint.h> 
#endif

#include <stdexcept>
#include <string>
#include <cstdio>
#include <cstdlib>
//#include <stdint.h>
struct MNEMONIC {
  int16_t arrayposition;
  int16_t segment;
  std::string system;
  std::string subsystem;
  std::string arraysubposition;
  std::string collectedcharge;
  std::string outputsensor;
};

static void SetRFMNEMONIC(std::string *name,MNEMONIC *mnemonic) {
   mnemonic->system.assign(*name,0,2);
   mnemonic->subsystem.assign("X");
   mnemonic->arrayposition = 0;
   mnemonic->arraysubposition.assign("X");
   mnemonic->collectedcharge.assign("X");
   mnemonic->segment = 0;
   mnemonic->outputsensor.assign("X");
}

static void ParseMNEMONIC(std::string *name,MNEMONIC *mnemonic) {
	if(!name || name->length()<9) {
     if((name->length()<1) && (name->compare(0,2,"RF")==0))
       SetRFMNEMONIC(name,mnemonic);
     return;
   }   
   std::string buf;
   mnemonic->system.assign(*name,0,2);
   mnemonic->subsystem.assign(*name,2,1);
   buf.clear(); buf.assign(*name,3,2);
   mnemonic->arrayposition = (uint16_t)atoi(buf.c_str());
   mnemonic->arraysubposition.assign(*name,5,1);
   mnemonic->collectedcharge.assign(*name,6,1);
   buf.clear(); buf.assign(*name,7,2);
   mnemonic->segment = (uint16_t)atoi(buf.c_str());
   mnemonic->outputsensor.assign(*name,9,1);
   return;
}

static void ParseMNEMONIC(const char *name,MNEMONIC *mnemonic) {
   std::string sname = name;
   ParseMNEMONIC(&sname,mnemonic);
   return;
}

static void PrintMNEMONIC(MNEMONIC *mnemonic) {
	if(!mnemonic)
		return;
	printf("======MNEMONIC ======\n");
	printf("arrayposition    = %i\n", mnemonic->arrayposition);
	printf("segment          = %i\n", mnemonic->segment);
	printf("system           = %s\n", mnemonic->system.c_str());
	printf("subsystem        = %s\n", mnemonic->subsystem.c_str());
	printf("arraysubposition = %s\n", mnemonic->arraysubposition.c_str());
	printf("collectedcharge  = %s\n", mnemonic->collectedcharge.c_str());
	printf("outputsensor     = %s\n", mnemonic->outputsensor.c_str());
	printf("===============================\n");
	return;
}

static void ClearMNEMONIC(MNEMONIC *mnemonic) {
   if(!mnemonic)
      return;

   mnemonic->arrayposition = -1;
   mnemonic->segment       = -1;
   mnemonic->system.clear();
   mnemonic->subsystem.clear();
   mnemonic->arraysubposition.clear();
   mnemonic->collectedcharge.clear();
   mnemonic->outputsensor.clear();
}


const std::string &ProgramName(void);

struct exit_exception : public std::exception
{
   public:
      exit_exception(int c,const char *msg = "") : code(c), message(msg){}
      virtual ~exit_exception() throw() {}
 /*     virtual const char* what() const throw {
         //  LOG(what); // write to log file
         return what.c_str();
      }*/

      const int code;
      const char* message;
};

#endif
