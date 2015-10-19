#include "TTip.h"
#include "TTipHit.h"

////////////////////////////////////////////////////////////
//                    
// TTipHit
//
// The TTipHit class defines the observables and algorithms used
// when analyzing TIP hits. It includes detector positions, etc. 
//
////////////////////////////////////////////////////////////

ClassImp(TTipHit)

TTipHit::TTipHit() {
   Class()->IgnoreTObjectStreamer(true);
   Clear();
}

TTipHit::~TTipHit() { }

TTipHit::TTipHit(const TTipHit &rhs) : TGRSIDetectorHit() {
   Class()->IgnoreTObjectStreamer(kTRUE);
   Clear();
   rhs.Copy(*this);
}

void TTipHit::Copy(TObject &rhs) const {
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TTipHit&>(rhs).filter  = filter;
   static_cast<TTipHit&>(rhs).fPID     = fPID;
}                                       

bool TTipHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TTipHit::Clear(Option_t *opt) {
   filter = 0;
   fPID   = 0;
   //position.SetXYZ(0,0,1);

  // waveform.clear();
}

void TTipHit::Print(Option_t *opt) const {
   printf("Tip Detector: %i\n",GetDetector());
   printf("Tip hit energy: %.2f\n",GetEnergy());
   printf("Tip hit time:   %.f\n",GetTime());
}

