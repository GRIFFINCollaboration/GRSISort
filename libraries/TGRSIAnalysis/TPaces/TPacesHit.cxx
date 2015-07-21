
#include "TPaces.h"
#include "TPacesHit.h"
#include "Globals.h"

ClassImp(TPacesHit)

TPacesHit::TPacesHit():TGRSIDetectorHit()	{	
   Class()->IgnoreTObjectStreamer(kTRUE);
	Clear();
}

TPacesHit::TPacesHit(const TPacesHit &rhs)	{	
   Class()->IgnoreTObjectStreamer(kTRUE);
	Clear();
   ((TPacesHit&)rhs).Copy(*this);
}

TPacesHit::~TPacesHit()  {	}

void TPacesHit::Copy(TPacesHit &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
  ((TPacesHit&)rhs).filter          = filter;
  ((TPacesHit&)rhs).ppg             = ppg;
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
   ppg             =  0;
   detector        = 0xFFFF;

   is_crys_set     = false;

   //I think we want to make sure the entire Hit is cleared including the BASE.
   TGRSIDetectorHit::Clear();
}


void TPacesHit::Print(Option_t *opt) const	{
   printf("Paces Detector: %i\n",GetDetector());
   printf("Paces Energy:   %lf\n",GetEnergy());
	printf("Paces hit time:   %f\n",GetTime());
}

double TPacesHit::GetTime(Option_t *opt) const {
  //still need to figure out how to handle the times
  return time;
}

TVector3 TPacesHit::GetPosition(Double_t dist) const{
	return TPaces::GetPosition(GetDetector());
}
