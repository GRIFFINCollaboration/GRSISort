#include "TDataFrameLibrary.h"
#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,14,0)

#define dlsym __bull__
#include <dlfcn.h>
#undef dlsym

#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

#include "TGRSIOptions.h"
#include "TGRSIUtilities.h"
#include "TGRSIFrame.h"

// redeclare dlsym to be a function returning a function pointer instead of void *
extern "C" void *(*dlsym(void *handle, const char *symbol))();

TDataFrameLibrary::~TDataFrameLibrary() {
	if(fHandle != nullptr) {
		dlclose(fHandle);
	}
}

void TDataFrameLibrary::Load() {
	if(fHandle != nullptr) {
		std::cout<<"Already loaded handle "<<fHandle<<std::endl;
		return;
	}

	std::string libraryPath = TGRSIOptions::Get()->DataFrameLibrary();
	if(libraryPath.empty()) {
		std::ostringstream str;
		str<<DRED<<"No data frame library provided! Please provided the location of the data frame library on the command line."<<RESET_COLOR;
		throw std::runtime_error(str.str());
	}

	// check if the provided file ends in .c (which means we should compile it into a library) otherwise we just try to load it
	// if there is no dot in the path or the dot is before a slash in the path we just try to load this weirdly named library
	size_t   dot = libraryPath.find_last_of('.');
	size_t slash = libraryPath.find_last_of('/');
	if(dot != std::string::npos && (dot > slash || slash == std::string::npos) && libraryPath.substr(dot) == ".cxx") {
		// let's get the full path first (or maybe move this into the function?)
		Compile(libraryPath, dot, slash);
		// replace the .cxx extension with .so
		libraryPath.replace(dot, std::string::npos, ".so");
	}

	if(!file_exists(libraryPath.c_str())) {
		std::ostringstream str;
		str<<DRED<<"Library '"<<libraryPath<<"' does not exist or we do not have permissions to access it!"<<RESET_COLOR;
		throw std::runtime_error(str.str());
	}

	fHandle = dlopen(libraryPath.c_str(), RTLD_LAZY);
	if(fHandle == nullptr) {
		std::ostringstream str;
		str<<DRED<<"Failed to open data frame library '"<<libraryPath<<"': "<<dlerror()<<"!"<<RESET_COLOR;
		std::cout<<"dlerror: '"<<dlerror()<<"'"<<std::endl;
		throw std::runtime_error(str.str());
	}
	// try and get constructor and destructor functions from opened library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
	fCreateHelper     = reinterpret_cast<TGRSIHelper* (*)(TList*)>(dlsym(fHandle, "CreateHelper"));
	fDestroyHelper    = reinterpret_cast<void (*)(TGRSIHelper*)>(dlsym(fHandle, "DestroyHelper"));
#pragma GCC diagnostic pop

	if(fCreateHelper == nullptr || fDestroyHelper == nullptr) {
		std::ostringstream str;
		str<<DRED<<"Failed to find CreateHelper, and/or DestroyHelper functions in library '"<<libraryPath<<"'!"<<RESET_COLOR;
		throw std::runtime_error(str.str());
	}
	std::cout<<"\tUsing library "<<libraryPath<<std::endl;
}

void TDataFrameLibrary::Compile(std::string& path, const size_t& dot, const size_t& slash)
{
	/// Try and compile the provided .c file into a shared object library
	// first we create the paths for the header file, and shared library
	// we know dot != npos and either dot > slash or slash = npos
	std::string sourceFile = path;
	std::string headerFile = path.replace(dot, std::string::npos, ".hh");
	std::string sharedLibrary = path.replace(dot, std::string::npos, ".so");
	// first we get the stats of the file's involved (.cxx, .hh, .so, and the libTGRSIFrame.so)
	struct stat sourceStat;
	if(stat(sourceFile.c_str(), &sourceStat) != 0) {
		std::stringstream str;
		str<<"Unable to access stat of source file "<<sourceFile<<std::endl;
		throw std::runtime_error(str.str());
	}
	struct stat headerStat;
	if(stat(headerFile.c_str(), &headerStat) != 0) {
		std::stringstream str;
		str<<"Unable to access stat of header file "<<headerFile<<std::endl;
		throw std::runtime_error(str.str());
	}
	struct stat frameLibStat;
	// get path of libTGRSIFrame via 
	Dl_info info;
	if(!dladdr(reinterpret_cast<void*>(DummyFunctionToLocateTGRSIFrameLibrary), &info)) {
		std::stringstream str;
		str<<"Unable to find location of DummyFunctionToLocateTGRSIFrameLibrary"<<std::endl;
		throw std::runtime_error(str.str());
	}
	if(stat(info.dli_fname, &frameLibStat) != 0) {
		std::stringstream str;
		str<<"Unable to access stat of "<<info.dli_fname<<std::endl;
		throw std::runtime_error(str.str());
	}
	struct stat sharedLibStat;
	if(stat(sharedLibrary.c_str(), &sharedLibStat) == 0 && 
			sharedLibStat.st_atime > sourceStat.st_atime && 
			sharedLibStat.st_atime > headerStat.st_atime && 
			sharedLibStat.st_atime > frameLibStat.st_atime) {
		std::cout<<DCYAN<<"shared library "<<sharedLibrary<<" exists and is newer than "<<sourceFile<<", "<<headerFile<<", and $GRSISYS/lib/libTGRSIFrame.so"<<RESET_COLOR<<std::endl;
		return;
	}
	// get include path
	std::string includePath = ".";
	if(slash != std::string::npos) {
		includePath = path.substr(0, slash);
	}
	std::cout<<DCYAN<<"----------  starting compilation of user code  ----------"<<RESET_COLOR<<std::endl;
	// TODO: replace --GRSIData-flags with something based on which data parser library we've loaded
	std::string parserLibraryPath = TGRSIOptions::Get()->ParserLibrary();
	// this should look something like $GRSISYS/<library name>/lib/lib<library name>.so
	// so we can simply take everything between "last '/' + 3" and "last '.'" to be the name?
	std::string parserLibraryName = parserLibraryPath.substr(parserLibraryPath.find_last_of('/')+4, parserLibraryPath.find_last_of('.')-parserLibraryPath.find_last_of('/')-4);
	std::string objectFile = path.replace(dot, std::string::npos, ".o");
	std::stringstream command;
	command<<"g++ -c -fPIC -g `grsi-config --cflags --"<<parserLibraryName<<"-cflags` `root-config --cflags --glibs` -I"<<includePath<<" -o "<<objectFile<<" "<<sourceFile<<std::endl;
	if(std::system(command.str().c_str()) != 0) {
		std::stringstream str;
		str<<"Unable to compile source file "<<sourceFile<<" using '"<<command.str()<<"'"<<std::endl;
		throw std::runtime_error(str.str());
	}
	std::cout<<DCYAN<<"----------  starting linking user code  -----------------"<<RESET_COLOR<<std::endl;
	command.clear();
	command<<"g++ -fPIC -g -shared -o "<<sharedLibrary<<" "<<objectFile<<std::endl;
	if(std::system(command.str().c_str()) != 0) {
		std::stringstream str;
		str<<"Unable to link shared object library "<<sharedLibrary<<" using '"<<command.str()<<"'"<<std::endl;
		throw std::runtime_error(str.str());
	}
	std::cout<<DCYAN<<"----------  done compiling user code  -------------------"<<RESET_COLOR<<std::endl;
}
#endif
