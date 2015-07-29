
#include "TGriffin.h"
#include "TGriffinHit.h"
#include "Globals.h"

ClassImp(TGriffinHit)

TGriffinHit::TGriffinHit():TGRSIDetectorHit()	{	
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
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
   crystal         = 0xFFFF;

   is_crys_set     = false;

}


void TGriffinHit::Print(Option_t *opt) const	{
   printf("Griffin Detector: %i\n",GetDetector());
	printf("Griffin Crystal:  %i\n",GetCrystal());
   printf("Griffin Energy:   %lf\n",GetEnergy());
	printf("Griffin hit time:   %lf\n",GetTime());
   printf("Griffin hit TV3 theta: %.2f\tphi%.2f\n",GetPosition().Theta() *180/(3.141597),GetPosition().Phi() *180/(3.141597));
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

TVector3 TGriffinHit::GetPosition(Double_t dist) const{
	return TGriffin::GetPosition(GetDetector(),GetCrystal(),dist);
}

UInt_t TGriffinHit::GetCrystal() const { 
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

UInt_t TGriffinHit::GetCrystal() {
   if(is_crys_set)
      return crystal;

   TChannel *chan = GetChannel();
   if(!chan)
      return -1;
   MNEMONIC mnemonic;
   ParseMNEMONIC(chan->GetChannelName(),&mnemonic);
   char color = mnemonic.arraysubposition[0];
   return SetCrystal(color);
}

UInt_t TGriffinHit::SetCrystal(UInt_t crynum) {
   crystal = crynum;
   is_crys_set = true;
   return crystal;
}

UInt_t TGriffinHit::SetCrystal(char color) { 
   switch(color) {
      case 'B':
         crystal = 0;
      case 'G':
         crystal = 1;
      case 'R':
         crystal = 2;
      case 'W':
         crystal = 3;  
   };
   is_crys_set = true;
   return crystal;
}

bool TGriffinHit::CompareEnergy(const TGriffinHit *lhs, const TGriffinHit *rhs)	{
   return(lhs->GetEnergy()) > rhs->GetEnergy();
}

void TGriffinHit::Add(const TGriffinHit *hit)	{
   // add another griffin hit to this one (for addback), 
   // using the time and position information of the one with the higher energy
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();
      this->time   = hit->GetTime();
      this->position = hit->GetPosition();
      this->address = hit->GetAddress();
   }

   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
   //this has to be done at the very end, otherwise this->GetEnergy() might not work
   this->SetCharge(0);
}

//Bool_t TGriffinHit::BremSuppressed(TSceptarHit* schit){
//   return false;
//}




