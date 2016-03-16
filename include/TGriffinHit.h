#ifndef GRIFFINHIT_H
#define GRIFFINHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TVirtualFragment.h"
#include "TChannel.h"

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
      Int_t fFilter;              //  The Filter Word
		UChar_t fGriffinHitBits;    //  Transient Member Flags
      UInt_t fCrystal;            //!<! Crystal Number       
      Bool_t fBremSuppressed_flag;//!<! Bremsstrahlung Suppression flag.

	public:
		/////////////////////////  Setters	/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { fFilter = x;   }                  //!<! 

		/////////////////////////  Getters	/////////////////////////////////////
      inline Int_t    GetFilterPattern() const         {   return fFilter;   }          //!<!

      /////////////////////////  Recommended Functions/////////////////////////



		/////////////////////////  TChannel Helpers /////////////////////////////////////
      UInt_t GetCrystal()  const;//!<!
      UInt_t GetCrystal();
      UInt_t SetCrystal(char color);
      UInt_t SetCrystal(UInt_t crynum);
      Bool_t IsCrystalSet() const {return IsSubDetSet();}

      UShort_t NPileUps() const; 
      UShort_t PUHit() const;    
      void SetNPileUps(UChar_t npileups);
      void SetPUHit(UChar_t puhit);
      
		/////////////////////////		/////////////////////////////////////

      inline UShort_t GetArrayNumber() { return( 4*(GetDetector()-1)+(GetCrystal()+1)); } //!<!
      // returns a number 1-64 ( 1 = Detector 1 blue;  64 =  Detector 16 white; ) 
      Bool_t GetIsBremSuppressed() const { return fBremSuppressed_flag; }
      void SetIsBremSuppressed(const Bool_t &supp_flag) { fBremSuppressed_flag = supp_flag; }
      void MakeBremSuppressed(const Bool_t &supp_flag) { fBremSuppressed_flag |= supp_flag; }

      bool   InFilter(Int_t);  //!<!

      static bool CompareEnergy(const TGriffinHit*, const TGriffinHit*);  //!<!
      void Add(const TGriffinHit*);    //!<! 
      //Bool_t BremSuppressed(TSceptarHit*);

	public:
		virtual void Clear(Option_t *opt = "");		 //!<!
		virtual void Print(Option_t *opt = "") const; //!<!
      virtual void Copy(TObject&) const;        //!<!

   private:
      TVector3 GetChannelPosition(Double_t dist = 110.0) const; //!<!
      void SetGriffinFlag(enum EGriffinHitBits,Bool_t set);
	
/// \cond CLASSIMP
      ClassDef(TGriffinHit,5); //Information about a GRIFFIN Hit
/// \endcond
};
/*! @} */
#endif
