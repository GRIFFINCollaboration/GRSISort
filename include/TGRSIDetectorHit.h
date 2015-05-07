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
		virtual void Clear(Option_t* opt = "");	//!
		virtual void Print(Option_t* opt = "");	//!
      //We need a common function for all detectors in here
		//static bool Compare(TGRSIDetectorHit *lhs,TGRSIDetectorHit *rhs); //!

		//virtual TVector3 GetPosition() = 0;	//!
		//virtual void SetPosition(TGRSIDetectorHit &) = 0;	//!
      virtual TVector3 GetPosition() const {return position;}
      inline UInt_t   GetAddress() const             {   return address; } //!
      inline void SetAddress(const UInt_t &x)      { address = x; } //!

      virtual Bool_t BremSuppressed(TGRSIDetectorHit*);

   protected:
      UInt_t address;
      TVector3 position; //Position of hit detector

	ClassDef(TGRSIDetectorHit,1) //Stores the information for a detector hit
};




#endif
