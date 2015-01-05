#ifndef TGRSIINT_H
#define TGRSIINT_H


#include "Globals.h"

#include <cstdio>
#include <string>

#include <TSystem.h>
#include <TSysEvtHandler.h>
#include <TRint.h>
#include <TList.h>
#include <TEnv.h>

class TGRSIint : public TRint {

   private:
      TGRSIint(int argc, char **argv,void *options = 0, 
            int numOptions = 0, bool noLogo = false, const char *appClassName = "grsisort") ;

      static TEnv *fGRSIEnv;

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

      static TEnv *GetEnv() { return fGRSIEnv; }

   private:
      bool FileAutoDetect(std::string filename, long filesize);
      void InitFlags();
      void ApplyOptions();
      //void SetEnv();
      void DrawLogo();

   private: 
      bool fPrintLogo;
      bool fPrintHelp;
      
      bool fAutoSort;
      bool fFragmentSort;
      bool fMakeAnalysisTree;


   ClassDef(TGRSIint,0);
};


class TGRSIInterruptHandler : public TSignalHandler {
   public:
      TGRSIInterruptHandler():TSignalHandler(kSigInterrupt,false) { }
      bool Notify();
};



#endif
