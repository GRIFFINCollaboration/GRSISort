
#include "TTigressHit.h"
#include <TClass.h>

ClassImp(TTigressHit)

TTigressHit::TTigressHit()	{	
   Class()->IgnoreTObjectStreamer(true);
	Clear();
}

TTigressHit::~TTigressHit()	{	}


void TTigressHit::Clear(Option_t *opt)	{

	detector = -1;
	crystal  = -1;
	first_segment = 0;
	first_segment_charge = 0.0;

	core.Clear();
	for(int x=0;x<segment.size();x++) { 
		segment[x].Clear();
	}
	segment.clear();
	for(int x=0;x<bgo.size();x++)	{
		bgo[x].Clear();
	}
	bgo.clear();

	lasthit.SetXYZ(0,0,0);
   lastenergy = 0.0;
}


void TTigressHit::Print(Option_t *opt)	{
	printf("Tigress hit energy: %.2f\n",GetCore()->GetEnergy());
	printf("Tigress hit time:   %.2f\n",GetCore()->GetTime());
	//printf("Tigress hit TV3 theta: %.2f\tphi%.2f\n",position.Theta() *180/(3.141597),position.Phi() *180/(3.141597));
}


bool TTigressHit::Compare(TTigressHit lhs, TTigressHit rhs)	{
	if (lhs.GetDetectorNumber() == rhs.GetDetectorNumber())	{
		return(lhs.GetCrystalNumber() < rhs.GetCrystalNumber());
	}
	else	{
		return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); 
	}
}


bool TTigressHit::CompareEnergy(TTigressHit lhs, TTigressHit rhs)	{
		return(lhs.GetEnergy()) > rhs.GetEnergy();
}


void TTigressHit::CheckFirstHit(int charge,int segment)	{
	if(abs(charge) > first_segment_charge)	{
 		first_segment        = segment;
      first_segment_charge = charge;
	}
	return;				
}

void TTigressHit::Add(TTigressHit *hit)	{
	if(this == hit)	{
		lasthit    = position;
      lastenergy = GetEnergy();
		lastpos    = std::make_tuple(GetDetectorNumber(),GetCrystalNumber(),GetInitialHit());
		return;
	}
   this->core.SetEnergy(this->GetEnergy() + hit->GetEnergy());

   if(lastenergy < hit->GetEnergy()) {
     this->lastenergy = hit->GetEnergy();
     this->lasthit    = hit->GetPosition();
     this->lastpos    = std::make_tuple(hit->GetDetectorNumber(),hit->GetCrystalNumber(),hit->GetInitialHit());
   } else {
     this->GetCore()->SetCfd(hit->GetTimeCFD());    
     this->GetCore()->SetTime(hit->GetTime());
     this->SetPosition(hit->GetPosition());
   }
}



