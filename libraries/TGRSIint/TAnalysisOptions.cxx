#include "TAnalysisOptions.h"

#include <algorithm>
#include <cctype>
#include <iostream>

#include "TEnv.h"
#include "TKey.h"

#include "Globals.h"
#include "DynamicLibrary.h"
#include "TGRSIUtilities.h"
#include "GRootCommands.h"

TAnalysisOptions::TAnalysisOptions()
{
   Clear();
}

void TAnalysisOptions::Clear(Option_t*)
{
   /// Clears all of the variables in the TAnalysisOptions
   fBuildWindow           = 200;
   fAddbackWindow         = 300;
   fStaticWindow          = false;
   fWaveformFitting       = false;
   fIsCorrectingCrossTalk = true;
}

void TAnalysisOptions::Print(Option_t*) const
{
   /// Print the current status of TAnalysisOptions, includes all names, lists and flags
   std::cout<<BLUE<<"fBuildWindow: "<<DCYAN<<fBuildWindow<<std::endl
            <<BLUE<<"fAddbackWindow: "<<DCYAN<<fAddbackWindow<<std::endl
            <<BLUE<<"fStaticWindow: "<<DCYAN<<fStaticWindow<<std::endl
            <<BLUE<<"fWaveformFitting: "<<DCYAN<<fWaveformFitting<<std::endl
            <<BLUE<<"fIsCorrectingCrossTalk: "<<DCYAN<<fIsCorrectingCrossTalk<<std::endl
            <<RESET_COLOR<<std::endl;
}

bool TAnalysisOptions::WriteToFile(TFile* file)
{
   /// Writes options information to the root file
   // Maintain old gDirectory info
   bool        success = true;
   TDirectory* oldDir  = gDirectory;

   if(file == nullptr) {
      file = gDirectory->GetFile();
   }
   file->cd();
   std::string oldoption = std::string(file->GetOption());
   if(oldoption == "READ") {
      file->ReOpen("UPDATE");
   }
   if(!gDirectory) {
      printf("No file opened to write to.\n");
      success = false;
   } else {
      Write();
   }

   printf("Writing Run Information to %s\n", gDirectory->GetFile()->GetName());
   if(oldoption == "READ") {
      printf("  Returning %s to \"%s\" mode.\n", gDirectory->GetFile()->GetName(), oldoption.c_str());
      file->ReOpen("READ");
   }
   oldDir->cd(); // Go back to original gDirectory

   return success;
}

void TAnalysisOptions::ReadFromFile(const std::string& file)
{
   TDirectory* oldDir = gDirectory;
   auto        f      = new TFile(file.c_str());
   if(f->IsOpen()) {
      TList* list = f->GetListOfKeys();
      TIter  iter(list);
      std::cout<<"Reading Options from file:"<<CYAN<<f<<RESET_COLOR<<std::endl;
      while(TKey* key = static_cast<TKey*>(iter.Next())) {
         if(!key || strcmp(key->GetClassName(), "TAnalysisOptions")) {
            continue;
         }

         *this = *static_cast<TAnalysisOptions*>(key->ReadObj());
         f->Close();
         oldDir->cd();
         return;
      }
   } else {
      std::cout<<R"(Failed to open file ")"<<file<<R"(")"<<std::endl;
   }
   oldDir->cd();
}

void TAnalysisOptions::SetCorrectCrossTalk(const bool flag, Option_t* opt)
{
   fIsCorrectingCrossTalk = flag;
   TString opt1           = opt;
   opt1.ToUpper();
   if(opt1.Contains("Q")) {
      return;
   }

   printf("Please call TGriffin::ResetFlags() on current event to avoid bugs\n");
}
