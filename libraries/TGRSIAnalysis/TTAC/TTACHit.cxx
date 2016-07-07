#include "TTACHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TTAC.h"

/// \cond CLASSIMP
ClassImp(TTACHit)
/// \endcond

TTACHit::TTACHit()	{
   //Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TTACHit::~TTACHit()	{	}

TTACHit::TTACHit(const TTACHit &rhs) : TGRSIDetectorHit() {
   //Copy Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

void TTACHit::Copy(TObject &rhs) const {
   //Copies a TTACHit
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TTACHit&>(rhs).fFilter = fFilter;
}

TVector3 TTACHit::GetChannelPosition(double dist) const {
   //There is no position for the TTAC's. This returns a null vector.
   return TVector3(0., 0., 0.); 
}

bool TTACHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   //currently does nothing
   return true;
}

void TTACHit::Clear(Option_t *opt)	{
   //Clears the TACHit
   fFilter = 0;
   TGRSIDetectorHit::Clear();
}

void TTACHit::Print(Option_t *opt) const	{
   //Prints the TACHit. Returns:
   //Detector
   //Energy
   //Time
   printf("TAC Detector: %i\n",GetDetector());
   printf("TAC hit energy: %.2f\n",GetEnergy());
   printf("TAC hit time:   %.lf\n",GetTime());
}
