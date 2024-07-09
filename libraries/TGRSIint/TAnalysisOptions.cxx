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

void TAnalysisOptions::Clear(Option_t*)
{
   /// Clears all of the variables in the TAnalysisOptions
   fBuildWindow            = 2000;
   fBuildEventsByTimeStamp = false;
   fAddbackWindow          = 300;
   fSuppressionWindow      = 300.;
   fSuppressionEnergy      = 0.;
   fStaticWindow           = false;
   fWaveformFitting        = false;
   fIsCorrectingCrossTalk  = true;
}

void TAnalysisOptions::Print(Option_t*) const
{
   /// Print the current status of TAnalysisOptions, includes all names, lists and flags
   std::cout << BLUE << "fBuildWindow: " << DCYAN << fBuildWindow << std::endl
             << BLUE << "fBuildEventsByTimeStamp: " << DCYAN << fBuildEventsByTimeStamp << std::endl
             << BLUE << "fAddbackWindow: " << DCYAN << fAddbackWindow << std::endl
             << BLUE << "fSuppressionWindow: " << DCYAN << fSuppressionWindow << std::endl
             << BLUE << "fSuppressionEnergy: " << DCYAN << fSuppressionEnergy << std::endl
             << BLUE << "fStaticWindow: " << DCYAN << fStaticWindow << std::endl
             << BLUE << "fWaveformFitting: " << DCYAN << fWaveformFitting << std::endl
             << BLUE << "fIsCorrectingCrossTalk: " << DCYAN << fIsCorrectingCrossTalk << std::endl
             << RESET_COLOR << std::endl;
}

bool TAnalysisOptions::WriteToFile(const std::string& fileName)
{
   TFile file(fileName.c_str(), "update");
   if(file.IsOpen()) {
      return WriteToFile(&file);
   }
   std::cout << R"(Failed to open file ")" << fileName << R"(" in update mode!)" << std::endl;
   return false;
}

bool TAnalysisOptions::WriteToFile(TFile* file)
{
   /// Writes options information to the root file
   // Maintain old gDirectory info
   bool        success = true;
   TDirectory* oldDir  = gDirectory;

   // if no file was provided, try to use the current file
   if(file == nullptr) {
      file = gDirectory->GetFile();
   }
   // check if we got a file
   if(file == nullptr) {
      std::cout << "Error, no file provided and no file open (gDirectory = " << gDirectory->GetName() << ")!" << std::endl;
      return !success;
   }

   file->cd();
   std::string oldoption = std::string(file->GetOption());
   if(oldoption == "READ") {
      file->ReOpen("UPDATE");
   }

   // check again that we have a directory to write to
   if(!gDirectory) {   // we don't compare to nullptr here, as ROOT >= 6.24.00 uses the TDirectoryAtomicAdapter structure with a bool() operator
      std::cout << "No file opened to write TAnalysisOptions to." << std::endl;
      return !success;
   }
   // write analysis options
   std::cout << "Writing TAnalysisOptions to " << gDirectory->GetFile()->GetName() << std::endl;
   Write("AnalysisOptions", TObject::kOverwrite);

   // check if we need to change back to read mode
   if(oldoption == "READ") {
      std::cout << "  Returning " << gDirectory->GetFile()->GetName() << " to \"" << oldoption.c_str() << "\" mode." << std::endl;
      file->ReOpen("READ");
   }

   // go back to original gDirectory
   oldDir->cd();

   return success;
}

void TAnalysisOptions::ReadFromFile(const std::string& fileName)
{
   TFile file(fileName.c_str(), "read");
   if(file.IsOpen()) {
      return ReadFromFile(&file);
   }
   std::cout << R"(Failed to open file ")" << fileName << R"(" in read mode!)" << std::endl;
}

void TAnalysisOptions::ReadFromFile(TFile* file)
{
   TDirectory* oldDir = gDirectory;
   if(file != nullptr && file->IsOpen()) {
      TList* list = file->GetListOfKeys();
      TIter  iter(list);
      while(TKey* key = static_cast<TKey*>(iter.Next())) {
         if((key == nullptr) || (strcmp(key->GetClassName(), "TAnalysisOptions") != 0)) {
            continue;
         }

         *this = *static_cast<TAnalysisOptions*>(key->ReadObj());
         oldDir->cd();
         return;
      }
      std::cout << R"(Failed to find analysis options in file ")" << CYAN << file->GetName() << RESET_COLOR << R"(":)" << std::endl;
   } else {
      std::cout << R"(File ")" << file << R"(" is null or not open)" << std::endl;
   }
   oldDir->cd();
}

void TAnalysisOptions::SetCorrectCrossTalk(const bool& flag, Option_t* opt)
{
   fIsCorrectingCrossTalk = flag;
   TString opt1           = opt;
   opt1.ToUpper();
   if(opt1.Contains("Q")) {
      return;
   }

   std::cout << "Please call TDetector::ResetFlags() on current event to avoid bugs" << std::endl;
}
