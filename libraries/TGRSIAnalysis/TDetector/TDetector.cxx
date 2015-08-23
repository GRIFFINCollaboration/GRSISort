

#include "TDetector.h"
#include <TClass.h>

ClassImp(TDetector)

////////////////////////////////////////////////////////////////
//                                                            //
// TDetector                                                  //
//                                                            //
// This is an abstract class that contains the basic info     //
// about a detector. This is where the hits are built and
// the data is filled. It's main role is to act as a wrapper
// For every other type of detector system.
//                                                            //
////////////////////////////////////////////////////////////////

TDetector::TDetector():TObject(){
   //Default constructor.
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TDetector::TDetector(const TDetector& rhs) {
   //Default Copy constructor.
  //Class()->IgnoreTObjectStreamer(kTRUE);
   ((TDetector&)rhs).Copy(*this);
}

TDetector::~TDetector()	{
//Default Destructor.
}

void TDetector::Copy(TObject &rhs) const {
  //if(!rhs.InheritsFrom("TDetector"))
  //   return;
  TObject::Copy((TObject&)rhs);
  
}


void TDetector::Print(Option_t *opt) const {
// Default print statement for TDetector. Currently does
// nothing
}

void TDetector::Clear(Option_t *opt) {
// Default clear statement for TDetector. Currently does
// nothing
}



