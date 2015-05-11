
#include "TGRSIDetectorHit.h"
#include <TClass.h>

ClassImp(TGRSIDetectorHit)

////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIDetectorHit                                           //
//                                                            //
// This is class that contains the basic info about detector  //
// hits. This is where the position of a detector is stored.  //
//                                                            //
////////////////////////////////////////////////////////////////

TGRSIDetectorHit::TGRSIDetectorHit()	{ 
  //Default constructor
  Class()->IgnoreTObjectStreamer(true);
}

TGRSIDetectorHit::~TGRSIDetectorHit()	{
//Default destructor
}

void TGRSIDetectorHit::Print(Option_t *opt) {
//General print statement for a TGRSIDetectorHit.
//Currently prints nothing.
}

void TGRSIDetectorHit::Clear(Option_t *opt) {
  //General clear statement for a TGRSIDetectorHit.
  position.SetXYZ(0,0,1);
}

