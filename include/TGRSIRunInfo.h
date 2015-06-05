#ifndef TGRSIRUNINFO_H
#define TGRSIRUNINFO_H



/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 * 
 * Please indicate changes with your initials.
 * 
 *
 */


/* 
 * TGRSIRunInfo designed to be made as the FragmentTree
 * is created.  Right now, it simple remembers the run and 
 * subrunnumber and sets which systems are present in the odb.
 * This information will be used when automatically creating the
 * AnalysisTree to know which detector branches to create and fill.
 *
 * Due to some root quarkiness, I have done something a bit strange.
 * The info is written ok at the end of the fragment tree process.
 * After reading the TGRSIRunInfo object from a TFile, the static function
 *
 *   TGRSIRunInfo::SetInfoFromFile(ptr_to_runinfo);
 *
 * must be called for any of teh functions here to work.
 *
 * Live example:
 
 root [1] TGRSIRunInfo *info = (TGRSIRunInfo*)_file0->Get("TGRSIRunInfo")
 root [2] TGRSIRunInfo::SetInfoFromFile(info);
 root [3] info->Print()
   TGRSIRunInfo Status:
   RunNumber:    29038
   SubRunNumber: 000
   TIGRESS:      true
   SHARC:        true
   GRIFFIN:      false
   SCEPTAR:      false
   =====================
 root [4] 

 *
 *
 */






#include "Globals.h"

#include <cstdio>

#include <TObject.h>
#include <TTree.h>
#include <TFile.h>

#include "TChannel.h"

class TGRSIRunInfo : public TObject {

   public:
      static TGRSIRunInfo *Get();
      ~TGRSIRunInfo();
      TGRSIRunInfo();   // This should not be used.
                        // root forces me have this here instead 
                        // of a private class member in 
                        // order to write this class to a tree.
                        // pcb.
      
      static void SetInfoFromFile(TGRSIRunInfo *temp);

      static const char* GetGRSIVersion() { return fGRSIVersion.c_str(); } 
      static void ClearGRSIVersion() { fGRSIVersion.clear(); } 
      static void SetGRSIVersion(const char *ver) { if(fGRSIVersion.length()!=0)
                                                    printf( ALERTTEXT "WARNING; VERSION ALREADY SET TO %s!!" RESET_COLOR "\n",fGRSIVersion.c_str());
                                                    else fGRSIVersion.assign(ver); }



      static void SetRunInfo(int runnum=0,int subrunnum=-1);
      static void SetAnalysisTreeBranches(TTree*);

      static inline void SetRunNumber(int tmp) { fGRSIRunInfo->fRunNumber = tmp; }
      static inline void SetSubRunNumber(int tmp) { fGRSIRunInfo->fSubRunNumber = tmp; }

      static inline int  RunNumber() { return fGRSIRunInfo->fRunNumber; }
      static inline int  SubRunNumber() { return fGRSIRunInfo->fSubRunNumber; }

      static inline void SetMajorIndex(const char *tmpstr) { fGRSIRunInfo->fMajorIndex.assign(tmpstr); }
      static inline void SetMinorIndex(const char *tmpstr) { fGRSIRunInfo->fMinorIndex.assign(tmpstr); }

      static inline std::string MajorIndex() { return fGRSIRunInfo->fMajorIndex; }
      static inline std::string MinorIndex() { return fGRSIRunInfo->fMinorIndex; }

      static inline void SetTigress(bool flag = true)     { fGRSIRunInfo->fTigress = flag; }
      static inline void SetSharc(bool flag = true)       { fGRSIRunInfo->fSharc = flag; }
      static inline void SetTriFoil(bool flag = true)     { fGRSIRunInfo->fTriFoil = flag; }
      static inline void SetRf(bool flag = true)          { fGRSIRunInfo->fRf = flag; }
      static inline void SetCSM(bool flag = true)         { fGRSIRunInfo->fCSM = flag; }
      static inline void SetSpice(bool flag = true)       { fGRSIRunInfo->fSpice = flag; }
      static inline void SetS3(bool flag = true)          { fGRSIRunInfo->fS3 = flag;  }
      static inline void SetTip(bool flag = true)         { fGRSIRunInfo->fTip = flag; }

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

