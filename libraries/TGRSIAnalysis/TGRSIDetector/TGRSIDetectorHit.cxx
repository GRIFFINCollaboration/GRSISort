
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

TGRSIDetectorHit::TGRSIDetectorHit(const int &Address) : TObject() { 
  //Default constructor
  Clear();
  faddress = Address;
  if(!fPPG)
   fPPG = static_cast<TPPG*>(gDirectory->Get("TPPG")); //There Might be a better way to do this

#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::TGRSIDetectorHit(const TGRSIDetectorHit& rhs) : TObject() { 
	//Default Copy constructor
	rhs.Copy(*this);
	//((TGriffinHit&)rhs).cfd             = cfd;
	//rhs.time            = time;
   fPPG = (TPPG*)gDirectory->Get("TPPG"); //There Might be a better way to do this
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::~TGRSIDetectorHit() {
//Default destructor
}

void TGRSIDetectorHit::Streamer(TBuffer &R__b)
{
   // Stream an object of class TGRSIDetectorHit.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TGRSIDetectorHit::Class(),this);
   } else {
      fbitflags = 0;
      R__b.WriteClassBuffer(TGRSIDetectorHit::Class(),this);
   }
}

Double_t TGRSIDetectorHit::GetTime(Option_t *opt) const{
   if(IsTimeSet())
      return ftime;

   Double_t dtime = static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform());
   TChannel *chan = GetChannel();
   if(!chan)
      return dtime;

   return dtime-chan->GetTZero(GetEnergy());
}

Double_t TGRSIDetectorHit::GetTime(Option_t *opt) {
   if(IsTimeSet())
      return ftime;

   Double_t dtime = static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform());
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
   printf("GetEnergy\n");
   if(IsEnergySet()){
   printf("GetEnergy is set\n");
      return fenergy;
   }

   TChannel *chan = GetChannel();
   if(!chan){
      printf("no TChannel set for this address\n");
      return 0.00;
   }
      SetEnergy(chan->CalibrateENG(GetCharge()));
   printf("SetEnergy\n");
      return fenergy;

}

void TGRSIDetectorHit::Copy(TObject &rhs) const {
  //if(!rhs.InheritsFrom("TGRSIDetectorHit")
  //   return;

  TObject::Copy(rhs);
  static_cast<TGRSIDetectorHit&>(rhs).faddress         = faddress;
  static_cast<TGRSIDetectorHit&>(rhs).fposition       = fposition;
  static_cast<TGRSIDetectorHit&>(rhs).fwaveform       = fwaveform;
  static_cast<TGRSIDetectorHit&>(rhs).fcfd            = fcfd;
  static_cast<TGRSIDetectorHit&>(rhs).ftimestamp      = ftimestamp;
  static_cast<TGRSIDetectorHit&>(rhs).fcharge         = fcharge;
  static_cast<TGRSIDetectorHit&>(rhs).fdetector       = fdetector;
  static_cast<TGRSIDetectorHit&>(rhs).fenergy         = fenergy;
  static_cast<TGRSIDetectorHit&>(rhs).ftime           = ftime;

  static_cast<TGRSIDetectorHit&>(rhs).fbitflags       =  fbitflags;
  static_cast<TGRSIDetectorHit&>(rhs).fPPGStatus      =  fPPGStatus;
  static_cast<TGRSIDetectorHit&>(rhs).fCycleTimeStamp =  fCycleTimeStamp;

//  ((TGRSIDetectorHit&)rhs).parent  = parent;  
}

void TGRSIDetectorHit::Print(Option_t *opt) const {
//General print statement for a TGRSIDetectorHit.
//Currently prints nothing.
   fposition.Print();
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

UInt_t TGRSIDetectorHit::SetDetector(const UInt_t &det) {
   fdetector = det;
   SetFlag(kIsDetSet,true);
   return fdetector;
}

TVector3 TGRSIDetectorHit::SetPosition(Double_t dist) {
   //This should not be overridden. It's job is to call the correct 
   //position for the derived TGRSIDetector object.
   SetFlag(kIsPositionSet,true);
	fposition = GetChannelPosition(dist); //Calls a general Hit GetPosition function
   return fposition;
}

TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) const{
   //This should not be overridden and instead GetChannelPosition should
   //be used in the derived class.
   if(IsPosSet())
      return fposition;

//   if(IsDetSet())
   return GetChannelPosition(dist); //Calls the derivative GetPosition function
   //We must do a check in here to make sure it is returning something reasonable

}

TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) {
   //This should not be overridden and instead GetChannelPosition should
   //be used in the derived class.
  if(IsPosSet())
      return fposition;

   if(IsDetSet())
      return TGRSIDetectorHit::SetPosition(dist); 

   GetDetector();
   if(IsDetSet())
      return TGRSIDetectorHit::SetPosition(dist); 

   printf("no position found for current hit\n");
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
