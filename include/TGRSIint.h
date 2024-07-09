#ifndef TGRSIINT_H
#define TGRSIINT_H

/** \addtogroup Sorting
 *  @{
 */

//////////////////////////////////////////////////
///
/// \class TGRSIint
///
/// TGRSIint is a class that acts as an interpreter for GRSISort. The
/// idea is that TGRSIint will read in cal, info, mid and root files
/// and open them appropriately (order) and do the correct thing with
/// them. It also applies the command line options. Finally, TGRSIint
/// allows us to take over the interpreter to do some GRSI things.
///
///
///////////////////////////////////////////////////

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
#include "TChain.h"
#include "TStopwatch.h"

#include "Globals.h"
#include "TRawFile.h"

extern TChain* gFragment;
extern TChain* gAnalysis;

class TGRSIint : public TRint {
private:
   TGRSIint(int argc, char** argv, void* options = nullptr, int numOptions = 0, bool noLogo = false,
            const char* appClassName = "grsisort");

   static TEnv* fGRSIEnv;   ///< GRSI environment

public:
   static TGRSIint* fTGRSIint;   ///< Static pointer (singleton)
   static TGRSIint* instance(int argc = 0, char** argv = nullptr, void* options = nullptr, int numOptions = -1,
                             bool noLogo = false, const char* appClassName = "grsisort");

   ~TGRSIint() override;

   static void PrintHelp(bool);
   void        PrintLogo(bool) override;
   bool        HandleTermInput() override;
   int         TabCompletionHook(char*, int*, std::ostream&) override;

   TFile*    OpenRootFile(const std::string& filename, Option_t* opt = "read");
   TRawFile* OpenRawFile(const std::string& filename);
   void      RunMacroFile(const std::string& filename);

   void Terminate(Int_t status = 0) override;

   static TEnv* GetEnv() { return fGRSIEnv; }

   Long_t ProcessLine(const char* line, Bool_t sync = kFALSE, Int_t* error = nullptr) override;

   void DelayedProcessLine_Action();

private:
   void        SetupPipeline();
   void        LoopUntilDone();
   void        ApplyOptions();
   static void DrawLogo();
   static void LoadGROOTGraphics();
   void        LoadExtraClasses();

   Long_t DelayedProcessLine(std::string command);

   TTimer* fKeepAliveTimer;   ///< Time of process
#ifndef __CINT__
   std::thread::id main_thread_id;   ///< Main sorting thread id
#endif

   bool        fIsTabComplete;        ///< Flag for tab completion hook
   bool        fAllowedToTerminate;   ///< Flag for shutting down GRSISort
   int         fRootFilesOpened;      ///< Number of ROOT files opened
   int         fRawFilesOpened;       ///< Number of Raw Files opened
   std::string fNewFragmentFile;      ///< New fragment file name

   std::vector<TRawFile*> fRawFiles;   ///< List of Raw files opened

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIint, 0);   // Interpreter for GRSISort
   /// \endcond
};

//////////////////////////////////////////////////
///
/// \class TGRSIInterruptHandler
///
/// Handles interruptions such as ctrl-c
///
///////////////////////////////////////////////////

class TGRSIInterruptHandler : public TSignalHandler {
public:
   TGRSIInterruptHandler() : TSignalHandler(kSigInterrupt, false) {}
   bool Notify() override;
};
/*! @} */
#endif
