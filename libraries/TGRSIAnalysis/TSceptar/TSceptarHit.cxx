#include "TSceptar.h"
#include "TSceptarHit.h"

ClassImp(TSceptarHit)

TSceptarHit::TSceptarHit()	{	
   Class()->IgnoreTObjectStreamer(true);
	Clear();
}

TSceptarHit::~TSceptarHit()	{	}

bool TSceptarHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TSceptarHit::Clear(Option_t *opt)	{
	detector = 0;
   address = 0xffffffff;
   filter = 0;
   charge = -1;
   cfd    = -1;
   energy = 0.0;
   time   = 0;

   position.SetXYZ(0,0,1);

   waveform.clear();
}

void TSceptarHit::Print(Option_t *opt)	{
   printf("Sceptar Detector: %i\n",detector);
	printf("Sceptar hit energy: %.2f\n",GetEnergy());
	printf("Sceptar hit time:   %.lld\n",GetTime());
}


bool TSceptarHit::CompareEnergy(TSceptarHit *lhs, TSceptarHit *rhs)	{
   return(lhs->GetEnergy()) > rhs->GetEnergy();
}


void TSceptarHit::Add(TSceptarHit *hit)	{
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->time   = hit->GetTime();
      this->position = hit->GetPosition();
   }
   this->SetCharge(0);

   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
}