      static Bool_t  ReadInfoFile(const char *filename = "");
      static Bool_t  ParseInputData(const char *inputdata = "",Option_t *opt = "q");

      static inline int  GetNumberOfSystems() { return fGRSIRunInfo->fNumberOfTrueSystems; }

      static inline bool Tigress()   { return fGRSIRunInfo->fTigress; }
      static inline bool Sharc()     { return fGRSIRunInfo->fSharc; }
      static inline bool TriFoil()   { return fGRSIRunInfo->fTriFoil; }
      static inline bool Rf()        { return fGRSIRunInfo->fRf; }
      static inline bool CSM()       { return fGRSIRunInfo->fCSM; }
      static inline bool Spice()     { return fGRSIRunInfo->fSpice; }
      static inline bool Tip()       { return fGRSIRunInfo->fTip; }
      static inline bool S3()        { return fGRSIRunInfo->fS3; }

      static inline bool Griffin()   { return fGRSIRunInfo->fGriffin; }
      static inline bool Sceptar()   { return fGRSIRunInfo->fSceptar; }
      static inline bool Paces()     { return fGRSIRunInfo->fPaces; }
      static inline bool Dante()     { return fGRSIRunInfo->fDante; }
      static inline bool ZeroDegree(){ return fGRSIRunInfo->fZeroDegree; }
      static inline bool Descant()   { return fGRSIRunInfo->fDescant; }

      inline void SetRunInfoFileName(const char *fname) {  fRunInfoFileName.assign(fname); }
      inline void SetRunInfoFile(const char *ffile)     {  fRunInfoFile.assign(ffile); }
     

      inline void SetBuildWindow(const long int t_bw)    { fBuildWindow = t_bw; } 
      inline void SetAddBackWindow(const double   t_abw) { fAddBackWindow = t_abw; } 

      static inline long int BuildWindow()    { return Get()->fBuildWindow; }
      static inline double   AddBackWindow()  { return Get()->fAddBackWindow; }

      inline void SetHPGeArrayPosition(const int arr_pos) { fHPGeArrayPosition = arr_pos; }
      static inline int  HPGeArrayPosition()  { return Get()->fHPGeArrayPosition; }

   private:
      static TGRSIRunInfo *fGRSIRunInfo;
      //TGRSIRunInfo();

      int fRunNumber;
      int fSubRunNumber;

      int fNumberOfTrueSystems;

      static std::string fGRSIVersion;

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


      bool fTigress;
      bool fSharc;
      bool fTriFoil;
      bool fRf;
      bool fCSM;
      bool fSpice;
      bool fTip;
      bool fS3;

      bool fGriffin;
      bool fSceptar;
      bool fPaces;
      bool fDante;
      bool fZeroDegree;
      bool fDescant;

      std::string fCalFileName;
      std::string fCalFile;

      std::string fXMLODBFileName;
      std::string fXMLODBFile;

      std::string fMajorIndex;  
      std::string fMinorIndex;  

      /////////////////////////////////////////////////
      //////////////// Building Options ///////////////
      /////////////////////////////////////////////////

      std::string fRunInfoFileName;
      std::string fRunInfoFile;
	   static void trim(std::string *, const std::string & trimChars = " \f\n\r\t\v");

      long int fBuildWindow;          // if building with a window(GRIFFIN) this is the size of the window. (default = 2us (200))
      double   fAddBackWindow;        // Time used to build Addback-Ge-Events for TIGRESS/GRIFFIN.   (default =150 ns (15.0))
      
      double  fHPGeArrayPosition;        // Position of the HPGe Array (default = 110.0 mm );
  

   public:
      void Print(Option_t *opt = "");
      void Clear(Option_t *opt = "");

   ClassDef(TGRSIRunInfo,3);
};

#endif
