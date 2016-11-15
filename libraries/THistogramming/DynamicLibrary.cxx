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
  : fLibName(libname_param) {
  if(unique_name){
    std::stringstream ss;
    ss << "/tmp/temp_dynlib_" << getpid() << "_" << incremental_id() << ".so";
    fTempName = ss.str();

    // Need to symlink to full path, not a relative path.
    // If a relative path is given, then the symlink will look for that library
    //  relative to /tmp, instead of relative to the current directory.
    fLibName = full_path(fLibName);

    int error = symlink(fLibName.c_str(), fTempName.c_str());
    if(error){
      return;
      //throw RuntimeSymlinkCreation("Could not make temp symlink");
    }
    fLibrary = dlopen(fTempName.c_str(), RTLD_NOW);
  } else {
    fLibrary = dlopen(fLibName.c_str(), RTLD_NOW);
  }

  if(!fLibrary){
    return;
    //throw RuntimeFileNotFound(dlerror());
  }
}

DynamicLibrary::~DynamicLibrary() {
  if(fLibrary) {
    dlclose(fLibrary);
    if(fTempName.length()){
      unlink(fTempName.c_str());
    }
  }
}

DynamicLibrary::DynamicLibrary(DynamicLibrary&& other)
  : fLibrary(NULL){
  swap(other);
}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other){
  swap(other);
  return *this;
}

void DynamicLibrary::swap(DynamicLibrary& other){
  std::swap(fLibrary, other.fLibrary);
  std::swap(fLibName, other.fLibName);
  std::swap(fTempName, other.fTempName);
}

void* DynamicLibrary::GetSymbol(const char* symbol) {
  return dlsym(fLibrary, symbol);
}
