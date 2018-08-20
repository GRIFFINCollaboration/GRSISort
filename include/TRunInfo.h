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

   ~TRunInfo() override;
   TRunInfo(); // This should not be used.
   // root forces me have this here instead
   // of a private class member in
   // order to write this class to a tree.
   // pcb.

   static Bool_t ReadInfoFromFile(TFile* tempf = nullptr);

   static const char* GetVersion() { return fVersion.c_str(); }
   static void        ClearVersion() { fVersion.clear(); }
   static void SetVersion(const char* ver)
   {
      if(fVersion.length() != 0) {
         printf(ALERTTEXT "WARNING; VERSION ALREADY SET TO %s!!" RESET_COLOR "\n", fVersion.c_str());
      } else {
         fVersion.assign(ver);
      }
   }

   static void SetRunInfo(int runnum = 0, int subrunnum = -1);
   static void SetAnalysisTreeBranches(TTree*);

   static inline void SetRunNumber(int tmp) { Get()->fRunNumber = tmp; }
   static inline void SetSubRunNumber(int tmp) { Get()->fSubRunNumber = tmp; }

   static inline int RunNumber() { return Get()->fRunNumber; }
   static inline int SubRunNumber() { return Get()->fSubRunNumber; }

   inline void SetRunTitle(const char* run_title) { if(run_title != nullptr) Get()->fRunTitle.assign(run_title); }
   inline void SetRunComment(const char* run_comment) { if(run_comment != nullptr) Get()->fRunComment.assign(run_comment); }

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

   inline void SetRunInfoFileName(const char* fname) { Get()->fRunInfoFileName.assign(fname); }
   inline void SetRunInfoFile(const char* ffile) { Get()->fRunInfoFile.assign(ffile); }

   inline void SetHPGeArrayPosition(const double arr_pos) { Get()->fHPGeArrayPosition = arr_pos; }
   static inline double                          HPGeArrayPosition() { return Get()->fHPGeArrayPosition; }

   Long64_t Merge(TCollection* list);
   void Add(TRunInfo* runinfo)
   {
		// add the run length together
      if(runinfo->fRunLength > 0) {
			if(Get()->fRunLength > 0) {
				Get()->fRunLength += runinfo->fRunLength;
			} else {
				Get()->fRunLength = runinfo->fRunLength;
			}
		}
		if(runinfo->fRunNumber != Get()->fRunNumber) {
			// the run number is meaningful only when the run numbers are the same
			Get()->fRunNumber = 0;
			Get()->fSubRunNumber = -1;
			Get()->fRunStart = 0.;
			Get()->fRunStop  = 0.;
		} else if(runinfo->fSubRunNumber == Get()->fSubRunNumber + 1) {
			// if the run numbers are the same and we have subsequent sub runs we can update the run stop
			Get()->fRunStop = runinfo->fRunStop;
			Get()->fSubRunNumber = runinfo->fSubRunNumber; // so we can check the next subrun as well
		} else {
			// with multiple files added, the sub run number has no meaning anymore
			Get()->fSubRunNumber = -1;
			Get()->fRunStart = 0.;
			Get()->fRunStop  = 0.;
		}
   }

	virtual TEventBuildingLoop::EBuildMode BuildMode() const;

   void PrintBadCycles() const;
   void AddBadCycle(int bad_cycle);
   void RemoveBadCycle(int cycle);
   bool IsBadCycle(int cycle) const;

	static void SetDetectorInformation(TDetectorInformation* inf) { Get()->fDetectorInformation = inf; }
	static TDetectorInformation* GetDetectorInformation() { return Get()->fDetectorInformation; }

private:
   std::string fRunTitle;     ///< The title of the run
   std::string fRunComment;   ///< The comment on the run
   int         fRunNumber;    // The current run number
   int         fSubRunNumber; // The current sub run number

   double fRunStart{0.};  // The start  of the current run in seconds
   double fRunStop{0.};   // The stop   of the current run in seconds
   double fRunLength{0.}; // The length of the current run in seconds

   static std::string fVersion; // The version of GRSISort that generated the file

   std::string fCalFileName; // Name of calfile that generated cal
   std::string fCalFile;     // Cal File to load into Cal of tree

   std::string fXMLODBFileName; // Name of XML Odb file
   std::string fXMLODBFile;     // The odb

   /////////////////////////////////////////////////
   //////////////// Building Options ///////////////
   /////////////////////////////////////////////////

   std::string fRunInfoFileName; // The name of the Run info file
   std::string fRunInfoFile;     // The contents of the run info file
   static void trim(std::string*, const std::string& trimChars = " \f\n\r\t\v");

   double fHPGeArrayPosition; // Position of the HPGe Array (default = 110.0 mm );

   unsigned int     fBadCycleListSize;
   std::vector<int> fBadCycleList; //!<!List of bad cycles to be used for cycle rejection

	TDetectorInformation* fDetectorInformation; //!<! pointer to detector specific information (set by each parser library)

public:
   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   static bool WriteToRoot(TFile* fileptr = nullptr);
   static bool WriteInfoFile(const std::string& filename);
   std::string PrintToString(Option_t* opt = "");

   /// \cond CLASSIMP
   ClassDefOverride(TRunInfo, 14); // Contains the run-dependent information.
   /// \endcond
};
/*! @} */
#endif
