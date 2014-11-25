
#include "TPaces.h"
#include "TPacesHit.h"

ClassImp(TPacesHit)

TPacesHit::TPacesHit()	{	
   //Class()->IgnoreTObjectStreamer(true);
	Clear();
}

TPacesHit::~TPacesHit()	{	}


bool TPacesHit::InFilter(Int_t wantedfilter) {
 // check if the desired filter is in wanted filter;
 // return the answer;
 return true;
}


void TPacesHit::Clear(Option_t *opt)	{

   address = 0xffffffff;
   
   crystal  = 6;

   charge_lowgain = -1;
   charge_highgain = -1;

   filter = 0;
   ppg = 0;

   cfd    = -1;
   energy_lowgain = 0.0;
   energy_highgain = 0.0;
   time   = 0;

   waveform.clear();
}

void TPacesHit::Print(Option_t *opt) const	{

   printf("Paces Detector/Crystal: %i\n",crystal);
	printf("Paces hit energy: %.2f\n",GetEnergyLow()); //This will have to be changed to check for high/low
	printf("Paces hit time:   %ld\n",GetTime());

}


bool TPacesHit::CompareEnergy(TPacesHit *lhs, TPacesHit *rhs)	{
		return(lhs->GetEnergyLow()) > rhs->GetEnergyLow();
}


void TPacesHit::Add(TPacesHit *hit)	{
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->time   = hit->GetTime();
    //  this->position = hit->GetPosition();
   }
   this->SetChargeLow(0);
   this->SetChargeHigh(0);

   this->SetEnergyHigh(this->GetEnergyHigh() + hit->GetEnergyHigh());
   this->SetEnergyLow(this->GetEnergyLow() + hit->GetEnergyLow());
}
