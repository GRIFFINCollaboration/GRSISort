#include <map>
#include "Globals.h"
#include "TFile.h"
#include "TSystem.h"
#include "TEnv.h"

#include "TGRSIOptions.h"
#include "TChannel.h"
#include "TParserLibrary.h"
#include "TRunInfo.h"

int main(int argc, char** argv)
{
   if(argc < 2) {
      std::cout<<"Usage: "<<argv[0]<<" <root file(s)"<<std::endl;
      return 1;
   }

   std::vector<const char*> badFile;
   std::vector<const char*> badChannel;

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
         std::cerr<<DRED<<e.what()<<RESET_COLOR<<std::endl;
      }
   } else {
      std::cout<<"No parser library set!"<<std::endl;
   }

   // Loop over the files in the argv list
   for(int i = 1; i < argc; ++i) {
      if(gSystem->AccessPathName(argv[i])) {
         std::cout<<DRED<<"No file "<<argv[i]<<" found."<<RESET_COLOR<<std::endl;
         badFile.push_back(argv[i]);
         continue;
      }

      TFile f(argv[i]);
      if(!f.IsOpen()) {
         std::cout<<DRED<<"Could not open "<<argv[i]<<"."<<RESET_COLOR<<std::endl;
         badFile.push_back(argv[i]);
         continue;
      }

      if((f.Get("Channel") == nullptr) || (f.Get("RunInfo") == nullptr)) {
         std::cout<<DRED<<"Could not find Channel or RunInfo in "<<argv[i]<<"."<<RESET_COLOR<<std::endl;
         badChannel.push_back(argv[i]);
         continue;
      }

      int runNumber    = TRunInfo::Get()->RunNumber();
      int subRunNumber = TRunInfo::Get()->SubRunNumber();

      // This ensures that no information from the previous file is written to the cal-file for this file.
      TChannel::DeleteAllChannels();
      TChannel::ReadCalFromCurrentFile();
      TChannel::WriteCalFile(Form("%05d_%03d.cal", runNumber, subRunNumber));
   }

   if(!badFile.empty()) {
      std::cout<<"File(s) that could not be opened:";
      for(auto& file : badFile) {
         std::cout<<" "<<BLUE<<file<<RESET_COLOR;
      }
   }
   std::cout<<std::endl;

   if(!badChannel.empty()) {
      std::cout<<"File(s) that had no Channel or no RunInfo:";
      for(auto& file : badChannel) {
         std::cout<<" "<<BLUE<<file<<RESET_COLOR;
      }
   }
   std::cout<<std::endl;

   return 0;
}
