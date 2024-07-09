#ifndef TRUNINFO_H
#define TRUNINFO_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TRunInfo
///
/// This Class is designed to store and run dependent
/// information. It is used to store run numbers, existence of
/// detector systems, reconstruction windows, etc. The
/// TRunInfo is written alongside both the fragment and
/// analysis trees.
///
/// TRunInfo designed to be made as the FragmentTree
/// is created.  Right now, it simple remembers the run and
/// subrunnumber and sets which systems are present in the odb.
///
/// Due to some root quarkiness, I have done something a bit strange.
/// The info is written ok at the end of the fragment tree process.
/// \code
/// root [1] TRunInfo *info = (TRunInfo*)_file0->Get("TRunInfo")
/// root [2] TRunInfo::ReadInfoFromFile(info);
/// root [3] info->Print()
///   TRunInfo Status:
///   RunNumber:    29038
///   SubRunNumber: 000
///   TIGRESS:      true
///   SHARC:        true
///   GRIFFIN:      false
///   SCEPTAR:      false
///   =====================
/// root [4]
/// \endcode
///
/// \author  P.C. Bender, <pcbend@gmail.com>
/////////////////////////////////////////////////////////////////

#include <cstdio>

#include "TObject.h"
#include "TTree.h"
#include "TFile.h"
#include "TKey.h"

#include "Globals.h"

#include "TSingleton.h"
#include "TChannel.h"
#include "TEventBuildingLoop.h"
#include "TDetectorInformation.h"

class TRunInfo : public TSingleton<TRunInfo> {
public:
   friend class TSingleton<TRunInfo>;

   ~TRunInfo() override = default;
   TRunInfo();   // This should not be used.
   // root forces me have this here instead
   // of a private class member in
   // order to write this class to a tree.
   // pcb.

   static Bool_t ReadInfoFromFile(TFile* tempf = nullptr);

   static const char* GetVersion() { return fVersion.c_str(); }
   static void        ClearVersion() { fVersion.clear(); }
   static void        SetVersion(const char* ver)
   {
      if(fVersion.length() != 0) {
         std::cout << ALERTTEXT << "WARNING; VERSION ALREADY SET TO " << fVersion << "!!" << RESET_COLOR << std::endl;
      } else {
         fVersion.assign(ver);
      }
   }

   static const char* GetFullVersion() { return fFullVersion.c_str(); }
   static void        ClearFullVersion() { fFullVersion.clear(); }
   static void        SetFullVersion(const char* ver)
   {
      if(fFullVersion.length() != 0) {
         std::cout << ALERTTEXT << "WARNING; FULL VERSION ALREADY SET TO " << fFullVersion << "!!" << RESET_COLOR << std::endl;
      } else {
         fFullVersion.assign(ver);
      }
   }

   static const char* GetDate() { return fDate.c_str(); }
   static void        ClearDate() { fDate.clear(); }
   static void        SetDate(const char* ver)
   {
      if(fDate.length() != 0) {
         std::cout << ALERTTEXT << "WARNING; DATE ALREADY SET TO " << fDate << "!!" << RESET_COLOR << std::endl;
      } else {
         fDate.assign(ver);
      }
   }

   static const char* GetLibraryVersion() { return fLibraryVersion.c_str(); }
   static void        ClearLibraryVersion() { fLibraryVersion.clear(); }
   static void        SetLibraryVersion(const char* ver)
   {
      if(fLibraryVersion.length() != 0) {
         std::cout << ALERTTEXT << "WARNING; VERSION ALREADY SET TO " << fLibraryVersion << "!!" << RESET_COLOR << std::endl;
      } else {
         fLibraryVersion.assign(ver);
      }
   }

   static void SetRunInfo(int runnum = 0, int subrunnum = -1);
   static void SetAnalysisTreeBranches(TTree*);

   static inline void SetRunNumber(int tmp) { Get()->fRunNumber = tmp; }
   static inline void SetSubRunNumber(int tmp) { Get()->fSubRunNumber = tmp; }

   static inline int RunNumber() { return Get()->fRunNumber; }
   static inline int SubRunNumber() { return Get()->fSubRunNumber; }

   static inline int FirstRunNumber() { return Get()->fFirstRunNumber; }
   static inline int FirstSubRunNumber() { return Get()->fFirstSubRunNumber; }

   static inline int LastRunNumber() { return Get()->fLastRunNumber; }
   static inline int LastSubRunNumber() { return Get()->fLastSubRunNumber; }

   static inline void SetRunTitle(const char* run_title)
   {
      if(run_title != nullptr) { Get()->fRunTitle.assign(run_title); }
   }
   static inline void SetRunComment(const char* run_comment)
   {
      if(run_comment != nullptr) { Get()->fRunComment.assign(run_comment); }
   }

   static inline std::string RunTitle() { return Get()->fRunTitle; }
   static inline std::string RunComment() { return Get()->fRunComment; }

   static inline void SetRunStart(double tmp) { Get()->fRunStart = tmp; }
   static inline void SetRunStop(double tmp) { Get()->fRunStop = tmp; }
   static inline void SetRunLength(double tmp) { Get()->fRunLength = tmp; }
   static inline void SetRunLength() { Get()->fRunLength = Get()->fRunStop - Get()->fRunStart; }

