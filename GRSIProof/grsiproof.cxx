#include "TEnv.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TProofLite.h"
#include "TProofLog.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TChainElement.h"
#include "TROOT.h"
#include "TInterpreter.h"

#include "TGRSIProof.h"
#include "TGRSIOptions.h"
#include "TChannel.h"
#include "TRunInfo.h"
#include "TObjectWrapper.h"
#include "TStopwatch.h"
#include "TGRSIMap.h"
#include "TPPG.h"
#include "TCutG.h"

#include <iostream>
#include <vector>
#include <string>
#include <csignal>
#include <libgen.h>

TGRSIProof*   gGRSIProof;
TGRSIOptions* gGRSIOpt;
TStopwatch*   gStopwatch;
TPPG*         gPpg;

bool startedProof = false;
bool controlC     = false;

void Analyze(const char* treeType)
{
   std::vector<std::string> treeList;
   TChannel*                channel = nullptr;

   // Loop over all of the file names, find all the files with the tree type in them and are "openable"
   for(const auto& i : gGRSIOpt->RootInputFiles()) {
      TFile* inputFile = TFile::Open(i.c_str());
      if((inputFile != nullptr) && inputFile->IsOpen()) {
         if(inputFile->FindObjectAny(treeType) != nullptr) {
            treeList.push_back(i);

            TRunInfo::AddCurrent();

            // try and add PPG from this file to global PPG
            if(inputFile->Get("TPPG") != nullptr) {
               std::cout << inputFile->GetName() << ": adding ppg" << std::endl;
               gPpg->Add(static_cast<TPPG*>(inputFile->Get("TPPG")));
            }
         }
         if(channel == nullptr || inputFile->FindObjectAny("Channel") != nullptr) {
            channel = static_cast<TChannel*>(inputFile->Get("Channel"));
         }
         inputFile->Close();   // Close the files when you are done with them
      }
   }

   // add the channel map to the input list
   gGRSIProof->AddInput(channel);

   if(treeList.empty()) {
      return;
   }

   TRunInfo::Get()->Print();
   gGRSIProof->AddInput(TRunInfo::Get());

   // add the global PPG to the input list
   gGRSIProof->AddInput(gPpg);

   auto* proofChain = new TChain(treeType);
   // loop over the list of files that belong to this tree type and add them to the chain
   for(auto& i : treeList) {
      proofChain->Add(i.c_str());   // First add the file to the chain.
   }
   // Start getting ready to run proof
   gGRSIProof->ClearCache();
   if(!(gGRSIOpt->SelectorOnly())) {
      proofChain->SetProof();
   }

   for(const auto& macroIt : gGRSIOpt->MacroInputFiles()) {
      std::cout << "Currently Running: " << (Form("%s", macroIt.c_str())) << std::endl;
      try {
         if(gGRSIOpt->NumberOfEvents() == 0) {
            proofChain->Process(Form("%s+", macroIt.c_str()));
         } else {
            proofChain->Process(Form("%s+", macroIt.c_str()), "", gGRSIOpt->NumberOfEvents());
         }
      } catch(TGRSIMapException<std::string>& e) {
         std::cout << DRED << "Exception when processing chain: " << e.detail() << RESET_COLOR << std::endl;
         throw e;
      }
      std::cout << "Done with " << (Form("%s", macroIt.c_str())) << std::endl;
   }

   // Delete the proof chain now that we are done with it.
	delete proofChain;
}

void AtExitHandler()
{
   // this function is called on normal exits (via std::atexit) or
   // if the programm is killed with ctrl-c (via sigaction and HandleSignal)
   if(controlC) return;
   controlC = true;
   if(startedProof) {
      std::cout << "getting session logs ..." << std::endl;
      TProofLog* pl = TProof::Mgr("proof://__lite__")->GetSessionLogs();
      if(pl != nullptr) {
         Int_t     runNumber    = TRunInfo::RunNumber();
         Int_t     subRunNumber = TRunInfo::SubRunNumber();

         std::string firstMacro;
         if(!gGRSIOpt->MacroInputFiles().empty()) { firstMacro = gGRSIOpt->MacroInputFiles().at(0); }
         firstMacro = basename(const_cast<char*>(firstMacro.c_str()));      // remove path
         firstMacro = firstMacro.substr(0, firstMacro.find_last_of('.'));   // remove extension

         if(runNumber != 0 && subRunNumber != -1) {
            // both run and subrun number set => single file processed
            pl->Save("*", Form("%s%05d_%03d.log", firstMacro.c_str(), runNumber, subRunNumber));
            std::cout << "Wrote logs to '" << Form("%s%05d_%03d.log", firstMacro.c_str(), runNumber, subRunNumber) << "'" << std::endl;
         } else if(runNumber != 0) {
            // multiple subruns of a single run
            pl->Save("*", Form("%s%05d_%03d-%03d.log", firstMacro.c_str(), runNumber, TRunInfo::FirstSubRunNumber(), TRunInfo::LastSubRunNumber()));
            std::cout << "Wrote logs to '" << Form("%s%05d_%03d-%03d.log", firstMacro.c_str(), runNumber, TRunInfo::FirstSubRunNumber(), TRunInfo::LastSubRunNumber()) << "'" << std::endl;
         } else {
            // multiple runs
            pl->Save("*", Form("%s%05d-%05d.log", firstMacro.c_str(), TRunInfo::FirstRunNumber(), TRunInfo::LastRunNumber()));
            std::cout << "Wrote logs to '" << Form("%s%05d-%05d.log", firstMacro.c_str(), TRunInfo::FirstRunNumber(), TRunInfo::LastRunNumber()) << "'" << std::endl;
         }
      } else {
         std::cout << "Failed to get logs!" << std::endl;
      }

      std::cout << "stopping all workers ..." << std::endl;
      gGRSIProof->StopProcess(true);
   }

   // print time it took to run grsiproof
   double realTime = gStopwatch->RealTime();
   int    hour     = static_cast<int>(realTime / 3600);
   realTime -= hour * 3600;
   int min = static_cast<int>(realTime / 60);
   realTime -= min * 60;
   std::cout << DMAGENTA << std::endl
             << "Done after " << hour << ":" << std::setfill('0') << std::setw(2) << min << ":"
             << std::setprecision(3) << std::fixed << realTime << " h:m:s"
             << RESET_COLOR << std::endl;
}

