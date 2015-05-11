// Author: Peter C. Bender
#include "TPaces.h"
#include "TPacesHit.h"

////////////////////////////////////////////////////////////
//                    
// TPacesHit
//
// The TPacesHit class defines the individual observables from a
// single PACES crystal hit. They are put together with other TPacesHits
// to fill a TPaces.
//
////////////////////////////////////////////////////////////

ClassImp(TPacesHit)

TPacesHit::TPacesHit()	{	
   //Default constructor

   //Class()->IgnoreTObjectStreamer(true);
	Clear();
}

TPacesHit::~TPacesHit()	{
   //Default Destructor
}


bool TPacesHit::InFilter(Int_t wantedfilter) {
 // check if the desired filter is in wanted filter;
 // return the answer;
 // not implemented
 return true;
}


void TPacesHit::Clear(Option_t *opt)	{
 //Clear the TPacesHit
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
   //Print the crystal number, hit energy, and hit time of the TPacesHit.
   printf("Paces Detector/Crystal: %i\n",crystal);
	printf("Paces hit energy: %.2f\n",GetEnergyLow()); //This will have to be changed to check for high/low
	printf("Paces hit time:   %ld\n",GetTime());

}


bool TPacesHit::CompareEnergy(TPacesHit *lhs, TPacesHit *rhs)	{
   //returns true if the lhs lowgain energy is greater than the rhs
   //low gain energy
		return(lhs->GetEnergyLow()) > rhs->GetEnergyLow();
}


void TPacesHit::Add(TPacesHit *hit)	{
   //Sets the TPacesHit CFD and Timestamp to the hit values. Sets the
   //charge values to 0, and adds the energy of hit to the energy of
   //this TPacesHit
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
