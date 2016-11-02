#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TProof.h"
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

#include <unistd.h>
const char * const nulFileName = "/dev/null";
#define CROSS_DUP(fd) dup(fd)
#define CROSS_DUP2(fd, newfd) dup2(fd, newfd)

void LoadLibsIntoProof(TProof* proof){
   //This function Loops through all libraries in GRSISort, and brute-force loads them into proof
   //This is done because order of loading matters. We suppress the cout output because proof->Exec
   //barfs all of the output. To me there is no obvious way to load every library on each of the PROOF slaves
   
   const char* pPath = getenv("GRSISYS");

   //First set the include path on each slave
   proof->Exec(Form("gInterpreter->AddIncludePath(\"%s/include\")",pPath));
   std::cout << "Loading Libraries" << std::endl;
   //This block builds a list of all of the files that end in .so in the lib directory
   TSystemDirectory dir(Form("%s/lib",pPath),Form("%s/lib",pPath));
   TList *files = dir.GetListOfFiles();
   std::vector<TString> files_copy;
   if(files) {
      TSystemFile *file;
      TString fname;
      TIter next(files);
      while((file=dynamic_cast<TSystemFile*>(next()))) {
         fname = file->GetName();
         if(!file->IsDirectory() && fname.EndsWith(".so")) {
            files_copy.push_back(fname);
         }
      }
   }

   //This block quietly loops through the libraries a total of nLibraries times to brute force load all dependencies
   int stdoutBackupFd;
   FILE *nullOut;
   /* duplicate stdout */
   stdoutBackupFd = CROSS_DUP(STDOUT_FILENO);
   fflush(stdout);
   //Now load all libraries brute force until it works.
   for(size_t i = 0; i<files_copy.size(); ++i){
      for(auto it = files_copy.begin(); it != files_copy.end(); ++it){
         nullOut = fopen(nulFileName, "w");
           CROSS_DUP2(fileno(nullOut), STDOUT_FILENO);
    
         proof->Exec(Form("gSystem->Load(\"%s/lib/%s\")",pPath,it->Data()));
         
         fflush(stdout);
         fclose(nullOut);
         // Restore stdout
         CROSS_DUP2(stdoutBackupFd, STDOUT_FILENO);
         close(stdoutBackupFd);
         fflush(stdout);
      }
   }
}

int main(int argc, char **argv) {

   TGRSIOptions *opt = TGRSIOptions::Get(argc,argv);;

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
   TProof *proof = TProof::Open("");
   if(!proof){
      std::cout << "Can't connect to proof" << std::endl;
      return 0;
   }
   proof->AddEnvVar("GRSISYS",pPath);
   proof->AddIncludePath(Form("%s/include",pPath));
   proof->AddDynamicPath(Form("%s/lib",pPath));
   LoadLibsIntoProof(proof);

   std::vector<std::string> fragment_list;
   std::vector<std::string> analysis_list;
   //Find which files have fragment trees in them
   for(auto i = opt->RootInputFiles().begin(); i!= opt->RootInputFiles().end(); ++i){
      TFile *in_file = TFile::Open(i->c_str());
      if(in_file && in_file->IsOpen()){
         if(in_file->FindObjectAny("FragmentTree")){
            fragment_list.push_back(*i);
         }
         if(in_file->FindObjectAny("AnalysisTree")){
            analysis_list.push_back(*i);
         }
         in_file->Close();
      }
   }

   //We want to loop over the root files that were added to grsiproof
   for(auto i = fragment_list.begin(); i!= fragment_list.end(); ++i){
      TChain *proof_chain = new TChain("FragmentTree");
      proof_chain->Add(i->c_str());
      
      //Go through chain to get calibration. This seems more complicated than it needs to be but will let us do real chains in the future.
      TObjArray *file_elements = proof_chain->GetListOfFiles();
      TIter next(file_elements);
      TChainElement *chEl=0;
      while((chEl = dynamic_cast<TChainElement*>(next()))) {
         TFile f(chEl->GetTitle());
         f.cd();
         TChannel::ReadCalFromCurrentFile();
      }
      proof_chain->GetListOfFiles()->Print();
      //Start getting ready to run proof
      proof->ClearCache();
      proof_chain->SetProof();

      for(auto macro_it = opt->MacroInputFiles().begin(); macro_it != opt->MacroInputFiles().end(); ++macro_it){
         std::cout <<"Currently Running: " << (Form("%s",macro_it->c_str()))<<std::endl;
         proof_chain->Process(Form("%s+",macro_it->c_str()));
      }
      
      if(proof_chain){ delete proof_chain; proof_chain = nullptr; }
   }

   for(auto i = analysis_list.begin(); i!= analysis_list.end(); ++i){
      TChain *proof_chain = new TChain("AnalysisTree");
      proof_chain->Add(i->c_str());
      
      //Go through chain to get calibration. This seems more complicated than it needs to be but will let us do real chains in the future.
      TObjArray *file_elements = proof_chain->GetListOfFiles();
      TIter next(file_elements);
      TChainElement *chEl=0;
      while((chEl = dynamic_cast<TChainElement*>(next()))) {
         TFile f(chEl->GetTitle());
         f.cd();
         TChannel::ReadCalFromCurrentFile();
      }
      proof_chain->GetListOfFiles()->Print();
      //Start getting ready to run proof
      proof->ClearCache();
      proof_chain->SetProof();
      for(auto macro_it = opt->MacroInputFiles().begin(); macro_it != opt->MacroInputFiles().end(); ++macro_it){
         std::cout <<"Currently Running: " << (Form("%s",macro_it->c_str()))<<std::endl;
         proof_chain->Process(Form("%s+",macro_it->c_str()));
      }
      
      if(proof_chain){ delete proof_chain; proof_chain = nullptr; }
   }

}



