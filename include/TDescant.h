#ifndef TDESCANT_H
#define TDESCANT_H

#include "Globals.h"

#include <vector>
#include <cstdio>

#include <TDescantHit.h>
#ifndef __CINT__
#include <TDescantData.h>
#else
class TDescantData;
#endif
#include <TVector3.h> 

#include <TGRSIDetector.h> 


class TDescant : public TGRSIDetector {

  public:
     TDescant();
     virtual ~TDescant();

  public: 
     TDescantHit *GetDescantHit(int i)        {	return &descant_hits[i];   }	//!
     Short_t GetMultiplicity() const	       {	return descant_hits.size();}	//!

     static TVector3 GetPosition(int DetNbr)  { return gPosition[DetNbr];}	//!

     void BuildHits(TGRSIDetectorData *data =0,Option_t *opt = "");           //!
     void FillData(TFragment*,TChannel*,MNEMONIC*);                           //!

     TDescant& operator=(const TDescant&);  // 

   private: 
     TDescantData *descantdata;                                               //!  Used to build DESCANT Hits
     std::vector <TDescantHit> descant_hits;                                  //   The set of crystal hits

     static bool fSetWave;		                                                //  Flag for Waveforms ON/OFF

     bool hit_flag;                                                               //   Is there a Descant hit?

   public:
     static bool SetWave()      { return fSetWave;  }	                        //!
     bool SetHit(bool flag = true) { hit_flag = flag; }                          //!
     bool Hit()                {return hit_flag;}                                //!  

   private:
     static TVector3 gPosition[21];                                     //!  Position of each Paddle

   public:         
     void Clear(Option_t *opt = "");		//!
     void Print(Option_t *opt = "");		//!

   ClassDef(TDescant,1)  // Descant Physics structure


};

#endif


