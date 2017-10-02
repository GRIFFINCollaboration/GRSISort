#include "TChannel.h"
#include <map>
#include "Globals.h"
#include "TFile.h"
#include "TSystem.h"

int main(int argc, char** argv)
{
   if(argc < 3) {
      printf("Usage: WriteCalToRoot <calfile.cal> <root file 1> <root file 2> ...\n");
      return 0;
   }

   std::vector<const char*> bad_file;
   std::vector<const char*> bad_tree;

   // See if we can open the cal file
   if(TChannel::ReadCalFile(argv[1]) < 1) {
      printf("Bad Cal File: %s\n", argv[1]);
      exit(1);
   }

   // Loop over the files in the argv list
   for(int i = 2; i < argc; ++i) {
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

      TChannel::ReadCalFile(argv[1]);
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

   return 1;
}
