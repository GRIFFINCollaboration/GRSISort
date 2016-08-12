#ifndef __TGRSIUTILITIES_H_
#define __TGRSIUTILITIES_H_

#include <fstream>
#include <string>
#include <vector>

#include "TGRSITypes.h"

bool file_exists(const char *filename);
bool all_files_exist(const std::vector<std::string>& filenames);

std::string get_short_filename(std::string filename);
std::string get_run_number(std::string input);

inline size_t FindFileSize(const char* fname) {
  std::ifstream temp;
  temp.open(fname, std::ios::in | std::ios::ate);
  size_t fsize = temp.tellg();
  temp.close();
  return fsize;
}

#endif
