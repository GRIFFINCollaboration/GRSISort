#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <cstdlib>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "TString.h"

#include "TLstFile.h"
#include "TLstEvent.h"
#include "TGRSIRunInfo.h"
#include "GVersion.h"

/// \cond CLASSIMP
ClassImp(TLstFile)
/// \endcond

TLstFile::TLstFile() {
   //Default Constructor
  fBytesRead = 0;
  fFileSize = 0;
}

TLstFile::TLstFile(const char* filename, EOpenType open_type)
  : TLstFile() {
  switch(open_type) {
    case kRead:
      Open(filename);
      break;

    case kWrite:
      break;
  }
}

TLstFile::~TLstFile() {
   //Default dtor. It closes the read in midas file as well as the output midas file.
  Close();
}

std::string TLstFile::Status(bool) {
  return Form(HIDE_CURSOR " Processed event, have processed %.2fMB/%.2f MB              " SHOW_CURSOR "\r",
              (fBytesRead/1000000.0),(fFileSize/1000000.0));
}

/// Open a midas .mid file with given file name.
///
/// Remote files can be accessed using these special file names:
/// - pipein://command - read data produced by given command, see examples below
/// - ssh://username\@hostname/path/file.mid - read remote file through an ssh pipe
/// - ssh://username\@hostname/path/file.mid.gz and file.mid.bz2 - same for compressed files
/// - dccp://path/file.mid (also file.mid.gz and file.mid.bz2) - read data from dcache, requires dccp in the PATH
///
/// Examples:
/// - ./event_dump.exe /ladd/data9/t2km11/data/run02696.mid.gz - read normal compressed file
/// - ./event_dump.exe ssh://ladd09//ladd/data9/t2km11/data/run02696.mid.gz - read compressed file through ssh to ladd09 (note double "/")
/// - ./event_dump.exe pipein://"cat /ladd/data9/t2km11/data/run02696.mid.gz | gzip -dc" - read data piped from a command or script (note quotes)
/// - ./event_dump.exe pipein://"gzip -dc /ladd/data9/t2km11/data/run02696.mid.gz" - another way to read compressed files
/// - ./event_dump.exe dccp:///pnfs/triumf.ca/data/t2km11/aug2008/run02837.mid.gz - read file directly from a dcache pool (note triple "/")
///
/// \param[in] filename The file to open.
/// \returns "true" for succes, "false" for error, use GetLastError() to see why
bool TLstFile::Open(const char *filename) {
	fFilename = filename;
	//the first 9 words (= 36 bytes) seem to be a kind of header
	int headerSize = 36;
	try {
		std::ifstream in(GetFilename(), std::ifstream::in | std::ifstream::binary);
		in.seekg(0, std::ifstream::end);
		if(in.tellg() < 0) {
			std::cout<<"Failed to open \""<<GetFilename()<<"/"<<fFilename<<"\"!"<<std::endl;
			return false;
		} else {
			fFileSize = in.tellg();
			fReadBuffer.resize(fFileSize-headerSize);
			in.seekg(headerSize, std::ifstream::beg);
			in.read(fReadBuffer.data(), fFileSize);
			in.close();
		}
	} catch(std::exception& e) {
		std::cout<<"Caught "<<e.what()<<std::endl;
	}
	// Do we need these?
	//signal(SIGPIPE,SIG_IGN); // crash if reading from closed pipe
	//signal(SIGXFSZ,SIG_IGN); // crash if reading from file >2GB without O_LARGEFILE

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

	TGRSIRunInfo::SetRunInfo(GetRunNumber(),GetSubRunNumber());
	TGRSIRunInfo::SetGRSIVersion(GRSI_RELEASE);

	std::cout<<"Successfully read "<<fFileSize-headerSize<<" bytes into buffer!"<<std::endl;

	return true;
}

void TLstFile::Close() {
}

/// \param [in] midasEvent Pointer to an empty TLstEvent
/// \returns "true" for success, "false" for failure, see GetLastError() to see why
///
///  EDITED FROM THE ORIGINAL TO RETURN TOTAL SUCESSFULLY BYTES READ INSTEAD OF TRUE/FALSE,  PCB
///
int TLstFile::Read(std::shared_ptr<TRawEvent> lstEvent) {
	if(fBytesRead < fFileSize) {
		try {
			std::static_pointer_cast<TLstEvent>(lstEvent)->SetData(fReadBuffer);
		} catch(std::exception& e) {
			std::cout<<e.what()<<std::endl;
		}
		fBytesRead = fFileSize;
		return fFileSize;
	}
	return 0;
}

int TLstFile::GetRunNumber() {
	//Parse the run number from the current TMidasFile. This assumes a format of
	//run#####_###.lst or run#####.lst.
	if(fFilename.length()==0) {
		return 0;
	}
	std::size_t foundslash = fFilename.rfind('/');
	std::size_t found = fFilename.rfind(".lst");
	if(found == std::string::npos) {
		return 0;
	}
	std::size_t found2 = fFilename.rfind('-');
	if((found2 < foundslash && foundslash != std::string::npos) || found2 == std::string::npos)
		found2 = fFilename.rfind('_');
	//   printf("found 2 = %i\n",found2);
	if(found2 < foundslash && foundslash != std::string::npos)
		found2 = std::string::npos;
	std::string temp;
	if(found2 == std::string::npos || fFilename.compare(found2+4,4,".lst") !=0 ) {
		temp = fFilename.substr(found-5,5);
	}
	else {
		temp = fFilename.substr(found-9,5);
	}
	//printf(" %s \t %i \n",temp.c_str(),atoi(temp.c_str()));
	return atoi(temp.c_str());
}


int TLstFile::GetSubRunNumber()	{
	//Parse the sub run number from the current TMidasFile. This assumes a format of
	//run#####_###.lst or run#####.lst.
	if(fFilename.length()==0)
		return -1;
	std::size_t foundslash = fFilename.rfind('/');
	std::size_t found = fFilename.rfind("-");
	if((found < foundslash && foundslash != std::string::npos) || found == std::string::npos)
		found = fFilename.rfind('_');
	if(found < foundslash && foundslash != std::string::npos)
		found = std::string::npos;
	if(found != std::string::npos) {
		std::string temp = fFilename.substr(found+1,3);
		//printf("%i \n",atoi(temp.c_str()));
		return atoi(temp.c_str());
	}
	return -1;
}

// end
