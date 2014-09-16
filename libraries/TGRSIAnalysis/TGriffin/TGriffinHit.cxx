
#include "TGriffin.h"
#include "TGriffinHit.h"

ClassImp(TGriffinHit)

TGriffinHit::TGriffinHit()	{	
   Class()->IgnoreTObjectStreamer(true);
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
	crystal  = 5;

   charge = -1;
   cfd    = -1;
   energy = 0.0;
   time   = 0.0;

   position.SetXYZ(0,0,1);

	for(int x=0;x<bgo.size();x++)	{
		bgo[x].Clear();
	}
	bgo.clear();
}

void TGriffinHit::SetPosition(double dist) {
	position = TGriffin::GetPosition(detector,crystal,dist);
}


void TGriffinHit::Print(Option_t *opt)	{
	printf("Griffin hit energy: %.2f\n",GetEnergy());
	printf("Griffin hit time:   %.2f\n",GetTime());
	//printf("Griffin hit TV3 theta: %.2f\tphi%.2f\n",position.Theta() *180/(3.141597),position.Phi() *180/(3.141597));
}


bool TGriffinHit::CompareEnergy(TGriffinHit *lhs, TGriffinHit *rhs)	{
		return(lhs->GetEnergy()) > rhs->GetEnergy();
}


void TGriffinHit::Add(TGriffinHit *hit)	{
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->time   = hit->GetTime();
      this->charge = hit->GetCharge();
      this->position = hit->GetPosition();
   }
   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
}





