#ifndef TGRSIINT_H
#define TGRSIINT_H

/** \addtogroup GROOT
 *  @{
 */

#ifndef __CINT__
#include <thread>
#endif

#include <cstdio>
#include <string>

#include "TSystem.h"
#include "TSysEvtHandler.h"
#include "TRint.h"
#include "TList.h"
#include "TEnv.h"
#include "TFile.h"

#include "Globals.h"
#include "TRawFile.h"
#include "TMidasFile.h"
#include "TLstFile.h"

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

  //void GetOptions(int* argc,char** argv);
  void PrintHelp(bool);
  void PrintLogo(bool);
  bool HandleTermInput();
  int  TabCompletionHook(char*,int*,std::ostream&);

  TFile* OpenRootFile(const std::string& filename, Option_t* opt="read");
  TMidasFile* OpenMidasFile(const std::string& filename);
  TLstFile* OpenLstFile(const std::string& filename);
  void RunMacroFile(const std::string& filename);

  void Terminate(Int_t exit_status = 0);

  static TEnv* GetEnv() { return fGRSIEnv; }

  Long_t ProcessLine(const char* line,Bool_t sync=kFALSE,Int_t* error=0);

  void DelayedProcessLine_Action();

private:
  void SetupPipeline();
  void LoopUntilDone();
  //bool FileAutoDetect(std::string fileName, long fileSize);
  void InitFlags();
  void ApplyOptions();
  void DrawLogo();
  void LoadGROOTGraphics();
  void LoadExtraClasses();

  Long_t DelayedProcessLine(std::string message);

  TTimer* fKeepAliveTimer;
#ifndef __CINT__
  std::thread::id main_thread_id;
#endif

private:
  //bool fPrintLogo;
  // bool fPrintHelp;

  // bool fAutoSort;
  // bool fFragmentSort;
  // bool fMakeAnalysisTree;
  bool fIsTabComplete;
  bool fAllowedToTerminate;
  int fRootFilesOpened;
  int fMidasFilesOpened;
  std::string fNewFragmentFile;

  std::vector<TRawFile*> fRawFiles;

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
