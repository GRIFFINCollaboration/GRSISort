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
   Clear();
}

TTipHit::~TTipHit()	{	}

bool TTipHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TTipHit::Clear(Option_t *opt)	{
	detector = 0;
   address = 0xffffffff;
   filter = 0;
   charge = -1;
   cfd    = -1;
   energy = 0.0;
   time   = 0;

   position.SetXYZ(0,0,1);

  // waveform.clear();
}

void TTipHit::Print(Option_t *opt)	{
   printf("Tip Detector: %i\n",detector);
	printf("Tip hit energy: %.2f\n",GetEnergy());
	printf("Tip hit time:   %.ld\n",GetTime());
}

