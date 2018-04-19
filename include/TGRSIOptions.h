#ifndef _TGRSIOPTIONS_H_
#define _TGRSIOPTIONS_H_

/** \addtogroup Sorting
 *  @{
 */

#include <map>

#include "TObject.h"
#include "TFile.h"

#include "TGRSITypes.h"
#include "TAnalysisOptions.h"

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIOptions
///
/// This class stores the command line arguments that are passed to GRSISort.
/// This includes file names (root, mid, cal, etc.) as well as command line
/// arguments. This is a singleton class that can be accessed with
/// TGRSIOptions::Get() in order to determine any options that have been passed
/// on the command line.
///
/////////////////////////////////////////////////////////////////

class TGRSIOptions : public TObject {
public:
	TGRSIOptions() {}; /// Do not use!
	static TGRSIOptions* Get(int argc = 0, char** argv = nullptr);

	void Clear(Option_t* opt = "") override;
	void Load(int argc, char** argv);
	void Print(Option_t* opt = "") const override;

	static bool WriteToFile(TFile* file = nullptr);
	static void SetOptions(TGRSIOptions* tmp);
	static Bool_t ReadFromFile(TFile* file = nullptr);

	bool                            ShouldExit() { return fShouldExit; }
	const std::vector<std::string>& InputMidasFiles() { return fInputMidasFiles; }
	const std::vector<std::string>& InputLstFiles() { return fInputLstFiles; }
	const std::vector<std::string>& InputTdrFiles() { return fInputTdrFiles; }
	const std::vector<std::string>& RootInputFiles() { return fInputRootFiles; }
	const std::vector<std::string>& CalInputFiles() { return fInputCalFiles; }
	const std::vector<std::string>& ValInputFiles() { return fInputValFiles; }
	const std::vector<std::string>& InputOdbFiles() { return fInputOdbFiles; }
	const std::vector<std::string>& ExternalRunInfo() { return fExternalRunInfo; }
	const std::vector<std::string>& InputCutFiles() { return fInputCutsFiles; }
	const std::vector<std::string>& WinInputFiles() { return fInputWinFiles; }
	const std::vector<std::string>& MacroInputFiles() { return fMacroFiles; }

	const std::string& OutputFragmentFile() { return fOutputFragmentFile; }
	const std::string& OutputAnalysisFile() { return fOutputAnalysisFile; }

	const std::string& OutputFilteredFile() { return fOutputFilteredFile; }
	const std::string& OutputFragmentHistogramFile() { return fOutputFragmentHistogramFile; }
	const std::string& OutputAnalysisHistogramFile() { return fOutputAnalysisHistogramFile; }
	std::string        InputRing() { return fInputRing; }
	std::string        FragmentHistogramLib() { return fFragmentHistogramLib; }
	std::string        AnalysisHistogramLib() { return fAnalysisHistogramLib; }
	std::string        CompiledFilterFile() { return fCompiledFilterFile; }

	const std::vector<std::string>& OptionFiles() { return fOptionsFile; }

	std::string LogFile() { return fLogFile; }

	static TAnalysisOptions* AnalysisOptions() { return fAnalysisOptions; }

	bool SeparateOutOfOrder() const { return fSeparateOutOfOrder; }
	bool RecordDialog() const { return fRecordDialog; }
	bool StartGui() const { return fStartGui; }

	bool SuppressErrors() const { return fSuppressErrors; }
	bool ReconstructTimeStamp() const { return fReconstructTimeStamp; }

	bool CloseAfterSort() const { return fCloseAfterSort; }

	bool LogErrors() const { return fLogErrors; }
	bool UseMidFileOdb() const { return fUseMidFileOdb; }

	bool MakeAnalysisTree() const { return fMakeAnalysisTree; }
	bool ProgressDialog() const { return fProgressDialog; }
	bool ReadingMaterial() const { return fReadingMaterial; }
	bool IgnoreFileOdb() const { return fIgnoreFileOdb; }

	bool IgnoreScaler() const { return fIgnoreScaler; }
	bool IgnoreEpics() const { return fIgnoreEpics; }
	bool WriteBadFrags() const { return fWriteBadFrags; }
	bool WriteDiagnostics() const { return fWriteDiagnostics; }
	int  WordOffset() const { return fWordOffset; }

	bool Batch() const { return fBatch; }

	bool ShowedVersion() const { return fShowedVersion; }
	bool ShowLogo() const { return fShowLogo; }
	bool SortRaw() const { return fSortRaw; }
	bool ExtractWaves() const { return fExtractWaves; }
	bool MakeHistos() const { return fMakeHistos; }
	bool SortMultiple() const { return fSortMultiple; }

	bool Debug() const { return fDebug; }

	bool IsOnline() const { return fIsOnline; }

	size_t FragmentWriteQueueSize() const { return fFragmentWriteQueueSize; }
	size_t AnalysisWriteQueueSize() const { return fAnalysisWriteQueueSize; }

	size_t NumberOfClients() const { return fNumberOfClients; }

	bool TimeSortInput() const { return fTimeSortInput; }
	int  SortDepth() const { return fSortDepth; }

	bool ShouldExitImmediately() const { return fShouldExit; }

	kFileType DetermineFileType(const std::string& filename) const;

	std::string GenerateOutputFilename(const std::string& filename);
	std::string GenerateOutputFilename(const std::vector<std::string>& filename);

