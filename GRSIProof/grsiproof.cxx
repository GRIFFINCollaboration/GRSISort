#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TGRSIProof.h"
#include "TProofLite.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TChainElement.h"
#include "TROOT.h"
#include "TInterpreter.h"
#include "TGRSIOptions.h"
#include "TChannel.h"

#include <iostream>
#include <vector>
#include <string>

void Analyze(const char* tree_type, TProof* proof){
   TGRSIOptions *opt = TGRSIOptions::Get();
   std::vector<std::string> tree_list;

   //Loop over all of the file names, find all the files with the tree type in them and are "openable"
   for(auto i = opt->RootInputFiles().begin(); i!= opt->RootInputFiles().end(); ++i){
      TFile *in_file = TFile::Open(i->c_str());
      if(in_file && in_file->IsOpen()){
         if(in_file->FindObjectAny(tree_type)){
            tree_list.push_back(*i);
         }
         in_file->Close(); //Close the files when you are done with them
      }
   }

   TChain *proof_chain = new TChain(tree_type);
   //loop over the list of files that belong to this tree type and add them to the chain
   for(auto i = tree_list.begin(); i!= tree_list.end(); ++i){
      proof_chain->Add(i->c_str()); //First add the file to the chain.
      
      //Start getting ready to run proof
      proof->ClearCache();
      proof_chain->SetProof();

      for(auto macro_it = opt->MacroInputFiles().begin(); macro_it != opt->MacroInputFiles().end(); ++macro_it){
         std::cout <<"Currently Running: " << (Form("%s",macro_it->c_str()))<<std::endl;
         proof_chain->Process(Form("%s+",macro_it->c_str()));
      }
   }
   //Delete the proof chain now that we are done with it.
   if(proof_chain){ delete proof_chain; proof_chain = nullptr; }

}



int main(int argc, char **argv) {

   TGRSIOptions *opt = TGRSIOptions::Get(argc,argv);

   //Add the path were we store headers for GRSIProof macros to see
   const char* pPath = getenv("GRSISYS");
   gInterpreter->AddIncludePath(Form("%s/include",pPath));
   //The first thing we want to do is see if we can compile the macros that are passed to us
   if(!opt->MacroInputFiles().size()){
      std::cout <<DRED << "Can't PROOF if there is no MACRO" << RESET_COLOR << std::endl;
      return 0;
   }
   std::cout<< DCYAN << "************************* MACRO COMPILATION ****************************" << RESET_COLOR << std::endl;
   for(auto i = opt->MacroInputFiles().begin(); i != opt->MacroInputFiles().end(); ++i){
      Int_t error_code = gROOT->LoadMacro(Form("%s+",i->c_str()));
      if(error_code != 0){//TODO: Fix this check
         std::cout<< DRED << *i << " failed to compile properly.. ABORT!" << RESET_COLOR << std::endl;
         return 0;
      }
   }
   std::cout<< DCYAN << "************************* END COMPILATION ******************************" << RESET_COLOR << std::endl;

   if(opt->CalInputFiles().size())
      std::cout << DRED << "Cal Files are currently ignored in GRSIProof, please write calibration to tree" << RESET_COLOR << std::endl;

   if(opt->InputMidasFiles().size())
      std::cout << DRED << "Can't Proof a Midas file..." << RESET_COLOR << std::endl;

   //The first thing we do is get the PROOF Lite instance to run
   TGRSIProof *proof = TGRSIProof::Open("");
   if(!proof){
      std::cout << "Can't connect to proof" << std::endl;
      return 0;
   }
   proof->AddEnvVar("GRSISYS",pPath);
   gInterpreter->AddIncludePath(Form("%s/include",pPath));
   proof->AddIncludePath(Form("%s/include",pPath));
   proof->AddDynamicPath(Form("%s/lib",pPath));

   Analyze("FragmentTree",proof);
   Analyze("AnalysisTree",proof);

}



