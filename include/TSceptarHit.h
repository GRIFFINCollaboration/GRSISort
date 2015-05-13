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
      inline void SetEnergy(const Double_t &x)     { energy   = x; }   //!
      inline void SetTime(const Long_t &x)         { time     = x; }   //!
   //   inline void SetPosition(TVector3 x)          { position = x; }   //!

      inline void SetWaveform(std::vector<Short_t> x) { waveform = x; } //!

		/////////////////////////		/////////////////////////////////////
      inline Int_t    GetFiterPatter()    const     {   return filter;   }  //!
		inline Long_t   GetTime()			   const     {	return time;     }  //!
      TVector3 GetPosition(Double_t dist); //!

      inline std::vector<Short_t> GetWaveform() { return waveform; }  //!

      bool   InFilter(Int_t);                                          //!

      static bool CompareEnergy(TSceptarHit*,TSceptarHit*);            //!
      void Add(TSceptarHit*);                                          //!

	public:
		void Clear(Option_t *opt = "");		                    //!
		void Print(Option_t *opt = "") const;		                    //!
      virtual void Copy(TSceptarHit&) const;        //!

   protected:
      TVector3 GetPosition(double param = 0) const;

	ClassDef(TSceptarHit,1)
};




#endif