void HandleSignal(int)
{
   // sigaction requires a function that takes an integer as argument
   // since we don't care what the signal was, we just call AtExitHandler
   AtExitHandler();
}

static void CatchSignals()
{
   struct sigaction action{};
   action.sa_handler = HandleSignal;
   action.sa_flags   = 0;
   sigemptyset(&action.sa_mask);
   sigaction(SIGINT, &action, nullptr);
   sigaction(SIGTERM, &action, nullptr);
}

void SetGRSIEnv()
{
   std::string grsiPath = getenv("GRSISYS");   // Finds the GRSISYS path to be used by other parts of the grsisort code
   if(grsiPath.length() > 0) {
      grsiPath += "/";
   }
   // Read in grsirc in the GRSISYS directory to set user defined options on grsisort startup
   grsiPath += ".grsirc";
   gEnv->ReadFile(grsiPath.c_str(), kEnvChange);
}

int main(int argc, char** argv)
{
   gStopwatch = new TStopwatch;
   try {
      SetGRSIEnv();
      gGRSIOpt = TGRSIOptions::Get(argc, argv);
      if(gGRSIOpt->ShouldExit()) {
         return 0;
      }
   } catch(ParseError& e) {
      return 1;
   }
   gPpg = new TPPG;

   std::atexit(AtExitHandler);
   CatchSignals();

   // Add the path were we store headers for GRSIProof macros to see
   const char* pPath = getenv("GRSISYS");
   gInterpreter->AddIncludePath(Form("%s/include", pPath));
   // if we have a data parser/detector library, add it's include path as well
   std::string library = gGRSIOpt->ParserLibrary();
   if(!library.empty()) {
      size_t tmpPos = library.rfind("/lib/lib");
      if(tmpPos != std::string::npos) {
         gInterpreter->AddIncludePath(Form("%s/include", library.substr(0, tmpPos).c_str()));
      } else {
         std::cout << "Warning, expected dataparser/detector library location to be of form <path>/lib/lib<name>.so, but it is " << library << ". Won't be able to add include path!" << std::endl;
      }
      // no need to load the parser library, that is already taken care of by TGRSIProof class
   } else {
      std::cout << "Warning, no dataparser/detector library provided, won't be able to add include path!" << std::endl;
   }
   // The first thing we want to do is see if we can compile the macros that are passed to us
   if(gGRSIOpt->MacroInputFiles().empty()) {
      std::cout << DRED << "Can't PROOF if there is no MACRO" << RESET_COLOR << std::endl;
      return 1;
   }
   std::cout << DCYAN << "************************* MACRO COMPILATION ****************************" << RESET_COLOR
             << std::endl;
   for(const auto& i : gGRSIOpt->MacroInputFiles()) {
      Int_t errorCode = gSystem->CompileMacro(i.c_str(), "kgOs");   // k - keep shared library after session ends, g - add debuging symbols, O - optimize the code, s - silent, v - verbose output
      if(errorCode == 0) {
         std::cout << DRED << i << " failed to compile properly.. ABORT!" << RESET_COLOR << std::endl;
         return 1;
      }
   }
   std::cout << DCYAN << "************************* END COMPILATION ******************************" << RESET_COLOR
             << std::endl;

   if(!gGRSIOpt->InputFiles().empty()) {
      std::cout << DRED << "Can't Proof a Midas file..." << RESET_COLOR << std::endl;
   }

   // The first thing we do is get the PROOF Lite instance to run
   if(gGRSIOpt->GetMaxWorkers() >= 0) {
      std::cout << "Opening proof with '" << Form("workers=%d", gGRSIOpt->GetMaxWorkers()) << "'" << std::endl;
      gGRSIProof = TGRSIProof::Open(Form("workers=%d", gGRSIOpt->GetMaxWorkers()));
   } else if(gGRSIOpt->SelectorOnly()) {
      std::cout << "Opening proof with one worker (selector-only)" << std::endl;
      gGRSIProof = TGRSIProof::Open("workers=1");
   } else {
      gGRSIProof = TGRSIProof::Open("");
   }

   if(gGRSIProof == nullptr) {
      std::cout << "Couldn't connect to proof on first attempt, trying again" << std::endl;
      if(gGRSIOpt->GetMaxWorkers() >= 0) {
         std::cout << "Opening proof with '" << Form("workers=%d", gGRSIOpt->GetMaxWorkers()) << "'" << std::endl;
         gGRSIProof = TGRSIProof::Open(Form("workers=%d", gGRSIOpt->GetMaxWorkers()));
      } else if(gGRSIOpt->SelectorOnly()) {
         std::cout << "Opening proof with one worker (selector-only)" << std::endl;
         gGRSIProof = TGRSIProof::Open("workers=1");
      } else {
         gGRSIProof = TGRSIProof::Open("");
      }
   }

   if(gGRSIProof == nullptr) {
      std::cout << "Still can't connect to proof, try running it again?" << std::endl;
      return 0;
   }

   startedProof = true;

   // set some proof parameters
   // average rate, can also be set via Proof.RateEstimation in gEnv ("current" or "average)
   if(gGRSIOpt->AverageRateEstimation()) { gGRSIProof->SetParameter("PROOF_RateEstimation", "average"); }

   // Parallel unzip, can also be set via ProofPlayer.UseParallelUnzip
   if(gGRSIOpt->ParallelUnzip()) {
      gGRSIProof->SetParameter("PROOF_UseParallelUnzip", 1);
   } else {
      gGRSIProof->SetParameter("PROOF_UseParallelUnzip", 0);
   }

   // Tree cache
   if(gGRSIOpt->CacheSize() >= 0) {
      if(gGRSIOpt->CacheSize() == 0) {
         gGRSIProof->SetParameter("PROOF_UseTreeCache", 0);
      } else {
         gGRSIProof->SetParameter("PROOF_UseTreeCache", 1);
         gGRSIProof->SetParameter("PROOF_CacheSize", gGRSIOpt->CacheSize());
      }
   } else {
      // Use defaults
      gGRSIProof->DeleteParameters("PROOF_UseTreeCache");
      gGRSIProof->DeleteParameters("PROOF_CacheSize");
   }

   // Enable submergers, if required
   if(gGRSIOpt->Submergers() >= 0) {
      gGRSIProof->SetParameter("PROOF_UseMergers", gGRSIOpt->Submergers());
   } else {
      gGRSIProof->DeleteParameters("PROOF_UseMergers");
   }

   // enable perfomance output
   if(gGRSIOpt->ProofStats()) {
      gGRSIProof->SetParameter("PROOF_StatsHist", "");
      gGRSIProof->SetParameter("PROOF_StatsTrace", "");
      gGRSIProof->SetParameter("PROOF_RateTrace", "");
      gGRSIProof->SetParameter("PROOF_SlaveStatsTrace", "");
   }

   gGRSIProof->SetBit(TProof::kUsingSessionGui);
	TGRSIProof::AddEnvVar("GRSISYS", pPath);
   gInterpreter->AddIncludePath(Form("%s/include", pPath));
   gGRSIProof->AddIncludePath(Form("%s/include", pPath));
   gGRSIProof->AddDynamicPath(Form("%s/lib", pPath));

   gGRSIProof->AddInput(TGRSIOptions::AnalysisOptions());
   gGRSIProof->AddInput(new TNamed("pwd", getenv("PWD")));
   int index = 0;
   for(const auto& valFile : gGRSIOpt->ValInputFiles()) {
      gGRSIProof->AddInput(new TNamed(Form("valFile%d", index++), valFile.c_str()));
   }
   index = 0;
   for(const auto& calFile : gGRSIOpt->CalInputFiles()) {
      gGRSIProof->AddInput(new TNamed(Form("calFile%d", index++), calFile.c_str()));
   }
   index = 0;
   for(const auto& cutFile : gGRSIOpt->InputCutFiles()) {
      gGRSIProof->AddInput(new TNamed(Form("cutFile%d", index++), cutFile.c_str()));
   }
   gGRSIProof->AddInput(new TNamed("ParserLibrary", library.c_str()));

   if(gGRSIOpt->TreeName().empty()) {
      Analyze("FragmentTree");
      Analyze("AnalysisTree");
      Analyze("Lst2RootTree");
   } else {
      std::cout << "Running selector on tree '" << gGRSIOpt->TreeName() << "'" << std::endl;
      Analyze(gGRSIOpt->TreeName().c_str());
   }

   AtExitHandler();

   return 0;
}
