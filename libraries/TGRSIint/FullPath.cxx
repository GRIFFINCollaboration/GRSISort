#include "FullPath.h"

//#ifdef __LINUX__

#include <cstdlib>
#include <climits>
#include <array>

std::string full_path(const std::string& path)
{
	std::array<char, PATH_MAX + 1> buff;
   char* success = realpath(path.c_str(), buff.data());
   if(success != nullptr) {
      return std::string(std::begin(buff), std::end(buff));
   }
   // TODO: Give some sort of error message.
   return "";
}
//#endif

// TODO: Make a windows version.
