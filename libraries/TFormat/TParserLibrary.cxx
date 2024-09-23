#include "TParserLibrary.h"

#define dlsym __bull__
#include <dlfcn.h>
#undef dlsym

#include "TGRSIOptions.h"
#include "TGRSIUtilities.h"
#include "TRunInfo.h"

// redeclare dlsym to be a function returning a function pointer instead of void *
extern "C" void* (*dlsym(void* handle, const char* symbol))();

TParserLibrary::~TParserLibrary()
{
   if(fHandle != nullptr) {
      dlclose(fHandle);
   }
}

void TParserLibrary::Load()
{
   if(fHandle != nullptr) {
      std::cout << "Already loaded handle " << fHandle << std::endl;
      return;
   }

   if(TGRSIOptions::Get()->ParserLibrary().empty()) {
      std::ostringstream str;
      str << DRED << "No data parser library provided! Please provided the location of the parser library via .grsirc file or on the command line." << RESET_COLOR;
      throw std::runtime_error(str.str());
   }

   if(!file_exists(TGRSIOptions::Get()->ParserLibrary().c_str())) {
      std::ostringstream str;
      str << DRED << "Library '" << TGRSIOptions::Get()->ParserLibrary() << "' does not exist or we do not have permissions to access it!" << RESET_COLOR;
      throw std::runtime_error(str.str());
   }

   fHandle = dlopen(TGRSIOptions::Get()->ParserLibrary().c_str(), RTLD_LAZY);
   if(fHandle == nullptr) {
      std::ostringstream str;
      str << DRED << "Failed to open raw file library '" << TGRSIOptions::Get()->ParserLibrary() << "': " << dlerror() << "!" << RESET_COLOR;
      std::cout << "dlerror: '" << dlerror() << "'" << std::endl;
      throw std::runtime_error(str.str());
   }
   // try and get constructor and destructor functions from opened library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
   fInitLibrary    = reinterpret_cast<void (*)()>(dlsym(fHandle, "InitLibrary"));
   fLibraryVersion = reinterpret_cast<std::string (*)()>(dlsym(fHandle, "LibraryVersion"));

   fCreateRawFile  = reinterpret_cast<TRawFile* (*)(const std::string&)>(dlsym(fHandle, "CreateFile"));
   fDestroyRawFile = reinterpret_cast<void (*)(TRawFile*)>(dlsym(fHandle, "DestroyFile"));

   fCreateDataParser  = reinterpret_cast<TDataParser* (*)()>(dlsym(fHandle, "CreateParser"));
   fDestroyDataParser = reinterpret_cast<void (*)(TDataParser*)>(dlsym(fHandle, "DestroyParser"));
#pragma GCC diagnostic pop

   if(fInitLibrary == nullptr || fLibraryVersion == nullptr || fCreateRawFile == nullptr || fDestroyRawFile == nullptr || fCreateDataParser == nullptr || fDestroyDataParser == nullptr) {
      std::ostringstream str;
      str << DRED << "Failed to find CreateFile, DestroyFile, CreateParser, DestroyParser, LibraryVersion, and/or InitLibrary functions in library '" << TGRSIOptions::Get()->ParserLibrary() << "'!" << RESET_COLOR;
      throw std::runtime_error(str.str());
   }
   fInitLibrary();
   std::cout << "\tUsing library " << TGRSIOptions::Get()->ParserLibrary() << " version " << fLibraryVersion() << std::endl;
   TRunInfo::SetLibraryVersion(fLibraryVersion());
   TRunInfo::SetLibraryPath(TGRSIOptions::Get()->ParserLibrary());
}
