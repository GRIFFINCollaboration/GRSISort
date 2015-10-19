
#include "TTigress.h"
#include "TTigressHit.h"
#include <TClass.h>

ClassImp(TTigressHit)

TVector3 TTigressHit::beam;

TTigressHit::TTigressHit() {	
	Clear();
   segment.SetClass("TCrystalHit");
   bgo.SetClass("TCrystalHit");
   beam.SetXYZ(0,0,1);
}

TTigressHit::~TTigressHit() {	}

TTigressHit::TTigressHit(const TTigressHit& rhs) : TGRSIDetectorHit() {	
   ((TGRSIDetectorHit&)rhs).Copy(*this);
}


void TTigressHit::AddSegment(TCrystalHit &temp) {
   TCrystalHit *newhit = (TCrystalHit*)segment.ConstructedAt(GetSegmentMultiplicity());	
   temp.Copy((TCrystalHit&)(*newhit));
}


void TTigressHit::AddBGO(TCrystalHit &temp) {
   TCrystalHit *newhit = (TCrystalHit*)bgo.ConstructedAt(GetSegmentMultiplicity());	
   temp.Copy((TCrystalHit&)(*newhit));
}



void TTigressHit::Clear(Option_t *opt) {
   TGRSIDetectorHit::Clear(opt);
	//detector = -1;
	crystal  = -1;
	first_segment = 0;
	first_segment_charge = 0.0;
	core.Clear();
	//for(int x=0;x<segment.size();x++) { 
	//	segment[x].Clear();
	//}
	segment.Clear("C");
	//for(int x=0;x<bgo.size();x++) {
	//	bgo[x].Clear();
	//}
	bgo.Clear("C");

	lasthit.SetXYZ(0,0,0);
}

void TTigressHit::Copy(TObject &rhs) const {
  TGRSIDetectorHit::Copy(rhs);
  segment.Copy(static_cast<TTigressHit&>(rhs).segment);
  bgo.Copy(static_cast<TTigressHit&>(rhs).bgo);
  (static_cast<TTigressHit&>(rhs)).crystal = crystal;
  (static_cast<TTigressHit&>(rhs)).first_segment = first_segment;
  (static_cast<TTigressHit&>(rhs)).first_segment_charge = first_segment_charge;
  lasthit.Copy(static_cast<TTigressHit&>(rhs).lasthit);
}


void TTigressHit::Print(Option_t *opt) const	{
	printf("Tigress hit energy: %.2f\n",GetEnergy());
	printf("Tigress hit time:   %.2f\n",GetTime());
	//printf("Tigress hit TV3 theta: %.2f\tphi%.2f\n",position.Theta() *180/(3.141597),position.Phi() *180/(3.141597));
}


bool TTigressHit::Compare(TTigressHit lhs, TTigressHit rhs) {
	if (lhs.GetDetector() == rhs.GetDetector()) {
		return(lhs.GetCrystal() < rhs.GetCrystal());
	}
	else {
		return (lhs.GetDetector() < rhs.GetDetector()); 
	}
}


bool TTigressHit::CompareEnergy(TTigressHit lhs, TTigressHit rhs) {
		return(lhs.GetEnergy()) > rhs.GetEnergy();
}


void TTigressHit::CheckFirstHit(int charge,int segment) {
	if(fabs(charge) > first_segment_charge) {
 		first_segment = segment;
	}
	return;				
}

void TTigressHit::SumHit(TTigressHit *hit) {
	if(this == hit) {
//		lasthit = position;
		lastpos = std::make_tuple(GetDetector(),GetCrystal(),GetInitialHit());
		return;
	}
	this->core.SetEnergy(this->GetEnergy() + hit->GetEnergy());
	this->lasthit = hit->GetPosition();
	this->lastpos = std::make_tuple(hit->GetDetector(),hit->GetCrystal(),hit->GetInitialHit());
}


TVector3 TTigressHit::GetPosition(Double_t dist) const {
   //Returns the Position of the crystal of the current Hit.
	return TTigress::GetPosition(GetDetector(),GetCrystal(),dist);
}

int TTigressHit::GetCrystal() const {
   if(IsCrystalSet())
      return crystal;

   TChannel *chan = GetChannel();
   if(!chan)
      return -1;
   MNEMONIC mnemonic;
   ParseMNEMONIC(chan->GetChannelName(),&mnemonic);
   char color = mnemonic.arraysubposition[0];
   switch(color) {
      case 'B':
         return 0;
      case 'G':
         return 1;
      case 'R':
         return 2;
      case 'W':
         return 3;  
   };
   return -1;  

}
