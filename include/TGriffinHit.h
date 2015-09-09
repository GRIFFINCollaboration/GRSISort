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
			kTotalPU1   = 1<<0,
			kTotalPU2   = 1<<1,
			kPUHit1     = 1<<2,
			kPUHit2     = 1<<3,
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

      UChar_t NPileUps() const; 
      UChar_t PUHit() const;    
      void SetNPileUps(UChar_t npileups);
      void SetPUHit(UChar_t puhit);
      
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

   private:
      void SetGriffinFlag(enum EGriffinHitBits,Bool_t set);

	ClassDef(TGriffinHit,4);
};




#endif
