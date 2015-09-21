
#include "TGRSIDetectorHit.h"
#include <TClass.h>

ClassImp(TGRSIDetectorHit)

////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIDetectorHit                                           //
//                                                            //
// This is class that contains the basic info about detector  //
// hits. This is where the position of a detector is stored.  //
//                                                            //
////////////////////////////////////////////////////////////////

TPPG* TGRSIDetectorHit::fPPG = 0;

TGRSIDetectorHit::TGRSIDetectorHit(const int &Address):TObject()	{ 
  //Default constructor
  Clear();
  faddress = Address;
  if(!fPPG)
   fPPG = (TPPG*)gDirectory->Get("TPPG"); //There Might be a better way to do this

#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::TGRSIDetectorHit(const TGRSIDetectorHit& rhs)	{ 
  //Default Copy constructor
  ((TGRSIDetectorHit&)rhs).Copy(*this);
  //((TGriffinHit&)rhs).cfd             = cfd;
  //rhs.time            = time;
   fPPG = (TPPG*)gDirectory->Get("TPPG"); //There Might be a better way to do this
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::~TGRSIDetectorHit()	{
//Default destructor
}

Double_t TGRSIDetectorHit::GetTime(Option_t *opt) const{
   if(IsTimeSet())
      return ftime;

   Double_t dtime = (Double_t)(GetTimeStamp()) + gRandom->Uniform();
   TChannel *chan = GetChannel();
   if(!chan)
      return dtime;

   return dtime-chan->GetTZero(GetEnergy());
}

Double_t TGRSIDetectorHit::GetTime(Option_t *opt) {
   if(IsTimeSet())
      return ftime;

   Double_t dtime = (Double_t)(GetTimeStamp()) + gRandom->Uniform();
   TChannel *chan = GetChannel();
   if(!chan)
      return dtime;
   
   SetTime(dtime-chan->GetTZero(GetEnergy()));

   return ftime;
}


double TGRSIDetectorHit::GetEnergy(Option_t *opt) const {
   if(IsEnergySet())
      return fenergy;

   TChannel *chan = GetChannel();
   if(!chan){
      printf("no TChannel set for this address\n");
      return 0.00;
   }
      return chan->CalibrateENG(GetCharge());
}

double TGRSIDetectorHit::GetEnergy(Option_t *opt){
   if(IsEnergySet()){
      return fenergy;
   }

   TChannel *chan = GetChannel();
   if(!chan){
      printf("no TChannel set for this address\n");
      return 0.00;
   }
      SetEnergy(chan->CalibrateENG(GetCharge()));
      return fenergy;

}

void TGRSIDetectorHit::Copy(TObject &rhs) const {
  //if(!rhs.InheritsFrom("TGRSIDetectorHit")
  //   return;

  TObject::Copy(rhs);
  ((TGRSIDetectorHit&)rhs).faddress         = ((TGRSIDetectorHit&)*this).faddress;
  ((TGRSIDetectorHit&)rhs).fposition        = ((TGRSIDetectorHit&)*this).fposition;
  ((TGRSIDetectorHit&)rhs).fwaveform        = ((TGRSIDetectorHit&)*this).fwaveform;
  ((TGRSIDetectorHit&)rhs).fcfd             = ((TGRSIDetectorHit&)*this).fcfd;
  ((TGRSIDetectorHit&)rhs).ftimestamp      = ((TGRSIDetectorHit&)*this).ftimestamp;
  ((TGRSIDetectorHit&)rhs).fcharge          = ((TGRSIDetectorHit&)*this).fcharge;
  ((TGRSIDetectorHit&)rhs).fdetector        = ((TGRSIDetectorHit&)*this).fdetector;
  ((TGRSIDetectorHit&)rhs).fenergy          = ((TGRSIDetectorHit&)*this).fenergy;
  
  ((TGRSIDetectorHit&)rhs).fbitflags       = ((TGRSIDetectorHit&)*this).fbitflags;
  ((TGRSIDetectorHit&)rhs).fPPGStatus      = ((TGRSIDetectorHit&)*this).fPPGStatus;
  ((TGRSIDetectorHit&)rhs).fCycleTimeStamp = ((TGRSIDetectorHit&)*this).fCycleTimeStamp;

//  ((TGRSIDetectorHit&)rhs).parent  = parent;  
}

void TGRSIDetectorHit::Print(Option_t *opt) const {
//General print statement for a TGRSIDetectorHit.
//Currently prints nothing.
}

void TGRSIDetectorHit::Clear(Option_t *opt) {
  //General clear statement for a TGRSIDetectorHit.
  faddress = 0xffffffff;    // -1
  fposition.SetXYZ(0,0,1);  // unit vector along the beam.
  fwaveform.clear();        // reset size to zero.
  fcharge          = 0;
  fcfd             = -1;
  ftimestamp            = -1;
  fdetector        = -1;
  fenergy          =  0.0;
  fbitflags = 0;
  fPPGStatus      = TPPG::kJunk;
  fCycleTimeStamp = 0;
}

UInt_t TGRSIDetectorHit::GetDetector() const {
   if(IsDetSet())
     return fdetector;

   MNEMONIC mnemonic;
   TChannel *channel = GetChannel();
   if(!channel){
      Error("SetDetector","No TChannel exists for address %u",GetAddress());
      return -1;
   }
   ClearMNEMONIC(&mnemonic);
   ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
   return mnemonic.arrayposition;
}

UInt_t TGRSIDetectorHit::GetDetector() {
   if(IsDetSet())
      return fdetector;

   MNEMONIC mnemonic;
   TChannel *channel = GetChannel();
   if(!channel){
      Error("SetDetector","No TChannel exists for address %u",GetAddress());
      return -1;
   }
   ClearMNEMONIC(&mnemonic);
   ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
   return SetDetector(mnemonic.arrayposition);
}

UInt_t TGRSIDetectorHit::SetDetector(UInt_t det) {
   fdetector = det;
   SetFlag(kIsDetSet,true);
   return fdetector;
}

TVector3 TGRSIDetectorHit::SetPosition(Double_t dist) {
	fposition = TGRSIDetectorHit::GetPosition(dist); //Calls a general Hit GetPosition function
   return fposition;
}

TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) const{
   if(IsPosSet())
      return fposition;

   if(IsDetSet())
      return GetPosition(dist); //Calls the derivative GetPosition function

   return TVector3(0,0,1);

}

TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) {
  if(IsPosSet())
      return fposition;

   if(IsDetSet())
      return SetPosition(dist); //Calls the derivative GetPosition function

   return TVector3(0,0,1);

}

bool TGRSIDetectorHit::CompareEnergy(TGRSIDetectorHit *lhs, TGRSIDetectorHit *rhs) {
   return (lhs->GetEnergy() > rhs->GetEnergy());
}

uint16_t TGRSIDetectorHit::GetPPGStatus() const {
   if(IsPPGSet())
      return fPPGStatus;

   return TPPG::kJunk;
}

uint16_t TGRSIDetectorHit::GetPPGStatus() {
   if(IsPPGSet())
      return fPPGStatus;

   if(!fPPG)
      return TPPG::kJunk;

   fPPGStatus = fPPG->GetStatus(this->GetTime());
   fCycleTimeStamp = GetTime() - fPPG->GetLastStatusTime(GetTime());
   SetFlag(kIsPPGSet,true);
   return fPPGStatus;
}

uint16_t TGRSIDetectorHit::GetCycleTimeStamp() const {
   if(IsPPGSet())
      return fCycleTimeStamp;

   return 0;
}

uint16_t TGRSIDetectorHit::GetCycleTimeStamp() {
   if(IsPPGSet())
      return fCycleTimeStamp;

   if(!fPPG)
      return 0;

   fPPGStatus = fPPG->GetStatus(this->GetTime());
   fCycleTimeStamp = GetTime() - fPPG->GetLastStatusTime(GetTime());
   SetFlag(kIsPPGSet,true);
   return fCycleTimeStamp;
}

void TGRSIDetectorHit::CopyFragment(const TFragment &frag) {
  this->faddress  = frag.ChannelAddress;  
  this->fcharge   = frag.GetCharge();
  this->fcfd      = frag.GetCfd();
  this->ftimestamp= frag.GetTimeStamp();
  this->fposition = TVector3(0,0,1); 
  this->fenergy   = frag.GetEnergy();
 // this->SetDetector(frag.GetDetector());
}

void TGRSIDetectorHit::SetFlag(enum Ebitflag flag,Bool_t set){
   if(set)
      fbitflags |= flag;
   else
      fbitflags &= (~flag);
}

//unsigned int TGRSIDetectorHit::GetHighestBitSet(UChar_t flag){
//   return std::log2(flag);
//}
