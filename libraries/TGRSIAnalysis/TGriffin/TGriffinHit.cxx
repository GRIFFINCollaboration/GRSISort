
#include "TGriffin.h"
#include "TGriffinHit.h"
#include "Globals.h"
#include <cmath>
#include <iostream>

/// \cond CLASSIMP
ClassImp(TGriffinHit)
/// \endcond

TGriffinHit::TGriffinHit():TGRSIDetectorHit()	{	
   //Default Ctor. Ignores TObject Streamer in ROOT < 6.
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TGriffinHit::TGriffinHit(const TGriffinHit &rhs) : TGRSIDetectorHit() {	
   //Copy Ctor. Ignores TObject Streamer in ROOT < 6.
	Clear();
   rhs.Copy(*this);
}

TGriffinHit::~TGriffinHit()  {	}

void TGriffinHit::Copy(TObject &rhs) const {
  TGRSIDetectorHit::Copy(rhs);
  static_cast<TGriffinHit&>(rhs).fFilter                = fFilter;
  static_cast<TGriffinHit&>(rhs).fGriffinHitBits        = fGriffinHitBits;
  static_cast<TGriffinHit&>(rhs).fCrystal               = fCrystal;
  static_cast<TGriffinHit&>(rhs).fPPG                   = fPPG;
  static_cast<TGriffinHit&>(rhs).fBremSuppressed_flag   = fBremSuppressed_flag; // Bremsstrahlung Suppression flag.
  return;                                      
}                                       

bool TGriffinHit::InFilter(Int_t wantedfilter) {
 // check if the desired filter is in wanted filter;
 // return the answer;
 return true;
}


void TGriffinHit::Clear(Option_t *opt)	{
   //Clears the information stored in the TGriffinHit.
   TGRSIDetectorHit::Clear(opt);    // clears the base (address, position and waveform)
   fFilter              =  0;
   fGriffinHitBits      =  0;
   fCrystal             =  0xFFFF;
   fPPG                 =  0;
   fBremSuppressed_flag = false;

}


void TGriffinHit::Print(Option_t *opt) const	{
   //Prints the Detector Number, Crystal Number, Energy, Time and Angle.
   printf("Griffin Detector: %i\n",GetDetector());
	printf("Griffin Crystal:  %i\n",GetCrystal());
   printf("Griffin Energy:   %lf\n",GetEnergy());
	printf("Griffin hit time:   %lf\n",GetTime());
   printf("Griffin hit TV3 theta: %.2f\tphi%.2f\n",GetPosition().Theta() *180/(3.141597),GetPosition().Phi() *180/(3.141597));
}

TVector3 TGriffinHit::GetChannelPosition(Double_t dist) const{

   //Returns the Position of the crystal of the current Hit.
	return TGriffin::GetPosition(GetDetector(),GetCrystal(),dist);
}

UInt_t TGriffinHit::GetCrystal() const { 
   //Returns the Crystal Number of the Current hit.
   if(IsCrystalSet())
      return fCrystal;

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
   //Returns the Crystal Number of the Current hit.
   if(IsCrystalSet())
      return fCrystal;

   TChannel *chan = GetChannel();
   if(!chan)
      return -1;
   MNEMONIC mnemonic;
   ParseMNEMONIC(chan->GetChannelName(),&mnemonic);
   char color = mnemonic.arraysubposition[0];
   return SetCrystal(color);
}

UInt_t TGriffinHit::SetCrystal(UInt_t crynum) {
   fCrystal = crynum;
   return fCrystal;
}

UInt_t TGriffinHit::SetCrystal(char color) { 
   switch(color) {
      case 'B':
         fCrystal = 0;
         break;
      case 'G':
         fCrystal = 1;
         break;
      case 'R':
         fCrystal = 2;
         break;
      case 'W':
         fCrystal = 3;  
         break;
   };
   SetFlag(TGRSIDetectorHit::kIsSubDetSet,true);
   return fCrystal;
}

bool TGriffinHit::CompareEnergy(const TGriffinHit *lhs, const TGriffinHit *rhs)	{
   return(lhs->GetEnergy() > rhs->GetEnergy());
}

void TGriffinHit::Add(const TGriffinHit *hit)	{
   // add another griffin hit to this one (for addback), 
   // using the time and position information of the one with the higher energy
   if(!CompareEnergy(this,hit)) {
      this->SetCfd(hit->GetCfd());
      this->SetTime(hit->GetTime());
      this->SetPosition(hit->GetPosition());
      this->SetAddress(hit->GetAddress());
   }
   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
   //this has to be done at the very end, otherwise this->GetEnergy() might not work
   this->SetCharge(0);
}

void TGriffinHit::SetGriffinFlag(enum EGriffinHitBits flag,Bool_t set){
   if(set)
      fGriffinHitBits |= flag;
   else
      fGriffinHitBits &= (~flag);
}

UShort_t TGriffinHit::NPileUps() const {
   return static_cast<UShort_t>(((fGriffinHitBits & kTotalPU1) + (fGriffinHitBits & kTotalPU2)));
}

UShort_t TGriffinHit::PUHit() const { 
   return static_cast<UShort_t>(((fGriffinHitBits & kPUHit1) + (fGriffinHitBits & kPUHit2)) >> 2); 
} 

void TGriffinHit::SetNPileUps(UChar_t npileups) {
   SetGriffinFlag(kTotalPU1,(npileups & kTotalPU1));  
   SetGriffinFlag(kTotalPU2,(npileups & kTotalPU2));  
}

void TGriffinHit::SetPUHit(UChar_t puhit) {
   if(puhit > 2)
      puhit = 3;

   SetGriffinFlag(kPUHit1,(puhit << 2) & kPUHit1);  
   SetGriffinFlag(kPUHit2,(puhit << 2) & kPUHit2);  
}

Double_t TGriffinHit::GetNoCTEnergy(Option_t* opt) const{
	TChannel* chan = GetChannel();
	if(!chan) {
		Error("GetEnergy","No TChannel exists for address 0x%08x",GetAddress());
		return 0.;
	}
	return chan->CalibrateENG(GetCharge());
}

Double_t TGriffinHit::GetEnergy(Option_t* opt) const{
   if(!(TGRSIRunInfo::Get()->IsCorrectingCrossTalk())){
      return GetNoCTEnergy(opt);
   }

   return TGRSIDetectorHit::GetEnergy(opt);
}

Double_t TGriffinHit::GetEnergy(Option_t* opt){
   if(!(TGRSIRunInfo::Get()->IsCorrectingCrossTalk())){
      return GetNoCTEnergy(opt);
   }

   return TGRSIDetectorHit::GetEnergy(opt);
}
