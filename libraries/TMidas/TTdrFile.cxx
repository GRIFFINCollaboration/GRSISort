#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cassert>
#include <cstdlib>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "TString.h"

#include "TTdrFile.h"
#include "TTdrEvent.h"
#include "TGRSIRunInfo.h"
#include "GVersion.h"

/// \cond CLASSIMP
ClassImp(TTdrFile)
/// \endcond

TTdrFile::TTdrFile()
{
   // Default Constructor
   fBytesRead = 0;
   fFileSize  = 0;
}

TTdrFile::TTdrFile(const char* filename, TRawFile::EOpenType open_type) : TTdrFile()
{
   switch(open_type) {
	case TRawFile::EOpenType::kRead: Open(filename); break;

	case TRawFile::EOpenType::kWrite: break;
   }
}

TTdrFile::~TTdrFile()
{
   // Default dtor. It closes the read in midas file as well as the output midas file.
   Close();
}

std::string TTdrFile::Status(bool)
{
   return Form(HIDE_CURSOR " Processed event, have processed %.2fMB/%.2f MB              " SHOW_CURSOR "\r",
               (fBytesRead / 1000000.0), (fFileSize / 1000000.0));
}

/// Open a tdr file with given file name.
///
/// \param[in] filename The file to open.
/// \returns "true" for succes, "false" for error, use GetLastError() to see why
bool TTdrFile::Open(const char* filename)
{
   fFilename = filename;
   try {
      fInputFile.open(GetFilename(), std::ifstream::in | std::ifstream::binary);
      fInputFile.seekg(0, std::ifstream::end);
      if(fInputFile.tellg() < 0) {
         std::cout<<R"(Failed to open ")"<<GetFilename()<<"/"<<fFilename<<R"("!)"<<std::endl;
         return false;
      }
      fFileSize = fInputFile.tellg();
      fInputFile.seekg(0, std::ifstream::beg);
		fReadBuffer.resize(0x10000);
   } catch(std::exception& e) {
      std::cout<<"Caught "<<e.what()<<std::endl;
   }
// Do we need these?
// signal(SIGPIPE,SIG_IGN); // crash if reading from closed pipe
// signal(SIGXFSZ,SIG_IGN); // crash if reading from file >2GB without O_LARGEFILE

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

   TGRSIRunInfo::SetRunInfo(GetRunNumber(), GetSubRunNumber());
   TGRSIRunInfo::SetGRSIVersion(GRSI_RELEASE);

   std::cout<<"Successfully opened file with "<<fFileSize<<" bytes!"<<std::endl;

	//std::cout<<std::hex<<std::setfill('0');
	//for(size_t i = 0; i < fReadBuffer.size() && i < 256; ++i) {
	//	std::cout<<std::setw(2)<<(static_cast<int16_t>(fReadBuffer[i])&0xff)<<" ";
	//	if(i%16 == 15) std::cout<<std::endl;
	//}
	//std::cout<<std::dec<<std::setfill(' ');

   return true;
}

void TTdrFile::Close()
{
}

/// \param [in] tdrEvent Pointer to an empty TTdrEvent
/// \returns "true" for success, "false" for failure, see GetLastError() to see why
///
///  EDITED FROM THE ORIGINAL TO RETURN TOTAL SUCESSFULLY BYTES READ INSTEAD OF TRUE/FALSE,  PCB
///
int TTdrFile::Read(std::shared_ptr<TRawEvent> tdrEvent)
{
	if(!fInputFile.is_open()) {
		return 0;
	}
	// try to read next 64k buffer
	fInputFile.read(fReadBuffer.data(), 0x10000);
	if(!fInputFile.good()) {
		std::cout<<"Failed to read next 64k buffer, currently at "<<fBytesRead<<"/"<<fFileSize<<std::endl;
      fInputFile.close();
		return 0;
	}
	//read 24 byte header
	if(strncmp(fReadBuffer.data(), "EBYEDATA", 8) != 0) {
		std::cerr<<fBytesRead<<": Failed to find 'EBYEDATA' (or 0x45 42 59 45 44 41 54 41) at beginning of header (0x"<<std::hex<<std::setfill('0')<<std::setw(8)<<*reinterpret_cast<uint64_t*>(fReadBuffer.data())<<std::hex<<std::setfill(' ')<<")"<<std::endl;
		return 0;
	}
	try {
		std::static_pointer_cast<TTdrEvent>(tdrEvent)->SetHeader(fReadBuffer.data());
	} catch(std::exception& e) {
		std::cout<<e.what()<<std::endl;
	}
	uint32_t dataSize = std::static_pointer_cast<TTdrEvent>(tdrEvent)->GetHeader().fDataLength;
	if(24 + dataSize < 0x10000) {
		try {
			std::static_pointer_cast<TTdrEvent>(tdrEvent)->SetData(std::vector<char>(fReadBuffer.begin() + 24, fReadBuffer.begin() + 24 + dataSize));
      } catch(std::exception& e) {
         std::cout<<e.what()<<std::endl;
      }
		fBytesRead = fInputFile.tellg();
		if(fBytesRead == fFileSize) {
			fInputFile.close();
		}
      return 0x10000;
   }
   return 0;
}

int TTdrFile::GetRunNumber()
{
   // Parse the run number from the current TMidasFile. This assumes a format of
   // R#*_#* or R#* (#* denoting one or more digits).
   if(fFilename.length() == 0) {
      return 0;
   }
   std::size_t foundslash = fFilename.rfind('/');
   std::size_t found = fFilename.rfind('R');
	if(found < foundslash || found ==std::string::npos) {
		std::cout<<"Warning, failed to find 'R' in filename '"<<fFilename<<"'!"<<std::endl;
		return 0;
	}
   std::size_t found2 = fFilename.rfind('-');
   if((found2 < foundslash && foundslash != std::string::npos) || found2 == std::string::npos) {
      found2 = fFilename.rfind('_');
   }
   //   printf("found 2 = %i\n",found2);
   if(found2 < foundslash && foundslash != std::string::npos) {
      found2 = std::string::npos;
   }
   std::string temp;
   if(found2 == std::string::npos) {
		// no subrun number found, use rest of filename
      temp = fFilename.substr(found + 1);
   } else {
		// subrun number found, use everything between 'R' and '_'/'-'
      temp = fFilename.substr(found + 1, found2 - (found + 1));
   }
   return atoi(temp.c_str());
}

int TTdrFile::GetSubRunNumber()
{
   // Parse the sub run number from the current TMidasFile. This assumes a format of
   // R#*_#* or R#* (#* denoting one or more digits).
   if(fFilename.length() == 0) {
      return -1;
   }
   std::size_t foundslash = fFilename.rfind('/');
   std::size_t found      = fFilename.rfind('-');
   if((found < foundslash && foundslash != std::string::npos) || found == std::string::npos) {
      found = fFilename.rfind('_');
   }
   if(found < foundslash && foundslash != std::string::npos) {
      found = std::string::npos;
   }
   if(found != std::string::npos) {
      std::string temp = fFilename.substr(found + 1);
      return atoi(temp.c_str());
   }
   return -1;
}

// end
