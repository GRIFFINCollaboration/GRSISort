#ifndef SCEPTARHIT_H
#define SCEPTARHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
#include "TCrystalHit.h"

#include "TVector3.h"


#include "TGRSIDetectorHit.h"


class TSceptarHit : public TGRSIDetectorHit {
  public:
    TSceptarHit();
    ~TSceptarHit();
    TSceptarHit(const TSceptarHit&);

  private:
    Int_t    filter;
    Long_t   time;
   
    std::vector<Short_t> waveform; //!
   
  public:

  // void SetHit();
		/////////////////////////		/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { filter   = x; }   //! 
   //   inline void SetPosition(TVector3 x)          { position = x; }   //!

      inline void SetWaveform(std::vector<Short_t> x) { waveform = x; } //!

		/////////////////////////		/////////////////////////////////////
      inline Int_t    GetFilterPattern()    const     {   return filter;   }  //!
      TVector3 GetPosition(Double_t dist = 0) const; //!

      inline std::vector<Short_t> GetWaveform() { return waveform; }  //!

      bool   InFilter(Int_t);                                          //!

 //     static bool CompareEnergy(TSceptarHit*,TSceptarHit*);            //!
 //     void Add(TSceptarHit*);                                          //!

	public:
		void Clear(Option_t *opt = "");		                    //!
		void Print(Option_t *opt = "") const;		                    //!
      virtual void Copy(TSceptarHit&) const;        //!

	ClassDef(TSceptarHit,1)
};




#endif
