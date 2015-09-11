#ifndef GRIFFINHIT_H
#define GRIFFINHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
//#include "TCrystalHit.h"

#include "TVector3.h"

//#include "TSceptarHit.h"

#include "TGRSIDetectorHit.h"


class TGriffinHit : public TGRSIDetectorHit {
	public:
		enum EGriffinHitBits {
			kCrystalSet = 1<<0,
			kBit1       = 1<<1,
			kBit2       = 1<<2,
			kBit3       = 1<<3,
			kBit4       = 1<<4,
			kBit5       = 1<<5,
			kBit6       = 1<<6,
			kBit7       = 1<<7
		};

	public:
		TGriffinHit();
		TGriffinHit(const TGriffinHit&);
		virtual ~TGriffinHit();

	private:
      Int_t fFilter;             //  The Filter Word
		UChar_t fGriffinHitBits;   //  Transient Member Flags
      UInt_t fCrystal;           //! Crystal Number       

	public:
		/////////////////////////  Setters	/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { fFilter = x;   }                  //! 

      TVector3 GetPosition(Double_t dist = 110.0) const; //!

		/////////////////////////  Getters	/////////////////////////////////////
      inline Int_t    GetFilterPattern() const         {   return fFilter;   }          //!

      /////////////////////////  Recommended Functions/////////////////////////



		/////////////////////////  TChannel Helpers /////////////////////////////////////
      UInt_t GetCrystal()  const;//!
      UInt_t GetCrystal();
      UInt_t SetCrystal(char color);
      UInt_t SetCrystal(UInt_t crynum);
      Bool_t IsCrystalSet() const {return IsSubDetSet();}

		/////////////////////////		/////////////////////////////////////

      inline UShort_t GetArrayNumber() { return( 4*(GetDetector()-1)+(GetCrystal()+1)); } //!
      // returns a number 1-64 ( 1 = Detector 1 blue;  64 =  Detector 16 white; ) 

      bool   InFilter(Int_t);  //!

      static bool CompareEnergy(const TGriffinHit*, const TGriffinHit*);  //!
      void Add(const TGriffinHit*);    //! 
      //Bool_t BremSuppressed(TSceptarHit*);

	public:
		virtual void Clear(Option_t *opt = "");		 //!
		virtual void Print(Option_t *opt = "") const; //!
      virtual void Copy(TGriffinHit&) const;        //!

	ClassDef(TGriffinHit,3); //Information about a GRIFFIN Hit
};




#endif
