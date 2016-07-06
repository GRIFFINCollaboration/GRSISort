#include "TLaBrHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TLaBr.h"

/// \cond CLASSIMP
ClassImp(TLaBrHit)
/// \endcond

TLaBrHit::TLaBrHit()	{
   //Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TLaBrHit::~TLaBrHit()	{	}

TLaBrHit::TLaBrHit(const TLaBrHit &rhs) : TGRSIDetectorHit() {
   //Copy Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

void TLaBrHit::Copy(TObject &rhs) const {
   //Copies a TLaBrHit
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TLaBrHit&>(rhs).fFilter = fFilter;
}

TVector3 TLaBrHit::GetChannelPosition(double dist) const {
   //Gets the position of the current TLaBrHit
   //This should not be called externally, only TGRSIDetector::GetPosition should be
   return TLaBr::GetPosition(GetDetector());
}

bool TLaBrHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   //currently does nothing
   return true;
}

void TLaBrHit::Clear(Option_t *opt)	{
   //Clears the LaBrHit
   fFilter = 0;
   TGRSIDetectorHit::Clear();
}

void TLaBrHit::Print(Option_t *opt) const	{
   //Prints the LaBrHit. Returns:
   //Detector
   //Energy
   //Time
   printf("LaBr Detector: %i\n",GetDetector());
   printf("LaBr hit energy: %.2f\n",GetEnergy());
   printf("LaBr hit time:   %.lf\n",GetTime());
}
