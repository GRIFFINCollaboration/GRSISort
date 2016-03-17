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
/// This information will be used when automatically creating the
/// AnalysisTree to know which detector branches to create and fill.
///
/// Due to some root quarkiness, I have done something a bit strange.
/// The info is written ok at the end of the fragment tree process.
/// After reading the TGRSIRunInfo object from a TFile, the static function
///
///   TGRSIRunInfo::ReadInfoFromFile(ptr_to_runinfo);
///
/// must be called for any of the functions here to work.
///
/// Live example:
/// 
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

#include "TChannel.h"

class TGRSIRunInfo : public TObject {
   public:
      static TGRSIRunInfo *Get();
      virtual ~TGRSIRunInfo();
      TGRSIRunInfo();   // This should not be used.
      // root forces me have this here instead 
      // of a private class member in 
      // order to write this class to a tree.
      // pcb.

      static void SetRunInfo(TGRSIRunInfo *temp);
      static Bool_t ReadInfoFromFile(TFile *tempf = 0);

      static const char* GetGRSIVersion() { return fGRSIVersion.c_str(); } 
      static void ClearGRSIVersion() { fGRSIVersion.clear(); } 
      static void SetGRSIVersion(const char *ver) { 
         if(fGRSIVersion.length()!=0)
            printf( ALERTTEXT "WARNING; VERSION ALREADY SET TO %s!!" RESET_COLOR "\n",fGRSIVersion.c_str());
         else 
            fGRSIVersion.assign(ver); 
      }

      static void SetRunInfo(int runnum=0,int subrunnum=-1);
      static void SetAnalysisTreeBranches(TTree*);

      static inline void SetRunNumber(int tmp) { fGRSIRunInfo->fRunNumber = tmp; }
      static inline void SetSubRunNumber(int tmp) { fGRSIRunInfo->fSubRunNumber = tmp; }

      static inline int  RunNumber() { return fGRSIRunInfo->fRunNumber; }
      static inline int  SubRunNumber() { return fGRSIRunInfo->fSubRunNumber; }

      static inline void   SetRunStart(double tmp)  { fGRSIRunInfo->fRunStart = tmp; }
      static inline void   SetRunStop(double tmp)   { fGRSIRunInfo->fRunStop = tmp; }
      static inline void   SetRunLength(double tmp) { fGRSIRunInfo->fRunLength = tmp; }

      static inline double RunStart()  { return fGRSIRunInfo->fRunStart; }
      static inline double RunStop()   { return fGRSIRunInfo->fRunStop; }
      static inline double RunLength() { return fGRSIRunInfo->fRunLength; }

      static inline void SetMajorIndex(const char *tmpstr) { fGRSIRunInfo->fMajorIndex.assign(tmpstr); }
      static inline void SetMinorIndex(const char *tmpstr) { fGRSIRunInfo->fMinorIndex.assign(tmpstr); }

      static inline std::string MajorIndex() { return fGRSIRunInfo->fMajorIndex; }
      static inline std::string MinorIndex() { return fGRSIRunInfo->fMinorIndex; }

      static inline void SetTigress(bool flag = true)     { fGRSIRunInfo->fTigress = flag; }
      static inline void SetSharc(bool flag = true)       { fGRSIRunInfo->fSharc = flag; }
      static inline void SetTriFoil(bool flag = true)     { fGRSIRunInfo->fTriFoil = flag; }
      static inline void SetRF(bool flag = true)          { fGRSIRunInfo->fRf = flag; }
      static inline void SetCSM(bool flag = true)         { fGRSIRunInfo->fCSM = flag; }
      static inline void SetSpice(bool flag = true)       { fGRSIRunInfo->fSpice = flag; }
      static inline void SetS3(bool flag = true)          { fGRSIRunInfo->fS3 = flag;  }
      static inline void SetTip(bool flag = true)         { fGRSIRunInfo->fTip = flag; }
      static inline void SetBambino(bool flag = true)			{ fGRSIRunInfo->fBambino = flag; }

      static inline void SetGriffin(bool flag = true)     { fGRSIRunInfo->fGriffin = flag; }
      static inline void SetSceptar(bool flag = true)     { fGRSIRunInfo->fSceptar = flag; }
      static inline void SetPaces(bool flag = true)       { fGRSIRunInfo->fPaces = flag; }
      static inline void SetDante(bool flag = true)       { fGRSIRunInfo->fDante = flag; }
      static inline void SetZeroDegree(bool flag = true)  { fGRSIRunInfo->fZeroDegree = flag; }
      static inline void SetDescant(bool flag = true)     { fGRSIRunInfo->fDescant = flag; }

