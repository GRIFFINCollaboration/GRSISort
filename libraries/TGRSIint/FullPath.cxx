#include "FullPath.h"

#ifdef __linux__

#include <stdlib.h>
#include <limits.h>

std::string full_path(const std::string& path){
  char buff[PATH_MAX+1];
  char* success = realpath(path.c_str(), buff);
  if(success){
    return buff;
  } else {
    // TODO: Give some sort of error message.
    return "";
  }
}
#endif


// TODO: Make a windows version.