	size_t       ColumnWidth() const { return fColumnWidth; }
	size_t       StatusWidth() const { return fStatusWidth; }
	unsigned int StatusInterval() const { return fStatusInterval; }
	bool         LongFileDescription() const { return fLongFileDescription; }

	// Proof only
	int  GetMaxWorkers() const { return fMaxWorkers; }
	bool SelectorOnly() const { return fSelectorOnly; }

	void SuppressErrors(bool suppress) { fSuppressErrors = suppress; }

private:
	TGRSIOptions(int argc, char** argv);
	static TGRSIOptions* fGRSIOptions;

	bool FileAutoDetect(const std::string& filename);

	std::vector<std::string> fInputMidasFiles; ///< A list of the input Midas files
	std::vector<std::string> fInputLstFiles;   ///< A list of the input Lst files
	std::vector<std::string> fInputTdrFiles;   ///< A list of the input Tdr files
	std::vector<std::string> fInputRootFiles;  ///< A list of the input root files
	std::vector<std::string> fInputCalFiles;   ///< A list of the input cal files
	std::vector<std::string> fInputOdbFiles;   ///< A list of the input odb files
	std::vector<std::string> fExternalRunInfo; ///< A list of the input run info files
	std::vector<std::string> fMacroFiles;      ///< A list of the input macro (.C) files

	std::vector<std::string> fInputCutsFiles; ///< A list of input cut files
	std::vector<std::string> fInputValFiles;  ///< A list of the input GValue files
	std::vector<std::string> fInputWinFiles;  ///< A list of the input window files
	std::string              fInputRing;      ///< The name of hte input ring

	std::string fOutputFragmentFile; ///< The name of the fragment file to write to
	std::string fOutputAnalysisFile; ///< The name of the analysis file to write to
	std::string fOutputFilteredFile;
	std::string fOutputFragmentHistogramFile; ///< The name of the fragment histogram file
	std::string fOutputAnalysisHistogramFile; ///< The name of the analysis histogram file

	std::string fFragmentHistogramLib; ///< The name of the script for histogramming fragments
	std::string fAnalysisHistogramLib; ///< The name of the script for histogramming events
	std::string fCompiledFilterFile;

	std::vector<std::string> fOptionsFile; ///< A list of the input .info files

	std::string fLogFile; ///< The name of the output log file

	bool fCloseAfterSort;       ///< Flag to close after sorting (-q)
	bool fLogErrors;            ///< Flag to log errors (--log-errors)
	bool fUseMidFileOdb;        ///< Flag to read odb from midas
	bool fSuppressErrors;       ///< Flag to suppress errors (--suppress-errors)
	bool fReconstructTimeStamp; ///< Flag to reconstruct missing high bits of time stamps (--reconstruct-timestamp)

	bool fMakeAnalysisTree; ///< Flag to make analysis tree (-a)
	bool fProgressDialog;   ///< Flag to show progress in proof (not used)
	bool fReadingMaterial;  ///< Flag to show reading material (--reading-material)
	bool fIgnoreFileOdb;    ///< Flag to ignore midas file odb
	bool fRecordDialog;

	bool fIgnoreScaler;     ///< Flag to ignore scalers in GRIFFIN
	bool fIgnoreEpics;      ///< Flag to ignore epics
	bool fWriteBadFrags;    ///< Flag to write bad fragments
	bool fWriteDiagnostics; ///< Flag to write diagnostics
	int  fWordOffset;       ///< Offset for word count in GRIFFIN header (default 1)

	bool fBatch; ///< Flag to use batch mode (-b)

	bool fShowedVersion;///< Flag to show version
	bool fShowLogo;     ///< Flag to show logo (suppress with -l)
	bool fSortRaw;      ///< Flag to sort Midas file
	bool fExtractWaves; ///< Flag to keep waveforms (suppress with --no-waveforms)
	bool fIsOnline;     ///< Flag to sort online data
	bool fStartGui;     ///< Flag to start GUI (-g)
	bool fMakeHistos;   ///< Flag to make histograms (-H)
	bool fSortMultiple; ///< Flag to sort multiple files
	bool fDebug;        ///< Flag for debug mode

	size_t fFragmentWriteQueueSize; ///< Size of the Fragment write Q
	size_t fAnalysisWriteQueueSize; ///< Size of the analysis write Q

	size_t fNumberOfClients;        ///< Number of analysis write loop clients

	bool fTimeSortInput; ///< Flag to sort on time or triggers
	int  fSortDepth;     ///< Size of Q that stores fragments to be built into events

	static TAnalysisOptions* fAnalysisOptions; ///< contains all options for analysis

	bool fSeparateOutOfOrder; ///< Flag to build out of order into seperate event tree

	bool fShouldExit; ///< Flag to exit sorting

	bool fHelp; ///< help requested?

	size_t       fColumnWidth;    ///< Size of verbose columns
	size_t       fStatusWidth;    ///< Size of total verbose status
	unsigned int fStatusInterval; ///< Time between status updates
	bool         fLongFileDescription;

	// Proof only
	int  fMaxWorkers;   ///< Max workers used in grsiproof
	bool fSelectorOnly; ///< Flag to turn PROOF off in grsiproof

	/// \cond CLASSIMP
	ClassDefOverride(TGRSIOptions, 3); ///< Class for storing options in GRSISort
	/// \endcond
};
/*! @} */
#endif /* _TGRSIOPTIONS_H_ */
