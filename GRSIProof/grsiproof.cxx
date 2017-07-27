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

#include <iostream>
#include <vector>
#include <string>

void Analyze(const char* tree_type, TProof* proof)
{
   TGRSIOptions*            opt = TGRSIOptions::Get();
   std::vector<std::string> tree_list;

   // Loop over all of the file names, find all the files with the tree type in them and are "openable"
   for(const auto& i : opt->RootInputFiles()) {
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

   auto* proof_chain = new TChain(tree_type);
   // loop over the list of files that belong to this tree type and add them to the chain
   for(auto& i : tree_list) {
      proof_chain->Add(i.c_str()); // First add the file to the chain.
   }
   // Start getting ready to run proof
   proof->ClearCache();
   if(!(opt->SelectorOnly())) {
      proof_chain->SetProof();
   }

   for(const auto& macro_it : opt->MacroInputFiles()) {
      std::cout<<"Currently Running: "<<(Form("%s", macro_it.c_str()))<<std::endl;
      proof_chain->Process(Form("%s+", macro_it.c_str()));
   }

   /*
   TChain *proof_chain = new TChain(tree_type);
   //loop over the list of files that belong to this tree type and add them to the chain
   for(auto i = tree_list.begin(); i!= tree_list.end(); ++i){
      proof_chain->Add(i->c_str()); //First add the file to the chain.

      //Start getting ready to run proof
      proof->ClearCache();
      if(!(opt->SelectorOnly()))
         proof_chain->SetProof();

      for(auto macro_it = opt->MacroInputFiles().begin(); macro_it != opt->MacroInputFiles().end(); ++macro_it){
         std::cout <<"Currently Running: "<<(Form("%s",macro_it->c_str()))<<std::endl;
         proof_chain->Process(Form("%s+",macro_it->c_str()));
      }
   }*/
   // Delete the proof chain now that we are done with it.
   if(proof_chain != nullptr) {
      delete proof_chain;
      proof_chain = nullptr;
   }
}

int main(int argc, char** argv)
{
   TGRSIOptions* opt = TGRSIOptions::Get(argc, argv);

   // Add the path were we store headers for GRSIProof macros to see
   const char* pPath = getenv("GRSISYS");
   gROOT->SetMacroPath(Form("%s", pPath));
   gROOT->SetMacroPath(Form("%s/GRSIProof", pPath));
   gROOT->SetMacroPath(Form("%s/myAnalysis", pPath));
   gInterpreter->AddIncludePath(Form("%s/include", pPath));
   // The first thing we want to do is see if we can compile the macros that are passed to us
   if(opt->MacroInputFiles().empty()) {
      std::cout<<DRED<<"Can't PROOF if there is no MACRO"<<RESET_COLOR<<std::endl;
      return 0;
   }
   std::cout<<DCYAN<<"************************* MACRO COMPILATION ****************************"<<RESET_COLOR
            <<std::endl;
   for(const auto& i : opt->MacroInputFiles()) {
      // Int_t error_code = gROOT->LoadMacro(Form("%s+",i->c_str()));
      Int_t error_code = gSystem->CompileMacro(i.c_str(), "kO");
      if(error_code == 0) { // TODO: Fix this check
         std::cout<<DRED<<i<<" failed to compile properly.. ABORT!"<<RESET_COLOR<<std::endl;
         return 0;
      }
   }
   std::cout<<DCYAN<<"************************* END COMPILATION ******************************"<<RESET_COLOR
            <<std::endl;

   if(!opt->CalInputFiles().empty()) {
      std::cout<<DRED<<"Cal Files are currently ignored in GRSIProof, please write calibration to tree"
               <<RESET_COLOR<<std::endl;
   }

   if(!opt->InputMidasFiles().empty()) {
      std::cout<<DRED<<"Can't Proof a Midas file..."<<RESET_COLOR<<std::endl;
   }

   // The first thing we do is get the PROOF Lite instance to run
   TGRSIProof* proof = nullptr;
   if(opt->GetMaxWorkers() >= 0) {
      std::cout<<"Opening proof with '"<<Form("workers=%d", opt->GetMaxWorkers())<<"'"<<std::endl;
      proof = TGRSIProof::Open(Form("workers=%d", opt->GetMaxWorkers()));
   } else if(opt->SelectorOnly()) {
      std::cout<<"Opening proof with one worker (selector-only)"<<std::endl;
      proof = TGRSIProof::Open("workers=1");
   } else {
      proof = TGRSIProof::Open("");
   }
   if(proof == nullptr) {
      std::cout<<"Can't connect to proof"<<std::endl;
      return 0;
   }
   proof->SetBit(TProof::kUsingSessionGui);
   proof->AddEnvVar("GRSISYS", pPath);
   gInterpreter->AddIncludePath(Form("%s/include", pPath));
   proof->AddIncludePath(Form("%s/include", pPath));
   proof->AddDynamicPath(Form("%s/lib", pPath));

   proof->AddInput(opt->AnalysisOptions());
   proof->AddInput(new TNamed("pwd", getenv("PWD")));
   int i = 0;
   for(const auto& valFile : opt->ValInputFiles()) {
      proof->AddInput(new TNamed(Form("valFile%d", i++), valFile.c_str()));
   }
   i = 0;
   for(const auto& calFile : opt->CalInputFiles()) {
      proof->AddInput(new TNamed(Form("calFile%d", i++), calFile.c_str()));
   }

   Analyze("FragmentTree", proof);
   Analyze("AnalysisTree", proof);
   Analyze("Lst2RootTree", proof);

   TProofLog* pl = TProof::Mgr("proof://__lite__")->GetSessionLogs();
   if(pl != nullptr) {
      pl->Save("*", opt->LogFile().c_str());
   } else {
      std::cout<<"Failed to get logs!"<<std::endl;
   }
}
