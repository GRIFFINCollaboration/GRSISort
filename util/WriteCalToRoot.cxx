#include "Globals.h"
#include "TFile.h"
#include "TSystem.h"

#include "TGRSIOptions.h"
#include "TChannel.h"
#include "TParserLibrary.h"

int main(int argc, char** argv)
{
   if(argc < 4) {
      std::cout << "Usage: WriteCalToRoot <update/replace> <calfile.cal> <root file(s)>" << std::endl;
      return 1;
   }

   // there is a limit of 1024 open files on some (all?) linux systems, so we check that here
   if(argc > 1020) {
      std::cerr << DRED << "More than 1020 files provided, will only process first 1020 ones from " << argv[3] << " to " << argv[1019] << RESET_COLOR << std::endl;
      argc = 1020;
   }

   std::vector<const char*> badFile;
   std::vector<const char*> badTree;

   // read .grsirc
   grsi::SetGRSIEnv();
   // load TGRSIOptions (to get the parser library), we set the number of arguments to 1 so that the options
   // specific to this program are ignored by TGRSIOptions
   auto* opt = TGRSIOptions::Get(1, argv);
   // load parser library if provided
   if(!opt->ParserLibrary().empty()) {
      try {
         TParserLibrary::Get()->Load();
      } catch(std::runtime_error& e) {
         // if we failed to load the library, try to continue w/o it
         std::cerr << DRED << e.what() << RESET_COLOR << std::endl;
      }
   } else {
      std::cout << "No parser library set!" << std::endl;
   }

   // See if we can open the cal file
   if(TChannel::ReadCalFile(argv[2]) < 1) {
      std::cout << "Bad Cal File: " << argv[2] << std::endl;
      return 1;
   }

   // check if the should update or replace the existing calibration
   bool        update = false;
   std::string option = argv[1];
   std::transform(option.begin(), option.end(), option.begin(), ::tolower);
   if(option == "update") {
      update = true;
   } else if(option != "replace") {
      std::cout << R"(Wrong option ")" << option << R"(", should be either "update" or "replace")" << std::endl;
      return 1;
   }

   std::cout << (update ? "Updating" : "Replacing") << " calibration for " << argc - 3 << " file";
   if(argc > 4) { std::cout << "s from " << argv[3] << " to " << argv[argc - 1]; }
   std::cout << std::endl;

   // Open cal-file so that we can re-use it in the loop instead of opening tons of files
   std::ifstream calfile(argv[2]);

   // Loop over the files in the argv list
   for(int i = 3; i < argc; ++i) {
      std::cout << i - 2 << ". file: " << argv[i] << std::endl;

      if(gSystem->AccessPathName(argv[i])) {
         std::cout << DRED << "No file " << argv[i] << " found." << RESET_COLOR << std::endl;
         badFile.push_back(argv[i]);
         continue;
      }

      TFile file(argv[i], "update");
      if(!file.IsOpen()) {
         std::cout << DRED << "Could not open " << argv[i] << "." << RESET_COLOR << std::endl;
         badFile.push_back(argv[i]);
         continue;
      }

      if((file.Get("FragmentTree") == nullptr) && (file.Get("AnalysisTree") == nullptr)) {
         std::cout << DRED << "Could not find a fragment ot analysis tree in " << argv[i] << "." << RESET_COLOR << std::endl;
         badTree.push_back(argv[i]);
         continue;
      }

      TChannel::DeleteAllChannels();
      if(update) {
         TChannel::ReadCalFromCurrentFile();
      }
      TChannel::ReadCalFile(calfile);
      TChannel::WriteToRoot();
      file.Close();
   }

   calfile.close();

   if(!badFile.empty()) {
      std::cout << "File(s) that could not be opened:";
      for(auto& file : badFile) {
         std::cout << " " << BLUE << file << RESET_COLOR;
      }
   }
   std::cout << std::endl;

   if(!badTree.empty()) {
      std::cout << "File(s) missing analysis and fragment tree:";
      for(auto& file : badTree) {
         std::cout << " " << BLUE << file << RESET_COLOR;
      }
   }
   std::cout << std::endl;

   return 0;
}
