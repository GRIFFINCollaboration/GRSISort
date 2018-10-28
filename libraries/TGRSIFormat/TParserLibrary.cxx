#include "TParserLibrary.h"

#include <dlfcn.h>

#include "TGRSIOptions.h"

TParserLibrary::~TParserLibrary() {
	if(fHandle != nullptr) {
		dlclose(fHandle);
	}
}

void TParserLibrary::Load() {
	if(fHandle != nullptr) {
		return;
	}

	fHandle = dlopen(TGRSIOptions::Get()->ParserLibrary().c_str(), RTLD_LAZY);
	if(fHandle == nullptr) {
		std::ostringstream str;
		str<<"Failed to open raw file library '"<<TGRSIOptions::Get()->ParserLibrary()<<"': "<<dlerror()<<"!";
		std::cout<<"dlerror: '"<<dlerror()<<"'"<<std::endl;
		throw std::runtime_error(str.str());
	}
	// try and get constructor and destructor functions from opened library
	fInitLibrary    = (void (*)())                        dlsym(fHandle, "InitLibrary");
	fLibraryVersion = (std::string (*)())                 dlsym(fHandle, "LibraryVersion");

	fCreateRawFile  = (TRawFile* (*)(const std::string&)) dlsym(fHandle, "CreateFile");
	fDestroyRawFile = (void (*)(TRawFile*))               dlsym(fHandle, "DestroyFile");

	fCreateDataParser  = (TDataParser* (*)())     dlsym(fHandle, "CreateParser");
	fDestroyDataParser = (void (*)(TDataParser*)) dlsym(fHandle, "DestroyParser");

	if(fInitLibrary == nullptr || fLibraryVersion == nullptr || fCreateRawFile == nullptr || fDestroyRawFile == nullptr || fCreateDataParser == nullptr || fDestroyDataParser == nullptr) {
		std::ostringstream str;
		str<<"Failed to find CreateFile, DestroyFile, CreateParser, DestroyParser, LibraryVersion, and/or InitLibrary functions in library '"<<TGRSIOptions::Get()->ParserLibrary()<<"'!";
		throw std::runtime_error(str.str());
	}
	fInitLibrary();
	std::cout<<"\tUsing library "<<TGRSIOptions::Get()->ParserLibrary()<<" version "<<fLibraryVersion()<<std::endl;
}
