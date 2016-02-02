#include "TGRSIOptions.h"
#include "TGRSIRunInfo.h"

NamespaceImp(TGRSIOptions)

//using namespace TGRSIOptions::priv;

namespace TGRSIOptions {
	bool fLogErrors = false;
	bool fUseMidFileOdb = true;
	bool fMakeAnalysisTree = false;
	bool fProgressDialog = true;
	bool fWorkHarder = false;
	bool fReadingMaterial = false;
	bool fIgnoreFileOdb  = false;
	bool fIgnoreScaler   = false;
	bool fIgnoreEpics    = false;
	bool fIgnoreSCLR     = false;
	bool fCloseAfterSort = false;
	bool fWriteBadFrags  = false;
	bool fWriteDiagnostics = false;

	std::string fExptName;
	std::string fHostName;
  
	std::vector<std::string> fInputMidasFile;
	std::vector<std::string> fInputRootFile;
	std::vector<std::string> fInputOdbFile;
	std::vector<std::string> fInputCalFile;
	std::vector<std::string> fExternalRunInfo;
	std::vector<std::string> fMacroFile;

	std::string GetHostName() { return fHostName; }


	std::string GetExptName() { return fExptName; }

	std::vector<std::string> GetInputRoot()  {  return fInputRootFile;  }
	std::vector<std::string> GetInputMidas() {  return fInputMidasFile; }
	std::vector<std::string> GetInputCal()   {  return fInputCalFile;   }
	std::vector<std::string> GetInputOdb()   {  return fInputOdbFile;   }
	std::vector<std::string> GetMacroFile()  {  return fMacroFile;      }


	void AddExternalRunInfo(std::string file) { fExternalRunInfo.push_back(file); }

	void SetExternalRunInfo() {
		if(ExternalRunInfo())
			if(!TGRSIRunInfo::ReadInfoFile(fExternalRunInfo.at(0).c_str())) 
				printf("Problem reading run-info file %s\n",fExternalRunInfo.at(0).c_str());
	}

	bool ExternalRunInfo() { return fExternalRunInfo.size()>0; }

	const char* GetXMLODBFile(int runNumber, int subRunNumber) {
		if(!fInputOdbFile.empty())
			return fInputOdbFile.at(0).c_str();
		return "";
	}

	const char* GetCalFile(int runNumber, int subRunNumber) {
		if(!fInputCalFile.empty())
			return fInputCalFile.at(0).c_str();
		return "";
	}

	void SetIgnoreFileOdb(bool flag) { fIgnoreFileOdb=flag; }
	bool IgnoreFileOdb()             { return fIgnoreFileOdb; }

	void SetIgnoreScaler(bool flag) { fIgnoreScaler=flag; }
	bool IgnoreScaler()             { return fIgnoreScaler; }

	void SetIgnoreEpics(bool flag) { fIgnoreEpics=flag; }
	bool IgnoreEpics()             { return fIgnoreEpics; }

	void SetIgnoreSCLR(bool flag) { fIgnoreSCLR=flag; }
	bool IgnoreSCLR()             { return fIgnoreSCLR; }

	void SetCloseAfterSort(bool flag) { fCloseAfterSort=flag; }
	bool CloseAfterSort()                  { return fCloseAfterSort; }

	void SetWorkHarder(bool flag) { fWorkHarder=flag; }
	bool WorkHarder()             { return fWorkHarder; }

	void SetReadingMaterial(bool flag) { fReadingMaterial=flag; }
	bool ReadingMaterial()             { return fReadingMaterial; }

	void SetLogErrors(bool flag)      { fLogErrors=flag;   }
	bool LogErrors()			 { return fLogErrors; }

	void SetProgressDialog(bool flag) {fProgressDialog=flag; }
	bool ProgressDialog()                  { return fProgressDialog;}

	void SetUseMidFileOdb(bool flag)     { fUseMidFileOdb=flag;  }
	bool UseMidFileOdb()                      { return fUseMidFileOdb;}

	void SetMakeAnalysisTree(bool flag)  { fMakeAnalysisTree=flag;  }
	bool MakeAnalysisTree()              { return fMakeAnalysisTree; }

	void SetHostName(std::string& host) { fHostName.assign(host); }
	void SetExptName(std::string& expt) { fExptName.assign(expt); }

	void SetWriteBadFrags(bool flag)  { fWriteBadFrags=flag; }
	bool WriteBadFrags()              { return fWriteBadFrags; }

	void SetWriteDiagnostics(bool flag) { fWriteDiagnostics=flag; }
	bool WriteDiagnostics()             { return fWriteDiagnostics; }


	void AddInputRootFile(std::string& input)  {  fInputRootFile.push_back(input);  }
	void AddInputMidasFile(std::string& input) {  fInputMidasFile.push_back(input); }
	void AddInputCalFile(std::string& input)   {  SetUseMidFileOdb(false);  fInputCalFile.push_back(input); }
	void AddInputOdbFile(std::string& input)   {  SetUseMidFileOdb(false); fInputOdbFile.push_back(input);  }
	void AddMacroFile(std::string& input)      {  fMacroFile.push_back(input); }
}
