#ifndef TRUNINFO_H
#define TRUNINFO_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TRunInfo
///
/// This Class is designed to store run dependent information.
/// It is used to store run numbers, GRSISort version, existence of
/// detector systems, etc.
/// The TRunInfo is written to both the fragment and analysis trees,
/// as well as to files create by grsiframe.
///
/// An example run info looks like this:
///
/// \code
/// GRSI [0] TRunInfo::Get()->Print()
/// Singleton 0x5651a3d5dc10 was read from analysis22151_000.root
/// Title: All-singles-ZDF-off 133Ba_R0794
/// Comment: All-singles-ZDF-off 133Ba_R0794
/// 		RunNumber:          22151
/// 		SubRunNumber:       000
/// 		RunStart:           Fri Dec 22 18:11:29 2023
/// 		RunStop:            Fri Dec 22 18:42:40 2023
/// 		RunLength:          1871 s
/// GRSI [1]
/// \endcode
///
/// When subruns/runs are added together using gadd or when multiple
/// subruns/runs are used in grsiframe, their TRunInfos get merged.
///
/// If all files are from one run, and the subruns are added in order
/// without any missing, the information changes to reflect this:
///
/// \code
/// GRSI [0] TRunInfo::Get()->Print()
/// Singleton 0x55ab20f209a0 was read from Final21950_000-028.root
/// Title: 100Zr_beam_with_PACES_lasers_on
/// Comment: 100Zr_beam_with_PACES_lasers_on
/// 		RunNumber:          21950
/// 		SubRunNumbers:      000-028
/// 		RunStart:           Thu Dec 14 13:37:14 2023
/// 		RunStop:            Thu Dec 14 14:37:14 2023
/// 		RunLength:          3599 s
/// GRSI [1]
/// \endcode
///
/// Internally this is signaled by the subrun number having been
/// set to -1, while the run number is still nonzero, and the run
/// start and run stop are set as well as the numbers of the first
/// and last subrun.
///
/// If the subruns are not in order or a subrun is missing from the
/// range added, the number of the first and last subrun are both set
/// to -1, changing the line
/// \code
/// 		SubRunNumbers:      000-028
/// \endcode
/// to
/// \code
/// 		SubRunNumbers:      -1--1
/// \endcode
///
/// If the files are from multiple runs that are all in consecutive
/// order without any missing ones, the run info will say:
///
/// \code
/// Singleton 0x563c6c1c3a30 was read from Final21950-21980.root
/// Title: 100Zr_beam_with_PACES_lasers_on
/// Comment: 100Zr_beam_with_PACES_lasers_on
/// 		RunNumbers:         21950-21980
/// 		No missing runs
/// 		RunStart:           Thu Dec 14 13:37:14 2023
/// 		RunStop:            Fri Dec 15 19:06:11 2023
/// 		RunLength:          101306 s
/// \endcode
///
/// If however some runs in between are missing, it will say
/// \code
/// Singleton 0x55919586a8c0 was read from test50_51_54.root
/// Title: 100Zr_beam_with_PACES_lasers_on
/// Comment: 100Zr_beam_with_PACES_lasers_on
/// 		RunNumbers:         21950-21954
/// 		Missing runs:       21952, 21953
/// 		Combined RunLength: 10794 s
/// \endcode
///
/// From version 18 on TRunInfo also stores information about
/// the GRSISort version used to sort the data.
/// This can be accessed via
/// \code
/// TRunInfo::Get()->PrintVersion();
/// \endcode
///
/// Using a newer version of grsiframe on data created with an
/// older version of GRSISort should not overwrite the version
/// reported.
///
/// Summary of different states the run info can be in:
///
/// | state                                                                                 | identifying markers                                     |
/// |---------------------------------------------------------------------------------------|---------------------------------------------------------|
/// | single sub run                                                                        | non-zero run number and sub run number != -1            |
/// | consecutive sub runs of a single run (from FirstSubRunNumber() to LastSubRunNumber()) | non-zero run number and sub run number == -1            |
/// | consecutive runs (from FirstRunNumber() to LastRunNumber())                           | zero run number and FirstRunNumber() != LastRunNumber() |
/// | non-consecutive runs                                                                  | zero run number and non-empty run list                  |
///
/////////////////////////////////////////////////////////////////

#include "TTree.h"
#include "TFile.h"
#include "TKey.h"

#include "Globals.h"

#include "TSingleton.h"
#include "TEventBuildingLoop.h"
#include "TDetectorInformation.h"

class TRunInfo : public TSingleton<TRunInfo> {
public:
   friend class TSingleton<TRunInfo>;

   TRunInfo(const TRunInfo&)                = default;
   TRunInfo(TRunInfo&&) noexcept            = default;
   TRunInfo& operator=(const TRunInfo&)     = default;
   TRunInfo& operator=(TRunInfo&&) noexcept = default;
   ~TRunInfo()                              = default;
   TRunInfo();   // This should not be used.
   // root forces me have this here instead
   // of a private class member in
   // order to write this class to a tree.
   // pcb.

   static Bool_t ReadInfoFromFile(TFile* tempf = nullptr);