      static inline void SetCalFileName(const char *name) { fGRSIRunInfo->fCalFileName.assign(name); }
      static inline void SetCalFileData(const char *data) { fGRSIRunInfo->fCalFile.assign(data); }

      static inline void SetXMLODBFileName(const char *name) { fGRSIRunInfo->fXMLODBFileName.assign(name); }
      static inline void SetXMLODBFileData(const char *data) { fGRSIRunInfo->fXMLODBFile.assign(data); }

      static const char* GetCalFileName() { return fGRSIRunInfo->fCalFileName.c_str(); } 
      static const char* GetCalFileData() { return fGRSIRunInfo->fCalFile.c_str(); }

      static const char* GetXMLODBFileName() { return fGRSIRunInfo->fXMLODBFileName.c_str(); }
      static const char* GetXMLODBFileData() { return fGRSIRunInfo->fXMLODBFile.c_str(); }

      static const char* GetRunInfoFileName() { return fGRSIRunInfo->fRunInfoFileName.c_str(); }
      static const char* GetRunInfoFileData() { return fGRSIRunInfo->fRunInfoFile.c_str(); }

      static Bool_t  ReadInfoFile(const char *filename = "");
      static Bool_t  ParseInputData(const char *inputdata = "",Option_t *opt = "q");

      static inline int  GetNumberOfSystems() { return fGRSIRunInfo->fNumberOfTrueSystems; }

      static inline bool Tigress()   { return fGRSIRunInfo->fTigress; }
      static inline bool Sharc()     { return fGRSIRunInfo->fSharc; }
      static inline bool TriFoil()   { return fGRSIRunInfo->fTriFoil; }
      static inline bool RF()        { return fGRSIRunInfo->fRf; }
      static inline bool CSM()       { return fGRSIRunInfo->fCSM; }
      static inline bool Spice()     { return fGRSIRunInfo->fSpice; }
      static inline bool Bambino()	 { return fGRSIRunInfo->fBambino; }
      static inline bool Tip()       { return fGRSIRunInfo->fTip; }
      static inline bool S3()        { return fGRSIRunInfo->fS3; }

      static inline bool Griffin()   { return fGRSIRunInfo->fGriffin; }
      static inline bool Sceptar()   { return fGRSIRunInfo->fSceptar; }
      static inline bool Paces()     { return fGRSIRunInfo->fPaces; }
      static inline bool Dante()     { return fGRSIRunInfo->fDante; }
      static inline bool ZeroDegree(){ return fGRSIRunInfo->fZeroDegree; }
      static inline bool Descant()   { return fGRSIRunInfo->fDescant; }

      inline void SetRunInfoFileName(const char *fname)  {  fRunInfoFileName.assign(fname); }
      inline void SetRunInfoFile(const char *ffile)      {  fRunInfoFile.assign(ffile); }

      inline void SetBuildWindow(const long int t_bw)    { fBuildWindow = t_bw; } 
      inline void SetAddBackWindow(const double   t_abw) { fAddBackWindow = t_abw; } 
      inline void SetBufferDuration(const long int t_bd) { fBufferDuration = t_bd; }
      inline void SetBufferSize(const size_t t_bs)          { fBufferSize = t_bs; }

      inline void SetWaveformFitting(const bool flag)    {fWaveformFitting = flag; }
      static inline bool IsWaveformFitting()             {return Get()->fWaveformFitting; }

      inline void SetMovingWindow(const bool flag)       {fIsMovingWindow = flag; }
      static inline bool IsMovingWindow()                { return Get()->fIsMovingWindow; }

      static inline long int BuildWindow()    { return Get()->fBuildWindow; }
      static inline double   AddBackWindow()  { if(Get()->fAddBackWindow<1) return 15.0; return Get()->fAddBackWindow; }
      static inline long int BufferDuration() { return Get()->fBufferDuration; }
      static inline size_t   BufferSize()     { return Get()->fBufferSize; }

