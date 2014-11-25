#include "TDescant.h"
#include "TDescantHit.h"

ClassImp(TDescantHit)

TDescantHit::TDescantHit()	{	
   //Class()->IgnoreTObjectStreamer(true);
   //Class()->AddRule("TSceptarHit waveform attributes=NotOwner");
   Clear();
}

TDescantHit::~TDescantHit()	{	}

bool TDescantHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TDescantHit::Clear(Option_t *opt)	{
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

void TDescantHit::Print(Option_t *opt)	{
   printf("Descant Detector: %i\n",detector);
	printf("Descant hit energy: %.2f\n",GetEnergy());
	printf("Descant hit time:   %.ld\n",GetTime());
}


bool TDescantHit::CompareEnergy(TDescantHit *lhs, TDescantHit *rhs)	{
   return(lhs->GetEnergy()) > rhs->GetEnergy();
}


void TDescantHit::Add(TDescantHit *hit)	{
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->time   = hit->GetTime();
      this->position = hit->GetPosition();
   }
   this->SetCharge(0);

   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
}





