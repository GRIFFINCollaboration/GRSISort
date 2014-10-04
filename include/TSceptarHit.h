#ifndef SCEPTARHIT_H
#define SCEPTARHIT_H

#include <cstdio>
#include <cmath>

#include <TFragment.h>
#include <TChannel.h>
#include <TCrystalHit.h>

#include <TVector3.h>


#include <TGRSIDetectorHit.h>


class TSceptarHit : public TGRSIDetectorHit {
  public:
    TSceptarHit();
    ~TSceptarHit();

  private:
    UShort_t detector;
    UInt_t   address;
    Int_t    filter;
    Int_t    charge;
    Int_t    cfd;
    Double_t energy;
    Long_t   time;
   
    TVector3 position;

    //std::vector<Short_t> waveform; //!
   
  public:
		/////////////////////////		/////////////////////////////////////
      inline void SetDetectorNumber(const int &x)  { detector = x; }   //!
      inline void SetAddress(const UInt_t &x)      { address  = x; }   //!
      inline void SetFilterPattern(const int &x)   { filter   = x; }   //! 
      inline void SetCharge(const int &x)          { charge   = x; }   //!
      inline void SetCfd(const int &x)             { cfd      = x; }   //!
      inline void SetEnergy(const Double_t &x)     { energy   = x; }   //!
      inline void SetTime(const Long_t &x)         { time     = x; }   //!
      inline void SetPosition(TVector3 x)          { position = x; }   //!

//      inline void SetWaveform(std::vector<Short_t> x) { waveform = x; } //!

		/////////////////////////		/////////////////////////////////////
		inline UShort_t GetDetectorNumber()	     {	return detector; }  //!
      inline UInt_t   GetAddress()             {   return address;  }  //!
      inline Int_t    GetFiterPatter()         {   return filter;   }  //!
		inline Int_t    GetCharge()			     {	return charge;	  }  //!
      inline Int_t    GetCfd()                 {   return cfd;      }  //!
      inline Double_t GetEnergy()	   	     {	return energy;   }  //!
		inline Long_t   GetTime()			        {	return time;     }  //!
		inline TVector3 GetPosition()	           {	return position; }  //!

//      inline std::vector<Short_t> GetWaveform() { return waveform; }  //!

      bool   InFilter(Int_t);                                          //!

      static bool CompareEnergy(TSceptarHit*,TSceptarHit*);            //!
      void Add(TSceptarHit*);                                          //!

	public:
		void Clear(Option_t *opt = "");		                    //!
		void Print(Option_t *opt = "");		                    //!

	ClassDef(TSceptarHit,1)
};




#endif
