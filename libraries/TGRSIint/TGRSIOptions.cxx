#include "TGRSIOptions.h"

NamespaceImp(TGRSIOptions)

using namespace TGRSIOptions::priv;

namespace TGRSIOptions {

  bool fLogErrors = false;
  bool fUseMidFileOdb = true;
  bool fMakeAnalysisTree = false;
  bool fProgressDialog = true;
  bool fWorkHarder = false;

  bool fCloseAfterSort = false;

  std::string fexptname;
  std::string fhostname;
  
  std::vector<std::string> fInputMidasFile;
  std::vector<std::string> fInputRootFile;
  std::vector<std::string> fInputOdbFile;
  std::vector<std::string> fInputCalFile;

std::string GetHostName(){
  return fhostname;
}


std::string GetExptName()  {  return fexptname;  }

std::vector<std::string> GetInputRoot()  {  return fInputRootFile;  }
std::vector<std::string> GetInputMidas() {  return fInputMidasFile; }
std::vector<std::string> GetInputCal()   {  return fInputCalFile;   }
std::vector<std::string> GetInputOdb()   {  return fInputOdbFile;   }

const char *GetXMLODBFile(int runnumber,int subrunnumber);
const char *GetCalFile(int runnumber,int subrunnumber);


void SetCloseAfterSort(bool flag) { fCloseAfterSort=flag; }
bool CloseAfterSort()                  { return fCloseAfterSort; }

void SetWorkHarder(bool flag) { fWorkHarder=flag; }
bool WorkHarder()             { return fWorkHarder; }

void SetLogErrors(bool flag)      { fLogErrors=flag;   }
bool LogErrors()			 { return fLogErrors; }

void SetProgressDialog(bool flag) {fProgressDialog=flag; }
bool ProgressDialog()                  { return fProgressDialog;}

void SetUseMidFileOdb(bool flag)     { fUseMidFileOdb=flag;  }
bool UseMidFileOdb()                      { return fUseMidFileOdb;}

void SetMakeAnalysisTree(bool flag)  { fMakeAnalysisTree=flag;  }
bool MakeAnalysisTree()                   { return fMakeAnalysisTree;}

void SetHostName(std::string &host) {fhostname.assign(host);}
void SetExptName(std::string &expt) {fexptname.assign(expt);}

void AddInputRootFile(std::string &input)  {  fInputRootFile.push_back(input);    }
void AddInputMidasFile(std::string &input) {  fInputMidasFile.push_back(input);   }
void AddInputCalFile(std::string &input)   {  SetUseMidFileOdb(false);  fInputCalFile.push_back(input);     }
void AddInputOdbFile(std::string &input)   {  SetUseMidFileOdb(false); fInputOdbFile.push_back(input);     }

const char *GetXMLODBFile(int runnumber, int subrunnumber) {
	if(!fInputOdbFile.empty())
		return fInputOdbFile.at(0).c_str();
	return "";
}

const char *GetCalFile(int runnumber, int subrunnumber) {
	if(!fInputCalFile.empty())
		return fInputCalFile.at(0).c_str();
	return "";
}

void Print(Option_t *opt) {   }

void Clear(Option_t *opt) {   }


}

