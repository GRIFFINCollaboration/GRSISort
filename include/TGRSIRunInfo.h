#ifndef TGRSIRUNINFO_H
#define TGRSIRUNINFO_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIRunInfo
///
/// This Class is designed to store and run dependent
/// information. It is used to store run numbers, existence of
/// detector systems, reconstruction windows, etc. The
/// TGRSIRunInfo is written alongside both the fragment and
/// analysis trees.
///
/// TGRSIRunInfo designed to be made as the FragmentTree
/// is created.  Right now, it simple remembers the run and
/// subrunnumber and sets which systems are present in the odb.
///
/// Due to some root quarkiness, I have done something a bit strange.
/// The info is written ok at the end of the fragment tree process.
/// \code
/// root [1] TGRSIRunInfo *info = (TGRSIRunInfo*)_file0->Get("TGRSIRunInfo")
/// root [2] TGRSIRunInfo::ReadInfoFromFile(info);
/// root [3] info->Print()
///   TGRSIRunInfo Status:
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

class TGRSIRunInfo : public TSingleton<TGRSIRunInfo> {
public:
	friend class TSingleton<TGRSIRunInfo>;

   ~TGRSIRunInfo() override;
   TGRSIRunInfo(); // This should not be used.
   // root forces me have this here instead
   // of a private class member in
   // order to write this class to a tree.
   // pcb.

   static Bool_t ReadInfoFromFile(TFile* tempf = nullptr);

   static const char* GetGRSIVersion() { return fGRSIVersion.c_str(); }
   static void        ClearGRSIVersion() { fGRSIVersion.clear(); }
   static void SetGRSIVersion(const char* ver)
   {
      if(fGRSIVersion.length() != 0) {
         printf(ALERTTEXT "WARNING; VERSION ALREADY SET TO %s!!" RESET_COLOR "\n", fGRSIVersion.c_str());
      } else {
         fGRSIVersion.assign(ver);
      }
   }

   static void SetRunInfo(int runnum = 0, int subrunnum = -1);
   static void SetAnalysisTreeBranches(TTree*);

   static inline void SetRunNumber(int tmp) { Get()->fRunNumber = tmp; }
   static inline void SetSubRunNumber(int tmp) { Get()->fSubRunNumber = tmp; }

   static inline int RunNumber() { return Get()->fRunNumber; }
   static inline int SubRunNumber() { return Get()->fSubRunNumber; }

   inline void SetRunTitle(const char* run_title) { Get()->fRunTitle.assign(run_title); }
   inline void SetRunComment(const char* run_comment) { Get()->fRunComment.assign(run_comment); }

	static inline std::string RunTitle() { return Get()->fRunTitle; }
	static inline std::string RunComment() { return Get()->fRunComment; }

   static inline void SetRunStart(double tmp) { Get()->fRunStart = tmp; }
   static inline void SetRunStop(double tmp) { Get()->fRunStop = tmp; }
   static inline void SetRunLength(double tmp) { Get()->fRunLength = tmp; }
   static inline void SetRunLength() { Get()->fRunLength = Get()->fRunStop - Get()->fRunStart; }

   static inline double RunStart() { return Get()->fRunStart; }
   static inline double RunStop() { return Get()->fRunStop; }
   static inline double RunLength() { return Get()->fRunLength; }

   static inline void SetTigress(bool flag = true) { Get()->fTigress = flag; }
   static inline void SetSharc(bool flag = true) { Get()->fSharc = flag; }
   static inline void SetTriFoil(bool flag = true) { Get()->fTriFoil = flag; }
   static inline void SetRF(bool flag = true) { Get()->fRf = flag; }
   static inline void SetCSM(bool flag = true) { Get()->fCSM = flag; }
   static inline void SetSpice(bool flag = true) { Get()->fSpice = flag; }
   static inline void SetS3(bool flag = true) { Get()->fS3 = flag; }
   static inline void SetGeneric(bool flag = true) { Get()->fGeneric = flag; }
   static inline void SetTip(bool flag = true) { Get()->fTip = flag; }
   static inline void SetBambino(bool flag = true) { Get()->fBambino = flag; }

   static inline void SetGriffin(bool flag = true) { Get()->fGriffin = flag; }
   static inline void SetSceptar(bool flag = true) { Get()->fSceptar = flag; }
   static inline void SetPaces(bool flag = true) { Get()->fPaces = flag; }
   static inline void SetDante(bool flag = true) { Get()->fDante = flag; }
   static inline void SetZeroDegree(bool flag = true) { Get()->fZeroDegree = flag; }
   static inline void SetDescant(bool flag = true) { Get()->fDescant = flag; }

   static inline void SetBgo(bool flag = true) { Get()->fBgo = flag; }

   static inline void SetFipps(bool flag = true) { Get()->fFipps = flag; }

   static inline void SetTdrClover(bool flag = true) { Get()->fTdrClover = flag; }
   static inline void SetTdrTigress(bool flag = true) { Get()->fTdrTigress = flag; }
   static inline void SetTdrSiLi(bool flag = true) { Get()->fTdrSiLi = flag; }
   static inline void SetTdrPlastic(bool flag = true) { Get()->fTdrPlastic = flag; }

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

