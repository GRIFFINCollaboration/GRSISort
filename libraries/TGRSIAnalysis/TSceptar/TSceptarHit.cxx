#include "TSceptar.h"
#include "TSceptarHit.h"
#include "Globals.h"

ClassImp(TSceptarHit)

TSceptarHit::TSceptarHit()	{	
   //Class()->IgnoreTObjectStreamer(true);
   //Class()->AddRule("TSceptarHit waveform attributes=NotOwner");
   Clear();
}

TSceptarHit::~TSceptarHit()	{	}

TSceptarHit::TSceptarHit(const TSceptarHit &rhs)	{	
	Clear();
   ((TSceptarHit&)rhs).Copy(*this);
}

void TSceptarHit::Copy(TSceptarHit &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
}                                       

/*
void TSceptarHit::SetHit(){
   MNEMONIC mnemonic;
   TChannel *channel = TChannel::GetChannel(GetAddress());
   if(!channel){
      Error("SetHit","No TChannel exists for address %u",GetAddress());
      return;
   }
   ClearMNEMONIC(&mnemonic);
   ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
   SetDetectorNumber(mnemonic.arrayposition);
   fDetectorSet = true;
   SetEnergy(channel->CalibrateENG(GetCharge()));
   fEnergySet = true;
   SetPosition(TSceptar::GetPosition(GetDetectorNumber()));
   fPosSet = true;
   fHitSet = true;

}
*/

TVector3 TSceptarHit::GetPosition(double dist) {
	return TSceptar::GetPosition(detector);
}


bool TSceptarHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TSceptarHit::Clear(Option_t *opt)	{
	detector = 0xFFFF;
   filter = 0;
   TGRSIDetectorHit::Clear();

   //position.SetXYZ(0,0,1);

  // waveform.clear();
}

void TSceptarHit::Print(Option_t *opt) const	{
   printf("Sceptar Detector: %i\n",detector);
	printf("Sceptar hit energy: %.2f\n",GetEnergy());
	printf("Sceptar hit time:   %.ld\n",GetTime());
}


bool TSceptarHit::CompareEnergy(TSceptarHit *lhs, TSceptarHit *rhs)	{
   return(lhs->GetEnergy()) > rhs->GetEnergy();
}


void TSceptarHit::Add(TSceptarHit *hit)	{
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->time   = hit->GetTime();
      //this->position = hit->GetPosition();
   }
   this->SetCharge(0);

   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
}
/*
Double_t TSceptarHit::GetEnergy() const {
   if(fEnergySet)
      return energy;
   else{
      TChannel* channel = TChannel::GetChannel(GetAddress());
      if(!channel){
         Error("GetEnergy()","No TChannel exists for address %u",GetAddress());
         return 0.0;
      }
      return channel->CalibrateENG(GetCharge()); 
   }
}
*/


