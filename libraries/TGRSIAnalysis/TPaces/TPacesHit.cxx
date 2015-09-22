
#include "TPaces.h"
#include "TPacesHit.h"
#include "Globals.h"

ClassImp(TPacesHit)

TPacesHit::TPacesHit():TGRSIDetectorHit()	{	
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TPacesHit::TPacesHit(const TPacesHit &rhs)	{	
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
   ((TPacesHit&)rhs).Copy(*this);
}

TPacesHit::~TPacesHit()  {	}

void TPacesHit::Copy(TPacesHit &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
  ((TPacesHit&)rhs).filter          = filter;
  return;                                      
}                                       

bool TPacesHit::InFilter(Int_t wantedfilter) {
 // check if the desired filter is in wanted filter;
 // return the answer;
 return true;
}


void TPacesHit::Clear(Option_t *opt)	{
   TGRSIDetectorHit::Clear(opt);    // clears the base (address, position and waveform)
   filter          =  0;
   fdetector       =  0xFFFF;
}


void TPacesHit::Print(Option_t *opt) const	{
   printf("Paces Detector: %i\n",GetDetector());
   printf("Paces Energy:   %lf\n",GetEnergy());
	printf("Paces hit time:   %f\n",GetTime());
}

TVector3 TPacesHit::GetPosition(Double_t dist) const{
	return TPaces::GetPosition(GetDetector());
}