   static inline int GetNumberOfSystems() { return Get()->fNumberOfTrueSystems; }

   static inline bool Tigress() { return Get()->fTigress; }
   static inline bool Sharc() { return Get()->fSharc; }
   static inline bool TriFoil() { return Get()->fTriFoil; }
   static inline bool RF() { return Get()->fRf; }
   static inline bool CSM() { return Get()->fCSM; }
   static inline bool Spice() { return Get()->fSpice; }
   static inline bool Bambino() { return Get()->fBambino; }
   static inline bool Tip() { return Get()->fTip; }
   static inline bool S3() { return Get()->fS3; }
   static inline bool Generic() { return Get()->fGeneric; }

   static inline bool Griffin() { return Get()->fGriffin; }
   static inline bool Sceptar() { return Get()->fSceptar; }
   static inline bool Paces() { return Get()->fPaces; }
   static inline bool Dante() { return Get()->fDante; }
   static inline bool ZeroDegree() { return Get()->fZeroDegree; }
   static inline bool Descant() { return Get()->fDescant; }

   static inline bool Bgo() { return Get()->fBgo; }

   static inline bool Fipps() { return Get()->fFipps; }

   static inline bool TdrClover() { return Get()->fTdrClover; }
   static inline bool TdrTigress() { return Get()->fTdrTigress; }
   static inline bool TdrSiLi() { return Get()->fTdrSiLi; }
   static inline bool TdrPlastic() { return Get()->fTdrPlastic; }

   inline void SetRunInfoFileName(const char* fname) { Get()->fRunInfoFileName.assign(fname); }
   inline void SetRunInfoFile(const char* ffile) { Get()->fRunInfoFile.assign(ffile); }

   inline void SetHPGeArrayPosition(const double arr_pos) { Get()->fHPGeArrayPosition = arr_pos; }
   static inline double                          HPGeArrayPosition() { return Get()->fHPGeArrayPosition; }

   static inline void SetDescantAncillary(bool flag = true) { Get()->fDescantAncillary = flag; }
   static inline bool                          DescantAncillary() { return Get()->fDescantAncillary; }

   Long64_t Merge(TCollection* list);
   void Add(TGRSIRunInfo* runinfo)
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

   void PrintBadCycles() const;
   void AddBadCycle(int bad_cycle);
   void RemoveBadCycle(int cycle);
   bool IsBadCycle(int cycle) const;

private:
   std::string fRunTitle;     ///< The title of the run
   std::string fRunComment;   ///< The comment on the run
   int         fRunNumber;    // The current run number
   int         fSubRunNumber; // The current sub run number

   double fRunStart{0.};  // The start  of the current run in seconds
   double fRunStop{0.};   // The stop   of the current run in seconds
   double fRunLength{0.}; // The length of the current run in seconds

   int fNumberOfTrueSystems{0}; // The number of detection systems in the array

   static std::string fGRSIVersion; // The version of GRSISort that generated the file

   //  detector types to switch over in SetRunInfo()
   //  for more info, see: https://www.triumf.info/wiki/tigwiki/index.php/Detector_Nomenclature

   bool fTigress{false}; // flag for Tigress on/off
   bool fSharc{false};   // flag for Sharc on/off
   bool fTriFoil{false}; // flag for TriFoil on/off
   bool fRf{false};      // flag for RF on/off
   bool fCSM{false};     // flag for CSM on/off
   bool fSpice{false};   // flag for Spice on/off
   bool fTip{false};     // flag for Tip on/off
   bool fS3{false};      // flag for S3 on/off
   bool fGeneric{false}; // flag for Generic on/off
   bool fBambino{false}; // flag for Bambino on/off

   bool fGriffin{false};    // flag for Griffin on/off
   bool fSceptar{false};    // flag for Sceptar on/off
   bool fPaces{false};      // flag for Paces on/off
   bool fDante{false};      // flag for LaBr on/off
   bool fZeroDegree{false}; // flag for Zero Degree Scintillator on/off
   bool fDescant{false};    // flag for Descant on/off

   bool fBgo{false};        // flag for Bgo on/off

   bool fFipps{false};      // flag for Fipps on/off

   bool fTdrClover{false};  // flag for TdrClover on/off
   bool fTdrTigress{false}; // flag for TdrTigress on/off
   bool fTdrSiLi{false};    // flag for TdrSiLi on/off
   bool fTdrPlastic{false}; // flag for TdrPlastic on/off

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
   bool   fDescantAncillary;  // Descant is in the ancillary detector locations

   unsigned int     fBadCycleListSize;
   std::vector<int> fBadCycleList; //!<!List of bad cycles to be used for cycle rejection

public:
   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   static bool WriteToRoot(TFile* fileptr = nullptr);
   static bool WriteInfoFile(const std::string& filename);
   std::string PrintToString(Option_t* opt = "");

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIRunInfo, 13); // Contains the run-dependent information.
   /// \endcond
};
/*! @} */
#endif
