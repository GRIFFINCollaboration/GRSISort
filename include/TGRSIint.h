#ifndef TGRSIINT_H
#define TGRSIINT_H

#include <cstdio>
#include <string>


#include <TSystem.h>
#include <TRint.h>
#include <TList.h>

#include "Globals.h"

//#include <TEventLoop.h>

class TGRSIint : public TRint {

   private:
      TGRSIint(int argc, char **argv,void *options = 0, 
            int numOptions = 0, bool noLogo = false, const char *appClassName = "grsisort") ;

   public:
      static TGRSIint *fTGRSIint;
      static TGRSIint *instance(int argc = 0, char **argv = 0, void *options = 0, 
                             int numOptions = -1, bool noLogo = false, const char *appClassName = "grsisort");
      
      virtual ~TGRSIint();

      void GetOptions(int *argc,char **argv);
      void PrintHelp(bool);
      void PrintLogo(bool);
      bool HandleTermInput();
      int  TabCompletionHook(char*,int*,ostream&);

      //bool Sort() {return TEventLoop::Get()->SortMidas();}

   private:
      bool FileAutoDetect(std::string filename, long filesize);
      void InitFlags();
      void ApplyOptions();

   public:
      //static std::vector<std::string> *GetInputRoot()  {  return fInputRootFile;  }
      //static std::vector<std::string> *GetInputMidas() {  return fInputMidasFile; }
      //static std::vector<std::string> *GetInputCal()   {  return fInputCalFile;   }
      //static std::vector<std::string> *GetInputOdb()   {  return fInputOdbFile;   }

   private: 
      bool fPrintLogo;
      bool fPrintHelp;
      
      bool fAutoSort;


      //TList *fInputRootFiles;
      //TList *fInputMidasFiles;
      //static std::vector<std::string> *fInputRootFile;
      //static std::vector<std::string> *fInputMidasFile;
      //static std::vector<std::string> *fInputCalFile;
      //static std::vector<std::string> *fInputOdbFile;




   ClassDef(TGRSIint,0);
};

#endif