   static std::string GetVersion() { return Get()->fVersion; }
   static void        ClearVersion() { Get()->fVersion.clear(); }
   static void        SetVersion(const char* ver) { SetVersion(std::string(ver)); }
   static void        SetVersion(std::string ver)
   {
      if(!Get()->fVersion.empty() && Get()->fVersion != ver) {
         std::cout << ALERTTEXT << "WARNING; VERSION ALREADY SET TO " << Get()->fVersion << " NOT " << ver << "!!" << RESET_COLOR << std::endl;
      } else {
         Get()->fVersion = std::move(ver);
      }
   }

   static std::string GetFullVersion() { return Get()->fFullVersion; }
   static void        ClearFullVersion() { Get()->fFullVersion.clear(); }
   static void        SetFullVersion(const char* ver) { SetFullVersion(std::string(ver)); }
   static void        SetFullVersion(std::string ver)
   {
      if(!Get()->fFullVersion.empty() && Get()->fFullVersion != ver) {
         std::cout << ALERTTEXT << "WARNING; FULL VERSION ALREADY SET TO " << Get()->fFullVersion << " NOT " << ver << "!!" << RESET_COLOR << std::endl;
      } else {
         Get()->fFullVersion = std::move(ver);
      }
   }

   static std::string GetDate() { return Get()->fDate; }
   static void        ClearDate() { Get()->fDate.clear(); }
   static void        SetDate(const char* ver) { SetDate(std::string(ver)); }
   static void        SetDate(std::string ver)
   {
      if(!Get()->fDate.empty() && Get()->fDate != ver) {
         std::cout << ALERTTEXT << "WARNING; DATE ALREADY SET TO " << Get()->fDate << " NOT " << ver << "!!" << RESET_COLOR << std::endl;
      } else {
         Get()->fDate = std::move(ver);
      }
   }

   static std::string GetLibraryVersion() { return Get()->fLibraryVersion; }
   static void        ClearLibraryVersion() { Get()->fLibraryVersion.clear(); }
   static void        SetLibraryVersion(const char* ver) { SetLibraryVersion(std::string(ver)); }
   static void        SetLibraryVersion(std::string ver)
   {
      if(!Get()->fLibraryVersion.empty() && Get()->fLibraryVersion != ver) {
         std::cout << ALERTTEXT << "WARNING; LIBRARY VERSION ALREADY SET TO " << Get()->fLibraryVersion << " NOT " << ver << "!!" << RESET_COLOR << std::endl;
      } else {
         Get()->fLibraryVersion = std::move(ver);
      }
   }

   static std::string GetLibraryPath() { return Get()->fLibraryPath; }
   static void        ClearLibraryPath() { Get()->fLibraryPath.clear(); }
   static void        SetLibraryPath(const char* ver) { SetLibraryPath(std::string(ver)); }
   static void        SetLibraryPath(std::string ver)
   {
      if(!Get()->fLibraryPath.empty() && Get()->fLibraryPath != ver) {
         std::cout << ALERTTEXT << "WARNING; LIBRARY PATH ALREADY SET TO " << Get()->fLibraryPath << " NOT " << ver << "!!" << RESET_COLOR << std::endl;
      } else {
         Get()->fLibraryPath = std::move(ver);
      }
   }

   static void SetRunInfo(int runnum = 0, int subrunnum = -1);
   static void SetAnalysisTreeBranches(TTree*);

   static void SetRunNumber(int tmp) { Get()->fRunNumber = tmp; }
   static void SetSubRunNumber(int tmp) { Get()->fSubRunNumber = tmp; }

   static int RunNumber() { return Get()->fRunNumber; }
   static int SubRunNumber() { return Get()->fSubRunNumber; }

   static int FirstRunNumber() { return Get()->fFirstRunNumber; }
   static int FirstSubRunNumber() { return Get()->fFirstSubRunNumber; }

   static int LastRunNumber() { return Get()->fLastRunNumber; }
   static int LastSubRunNumber() { return Get()->fLastSubRunNumber; }

   static void SetRunTitle(const char* run_title)
   {
      if(run_title != nullptr) { Get()->fRunTitle.assign(run_title); }
   }
   static void SetRunComment(const char* run_comment)
   {
      if(run_comment != nullptr) { Get()->fRunComment.assign(run_comment); }
   }

   static std::string RunTitle() { return Get()->fRunTitle; }
   static std::string RunComment() { return Get()->fRunComment; }

   static void SetRunStart(double tmp) { Get()->fRunStart = tmp; }
   static void SetRunStop(double tmp) { Get()->fRunStop = tmp; }
   static void SetRunLength(double tmp) { Get()->fRunLength = tmp; }
   static void SetRunLength()
   {
      // if this is a single sub run or consecutive sub runs of a single run we can calculate the run length from the stop and start times
      if(RunNumber() != 0) {
         Get()->fRunLength = Get()->fRunStop - Get()->fRunStart;
      }
      // otherwise we have no idea how to calculate the run length (it should be summed up by the Add function)
      // so we do nothing
   }

