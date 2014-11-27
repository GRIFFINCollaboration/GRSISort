
#include "TGriffin.h"
#include "TGriffinHit.h"

ClassImp(TGriffinHit)

TGriffinHit::TGriffinHit()	{	
   //Class()->IgnoreTObjectStreamer(true);
	Clear();
}

TGriffinHit::~TGriffinHit()	{	}


bool TGriffinHit::InFilter(Int_t wantedfilter) {
 // check if the desired filter is in wanted filter;
 // return the answer;
 return true;
}


void TGriffinHit::Clear(Option_t *opt)	{

	detector = 0;
   address = 0xffffffff;
   
   crystal  = 5;

   charge_lowgain = -1;
   charge_highgain = -1;

   filter = 0;
   ppg = 0;

   cfd    = -1;
   energy_lowgain = 0.0;
   energy_highgain = 0.0;
   time   = 0;

   position.SetXYZ(0,0,1);
/*
	for(int x=0;x<bgo.size();x++)	{
		bgo[x].Clear();
	}
	bgo.clear();
*/
   waveform.clear();
}

void TGriffinHit::SetPosition(double dist) {
	position = TGriffin::GetPosition(detector,crystal,dist);
}


void TGriffinHit::Print(Option_t *opt) const	{

   printf("Griffin Detector: %i\n",detector);
	printf("Griffin Crystal:  %i\n",crystal);
	printf("Griffin hit energy: %.2f\n",GetEnergyLow());
	printf("Griffin hit time:   %ld\n",GetTime());


   //printf("Griffin hit TV3 theta: %.2f\tphi%.2f\n",position.Theta() *180/(3.141597),position.Phi() *180/(3.141597));
}


bool TGriffinHit::CompareEnergy(TGriffinHit *lhs, TGriffinHit *rhs)	{
		return(lhs->GetEnergyLow()) > rhs->GetEnergyLow();
}


void TGriffinHit::Add(TGriffinHit *hit)	{
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->time   = hit->GetTime();
      this->position = hit->GetPosition();
   }
   this->SetChargeLow(0);
   this->SetChargeHigh(0);

   this->SetEnergyHigh(this->GetEnergyHigh() + hit->GetEnergyHigh());
   this->SetEnergyLow(this->GetEnergyLow() + hit->GetEnergyLow());
}





