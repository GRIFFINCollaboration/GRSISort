#include "DynamicLibrary.h"

#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#include <dlfcn.h>
#include <unistd.h>

//#include "RuntimeExceptions.h"
#include "FullPath.h"

namespace {
  int incremental_id() {
    static std::mutex mutex;
    static int count = 0;
    std::lock_guard<std::mutex> lock(mutex);
    return count++;
  }
}

DynamicLibrary::DynamicLibrary(std::string libname_param, bool unique_name)
  : libname(libname_param) {
  if(unique_name){
    std::stringstream ss;
    ss << "/tmp/temp_dynlib_" << getpid() << "_" << incremental_id() << ".so";
    tempname = ss.str();

    // Need to symlink to full path, not a relative path.
    // If a relative path is given, then the symlink will look for that library
    //  relative to /tmp, instead of relative to the current directory.
    libname = full_path(libname);

    int error = symlink(libname.c_str(), tempname.c_str());
    if(error){
      return;
      //throw RuntimeSymlinkCreation("Could not make temp symlink");
    }
    library = dlopen(tempname.c_str(), RTLD_NOW);
  } else {
    library = dlopen(libname.c_str(), RTLD_NOW);
  }

  if(!library){
    return;
    //throw RuntimeFileNotFound(dlerror());
  }
}

DynamicLibrary::~DynamicLibrary() {
  if(library) {
    dlclose(library);
    if(tempname.length()){
      unlink(tempname.c_str());
    }
  }
}

DynamicLibrary::DynamicLibrary(DynamicLibrary&& other)
  : library(NULL){
  swap(other);
}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other){
  swap(other);
  return *this;
}

void DynamicLibrary::swap(DynamicLibrary& other){
  std::swap(library, other.library);
  std::swap(libname, other.libname);
  std::swap(tempname, other.tempname);
}

void* DynamicLibrary::GetSymbol(const char* symbol) {
  return dlsym(library, symbol);
}