   static double RunStart() { return Get()->fRunStart; }
   static double RunStop() { return Get()->fRunStop; }
   static double RunLength() { return Get()->fRunLength; }

   static void SetCalFileName(const char* name) { Get()->fCalFileName.assign(name); }
   static void SetCalFileData(const char* data) { Get()->fCalFile.assign(data); }

   static void SetXMLODBFileName(const char* name) { Get()->fXMLODBFileName.assign(name); }
   static void SetXMLODBFileData(const char* data) { Get()->fXMLODBFile.assign(data); }

   static const char* GetCalFileName() { return Get()->fCalFileName.c_str(); }
   static const char* GetCalFileData() { return Get()->fCalFile.c_str(); }

   static const char* GetXMLODBFileName() { return Get()->fXMLODBFileName.c_str(); }
   static const char* GetXMLODBFileData() { return Get()->fXMLODBFile.c_str(); }

   static const char* GetRunInfoFileName() { return Get()->fRunInfoFileName.c_str(); }
   static const char* GetRunInfoFileData() { return Get()->fRunInfoFile.c_str(); }

   static Bool_t ReadInfoFile(const char* filename = "");
   static Bool_t ParseInputData(const char* inputdata = "", Option_t* opt = "q");

   static void SetRunInfoFileName(const char* fname) { Get()->fRunInfoFileName.assign(fname); }
   static void SetRunInfoFile(const char* ffile) { Get()->fRunInfoFile.assign(ffile); }

   static void   SetHPGeArrayPosition(const double arr_pos) { Get()->fHPGeArrayPosition = arr_pos; }
   static double HPGeArrayPosition() { return Get()->fHPGeArrayPosition; }

   Long64_t Merge(TCollection* list);
   void     Add(TRunInfo* runinfo, bool verbose = false);

   virtual TEventBuildingLoop::EBuildMode BuildMode() const;

   static void PrintBadCycles();
   static void AddBadCycle(int bad_cycle);
   static void RemoveBadCycle(int cycle);
   static bool IsBadCycle(int cycle);

   void        PrintRunList() const;
   std::string ListOfMissingRuns(bool all = false) const;
   void        PrintVersion() const;

   static std::string CreateLabel(bool quiet = false);

   static void                  SetDetectorInformation(TDetectorInformation* inf) { Get()->fDetectorInformation = inf; }
   static TDetectorInformation* GetDetectorInformation() { return Get()->fDetectorInformation; }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   static bool        WriteToRoot(TFile* fileptr = nullptr);
   static bool        WriteInfoFile(const std::string& filename);
   static std::string PrintToString(Option_t* opt = "");

private:
   std::string                   fRunTitle;                ///< The title of the run
   std::string                   fRunComment;              ///< The comment on the run
   int                           fRunNumber{0};            ///< The current run number
   int                           fSubRunNumber{-1};        ///< The current sub run number
   int                           fFirstRunNumber{0};       ///< The first run number (for combined runs)
   int                           fFirstSubRunNumber{-1};   ///< The first sub run number (for combined subruns)
   int                           fLastRunNumber{0};        ///< The last run number (for combined runs)
   int                           fLastSubRunNumber{-1};    ///< The last sub run number (for combined subruns)
   std::set<std::pair<int, int>> fRunList;                 ///< List of all runs added to this run info

   double fRunStart{0.};    ///< The start  of the current run in seconds - no idea why we store this as double?
   double fRunStop{0.};     ///< The stop   of the current run in seconds - no idea why we store this as double?
   double fRunLength{0.};   ///< The length of the current run in seconds - no idea why we store this as double?

   std::string fVersion;          ///< The version of GRSISort that generated the file - GRSI_RELEASE from GVersion.h
   std::string fFullVersion;      ///< The full version of GRSISort that generated the file (includes last commit) - GRSI_GIT_COMMIT from GVersion.h
   std::string fDate;             ///< The date of the last commit used in this version - GRSI_GIT_COMMIT_TIME from GVersion.h
   std::string fLibraryVersion;   ///< The version of the parser/file library that generated the file
   std::string fLibraryPath;      ///< The path of the parser/file library that generated the file

   std::string fCalFileName;   ///< Name of calfile that generated cal
   std::string fCalFile;       ///< Cal File to load into Cal of tree

   std::string fXMLODBFileName;   ///< Name of XML Odb file
   std::string fXMLODBFile;       ///< The odb

   /////////////////////////////////////////////////
   //////////////// Building Options ///////////////
   /////////////////////////////////////////////////

   std::string fRunInfoFileName;   ///< The name of the Run info file
   std::string fRunInfoFile;       ///< The contents of the run info file

   double fHPGeArrayPosition{110.};   ///< Position of the HPGe Array (default = 110.0 mm );

   std::vector<int> fBadCycleList;   //!<!List of bad cycles to be used for cycle rejection

   TDetectorInformation* fDetectorInformation{nullptr};   //!<! pointer to detector specific information (set by each parser library)

   /// \cond CLASSIMP
   ClassDefOverride(TRunInfo, 18)   // NOLINT(readability-else-after-return,modernize-type-traits)
   /// \endcond
};
/*! @} */
#endif
