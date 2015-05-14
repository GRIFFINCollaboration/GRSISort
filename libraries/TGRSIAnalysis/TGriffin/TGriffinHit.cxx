
#include "TGriffin.h"
#include "TGriffinHit.h"
#include "Globals.h"

ClassImp(TGriffinHit)

TGriffinHit::TGriffinHit():TGRSIDetectorHit()	{	
	Clear();
}

TGriffinHit::TGriffinHit(const TGriffinHit &rhs)	{	
	Clear();
   ((TGriffinHit&)rhs).Copy(*this);
}

TGriffinHit::~TGriffinHit()  {	}

void TGriffinHit::Copy(TGriffinHit &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
  ((TGriffinHit&)rhs).filter          = filter;
  ((TGriffinHit&)rhs).ppg             = ppg;
  return;                                      
}                                       

/*
void TGriffinHit::SetHit() {
   MNEMONIC mnemonic;
   TChannel *channel = TChannel::GetChannel(GetAddress(kLow));
   if(!channel){
      Error("SetHit()","No TChannel Set");
      return;
   }
      
   ClearMNEMONIC(&mnemonic);
   ParseMNEMONIC(channel->GetChannelName(),&mnemonic);

   UShort_t CoreNbr=5;
   if(mnemonic.arraysubposition.compare(0,1,"B")==0)
      CoreNbr=0;
   else if(mnemonic.arraysubposition.compare(0,1,"G")==0)
      CoreNbr=1;
   else if(mnemonic.arraysubposition.compare(0,1,"R")==0)
      CoreNbr=2;
   else if(mnemonic.arraysubposition.compare(0,1,"W")==0)
      CoreNbr=3;
   
   SetDetectorNumber(mnemonic.arrayposition);
   SetCrystalNumber(CoreNbr);
   fDetectorSet = true;

   SetEnergyLow(channel->CalibrateENG(charge_lowgain));
   fEnergySet = true;

   //Try doing the high energy.
   channel = TChannel::GetChannel(GetAddress(kHigh));
   if(channel){
      SetEnergyHigh(channel->CalibrateENG(charge_highgain));
   }

   SetPosition(GetPosition());
   fPosSet = true;

   fHitSet = true;
   //Now set the high gains
//   channel = TChannel(GetAddress(kHigh));
//   if(!channel)
//      return;
//   //Check to see if the mnemonic is consistant
//   if(GetDetectorNumber() != mnomnic.arraposition){
//      Error("SetHit()","Low and High gain mnemonics do not agree!");
//   }
//   SetEnergyHigh(channel);

}
*/

bool TGriffinHit::InFilter(Int_t wantedfilter) {
 // check if the desired filter is in wanted filter;
 // return the answer;
 return true;
}


void TGriffinHit::Clear(Option_t *opt)	{
   TGRSIDetectorHit::Clear(opt);    // clears the base (address, position and waveform)
   filter          =  0;
   ppg             =  0;
   detector        = 0xFFFF;
   crystal         = 0xFFFF;

   is_crys_set     = false;

   //I think we want to make sure the entire Hit is cleared including the BASE.
   TGRSIDetectorHit::Clear();
}


void TGriffinHit::Print(Option_t *opt) const	{
   printf("Griffin Hit\n");
   //printf("Griffin Detector: %i\n",detector);
	//printf("Griffin Crystal:  %i\n",crystal);
	//printf("Griffin hit energy: %.2f\n",GetEnergyLow());
	//printf("Griffin hit time:   %ld\n",GetTime());
   //printf("Griffin hit TV3 theta: %.2f\tphi%.2f\n",position.Theta() *180/(3.141597),position.Phi() *180/(3.141597));
}
/*
double TGriffinHit::GetEnergy(Option_t *opt) const { 
  TChannel *chan = GetChannel();
  if(!chan || (charge_lowgain<0 && charge_highgain<0) ) {
    if(!chan)
      Error("GetEnergy(%s)","No TChannel set for address %u",opt,GetAddress());
    return 0.0;   
  }  
  if(TString(opt).Contains("high",TString::ECaseCompare::kIgnoreCase)) 
    return GetChannel()->CalibrateENG(charge_highgain); 
  return GetChannel()->CalibrateENG(charge_lowgain); 
}


Int_t TGriffinHit::GetCharge(Option_t *opt) const { 
  if(TString(opt).Contains("high",TString::ECaseCompare::kIgnoreCase)) 
    return charge_highgain; 
  return charge_lowgain; 
}
*/


double TGriffinHit::GetTime(Option_t *opt) const {
  //still need to figure out how to handle the times
  return time;
}

TVector3 TGriffinHit::GetPosition(Double_t dist){
	TGRSIDetectorHit::SetPosition(TGriffin::GetPosition(GetDetector(),GetCrystal(),dist));
   return position;
}

const UInt_t TGriffinHit::GetCrystal() const { 
   if(is_crys_set)
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

UInt_t TGriffinHit::SetCrystal() { 
   crystal = GetCrystal();
   return crystal;
}

//bool TGriffinHit::CompareEnergy(TGriffinHit *lhs, TGriffinHit *rhs)	{
//		return(lhs->GetEnergyLow()) > rhs->GetEnergyLow();
//}



//void TGriffinHit::Add(TGriffinHit *hit)	{
//   if(!CompareEnergy(this,hit)) {
//      this->cfd    = hit->GetCfd();    
//      this->time   = hit->GetTime();
//      this->position = hit->GetPosition();
//   }
//   this->SetChargeLow(0);
//   this->SetChargeHigh(0);
//
//   this->SetEnergyHigh(this->GetEnergyHigh() + hit->GetEnergyHigh());
//   this->SetEnergyLow(this->GetEnergyLow() + hit->GetEnergyLow());
//}

//Bool_t TGriffinHit::BremSuppressed(TSceptarHit* schit){
//   return false;
//}




