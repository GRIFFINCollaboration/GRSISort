
#include "TGriffin.h"
#include "TGriffinHit.h"

ClassImp(TGriffinHit)

TGriffinHit::TGriffinHit():TGRSIDetectorHit()	{	
	Clear();
}

TGriffinHit::~TGriffinHit()  {	}

void TGriffinHit::Clear(Option_t *opt)	{
   TGRSIDetectorHit::Clear(opt);    // clears the base (address, position and waveform)
   charge_lowgain  = -1;
   charge_highgain = -1;
   filter          =  0;
   ppg             =  0;
   cfd             = -1;
   time            = -1;
}

void TGriffinHit::Print(Option_t *opt) const	{
   //printf("Griffin Detector: %i\n",detector);
	//printf("Griffin Crystal:  %i\n",crystal);
	//printf("Griffin hit energy: %.2f\n",GetEnergyLow());
	//printf("Griffin hit time:   %ld\n",GetTime());
   //printf("Griffin hit TV3 theta: %.2f\tphi%.2f\n",position.Theta() *180/(3.141597),position.Phi() *180/(3.141597));
}

double TGriffinHit::GetEnergy() { 
  TChannel *chan = GetChannel();
  if(chan && charge_lowgain>0)
    return 0.0;   
  return GetChannel()->CalibrateENG(charge_lowgain); 
}

double TGriffinHit::GetTime() {
  //still need to figure out how to handle the times
  return time;
}

void TGriffinHit::SetPosition(double dist) {
	TGRSIDetectorHit::SetPosition(TGriffin::GetPosition(GetDetector(),GetCrystal(),dist));
}

Int_t TGriffinHit::GetDetector() { 
  TChannel *chan = GetChannel();
  if(!chan)
     return -1;
  MNEMONIC mnemonic;
  ParseMNEMONIC(chan->GetChannelName(),&mnemonic);
  return mnemonic.arrayposition;
}

Int_t TGriffinHit::GetCrystal() { 
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



