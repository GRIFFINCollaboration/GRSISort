#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TGRSIProof.h"
#include "TProofLite.h"
#include "TProofLog.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TChainElement.h"
#include "TROOT.h"
#include "TInterpreter.h"
#include "TGRSIOptions.h"
#include "TChannel.h"
#include "TGRSIRunInfo.h"
#include "TObjectWrapper.h"
#include "TStopwatch.h"
#include "TGRSIMap.h"

#include <iostream>
#include <vector>
#include <string>
#include <signal.h>

TGRSIProof* gGRSIProof;
TGRSIOptions* gGRSIOpt;
TStopwatch* gStopwatch;

bool startedProof = false;
bool controlC = false;

void Analyze(const char* tree_type)
{
   std::vector<std::string> tree_list;

   // Loop over all of the file names, find all the files with the tree type in them and are "openable"
   for(const auto& i : gGRSIOpt->RootInputFiles()) {
      TFile* in_file = TFile::Open(i.c_str());
      if((in_file != nullptr) && in_file->IsOpen()) {
         if(in_file->FindObjectAny(tree_type) != nullptr) {
            tree_list.push_back(i);

            // TODO: A smarter way of finding run info for run number and sub run number naming
            static bool info_set = false;
            if(!info_set) {
               TGRSIRunInfo::Get()->ReadInfoFromFile(in_file);
					TGRSIRunInfo::Get()->Print();
               info_set = true;
            }
         }
         in_file->Close(); // Close the files when you are done with them
      }
   }
	if(tree_list.empty()) {
		return;
	}

   auto* proof_chain = new TChain(tree_type);
   // loop over the list of files that belong to this tree type and add them to the chain
   for(auto& i : tree_list) {
      proof_chain->Add(i.c_str()); // First add the file to the chain.
   }
   // Start getting ready to run proof
   gGRSIProof->ClearCache();
   if(!(gGRSIOpt->SelectorOnly())) {
      proof_chain->SetProof();
   }

   for(const auto& macro_it : gGRSIOpt->MacroInputFiles()) {
      std::cout<<"Currently Running: "<<(Form("%s", macro_it.c_str()))<<std::endl;
		try {
			proof_chain->Process(Form("%s+", macro_it.c_str()));
		} catch(TGRSIMapException<std::string>& e) {
			std::cout<<DRED<<"Exception when processing chain: "<<e.detail()<<RESET_COLOR<<std::endl;
			throw e;
		}
   }

   // Delete the proof chain now that we are done with it.
   if(proof_chain != nullptr) {
      delete proof_chain;
      proof_chain = nullptr;
   }
}

void AtExitHandler()
{
	// this function is called on normal exits (via std::atexit) or
	// if the programm is killed with ctrl-c (via sigaction and HandleSignal)
	if(controlC) return;
	controlC = true;
	if(startedProof) {
		std::cout<<"getting session logs ..."<<std::endl;
		TProofLog* pl = TProof::Mgr("proof://__lite__")->GetSessionLogs();
		if(pl != nullptr) {
			pl->Save("*", gGRSIOpt->LogFile().c_str());
		} else {
			std::cout<<"Failed to get logs!"<<std::endl;
		}

		std::cout<<"stopping all workers ..."<<std::endl;
		gGRSIProof->StopProcess(true);
	}
	
	// print time it took to run grsiproof
   double realTime = gStopwatch->RealTime();
   int    hour     = static_cast<int>(realTime / 3600);
   realTime -= hour * 3600;
   int min = static_cast<int>(realTime / 60);
   realTime -= min * 60;
   std::cout<<DMAGENTA<<std::endl
            <<"Done after "<<hour<<":"<<std::setfill('0')<<std::setw(2)<<min<<":"
				<<std::setprecision(3)<<std::fixed<<realTime<<" h:m:s"
				<<RESET_COLOR<<std::endl;
}

void HandleSignal(int)
{
	// sigaction requires a function that takes an integer as argument
	// since we don't care what the signal was, we just call AtExitHandler
	AtExitHandler();
}

static void CatchSignals()
{
	struct sigaction action;
	action.sa_handler = HandleSignal;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
}

