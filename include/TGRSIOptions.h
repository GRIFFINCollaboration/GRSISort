#ifndef TGRSIOPTIONS_H
#define TGRSIOPTIONS_H

/** \addtogroup Sorting
 *  @{
 */

#include <map>

#include "TObject.h"
#include "TFile.h"

#include "TGRSITypes.h"
#include "TAnalysisOptions.h"
#include "TUserSettings.h"

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
   TGRSIOptions() = default;   /// Do not use!
   static TGRSIOptions* Get(int argc = 0, char** argv = nullptr);

   void Clear(Option_t* opt = "") override;
   void Load(int argc, char** argv);
   void Print(Option_t* opt = "") const override;

   static bool   WriteToFile(TFile* file = nullptr);
   static void   SetOptions(TGRSIOptions* tmp);
   static Bool_t ReadFromFile(TFile* file = nullptr);

   bool                            ShouldExit() const { return fShouldExit; }
   const std::vector<std::string>& InputFiles() const { return fInputFiles; }
   const std::vector<std::string>& RootInputFiles() const { return fInputRootFiles; }
   const std::vector<std::string>& CalInputFiles() const { return fInputCalFiles; }
   const std::vector<std::string>& ValInputFiles() const { return fInputValFiles; }
   const std::vector<std::string>& InputOdbFiles() const { return fInputOdbFiles; }
   const std::vector<std::string>& ExternalRunInfo() const { return fExternalRunInfo; }
   const std::vector<std::string>& InputCutFiles() const { return fInputCutFiles; }
   const std::vector<std::string>& WinInputFiles() const { return fInputWinFiles; }
   const std::vector<std::string>& MacroInputFiles() const { return fMacroFiles; }
   const std::string&              DataFrameLibrary() const { return fDataFrameLibrary; }

   const std::string& OutputFragmentFile() const { return fOutputFragmentFile; }
   const std::string& OutputAnalysisFile() const { return fOutputAnalysisFile; }

   const std::string& OutputFilteredFile() const { return fOutputFilteredFile; }
   const std::string& OutputFragmentHistogramFile() const { return fOutputFragmentHistogramFile; }
   const std::string& OutputAnalysisHistogramFile() const { return fOutputAnalysisHistogramFile; }
   std::string        InputRing() const { return fInputRing; }
   std::string        FragmentHistogramLib() const { return fFragmentHistogramLib; }
   std::string        AnalysisHistogramLib() const { return fAnalysisHistogramLib; }
   std::string        CompiledFilterFile() const { return fCompiledFilterFile; }

   const std::vector<std::string>& OptionFiles() const { return fOptionsFile; }

   std::string LogFile() const { return fLogFile; }
   void        LogFile(const std::string& val) { fLogFile = val; }

   static TAnalysisOptions* AnalysisOptions() { return fAnalysisOptions; }
   static TUserSettings*    UserSettings() { return fUserSettings; }

   bool SeparateOutOfOrder() const { return fSeparateOutOfOrder; }
   bool StartGui() const { return fStartGui; }

   bool SuppressErrors() const { return fSuppressErrors; }
   bool ReconstructTimeStamp() const { return fReconstructTimeStamp; }

   bool CloseAfterSort() const { return fCloseAfterSort; }

   bool LogErrors() const { return fLogErrors; }
   bool UseMidFileOdb() const { return fUseMidFileOdb; }

   bool MakeAnalysisTree() const { return fMakeAnalysisTree; }
   bool ReadingMaterial() const { return fReadingMaterial; }
   bool IgnoreFileOdb() const { return fIgnoreFileOdb; }
   bool IgnoreOdbChannels() const { return fIgnoreOdbChannels; }
   int  Downscaling() const { return fDownscaling; }

   bool IgnoreScaler() const { return fIgnoreScaler; }
   bool IgnoreEpics() const { return fIgnoreEpics; }
   bool WriteFragmentTree() const { return fWriteFragmentTree; }
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

   bool Debug() const { return fDebug; }   // also used by GRSIFrame

   bool IsOnline() const { return fIsOnline; }

   size_t FragmentWriteQueueSize() const { return fFragmentWriteQueueSize; }
   size_t AnalysisWriteQueueSize() const { return fAnalysisWriteQueueSize; }

   size_t NumberOfEvents() const { return fNumberOfEvents; }

   bool IgnoreMissingChannel() const { return fIgnoreMissingChannel; }
   bool SkipInputSort() const { return fSkipInputSort; }
   int  SortDepth() const { return fSortDepth; }

   bool ShouldExitImmediately() const { return fShouldExit; }

   static kFileType DetermineFileType(const std::string& filename);

   size_t       ColumnWidth() const { return fColumnWidth; }
   size_t       StatusWidth() const { return fStatusWidth; }
   unsigned int StatusInterval() const { return fStatusInterval; }
   bool         LongFileDescription() const { return fLongFileDescription; }

   // GRSIProof and GRSIFrame only
   int         GetMaxWorkers() const { return fMaxWorkers; }
   std::string TreeName() const { return fTreeName; }
   // Proof only
   bool SelectorOnly() const { return fSelectorOnly; }
   bool AverageRateEstimation() const { return fAverageRateEstimation; }
   bool ParallelUnzip() const { return fParallelUnzip; }
   int  CacheSize() const { return fCacheSize; }
   int  Submergers() const { return fSubmergers; }
   bool ProofStats() const { return fProofStats; }

   void SuppressErrors(bool suppress) { fSuppressErrors = suppress; }

   // shared object libraries
   void               ParserLibrary(std::string& library) { fParserLibrary = library; }
   const std::string& ParserLibrary() const { return fParserLibrary; }

