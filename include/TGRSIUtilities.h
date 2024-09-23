#ifndef TGRSIUTILITIES_H
#define TGRSIUTILITIES_H

#include <fstream>
#include <string>
#include <vector>

#include "TGRSITypes.h"

bool file_exists(const char* filename);
bool all_files_exist(const std::vector<std::string>& filenames);

void trim(std::string& line, const std::string& trimChars = " \f\n\r\t\v");
void trimWS(std::string& line);

int GetRunNumber(const std::string&);
int GetSubRunNumber(const std::string&);

inline size_t FindFileSize(const char* fname)
{
   std::ifstream temp(fname, std::ios::in | std::ios::ate);
   size_t        fsize = temp.tellg();
   temp.close();
   return fsize;
}

#endif
