
#include "TGRSIDetectorHit.h"
#include <TClass.h>

ClassImp(TGRSIDetectorHit)

////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIDetectorHit                                           //
//                                                            //
// This is class that contains the basic info about detector  //
// hits. This is where the position of a detector is stored.
//                                                            //
////////////////////////////////////////////////////////////////

TGRSIDetectorHit::TGRSIDetectorHit() : faddress(0xFFFFFFFF), fposition(0,0,1), fHitSet(false)	{ 
//Default constructor

  /* Class()->IgnoreTObjectStreamer(true); */
}

TGRSIDetectorHit::~TGRSIDetectorHit()	{
//Default destructor
}

Bool_t TGRSIDetectorHit::BremSuppressed(TGRSIDetectorHit *bremorigin){
   //Checks to see if the hit is bremsstrahlung suppressed or not.
   //Currently ALWAYS returns false.
   return false;
}

void TGRSIDetectorHit::Print(Option_t *opt) const {
//General print statement for a TGRSIDetectorHit.
//Currently prints nothing.
}

void TGRSIDetectorHit::Clear(Option_t *opt) {
   faddress = 0xFFFFFFFF;
   fposition.SetXYZ(0.0,0.0,1.0);
//General clear statement for a TGRSIDetectorHit.
//Currently does nothing.
}

