#include <map>
#include "Globals.h"
#include "TFile.h"
#include "TSystem.h"
#include "TEnv.h"

#include "TGRSIOptions.h"
#include "TChannel.h"
#include "TParserLibrary.h"

int main(int argc, char** argv)
{
   if(argc < 4) {
      printf("Usage: WriteCalToRoot <update/replace> <calfile.cal> <root file 1> <root file 2> ...\n");
      return 1;
   }

   std::vector<const char*> bad_file;
   std::vector<const char*> bad_tree;

	// read .grsirc
   std::string grsi_path = getenv("GRSISYS"); // Finds the GRSISYS path to be used by other parts of the grsisort code
   if(grsi_path.length() > 0) {
      grsi_path += "/";
   }
   // Read in grsirc in the GRSISYS directory to set user defined options on grsisort startup
   grsi_path += ".grsirc";
   gEnv->ReadFile(grsi_path.c_str(), kEnvChange);
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

   // See if we can open the cal file
   if(TChannel::ReadCalFile(argv[2]) < 1) {
      printf("Bad Cal File: %s\n", argv[1]);
      exit(1);
   }

	//check if the should update or replace the existing calibration
	bool update = false;
	std::string option = argv[1];
	std::transform(option.begin(), option.end(), option.begin(), ::tolower);
	if(option.compare("update") == 0) {
		update = true;
	} else if(option.compare("replace") != 0) {
		std::cout<<R"(Wrong option ")"<<option<<R"(", should be either "update" or "replace")"<<std::endl;
		return 1;
	}

   // Loop over the files in the argv list
   for(int i = 3; i < argc; ++i) {
      if(gSystem->AccessPathName(argv[i])) {
         printf(DRED "No file %s found.\n" RESET_COLOR, argv[i]);
         bad_file.push_back(argv[i]);
         continue;
      }

      TFile f(argv[i]);
      if(!f.IsOpen()) {
         printf(DRED "Could not open %s.\n" RESET_COLOR, argv[i]);
         bad_file.push_back(argv[i]);
         continue;
      }

      if((f.Get("FragmentTree") == nullptr) && (f.Get("AnalysisTree") == nullptr)) {
         printf(DRED "Could not find a fragment tree or analysis tree in %s\n." RESET_COLOR, argv[i]);
         bad_tree.push_back(argv[i]);
         continue;
      }

		TChannel::DeleteAllChannels();
		if(update) {
			TChannel::ReadCalFromCurrentFile();
		}
      TChannel::ReadCalFile(argv[2]);
      TChannel::WriteToRoot();
   }

   printf("Files that could not be opened:");
   for(auto& it : bad_file) {
      printf(BLUE " %s " RESET_COLOR, it);
   }

   printf("\n");

   printf("Files with bad trees:");
   for(auto& it : bad_tree) {
      printf(BLUE " %s " RESET_COLOR, it);
   }

   printf("\n");

   return 0;
}
