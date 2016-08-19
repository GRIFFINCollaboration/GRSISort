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
  
 private:
  int fSystem;
  int fSubSystem;
  int fArraySubPosition;
  int fCollectedCharge;
  int fOutputSensor;
/*
  struct ESubSystem {enum { kX, kG, kS, kE, kZ, kQ, kB, kD, kM, kI, kP, kC, kL, kW, kR, kF}; };
  struct EArraySubSystem {enum { kX, kB, kG, kR, kW, kD, kE, kF }; };
  struct ECollectedCharge { enum {kX, kN, kP, kT, kG, kL, kS }; };
  struct EOutPut {enum {kX, kA, kB }; };
  */
 public:
  //standard C++ makes these enumerations global to the class. ie, the name of the enumeration
  //EMnemonic or ESystem has no effect on the clashing of enumerated variable names.
  //These separations exist only to easily see the difference when looking at the code here.
  enum EMnemonic { kA, kB, kC, kD, kE, kF, kG, kI, kL, kM, kN, kP, kQ, kR, kS, kW, kX, kZ, kClear };
  enum ESystem   { kTigress, kSharc, kTriFoil, kRF, kCSM, kSiLi, kS3, kBambino, kTip, kGriffin, kSceptar, kPaces, kLaBr, kTAC, kZeroDegree, kDescant};

  void EnumerateMnemonic(std::string mnemonic_word, int &mnemonic_enum){

     char mnemonic_char = mnemonic_word[0];

     switch(mnemonic_char){
      case 'A':
         mnemonic_enum = kA;
         break;
      case 'B':
         mnemonic_enum = kB;
         break;
      case 'C':
         mnemonic_enum = kC;
         break;
      case 'D':
         mnemonic_enum = kD;
         break;
      case 'E':
         mnemonic_enum = kE;
         break;
      case 'F':
         mnemonic_enum = kF;
         break;
      case 'G':
         mnemonic_enum = kG;
         break;
      case 'I':
         mnemonic_enum = kI;
         break;
      case 'L':
         mnemonic_enum = kL;
         break;
      case 'M':
         mnemonic_enum = kM;
         break;
      case 'N':
         mnemonic_enum = kN;
         break;
      case 'P':
         mnemonic_enum = kP;
         break;
      case 'Q':
         mnemonic_enum = kQ;
         break;
      case 'R':
         mnemonic_enum = kR;
         break;
      case 'S':
         mnemonic_enum = kS;
         break;
      case 'W':
         mnemonic_enum = kW;
         break;
      case 'X':
         mnemonic_enum = kX;
         break;
      case 'Z':
         mnemonic_enum = kZ;
         break;
      default: 
         mnemonic_enum = kClear;
     };
  }

  void EnumerateSystem(){
      //Enumerating the system must come after the total mnemonic has been parsed as the details of other parts of
      //the mnemonic must be known
   if(system.compare("TI")==0) {
      fSystem = kTigress;
   } else if (system.compare("SH")==0) {
      fSystem = kSharc;
   } else if(system.compare("Tr")==0) {	
      fSystem = kTriFoil;
   } else if(system.compare("RF")==0) {	
      fSystem = kRF;
   } else if(system.compare("SP")==0) {
        if(SubSystem() == MNEMONIC::kI) {
            fSystem = kSiLi;
        } else {
            fSystem = kS3;
        }
   } else if(system.compare("CS")==0) {	
      fSystem = kCSM;
   } else if(system.compare("GR")==0) {
      fSystem = kGriffin;
   } else if(system.compare("SE")==0) {
      fSystem = kSceptar;
   } else if(system.compare("PA")==0) {	
      fSystem = kPaces;
   } else if(system.compare("DS")==0) {	
      fSystem = kDescant;
   } else if(system.compare("DA")==0) {
         if(CollectedCharge() == MNEMONIC::kN) {
            fSystem = kLaBr;
         } else {
            fSystem = kTAC;
         }
   } else if(system.compare("BA")==0) {
      fSystem = kS3;
   } else if(system.compare("ZD")==0) {	
      fSystem = kZeroDegree;
   } else if(system.compare("TP")==0) {	
      fSystem = kTip;
   } else {
      fSystem = kClear;
   }
}


   void Parse(std::string *name){
      if(!name || name->length()<9) {
         if((name->length()<1) && (name->compare(0,2,"RF")==0))
         SetRFMNEMONIC(name);
         return;
      }   
      std::string buf;
      system.assign(*name,0,2);
      subsystem.assign(*name,2,1);
      EnumerateMnemonic(subsystem,fSubSystem);
      buf.clear(); buf.assign(*name,3,2);
      arrayposition = (uint16_t)atoi(buf.c_str());
      arraysubposition.assign(*name,5,1);
      EnumerateMnemonic(arraysubposition,fArraySubPosition);
      collectedcharge.assign(*name,6,1);
      EnumerateMnemonic(collectedcharge,fCollectedCharge);
      buf.clear(); buf.assign(*name,7,2);
      segment = (uint16_t)atoi(buf.c_str());
      outputsensor.assign(*name,9,1);
      EnumerateMnemonic(outputsensor,fOutputSensor);
      //Enumerating the system must come last as the details of other parts of
      //the mnemonic must be known
      EnumerateSystem();
      return;

  }
   
   void Parse(const char *name){
      std::string sname = name;
      Parse(&sname);
      return;
   }

   void SetRFMNEMONIC(std::string *name){
      system.assign(*name,0,2);
      subsystem.assign("X");
      EnumerateMnemonic(subsystem,fSubSystem);
      arrayposition = 0;
      arraysubposition.assign("X");
      EnumerateMnemonic(arraysubposition,fArraySubPosition);
      collectedcharge.assign("X");
      EnumerateMnemonic(collectedcharge,fCollectedCharge);   
      segment = 0;
      outputsensor.assign("X");
      EnumerateMnemonic(outputsensor,fOutputSensor);

   }
   
  int System() const { return fSystem; }
  int SubSystem() const { return fSubSystem; }
  int ArraySubPosition() const { return fArraySubPosition; }
  int CollectedCharge() const { return fCollectedCharge; }
  int OutputSensor() const { return fOutputSensor; } 
	
  void Print() const{
      printf("======MNEMONIC ======\n");
	   printf("arrayposition    = %i\n", arrayposition);
   	printf("segment          = %i\n", segment);
	   printf("system           = %s\n", system.c_str());
	   printf("subsystem        = %s\n", subsystem.c_str());
	   printf("arraysubposition = %s\n", arraysubposition.c_str());
	   printf("collectedcharge  = %s\n", collectedcharge.c_str());
	   printf("outputsensor     = %s\n", outputsensor.c_str());
	   printf("===============================\n");
	   return;
  }

  void Clear() {
      arrayposition = -1;
      segment       = -1;
      system.clear();
      subsystem.clear(); fSubSystem = kClear;
      arraysubposition.clear(); fArraySubPosition = kClear;
      collectedcharge.clear(); fCollectedCharge = kClear;
      outputsensor.clear(); fOutputSensor = kClear;
      
  }

};


const std::string &ProgramName(void);

namespace grsi{
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
}
#endif
