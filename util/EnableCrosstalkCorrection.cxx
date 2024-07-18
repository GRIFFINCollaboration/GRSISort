#include <iostream>
#include <iomanip>
#include <vector>

#include "TFile.h"
#include "TSystem.h"

#include "TAnalysisOptions.h"

int main(int argc, char** argv)
{
   if(argc < 2) {
      std::cerr << "Usage: " << argv[0] << " <list of root-files>" << std::endl;
      return 1;
   }

   // read .grsirc
   grsi::SetGRSIEnv();

   std::vector<std::string> badFiles;

   // loop over all files
   for(int i = 1; i < argc; ++i) {
      if(gSystem->AccessPathName(argv[i])) {
         std::cerr << DRED << "No file " << argv[i] << " found." << RESET_COLOR << std::endl;
         badFiles.emplace_back(argv[i]);
         continue;
      }

      TFile file(argv[i], "update");
      if(!file.IsOpen()) {
         std::cerr << DRED << "Unable to open " << argv[i] << "." << RESET_COLOR << std::endl;
         badFiles.emplace_back(argv[i]);
         continue;
      }

      auto* opt = static_cast<TAnalysisOptions*>(file.Get("AnalysisOptions"));

      if(opt == nullptr) {
         std::cerr << DRED << "Failed to find AnalysisOptions in " << argv[i] << "." << RESET_COLOR << std::endl;
         badFiles.emplace_back(argv[i]);
         file.Close();
         continue;
      }

      opt->SetCorrectCrossTalk(true, "q");
      opt->WriteToFile(&file);
      file.Close();
   }

   if(!badFiles.empty()) {
      std::cerr << "Files that couldn't be opened or had no AnalysisOptions:";
      for(auto& file : badFiles) {
         std::cerr << " " << BLUE << file << RESET_COLOR;
      }
      std::cerr << std::endl;
   }

   return 0;
}
