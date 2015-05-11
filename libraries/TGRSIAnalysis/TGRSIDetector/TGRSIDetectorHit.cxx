
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

void TGRSIDetectorHit::Copy(TGRSIDetectorHit &rhs) const {
  TObject::Copy((TObject&)rhs);
  ((TGRSIDetectorHit&)rhs).address  = address;
  ((TGRSIDetectorHit&)rhs).position = position;
  ((TGRSIDetectorHit&)rhs).waveform = waveform;
}

void TGRSIDetectorHit::Print(Option_t *opt) const {
//General print statement for a TGRSIDetectorHit.
//Currently prints nothing.
}

void TGRSIDetectorHit::Clear(Option_t *opt) {
  //General clear statement for a TGRSIDetectorHit.
  address = 0xffffffff;    // -1
  position.SetXYZ(0,0,1);  // unit vector along the beam.
  wavefrom.clear();        // reset size to zero.
}

