#ifndef TGRSIINT_H
#define TGRSIINT_H

/** \addtogroup GROOT
 *  @{
 */

#include <cstdio>
#include <string>

#include "TSystem.h"
#include "TSysEvtHandler.h"
#include "TRint.h"
#include "TList.h"
#include "TEnv.h"

#include "Globals.h"

class TGRSIint : public TRint {
   private:
      TGRSIint(int argc, char **argv,void *options = 0, 
					int numOptions = 0, bool noLogo = false, const char *appClassName = "grsisort") ;

      static TEnv* fGRSIEnv;

   public:
      static TGRSIint* fTGRSIint;
      static TGRSIint* instance(int argc = 0, char** argv = 0, void* options = 0, 
                             int numOptions = -1, bool noLogo = false, const char* appClassName = "grsisort");
      
      virtual ~TGRSIint();

      void GetOptions(int* argc,char** argv);
      void PrintHelp(bool);
      void PrintLogo(bool);
      bool HandleTermInput();
      int  TabCompletionHook(char*,int*,std::ostream&);

      static TEnv* GetEnv() { return fGRSIEnv; }

      Long_t ProcessLine(const char* line,Bool_t sync=kFALSE,Int_t* error=0);

   private:
      bool FileAutoDetect(std::string fileName, long fileSize);
      void InitFlags();
      void ApplyOptions();
      void DrawLogo();
      void LoadGROOTGraphics();
      void LoadExtraClasses();

   private: 
      bool fPrintLogo;
      bool fPrintHelp;
      
      bool fAutoSort;
      bool fFragmentSort;
      bool fMakeAnalysisTree;

/// \cond CLASSIMP
   ClassDef(TGRSIint,0);
/// \endcond
};

class TGRSIInterruptHandler : public TSignalHandler {
   public:
      TGRSIInterruptHandler():TSignalHandler(kSigInterrupt,false) { }
      bool Notify();
};
/*! @} */
#endif