   static inline double RunStart() { return Get()->fRunStart; }
   static inline double RunStop() { return Get()->fRunStop; }
   static inline double RunLength() { return Get()->fRunLength; }

   static inline void SetCalFileName(const char* name) { Get()->fCalFileName.assign(name); }
   static inline void SetCalFileData(const char* data) { Get()->fCalFile.assign(data); }

   static inline void SetXMLODBFileName(const char* name) { Get()->fXMLODBFileName.assign(name); }
   static inline void SetXMLODBFileData(const char* data) { Get()->fXMLODBFile.assign(data); }

   static const char* GetCalFileName() { return Get()->fCalFileName.c_str(); }
   static const char* GetCalFileData() { return Get()->fCalFile.c_str(); }

   static const char* GetXMLODBFileName() { return Get()->fXMLODBFileName.c_str(); }
   static const char* GetXMLODBFileData() { return Get()->fXMLODBFile.c_str(); }

   static const char* GetRunInfoFileName() { return Get()->fRunInfoFileName.c_str(); }
   static const char* GetRunInfoFileData() { return Get()->fRunInfoFile.c_str(); }

   static Bool_t ReadInfoFile(const char* filename = "");
   static Bool_t ParseInputData(const char* inputdata = "", Option_t* opt = "q");

   static inline void SetRunInfoFileName(const char* fname) { Get()->fRunInfoFileName.assign(fname); }
   static inline void SetRunInfoFile(const char* ffile) { Get()->fRunInfoFile.assign(ffile); }

   static inline void   SetHPGeArrayPosition(const double arr_pos) { Get()->fHPGeArrayPosition = arr_pos; }
   static inline double HPGeArrayPosition() { return Get()->fHPGeArrayPosition; }

   Long64_t Merge(TCollection* list);
   void     Add(TRunInfo* runinfo, bool verbose = false);

   virtual TEventBuildingLoop::EBuildMode BuildMode() const;

   static void PrintBadCycles();
   static void AddBadCycle(int bad_cycle);
   static void RemoveBadCycle(int cycle);
   static bool IsBadCycle(int cycle);

   void PrintRunList();

   static std::string CreateLabel(bool quiet = false);

   static void                  SetDetectorInformation(TDetectorInformation* inf) { Get()->fDetectorInformation = inf; }
   static TDetectorInformation* GetDetectorInformation() { return Get()->fDetectorInformation; }

private:
   std::string                      fRunTitle;                ///< The title of the run
   std::string                      fRunComment;              ///< The comment on the run
   int                              fRunNumber{0};            ///< The current run number
   int                              fSubRunNumber{-1};        ///< The current sub run number
   int                              fFirstRunNumber{0};       ///< The first run number (for combined runs)
   int                              fFirstSubRunNumber{-1};   ///< The first sub run number (for combined subruns)
   int                              fLastRunNumber{0};        ///< The last run number (for combined runs)
   int                              fLastSubRunNumber{-1};    ///< The last sub run number (for combined subruns)
   std::vector<std::pair<int, int>> fRunList;                 ///< List of all runs added to this run info

   double fRunStart{0.};    // The start  of the current run in seconds - no idea why we store this as double?
   double fRunStop{0.};     // The stop   of the current run in seconds - no idea why we store this as double?
   double fRunLength{0.};   // The length of the current run in seconds - no idea why we store this as double?

   static std::string fVersion;          // The version of GRSISort that generated the file - GRSI_RELEASE from GVersion.h
   static std::string fFullVersion;      // The full version of GRSISort that generated the file (includes last commit) - GRSI_GIT_COMMIT from GVersion.h
   static std::string fDate;             // The date of the last commit used in this version - GRSI_GIT_COMMIT_TIME from GVersion.h
   static std::string fLibraryVersion;   // The version of the parser/file library that generated the file

   std::string fCalFileName;   // Name of calfile that generated cal
   std::string fCalFile;       // Cal File to load into Cal of tree

   std::string fXMLODBFileName;   // Name of XML Odb file
   std::string fXMLODBFile;       // The odb

   /////////////////////////////////////////////////
   //////////////// Building Options ///////////////
   /////////////////////////////////////////////////

   std::string fRunInfoFileName;   // The name of the Run info file
   std::string fRunInfoFile;       // The contents of the run info file

   double fHPGeArrayPosition{110.};   // Position of the HPGe Array (default = 110.0 mm );

   std::vector<int> fBadCycleList;   //!<!List of bad cycles to be used for cycle rejection

   TDetectorInformation* fDetectorInformation;   //!<! pointer to detector specific information (set by each parser library)

public:
   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   static bool        WriteToRoot(TFile* fileptr = nullptr);
   static bool        WriteInfoFile(const std::string& filename);
   static std::string PrintToString(Option_t* opt = "");

   /// \cond CLASSIMP
   ClassDefOverride(TRunInfo, 16);   // Contains the run-dependent information.
                                     /// \endcond
};
/*! @} */
#endif