int main(int argc, char** argv)
{
	gStopwatch = new TStopwatch;
	try {
		gGRSIOpt = TGRSIOptions::Get(argc, argv);
		if(gGRSIOpt->ShouldExit()) {
			return 0;
		}
	} catch(ParseError& e) {
		return 1;
	}

	std::atexit(AtExitHandler);
	CatchSignals();

   // Add the path were we store headers for GRSIProof macros to see
   const char* pPath = getenv("GRSISYS");
   gROOT->SetMacroPath(Form("%s", pPath));
   gROOT->SetMacroPath(Form("%s/GRSIProof", pPath));
   gROOT->SetMacroPath(Form("%s/myAnalysis", pPath));
   gInterpreter->AddIncludePath(Form("%s/include", pPath));
   // The first thing we want to do is see if we can compile the macros that are passed to us
   if(gGRSIOpt->MacroInputFiles().empty()) {
      std::cout<<DRED<<"Can't PROOF if there is no MACRO"<<RESET_COLOR<<std::endl;
      return 0;
   }
   std::cout<<DCYAN<<"************************* MACRO COMPILATION ****************************"<<RESET_COLOR
            <<std::endl;
   for(const auto& i : gGRSIOpt->MacroInputFiles()) {
      // Int_t error_code = gROOT->LoadMacro(Form("%s+",i->c_str()));
      Int_t error_code = gSystem->CompileMacro(i.c_str(), "kO");
      if(error_code == 0) { // TODO: Fix this check
         std::cout<<DRED<<i<<" failed to compile properly.. ABORT!"<<RESET_COLOR<<std::endl;
         return 0;
      }
   }
   std::cout<<DCYAN<<"************************* END COMPILATION ******************************"<<RESET_COLOR
            <<std::endl;

   if(!gGRSIOpt->CalInputFiles().empty()) {
      std::cout<<DRED<<"Cal Files are currently ignored in GRSIProof, please write calibration to tree"
               <<RESET_COLOR<<std::endl;
   }

   if(!gGRSIOpt->InputMidasFiles().empty()) {
      std::cout<<DRED<<"Can't Proof a Midas file..."<<RESET_COLOR<<std::endl;
   }

   // The first thing we do is get the PROOF Lite instance to run
   if(gGRSIOpt->GetMaxWorkers() >= 0) {
      std::cout<<"Opening proof with '"<<Form("workers=%d", gGRSIOpt->GetMaxWorkers())<<"'"<<std::endl;
      gGRSIProof = TGRSIProof::Open(Form("workers=%d", gGRSIOpt->GetMaxWorkers()));
   } else if(gGRSIOpt->SelectorOnly()) {
      std::cout<<"Opening proof with one worker (selector-only)"<<std::endl;
      gGRSIProof = TGRSIProof::Open("workers=1");
   } else {
      gGRSIProof = TGRSIProof::Open("");
   }

   if(gGRSIProof == nullptr) {
      std::cout<<"Can't connect to proof"<<std::endl;
      return 0;
   }

	startedProof = true;

   gGRSIProof->SetBit(TProof::kUsingSessionGui);
   gGRSIProof->AddEnvVar("GRSISYS", pPath);
   gInterpreter->AddIncludePath(Form("%s/include", pPath));
   gGRSIProof->AddIncludePath(Form("%s/include", pPath));
   gGRSIProof->AddDynamicPath(Form("%s/lib", pPath));

   gGRSIProof->AddInput(gGRSIOpt->AnalysisOptions());
   gGRSIProof->AddInput(new TNamed("pwd", getenv("PWD")));
   int i = 0;
   for(const auto& valFile : gGRSIOpt->ValInputFiles()) {
      gGRSIProof->AddInput(new TNamed(Form("valFile%d", i++), valFile.c_str()));
   }
   i = 0;
   for(const auto& calFile : gGRSIOpt->CalInputFiles()) {
      gGRSIProof->AddInput(new TNamed(Form("calFile%d", i++), calFile.c_str()));
   }

   Analyze("FragmentTree");
   Analyze("AnalysisTree");
   Analyze("Lst2RootTree");

	return 0;
}