private:
   TGRSIOptions(int argc, char** argv);
   static TGRSIOptions* fGRSIOptions;

   bool FileAutoDetect(const std::string& filename);

   std::vector<std::string> fInputFiles;         ///< A list of the input  files
   std::vector<std::string> fInputRootFiles;     ///< A list of the input root files
   std::vector<std::string> fInputCalFiles;      ///< A list of the input cal files
   std::vector<std::string> fInputOdbFiles;      ///< A list of the input odb files
   std::vector<std::string> fExternalRunInfo;    ///< A list of the input run info files
   std::vector<std::string> fMacroFiles;         ///< A list of the input macro (.C) files
   std::string              fDataFrameLibrary;   ///< library (or .cxx file) for dataframe processing (used with grsiframe)

   std::vector<std::string> fInputCutFiles;   ///< A list of input cut files
   std::vector<std::string> fInputValFiles;   ///< A list of the input GValue files
   std::vector<std::string> fInputWinFiles;   ///< A list of the input window files
   std::string              fInputRing;       ///< The name of hte input ring

   std::string fOutputFragmentFile;   ///< The name of the fragment file to write to
   std::string fOutputAnalysisFile;   ///< The name of the analysis file to write to
   std::string fOutputFilteredFile;
   std::string fOutputFragmentHistogramFile;   ///< The name of the fragment histogram file
   std::string fOutputAnalysisHistogramFile;   ///< The name of the analysis histogram file

   std::string fFragmentHistogramLib;   ///< The name of the script for histogramming fragments
   std::string fAnalysisHistogramLib;   ///< The name of the script for histogramming events
   std::string fCompiledFilterFile;

   std::vector<std::string> fOptionsFile;   ///< A list of the input .info files

   std::string fLogFile;   ///< The name of the output log file

   bool fCloseAfterSort{false};         ///< Flag to close after sorting (-q)
   bool fLogErrors{false};              ///< Flag to log errors (--log-errors)
   bool fUseMidFileOdb{false};          ///< Flag to read odb from midas
   bool fSuppressErrors{false};         ///< Flag to suppress errors (--suppress-errors)
   bool fReconstructTimeStamp{false};   ///< Flag to reconstruct missing high bits of time stamps (--reconstruct-timestamp)

   bool fMakeAnalysisTree{false};    ///< Flag to make analysis tree (-a)
   bool fReadingMaterial{false};     ///< Flag to show reading material (--reading-material)
   bool fIgnoreFileOdb{false};       ///< Flag to ignore midas file odb
   bool fIgnoreOdbChannels{false};   ///< Flag to ignore channels from midas file odb (but do use EPICS from ODB)
   int  fDownscaling{1};             ///< Downscaling factor for raw events to be processed

   bool fIgnoreScaler{false};        ///< Flag to ignore scalers in GRIFFIN
   bool fIgnoreEpics{false};         ///< Flag to ignore epics
   bool fWriteFragmentTree{false};   ///< Flag to write fragment tree
   bool fWriteBadFrags{false};       ///< Flag to write bad fragments
   bool fWriteDiagnostics{false};    ///< Flag to write diagnostics
   int  fWordOffset{-1};             ///< Offset for word count in GRIFFIN header (default 1)

   bool fBatch{false};   ///< Flag to use batch mode (-b)

   bool fShowedVersion{false};   ///< Flag to show version
   bool fShowLogo{false};        ///< Flag to show logo (suppress with -l)
   bool fSortRaw{true};          ///< Flag to sort raw file
   bool fExtractWaves{false};    ///< Flag to keep waveforms (suppress with --no-waveforms)
   bool fIsOnline{false};        ///< Flag to sort online data
   bool fStartGui{false};        ///< Flag to start GUI (-g)
   bool fMakeHistos{false};      ///< Flag to make histograms (-H)
   bool fSortMultiple{false};    ///< Flag to sort multiple files
   bool fDebug{false};           ///< Flag for debug mode

   size_t fFragmentWriteQueueSize{100000};   ///< Size of the Fragment write Q
   size_t fAnalysisWriteQueueSize{100000};   ///< Size of the analysis write Q

   size_t fNumberOfEvents{0};   ///< Number of events, fragments, etc. to process (0 - all)

   bool fIgnoreMissingChannel{false};   ///< Flag to completely ignore missing channels
   bool fSkipInputSort{false};          ///< Flag to sort on time or triggers
   int  fSortDepth{200000};             ///< Size of Q that stores fragments to be built into events

   static TAnalysisOptions* fAnalysisOptions;   ///< contains all options for analysis
   static TUserSettings*    fUserSettings;      ///< contains user settings read from text-file

   bool fSeparateOutOfOrder{false};   ///< Flag to build out of order into seperate event tree

   bool fShouldExit{false};   ///< Flag to exit sorting

   bool fHelp{false};   ///< help requested?

   size_t       fColumnWidth{20};      ///< Size of verbose columns
   size_t       fStatusWidth{120};     ///< Size of total verbose status
   unsigned int fStatusInterval{10};   ///< Time between status updates
   bool         fLongFileDescription{false};

   // Proof only
   int         fMaxWorkers{-1};                 ///< Max workers used in grsiproof
   bool        fSelectorOnly{false};            ///< Flag to turn PROOF off in grsiproof
   std::string fTreeName;                       ///< Name of tree to be analyzed (default is empty, i.e. FragmentTree, AnalysisTree, and Lst2RootTree are checked)
   bool        fAverageRateEstimation{false};   ///< enable average rate estimation
   bool        fParallelUnzip{false};           ///< enable use of parallel unzipping
   int         fCacheSize{-1};                  ///< set tree cache size, default is -1 (off)
   int         fSubmergers{-1};                 ///< set number of sub-mergers (0 = automatic), default is -1 (off)
   bool        fProofStats{false};              ///< enable proof stats

   // shared object libraries
   std::string fParserLibrary;   ///< location of shared object library for data parser and files

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIOptions, 5)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif /* _TGRSIOPTIONS_H_ */
