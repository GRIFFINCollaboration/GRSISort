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

TTipHit::TTipHit()	{	
   Class()->IgnoreTObjectStreamer(true);
   Clear();
}

TTipHit::~TTipHit()	{	}

TTipHit::TTipHit(const TTipHit &rhs)	{	
   Class()->IgnoreTObjectStreamer(kTRUE);
   Clear();
   ((TTipHit&)rhs).Copy(*this);
}

void TTipHit::Copy(TTipHit &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
	((TTipHit&)rhs).filter  = filter;
	((TTipHit&)rhs).PID     = PID;
}                                       

bool TTipHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TTipHit::Clear(Option_t *opt)	{
   filter = 0;
   PID   = 0;
   //position.SetXYZ(0,0,1);

  // waveform.clear();
}

void TTipHit::Print(Option_t *opt)	{
   printf("Tip Detector: %i\n",detector);
	printf("Tip hit energy: %.2f\n",GetEnergy());
	printf("Tip hit time:   %.ld\n",GetTime());
}

