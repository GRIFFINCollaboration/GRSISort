
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

TGriffinHit::TGriffinHit(const TFragment &frag) : TGRSIDetectorHit(frag) {
  SetNPileUps(frag.GetNumberOfPileups());
}

TGriffinHit::~TGriffinHit()  {	}

void TGriffinHit::Copy(TObject &rhs) const {
  TGRSIDetectorHit::Copy(rhs);
  static_cast<TGriffinHit&>(rhs).fFilter                = fFilter;
  static_cast<TGriffinHit&>(rhs).fGriffinHitBits        = 0; //We should copy over a 0 and let the hit recalculate, this is safest
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

TVector3 TGriffinHit::GetPosition(double dist) const {
  return TGriffin::GetPosition(GetDetector(),GetCrystal(),dist);
}

TVector3 TGriffinHit::GetPosition() const {
  return GetPosition(GetDefaultDistance());
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
      //this->SetPosition(hit->GetPosition());
      this->SetAddress(hit->GetAddress());
   }
   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
   //this has to be done at the very end, otherwise this->GetEnergy() might not work
   this->SetCharge(0);
   //Add all of the pileups.This should be changed when the max number of pileups changes
   if((this->NPileUps() + hit->NPileUps()) < 4){
      this->SetNPileUps(this->NPileUps() + hit->NPileUps());
   }  
   else{
      this->SetNPileUps(3);
   }
   if((this->PUHit() + hit->PUHit()) < 4){
      this->SetPUHit(this->PUHit() + hit->PUHit());
   }  
   else{
      this->SetPUHit(3);
   }
   //KValue is somewhate meaningless in addback, so I am using it as an indicator that a piledup hit was added-back RD
   if(this->GetKValue() > hit->GetKValue()){
      this->SetKValue(hit->GetKValue());
   }
}

void TGriffinHit::SetGriffinFlag(enum EGriffinHitBits flag,Bool_t set){
	fGriffinHitBits.SetBit(flag,set);
}

UShort_t TGriffinHit::NPileUps() const {
	//The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fGriffinHitBits.TestBits(kTotalPU1)+ fGriffinHitBits.TestBits(kTotalPU2));
}

UShort_t TGriffinHit::PUHit() const { 
	//The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fGriffinHitBits.TestBits(kPUHit1) + (fGriffinHitBits.TestBits(kPUHit2) >> kPUHitOffset)); 
} 

void TGriffinHit::SetNPileUps(UChar_t npileups) {
   SetGriffinFlag(kTotalPU1,(npileups & kTotalPU1));  
   SetGriffinFlag(kTotalPU2,(npileups & kTotalPU2));  
}

void TGriffinHit::SetPUHit(UChar_t puhit) {
   if(puhit > 2)
      puhit = 3;
	//The pluralized test bits returns the actual value of the fBits masked. Not just a bool.

   SetGriffinFlag(kPUHit1,(puhit << kPUHitOffset) & kPUHit1);  
   SetGriffinFlag(kPUHit2,(puhit << kPUHitOffset) & kPUHit2);  
}

Double_t TGriffinHit::GetNoCTEnergy(Option_t* opt) const{
  TChannel* chan = GetChannel();
  if(!chan) {
    Error("GetEnergy","No TChannel exists for address 0x%08x",GetAddress());
    return 0.;
  }
  return chan->CalibrateENG(Charge(),GetKValue());
}

