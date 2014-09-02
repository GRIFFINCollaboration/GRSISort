#ifndef TGRSIRUNINFO_H
#define TGRSIRUNINFO_H

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

      static void SetRunInfo();
      static void SetAnalysisTreeBranches(TTree*);

      static inline void SetRunNumber(int tmp) { fRunNumber = tmp; }
      static inline void SetSubRunNumber(int tmp) { fSubRunNumber = tmp; }

      static inline int  RunNumber(int tmp) { return fRunNumber; }
      static inline int  SubRunNumber(int tmp) { return fSubRunNumber; }

      
      static inline void SetTigress(bool flag = true)     { fTigress = flag; }
      static inline void SetSharc(bool flag = true)       { fSharc = flag; }
      static inline void SetTriFoil(bool flag = true)     { fTriFoil = flag; }
      static inline void SetRf(bool flag = true)          { fRf = flag; }
      static inline void SetCSM(bool flag = true)         { fCSM = flag; }
      static inline void SetSpice(bool flag = true)       { fSpice = flag; }
      static inline void SetTip(bool flag = true)         { fTip = flag; }

      static inline void SetGriffin(bool flag = true)     { fGriffin = flag; }
      static inline void SetSceptar(bool flag = true)     { fSceptar = flag; }
      static inline void SetPaces(bool flag = true)       { fPaces = flag; }
      static inline void SetDante(bool flag = true)       { fDante = flag; }
      static inline void SetZeroDegree(bool flag = true)  { fZeroDegree = flag; }
      static inline void SetDescant(bool flag = true)     { fDescant = flag; }

      static inline bool Tigress()   { return fTigress; }
      static inline bool Sharc()     { return fSharc; }
      static inline bool TriFoil()   { return fTriFoil; }
      static inline bool Rf()        { return fRf; }
      static inline bool CSM()       { return fCSM; }
      static inline bool Spice()     { return fSpice; }
      static inline bool Tip()       { return fTip; }

      static inline bool Griffin()   { return fGriffin; }
      static inline bool Sceptar()   { return fSceptar; }
      static inline bool Paces()     { return fPaces; }
      static inline bool Dante()     { return fDante; }
      static inline bool ZeroDegree(){ return fZeroDegree; }
      static inline bool Descant()   { return fDescant; }

   private:
      static TGRSIRunInfo *fGRSIRunInfo;
      TGRSIRunInfo();

      static int fRunNumber;
      static int fSubRunNumber;

      static bool fTigress;
      static bool fSharc;
      static bool fTriFoil;
      static bool fRf;
      static bool fCSM;
      static bool fSpice;
      static bool fTip;
      
      static bool fGriffin;
      static bool fSceptar;
      static bool fPaces;
      static bool fDante;
      static bool fZeroDegree;
      static bool fDescant;


   public:
      void Print(Option_t *opt = "");
      void Clear(Option_t *opt = "");

   ClassDef(TGRSIRunInfo,1);
};

#endif
