
#include "TGriffin.h"
#include "TGriffinHit.h"
#include "Globals.h"

ClassImp(TGriffinHit)

// GRIFFIN detector 1 (0 index) has two sceptar suppressors
  // SCEPTAR detectors 0 and 1. The detector numbers for those paddles are 1 and 2 respectively
/*unsigned int TGriffinHit::GriffinSceptarSuppressors_det[16][4];

TGriffinHit::GriffinSceptarSuppressors_det[0][0] = 0;
TGriffinHit::GriffinSceptarSuppressors_det[0][1] = 1;
TGriffinHit::GriffinSceptarSuppressors_det[0][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[0][3] = 9999;
// Next GRIFFIN detector
TGriffinHit::GriffinSceptarSuppressors_det[1][0] = 2;
TGriffinHit::GriffinSceptarSuppressors_det[1][1] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[1][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[1][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[2][0] = 3;
TGriffinHit::GriffinSceptarSuppressors_det[2][1] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[2][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[2][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[3][0] = 0;
TGriffinHit::GriffinSceptarSuppressors_det[3][1] = 4;
TGriffinHit::GriffinSceptarSuppressors_det[3][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[3][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[4][0] = 5;
TGriffinHit::GriffinSceptarSuppressors_det[4][1] = 10;
TGriffinHit::GriffinSceptarSuppressors_det[4][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[4][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[5][0] = 5;
TGriffinHit::GriffinSceptarSuppressors_det[5][1] = 10;
TGriffinHit::GriffinSceptarSuppressors_det[5][2] = 6;
TGriffinHit::GriffinSceptarSuppressors_det[5][3] = 11;
TGriffinHit::GriffinSceptarSuppressors_det[6][0] = 6;
TGriffinHit::GriffinSceptarSuppressors_det[6][1] = 11;
TGriffinHit::GriffinSceptarSuppressors_det[6][2] = 7;
TGriffinHit::GriffinSceptarSuppressors_det[6][3] = 12;
TGriffinHit::GriffinSceptarSuppressors_det[7][0] = 7;
TGriffinHit::GriffinSceptarSuppressors_det[7][1] = 12;
TGriffinHit::GriffinSceptarSuppressors_det[7][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[7][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[8][0] = 7;
TGriffinHit::GriffinSceptarSuppressors_det[8][1] = 12;
TGriffinHit::GriffinSceptarSuppressors_det[8][2] = 8;
TGriffinHit::GriffinSceptarSuppressors_det[8][3] = 13;
TGriffinHit::GriffinSceptarSuppressors_det[9][0] = 8;
TGriffinHit::GriffinSceptarSuppressors_det[9][1] = 13;
TGriffinHit::GriffinSceptarSuppressors_det[9][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[9][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[10][0] = 9;
TGriffinHit::GriffinSceptarSuppressors_det[10][1] = 14;
TGriffinHit::GriffinSceptarSuppressors_det[10][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[10][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[11][0] = 9;
TGriffinHit::GriffinSceptarSuppressors_det[11][1] = 14;
TGriffinHit::GriffinSceptarSuppressors_det[11][2] = 5;
TGriffinHit::GriffinSceptarSuppressors_det[11][3] = 10;
TGriffinHit::GriffinSceptarSuppressors_det[12][0] = 16;
TGriffinHit::GriffinSceptarSuppressors_det[12][1] = 15;
TGriffinHit::GriffinSceptarSuppressors_det[12][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[12][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[13][0] = 17;
TGriffinHit::GriffinSceptarSuppressors_det[13][1] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[13][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[13][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[14][0] = 18;
TGriffinHit::GriffinSceptarSuppressors_det[14][1] = 19;
TGriffinHit::GriffinSceptarSuppressors_det[14][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[14][3] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[15][0] = 18;
TGriffinHit::GriffinSceptarSuppressors_det[15][1] = 17;
TGriffinHit::GriffinSceptarSuppressors_det[15][2] = 9999;
TGriffinHit::GriffinSceptarSuppressors_det[15][3] = 9999;
*/
TGriffinHit::TGriffinHit()	{	
   //Class()->IgnoreTObjectStreamer(true);
	Clear();
}

TGriffinHit::~TGriffinHit()	{	}


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
/*   channel = TChannel(GetAddress(kHigh));
   if(!channel)
      return;
   //Check to see if the mnemonic is consistant
   if(GetDetectorNumber() != mnomnic.arraposition){
      Error("SetHit()","Low and High gain mnemonics do not agree!");
   }
   SetEnergyHigh(channel);
*/
}

bool TGriffinHit::InFilter(Int_t wantedfilter) {
 // check if the desired filter is in wanted filter;
 // return the answer;
 return true;
}


void TGriffinHit::Clear(Option_t *opt)	{

   address_high = 0xFFFF;
	detector = 0;
   
   crystal  = 5;

   charge_lowgain = -1;
   charge_highgain = -1;

   filter = 0;
   ppg = 0;

   cfd    = -1;
   energy_lowgain = 0.0;
   energy_highgain = 0.0;
   time   = 0;

   //position.SetXYZ(0,0,1);
/*
	for(int x=0;x<bgo.size();x++)	{
		bgo[x].Clear();
	}
	bgo.clear();
*/
   waveform.clear();
   fHitSet = false;
   fDetectorSet = false;
   fPosSet = false;
   fEnergySet = false;

}

TVector3 TGriffinHit::GetPosition(Double_t radial_pos) const {
   if(fPosSet)
      return fposition;

   return TGriffin::GetPosition(detector,crystal,radial_pos);
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
     // this->position = hit->GetPosition();
   }
   this->SetChargeLow(0);
   this->SetChargeHigh(0);

   this->SetEnergyHigh(this->GetEnergyHigh() + hit->GetEnergyHigh());
   this->SetEnergyLow(this->GetEnergyLow() + hit->GetEnergyLow());
}

Double_t TGriffinHit::GetEnergy(EGain gainlev) const { 
   if(!fEnergySet){
      TChannel *channel = TChannel::GetChannel(GetAddress(gainlev));
      if(!channel){
         Error("GetEnergy(EGain)","No TChannel set for address %u",GetAddress(gainlev));
         return 0.0;
      }

      return channel->CalibrateENG(GetCharge(gainlev));
   }
   else{
      switch(gainlev){
         case kLow:
            return GetEnergyLow();
         case kHigh:
            return GetEnergyHigh();
         default:
            Error("GetEnergy(EGain)","Gain level does not exists");
            return 0.0;
      };
   }
}

Int_t TGriffinHit::GetCharge(EGain gainlev) const {
   switch(gainlev){
      case kLow:
         return GetChargeLow();
      case kHigh:
         return GetChargeHigh();
      default:
         Error("GetCharge(EGain)","Gain level does not exists");
         return 0.0;
      };

}
/*Bool_t TGriffinHit::BremSuppressed(TSceptarHit* schit){
 

   return false;
}
*/


