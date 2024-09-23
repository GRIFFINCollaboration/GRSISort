#include "FullPath.h"

//#ifdef __LINUX__

#include <cstdlib>
#include <climits>
#include <array>

std::string full_path(const std::string& path)
{
   std::array<char, PATH_MAX + 1> buff;
   char*                          success = realpath(path.c_str(), buff.data());
   if(success != nullptr) {
      return {buff.data()};   // this ensures that we stop at the string limiting \0 (whereas buff.begin() to buff.end() would use all PATH_MAX+1 characters of the array)
   }
   // TODO: Give some sort of error message.
   return "";
}
//#endif

// TODO: Make a windows version.
