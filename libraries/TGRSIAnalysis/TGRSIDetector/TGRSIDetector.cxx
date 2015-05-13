

#include "TGRSIDetector.h"
#include "TGRSIDetectorHit.h"
#include <TClass.h>

ClassImp(TGRSIDetector)

////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIDetector                                              //
//                                                            //
// This is an abstract class that contains the basic info     //
// about a detector. This is where the hits are built and
// the data is filled.
//                                                            //
////////////////////////////////////////////////////////////////

TGRSIDetector::TGRSIDetector():TObject() {
   //Default constructor.
}

TGRSIDetector::TGRSIDetector(const TGRSIDetector& rhs) {
   //Default Copy constructor.
   ((TGRSIDetector&)rhs).Copy(*this);
}

TGRSIDetector::~TGRSIDetector()	{
//Default Destructor.
}

void TGRSIDetector::AddHit(TGRSIDetectorHit *hit,Option_t *opt) {
  hit->SetParent(this); 
  PushBackHit(hit);
  return;
}

void TGRSIDetector::Copy(TGRSIDetector &rhs) const {
   TObject::Copy((TObject&)rhs);
}

void TGRSIDetector::Print(Option_t *opt) const {
// Default print statement for TGRSIDetector. Currently does
// nothing
}

void TGRSIDetector::Clear(Option_t *opt) {
// Default clear statement for TGRSIDetector. Currently does
// nothing
}



