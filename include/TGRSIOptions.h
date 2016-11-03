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

		const std::string& OutputFragmentFile() { return output_fragment_file; }
		const std::string& OutputAnalysisFile() { return output_analysis_file; }


		const std::string& OutputFilteredFile()        { return output_filtered_file; }
		const std::string& OutputFragmentHistogramFile(){ return output_fragment_histogram_file; }
		const std::string& OutputAnalysisHistogramFile(){ return output_analysis_histogram_file; }
		std::string InputRing() { return input_ring; }
		std::string FragmentHistogramLib() { return fragment_histogram_lib; }
		std::string AnalysisHistogramLib() { return analysis_histogram_lib; }
		std::string CompiledFilterFile() { return compiled_filter_file; }

		const std::vector<std::string>& OptionFiles() { return options_file; }

		int BuildWindow() const { return fBuildWindow; }
		int AddbackWindow() const { return fAddbackWindow; }
		bool StaticWindow() const { return fStaticWindow; }
		bool RecordDialog() const { return fRecordDialog; }
		bool StartGui() const { return fStartGui; }

		bool SuppressErrors() const { return fSuppressErrors; }

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
		std::string input_ring;

		std::string output_fragment_file;
		std::string output_analysis_file;
		std::string output_filtered_file;
		std::string output_fragment_histogram_file;
		std::string output_analysis_histogram_file;

		std::string fragment_histogram_lib;
		std::string analysis_histogram_lib;
		std::string compiled_filter_file;

		std::vector<std::string> options_file;

		std::string log_file;

		bool fCloseAfterSort;
		bool fLogErrors;
		bool fUseMidFileOdb;
		bool fSuppressErrors;

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
		bool fDebug;

		size_t fFragmentWriteQueueSize;
		size_t fAnalysisWriteQueueSize;

		bool fTimeSortInput;
		int fSortDepth;

		int fBuildWindow;
		int fAddbackWindow;
		bool fStaticWindow;

		bool fShouldExit;

		size_t fColumnWidth;
		size_t fStatusWidth;
		unsigned int fStatusInterval;
		bool fLongFileDescription;

      //Proof only
      int fMaxWorkers;

		ClassDef(TGRSIOptions,1);
};

#endif /* _TGRSIOPTIONS_H_ */
