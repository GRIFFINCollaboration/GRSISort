#include "FullPath.h"

//#ifdef __linux__

#include <cstdlib>
#include <climits>

std::string full_path(const std::string& path)
{
   char  buff[PATH_MAX + 1];
   char* success = realpath(path.c_str(), buff);
   if(success != nullptr) {
      return buff;
   }
   // TODO: Give some sort of error message.
   return "";
}
//#endif

// TODO: Make a windows version.
