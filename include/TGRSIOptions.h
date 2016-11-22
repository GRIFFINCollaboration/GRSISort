#ifndef _TGRSIOPTIONS_H_
#define _TGRSIOPTIONS_H_

#include <map>

#include "TObject.h"

#include "TGRSITypes.h"

class TGRSIOptions : public TObject {
	public:
		static TGRSIOptions* Get(int argc = 0, char** argv = NULL);

		void Clear(Option_t* opt = "");
		void Load(int argc, char** argv);
		void Print(Option_t* opt = "") const;
		void PrintSortingOptions() const;

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

		const std::string& OutputFragmentFile() { return fOutputFragmentFile; }
		const std::string& OutputAnalysisFile() { return fOutputAnalysisFile; }


		const std::string& OutputFilteredFile()        { return fOutputFilteredFile; }
		const std::string& OutputFragmentHistogramFile(){ return fOutputFragmentHistogramFile; }
		const std::string& OutputAnalysisHistogramFile(){ return fOutputAnalysisHistogramFile; }
		std::string InputRing() { return fInputRing; }
		std::string FragmentHistogramLib() { return fFragmentHistogramLib; }
		std::string AnalysisHistogramLib() { return fAnalysisHistogramLib; }
		std::string CompiledFilterFile() { return fCompiledFilterFile; }

		const std::vector<std::string>& OptionFiles() { return fOptionsFile; }

		std::string LogFile() { return fLogFile; }

		int BuildWindow() const { return fBuildWindow; }
		int AddbackWindow() const { return fAddbackWindow; }
		bool StaticWindow() const { return fStaticWindow; }
		bool SeparateOutOfOrder() const { return fSeparateOutOfOrder; }
		bool RecordDialog() const { return fRecordDialog; }
		bool StartGui() const { return fStartGui; }

		bool SuppressErrors() const { return fSuppressErrors; }

		bool CloseAfterSort()     const { return fCloseAfterSort; }

		bool LogErrors()          const { return fLogErrors;        }
		bool UseMidFileOdb()      const { return fUseMidFileOdb;    }

		bool MakeAnalysisTree()   const { return fMakeAnalysisTree; }
		bool ProgressDialog()     const { return fProgressDialog;   }
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
		bool MakeHistos()         const { return fMakeHistos; }
		bool SortMultiple()       const { return fSortMultiple; }

		bool Debug()              const { return fDebug; }

		bool IsOnline()           const { return fIsOnline; }

		size_t FragmentWriteQueueSize() const { return fFragmentWriteQueueSize; }
		size_t AnalysisWriteQueueSize() const { return fAnalysisWriteQueueSize; }

		bool TimeSortInput()      const { return fTimeSortInput; }
		int SortDepth()           const { return fSortDepth; }

		bool ShouldExitImmediately() const { return fShouldExit; }

		kFileType   DetermineFileType(const std::string& filename) const;

		std::string GenerateOutputFilename(const std::string& filename);
		std::string GenerateOutputFilename(const std::vector<std::string>& filename);

		size_t ColumnWidth() const { return fColumnWidth; }
		size_t StatusWidth() const { return fStatusWidth; }
		unsigned int StatusInterval() const { return fStatusInterval; }
		bool LongFileDescription() const { return fLongFileDescription; }

		//Proof only
		int GetMaxWorkers() const { return fMaxWorkers; }
		bool SelectorOnly() const { return fSelectorOnly; } 

	private:
		TGRSIOptions(int argc, char** argv);

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
		std::string fInputRing;

		std::string fOutputFragmentFile;
		std::string fOutputAnalysisFile;
		std::string fOutputFilteredFile;
		std::string fOutputFragmentHistogramFile;
		std::string fOutputAnalysisHistogramFile;

		std::string fFragmentHistogramLib;
		std::string fAnalysisHistogramLib;
		std::string fCompiledFilterFile;

		std::vector<std::string> fOptionsFile;

		std::string fLogFile;

		bool fCloseAfterSort;
		bool fLogErrors;
		bool fUseMidFileOdb;
		bool fSuppressErrors;

		bool fMakeAnalysisTree;
		bool fProgressDialog;
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
		bool fDebug;

		size_t fFragmentWriteQueueSize;
		size_t fAnalysisWriteQueueSize;

		bool fTimeSortInput;
		int fSortDepth;

		int fBuildWindow;
		int fAddbackWindow;
		bool fStaticWindow;
		bool fSeparateOutOfOrder;

		bool fShouldExit;

		size_t fColumnWidth;
		size_t fStatusWidth;
		unsigned int fStatusInterval;
		bool fLongFileDescription;

		//Proof only
		int fMaxWorkers;
		bool fSelectorOnly;

		ClassDef(TGRSIOptions,1);
};

#endif /* _TGRSIOPTIONS_H_ */
