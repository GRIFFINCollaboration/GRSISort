#include "TGRSIUtilities.h"

#include <cstdlib>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

#include "TObjArray.h"
#include "TObjString.h"
#include "TPRegexp.h"
#include "TString.h"

bool file_exists(const char* filename)
{
   /// This checks if the path exist, and if it is a file and not a directory!
   struct stat buffer;
	int state = stat(filename, &buffer);
	// state != 0 means we couldn't get file attributes. This doesn't necessary mean the file
	// does not exist, we might just be missing permission to access it. But for our purposes
	// this is the same as the file not existing.
	if(state != 0) return false;
	// we got the file attributes, so it exsist, we just need to check if it is a directory.
   return !S_ISDIR(buffer.st_mode);
}

bool all_files_exist(const std::vector<std::string>& filenames)
{
   for(auto& filename : filenames) {
      if(!file_exists(filename.c_str())) {
         return false;
      }
   }
   return true;
}

int GetRunNumber(const std::string& fileName)
{
   if(fileName.length() == 0) {
      return 0;
   }
   std::size_t found = fileName.rfind(".root");
   if(found == std::string::npos) {
      return 0;
   }
   std::size_t found2 = fileName.rfind('-');

   if(found2 == std::string::npos) {
      found2 = fileName.rfind('_');
   }
   std::string temp;
   if(found2 == std::string::npos || fileName.compare(found2 + 4, 5, ".root") != 0) {
      temp = fileName.substr(found - 5, 5);
   } else {
      temp = fileName.substr(found - 9, 5);
   }
   return atoi(temp.c_str());
}

int GetSubRunNumber(const std::string& fileName)
{
   if(fileName.length() == 0) {
      return -1;
   }

   std::size_t found = fileName.rfind('-');
   if(found != std::string::npos) {
      std::string temp = fileName.substr(found + 1, 3);
      return atoi(temp.c_str());
   }
   found = fileName.rfind('_');
   if(found != std::string::npos) {
      std::string temp = fileName.substr(found + 1, 3);
      return atoi(temp.c_str());
   }
   return -1;
}
