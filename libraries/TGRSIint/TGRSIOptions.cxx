#include "TGRSIOptions.h"

NamespaceImp(TGRSIOptions)

//TGRSIOptions *TGRSIOptions::fTGRSIOptions = 0;

//TGRSIOptions::TGRSIOptions() { 
//}

//TGRSIOptions::~TGRSIOptions() { 
   TGRSIOptions::fLogErrors = false;
   TGRSIOptions::fUseMidFileOdb = true;
   TGRSIOptions::fMakeAnalysisTree = false;
   TGRSIOptions::fProgressDialog = true;


using namespace TGRSIOptions;

std::string TGRSIOptions::GetHostName(){
  return fhostname;
}


std::string TGRSIOptions::GetExptName()  {  return fexptname;  }

std::vector<std::string> TGRSIOptions::GetInputRoot()  {  return fInputRootFile;  }
std::vector<std::string> TGRSIOptions::GetInputMidas() {  return fInputMidasFile; }
std::vector<std::string> TGRSIOptions::GetInputCal()   {  return fInputCalFile;   }
std::vector<std::string> TGRSIOptions::GetInputOdb()   {  return fInputOdbFile;   }

const char *TGRSIOptions::GetXMLODBFile(int runnumber,int subrunnumber);
const char *TGRSIOptions::GetCalFile(int runnumber,int subrunnumber);


void TGRSIOptions::SetCloseAfterSort(bool flag) { fCloseAfterSort=flag; }
bool TGRSIOptions::CloseAfterSort()                  { return fCloseAfterSort; }

void TGRSIOptions::SetLogErrors(bool flag)      { fLogErrors=flag;   }
bool TGRSIOptions::LogErrors()			 { return fLogErrors; }

void TGRSIOptions::SetProgressDialog(bool flag) {fProgressDialog=flag; }
bool TGRSIOptions::ProgressDialog()                  { return fProgressDialog;}

void TGRSIOptions::SetUseMidFileOdb(bool flag)     { fUseMidFileOdb=flag;  }
bool TGRSIOptions::UseMidFileOdb()                      { return fUseMidFileOdb;}

void TGRSIOptions::SetMakeAnalysisTree(bool flag)  { fMakeAnalysisTree=flag;  }
bool TGRSIOptions::MakeAnalysisTree()                   { return fMakeAnalysisTree;}

void TGRSIOptions::SetHostName(std::string &host) {fhostname.assign(host);}
void TGRSIOptions::SetExptName(std::string &expt) {fexptname.assign(expt);}

void TGRSIOptions::AddInputRootFile(std::string &input)  {  fInputRootFile.push_back(input);    }
void TGRSIOptions::AddInputMidasFile(std::string &input) {  fInputMidasFile.push_back(input);   }
void TGRSIOptions::AddInputCalFile(std::string &input)   {  SetUseMidFileOdb(false);  fInputCalFile.push_back(input);     }
void TGRSIOptions::AddInputOdbFile(std::string &input)   {  SetUseMidFileOdb(false); fInputOdbFile.push_back(input);     }





const char *TGRSIOptions::GetXMLODBFile(int runnumber, int subrunnumber) {
	if(!TGRSIOptions::fInputOdbFile.empty())
		return TGRSIOptions::fInputOdbFile.at(0).c_str();
	return "";
}

const char *TGRSIOptions::GetCalFile(int runnumber, int subrunnumber) {
	if(!TGRSIOptions::fInputCalFile.empty())
		return TGRSIOptions::fInputCalFile.at(0).c_str();
	return "";
}

void TGRSIOptions::Print(Option_t *opt) {   }

void TGRSIOptions::Clear(Option_t *opt) {   }
