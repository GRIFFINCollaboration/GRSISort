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

  private:
    UShort_t detector;//!
    Int_t    filter;
    Int_t    charge;
    Int_t    cfd;
    Double_t energy;//!
    Long_t   time;
   
    std::vector<Short_t> waveform; //!
   
  public:

   void SetHit();
		/////////////////////////		/////////////////////////////////////
      inline void SetDetectorNumber(const int &x)  { detector = x; }   //!
      inline void SetFilterPattern(const int &x)   { filter   = x; }   //! 
      inline void SetCharge(const int &x)          { charge   = x; }   //!
      inline void SetCfd(const int &x)             { cfd      = x; }   //!
      inline void SetEnergy(const Double_t &x)     { energy   = x; }   //!
      inline void SetTime(const Long_t &x)         { time     = x; }   //!
   //   inline void SetPosition(TVector3 x)          { position = x; }   //!

      inline void SetWaveform(std::vector<Short_t> x) { waveform = x; } //!

		/////////////////////////		/////////////////////////////////////
      TVector3 GetPosition() const;
		inline UShort_t GetDetectorNumber()	const     {	return detector; }  //!
      inline Int_t    GetFiterPatter()    const     {   return filter;   }  //!
		inline Int_t    GetCharge()			const     {	return charge;	  }  //!
      inline Int_t    GetCfd()            const     {   return cfd;      }  //!
      Double_t GetEnergy() const;  //!
		inline Long_t   GetTime()			   const     {	return time;     }  //!

      inline std::vector<Short_t> GetWaveform() { return waveform; }  //!

      bool   InFilter(Int_t);                                          //!

      static bool CompareEnergy(TSceptarHit*,TSceptarHit*);            //!
      void Add(TSceptarHit*);                                          //!

	public:
		void Clear(Option_t *opt = "");		                    //!
		void Print(Option_t *opt = "");		                    //!

	ClassDef(TSceptarHit,1)
};




#endif
