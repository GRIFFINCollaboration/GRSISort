#ifndef TSCEPTAR_H
#define TSCEPTAR_H

#include "Globals.h"

#include <vector>
#include <cstdio>

#include "TSceptarHit.h"
#ifndef __CINT__
#include "TSceptarData.h"
#else
class TSceptarData;
#endif
#include "TVector3.h" 

#include "TGRSIDetector.h" 


class TSceptar : public TGRSIDetector {

  public:
     TSceptar();
     virtual ~TSceptar();
     TSceptar(const TSceptar& rhs);

  public: 
     TGRSIDetectorHit* GetHit(const Int_t idx =0);
     void Copy(TObject &rhs) const;
     TSceptarHit *GetSceptarHit(int i)        {	return &sceptar_hits[i];   }	//!
     Short_t GetMultiplicity() const	       {	return sceptar_hits.size();}	//!

     static TVector3 GetPosition(int DetNbr)  { return gPaddlePosition[DetNbr];}	//!

     void BuildHits(TDetectorData *data =0,Option_t *opt = "");           //!
     void FillData(TFragment*,TChannel*,MNEMONIC*);                           //!

     TSceptar& operator=(const TSceptar&);  //!

   private: 
     TSceptarData *sceptardata;                                               //!  Used to build SCEPTAR Hits
     std::vector <TSceptarHit> sceptar_hits;                                  //   The set of sceptar hits
      
     static bool fSetWave;		                                                //  Flag for Waveforms ON/OFF

   public:
     static bool SetWave()      { return fSetWave;  }	                        //!

   private:
     static TVector3 gPaddlePosition[21];                                     //!  Position of each Paddle

   public:         
     void Clear(Option_t *opt = "");		//!
     void Print(Option_t *opt = "") const;		//!

   protected:
     void PushBackHit(TGRSIDetectorHit* schit);

   ClassDef(TSceptar,1)  // Sceptar Physics structure


};

#endif


