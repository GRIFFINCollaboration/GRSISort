
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
  ((TGriffinHit&)rhs).fFilter          = fFilter;
  ((TGriffinHit&)rhs).fGriffinHitBits  = fGriffinHitBits;
  ((TGriffinHit&)rhs).fCrystal         = fCrystal;
  ((TGriffinHit&)rhs).fPPG             = fPPG;
  return;                                      
}                                       

bool TGriffinHit::InFilter(Int_t wantedfilter) {
 // check if the desired filter is in wanted filter;
 // return the answer;
 return true;
}


void TGriffinHit::Clear(Option_t *opt)	{
   TGRSIDetectorHit::Clear(opt);    // clears the base (address, position and waveform)
   fFilter          =  0;
   fGriffinHitBits  =  0;
   fCrystal         = 0xFFFF;
   fPPG             =  0;

}


void TGriffinHit::Print(Option_t *opt) const	{
   printf("Griffin Detector: %i\n",GetDetector());
	printf("Griffin Crystal:  %i\n",GetCrystal());
   printf("Griffin Energy:   %lf\n",GetEnergy());
	printf("Griffin hit time:   %lf\n",GetTime());
   printf("Griffin hit TV3 theta: %.2f\tphi%.2f\n",GetPosition().Theta() *180/(3.141597),GetPosition().Phi() *180/(3.141597));
}

TVector3 TGriffinHit::GetPosition(Double_t dist) const{
	return TGriffin::GetPosition(GetDetector(),GetCrystal(),dist);
}

UInt_t TGriffinHit::GetCrystal() const { 
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


