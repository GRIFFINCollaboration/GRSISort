#ifndef _TGRUTOPTIONS_H_
#define _TGRUTOPTIONS_H_

#include <map>

#include "TObject.h"

#include "TGRSITypes.h"

class TGRSIOptions2 : public TObject {
public:
  static TGRSIOptions2* Get(int argc = 0, char** argv = NULL);

  void Clear(Option_t* opt = "");
  void Load(int argc, char** argv);
  void Print(Option_t* opt = "") const;

  bool ShouldExit() { return fShouldExit; }
  const std::vector<std::string>& InputMidasFiles() { return fInputMidasFiles;}
  const std::vector<std::string>& RootInputFiles()  { return fInputRootFiles; }
  const std::vector<std::string>& CalInputFiles()   { return fInputCalFiles;  }
  const std::vector<std::string>& ValInputFiles()   { return fInputValFiles;  }
  const std::vector<std::string>& InputOdbFiles()   { return fInputOdbFiles;  }
  const std::vector<std::string>& ExternalRunInfo() { return fExternalRunInfo;}
  const std::vector<std::string>& InputCutFiles()   { return fInputCutsFiles;}
  const std::vector<std::string>& WinInputFiles()   { return fInputWinFiles;   }
  const std::vector<std::string>& MacroInputFiles() { return fMacroFiles;      }

  const std::string& OutputFile()                   { return output_file; }
  const std::string& OutputFilteredFile()        { return output_filtered_file; }
  const std::string& OutputHistogramFile()          { return output_histogram_file; }
  std::string InputRing() { return input_ring; }
  std::string CompiledHistogramFile() { return compiled_histogram_file; }
  std::string CompiledFilterFile() { return compiled_filter_file; }

  const std::vector<std::string>& OptionFiles() { return options_file; }

  int BuildWindow() const { return fBuildWindow; }
  bool RecordDialog() const { return fRecordDialog; }

  bool CloseAfterSort()     const { return fCloseAfterSort; }

  bool LogErrors()          const { return fLogErrors;        }
  bool UseMidFileOdb()      const { return fUseMidFileOdb;    }

  bool MakeAnalysisTree()   const { return fMakeAnalysisTree; }
  bool ProgressDialog()     const { return fProgressDialog;   }
  bool WorkHarder()         const { return fWorkHarder;       }
  bool ReadingMaterial()    const { return fReadingMaterial;  }
  bool IgnoreFileOdb()      const { return fIgnoreFileOdb;    }

  bool IgnoreScaler()       const { return fIgnoreScaler;     }
  bool IgnoreEpics()        const { return fIgnoreEpics;      }
  bool WriteBadFrags()      const { return fWriteBadFrags;    }
  bool WriteDiagnostics()   const { return fWriteDiagnostics; }


  bool ShowedHelp()         const { return fHelp; }
  bool ShowedVersion()      const { return fShowedVersion; }
  bool ShowLogo()           const { return fShowLogo; }
  bool SortRaw()            const { return fSortRaw; }
  bool SortRoot()           const { return fSortRoot; }
  bool ExtractWaves()       const { return fExtractWaves;  }
  bool StartGUI()           const { return fStartGui; }
  bool MakeHistos()         const { return fMakeHistos; }
  bool SortMultiple()       const { return fSortMultiple; }


  bool IsOnline()           const { return fIsOnline; }

  bool TimeSortInput()      const { return fTimeSortInput; }
  int TimeSortDepth()       const { return fTimeSortDepth; }

  bool ShouldExitImmediately() const { return fShouldExit; }

  kFileType   DetermineFileType(const std::string& filename) const;

  std::string GenerateOutputFilename(const std::string& filename);
  std::string GenerateOutputFilename(const std::vector<std::string>& filename);

  bool LongFileDescription() const { return fLongFileDescription; }


  void SetStartGUI(bool flag=true) { fStartGui=flag; }

private:
  TGRSIOptions2(int argc, char** argv);

  bool FileAutoDetect(const std::string& filename);

  std::vector<std::string> fInputMidasFiles;
  std::vector<std::string> fInputRootFiles;
  std::vector<std::string> fInputCalFiles;
  std::vector<std::string> fInputOdbFiles;
  std::vector<std::string> fExternalRunInfo;
  std::vector<std::string> fMacroFiles;

  std::vector<std::string> fInputCutsFiles;
  std::vector<std::string> fInputValFiles;
  std::vector<std::string> fInputWinFiles;
  std::string input_ring;

  std::string output_file;
  std::string output_filtered_file;
  std::string output_histogram_file;

  std::string compiled_histogram_file;
  std::string compiled_filter_file;

  std::vector<std::string> options_file;

  std::string log_file;

  bool fCloseAfterSort;
  bool fLogErrors;
  bool fUseMidFileOdb;

  bool fMakeAnalysisTree;
  bool fProgressDialog;
  bool fWorkHarder;
  bool fReadingMaterial;
  bool fIgnoreFileOdb;
  bool fRecordDialog;

  bool fIgnoreScaler;
  bool fIgnoreEpics;
  bool fWriteBadFrags;
  bool fWriteDiagnostics;

  bool fShowedVersion;
  bool fHelp;
  bool fShowLogo;
  bool fSortRaw;
  bool fSortRoot;
  bool fExtractWaves;
  bool fIsOnline;
  bool fStartGui;
  bool fMakeHistos;
  bool fSortMultiple;

  bool fTimeSortInput;
  int fTimeSortDepth;

  int fBuildWindow;

  bool fShouldExit;

  bool fLongFileDescription;

  ClassDef(TGRSIOptions2,0);
};

#endif /* _TGRUTOPTIONS_H_ */
