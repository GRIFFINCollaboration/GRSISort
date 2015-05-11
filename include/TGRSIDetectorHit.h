#ifndef TGRSIDETECTORHIT_H
#define TGRSIDETECTORHIT_H


#include "Globals.h"

#include <cstdio>
#include <utility>
#include <vector>

//#include "TChannel.h"
#include "TVector3.h" 
#include "TObject.h" 
#include "Rtypes.h"


////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIDetectorHit                                           //
//                                                            //
// This is class that contains the basic info about detector  //
// hits. This is where the position of a detector is stored.
//                                                            //
////////////////////////////////////////////////////////////////

class TGRSIDetectorHit : public TObject 	{
	public:
		TGRSIDetectorHit();
		virtual ~TGRSIDetectorHit();

	public:
      enum EGain { kHigh, kLow };
		virtual void Clear(Option_t* opt = "");	//!
		virtual void Print(Option_t* opt = "") const;	//!
  
      //Abstract methods. These are required in all derived classes
      virtual Double_t GetEnergy(EGain gainlev = kLow) const { AbstractMethod("GetEnergy()");}
      virtual void SetHit() { AbstractMethod("SetHit()");}
      //We need a common function for all detectors in here
		//static bool Compare(TGRSIDetectorHit *lhs,TGRSIDetectorHit *rhs); //!

		//virtual TVector3 GetPosition() = 0;	//!
		//virtual void SetPosition(TGRSIDetectorHit &) = 0;	//!
      virtual TVector3 GetPosition(Double_t pos_param) const {   return fposition; }
      virtual inline UInt_t   GetAddress(EGain gainlev = kLow) const {   return faddress;  } //!

      inline void SetAddress(const UInt_t &x)      { faddress = x; } //!
      void SetPosition(const TVector3 &pos)        { fposition = pos; } //! 

      virtual Bool_t BremSuppressed(TGRSIDetectorHit*);
      Bool_t IsHitSet() const { return fHitSet; }

   protected:
      UInt_t faddress;
      TVector3 fposition; //!
      Bool_t fHitSet;    //!
      Bool_t fDetectorSet;//!
      Bool_t fPosSet;//!
      Bool_t fEnergySet;//!

	ClassDef(TGRSIDetectorHit,1) //Stores the information for a detector hit
};




#endif
