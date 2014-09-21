#include "TGRSIOptions.h"

ClassImp(TGRSIOptions)

TGRSIOptions *TGRSIOptions::fTGRSIOptions = 0;

bool TGRSIOptions::fCloseAfterSort   = false;
bool TGRSIOptions::fLogErrors        = false;
bool TGRSIOptions::fUseMidFileOdb    = true;
bool TGRSIOptions::fMakeAnalysisTree = false;
bool TGRSIOptions::fProgressDialog   = true;

std::vector<std::string> TGRSIOptions::fInputRootFile;
std::vector<std::string> TGRSIOptions::fInputMidasFile;
std::vector<std::string> TGRSIOptions::fInputOdbFile;
std::vector<std::string> TGRSIOptions::fInputCalFile;


TGRSIOptions *TGRSIOptions::Get()   {
   if(!fTGRSIOptions)
      fTGRSIOptions = new TGRSIOptions;
   return fTGRSIOptions;
}

TGRSIOptions::TGRSIOptions() { 
}

TGRSIOptions::~TGRSIOptions() {  }


const char *TGRSIOptions::GetXMLODBFile(int runnumber, int subrunnumber) {
	if(!fInputOdbFile.empty())
		return fInputOdbFile.at(0).c_str();
	return "";
}

const char *TGRSIOptions::GetCalFile(int runnumber, int subrunnumber) {
	if(!fInputCalFile.empty())
		return fInputCalFile.at(0).c_str();
	return "";
}

void TGRSIOptions::Print(Option_t *opt) {   }

void TGRSIOptions::Clear(Option_t *opt) {   }