      inline void SetHPGeArrayPosition(const int arr_pos) { fHPGeArrayPosition = arr_pos; }
      static inline int  HPGeArrayPosition()  { return Get()->fHPGeArrayPosition; }

      static inline void SetDescantAncillary(bool flag = true) { fGRSIRunInfo->fDescantAncillary = flag; }
      static inline bool DescantAncillary()                    { return fGRSIRunInfo->fDescantAncillary; }

      Long64_t Merge(TCollection *list);
      void Add(TGRSIRunInfo* runinfo) { fRunStart = 0.; fRunStop = 0.; fRunLength += runinfo->RunLength(); }

   private:
      static TGRSIRunInfo *fGRSIRunInfo; //Static pointer to TGRSIRunInfo

      int fRunNumber;                     //The current run number
      int fSubRunNumber;                  //The current sub run number

      double fRunStart;                      //The start  of the current run in seconds
      double fRunStop;                       //The stop   of the current run in seconds
      double fRunLength;                     //The length of the current run in seconds

      int fNumberOfTrueSystems;           //The number of detection systems in the array

      static std::string fGRSIVersion;    //The version of GRSISort that generated the file

      //  detector types to switch over in SetRunInfo()
      //  for more info, see: https://www.triumf.info/wiki/tigwiki/index.php/Detector_Nomenclature
      //enum det_types {"TI", // TIGRESS
      //                "SH", // SHARC
      //                "TR", // TriFoil
      //                "RF", // RF
      //                "CS", // Colorado
      //                "SP", // SPICE
      //                "TP", // TIP
      //                "GR", // GRIFFIN
      //                "SE", // SCEPTAR
      //                "PA", // PACES
      //                "DA", // DANTE
      //                "ZD", // Zero Degree
      //                "DS"  // DESCANT
      //               };


      bool fTigress;    //flag for Tigress on/off
      bool fSharc;      //flag for Sharc on/off
      bool fTriFoil;    //flag for TriFoil on/off
      bool fRf;         //flag for RF on/off
      bool fCSM;        //flag for CSM on/off
      bool fSpice;      //flag for Spice on/off
      bool fTip;        //flag for Tip on/off
      bool fS3;         //flag for S3 on/off
      bool fBambino;		//flag for Bambino on/off

      bool fGriffin;    //flag for Griffin on/off
      bool fSceptar;    //flag for Sceptar on/off
      bool fPaces;      //flag for Paces on/off 
      bool fDante;      //flag for LaBr on/off
      bool fZeroDegree; //flag for Zero Degree Scintillator on/off
      bool fDescant;    //flag for Descant on/off

      std::string fCalFileName;  //Name of calfile that generated cal
      std::string fCalFile;      //Cal File to load into Cal of tree

      std::string fXMLODBFileName;  //Name of XML Odb file
      std::string fXMLODBFile;      //The odb

      std::string fMajorIndex;  //The Major index to order events during building
      std::string fMinorIndex;  //The Minor index to order events during building

      /////////////////////////////////////////////////
      //////////////// Building Options ///////////////
      /////////////////////////////////////////////////

      std::string fRunInfoFileName; //The name of the Run info file
      std::string fRunInfoFile;     //The contents of the run info file
      static void trim(std::string *, const std::string & trimChars = " \f\n\r\t\v");

      long int fBuildWindow;        // if building with a window(GRIFFIN) this is the size of the window. (default = 2us (200))
      double   fAddBackWindow;      // Time used to build Addback-Ge-Events for TIGRESS/GRIFFIN.   (default =150 ns (15.0))
      bool     fIsMovingWindow;     // if set to true the event building window moves. Static otherwise.

      long int fBufferDuration;     // GRIFFIN: the minimum length of the sorting buffer (default = 600s (60000000000))
      size_t   fBufferSize;           // GRIFFIN: the minimum size of the sorting buffer (default = 1 000 000)

      bool 	   fWaveformFitting;    // If true, waveform fitting with SFU algorithm will be performed

      double   fHPGeArrayPosition;  // Position of the HPGe Array (default = 110.0 mm );
      bool     fDescantAncillary;   // Descant is in the ancillary detector locations

   public:
      void Print(Option_t *opt = "") const;
      void Clear(Option_t *opt = "");

      /// \cond CLASSIMP
      ClassDef(TGRSIRunInfo,9);  //Contains the run-dependent information.
      /// \endcond
};
/*! @} */
#endif
