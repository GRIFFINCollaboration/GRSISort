#include "TGRSIDetectorHit.h"

#include <iostream>

#include "TClass.h"

/// \cond CLASSIMP
ClassImp(TGRSIDetectorHit)
/// \endcond

TPPG* TGRSIDetectorHit::fPPG = 0;

TVector3 TGRSIDetectorHit::fBeamDirection(0,0,1);

TGRSIDetectorHit::TGRSIDetectorHit(const int& Address) : TObject() {
  ///Default constructor
  Clear();
  fAddress = Address;
//  if(!fPPG)
 //   fPPG = TPPG::Get();//static_cast<TPPG*>(gDirectory->Get("TPPG")); //There Might be a better way to do this

#if MAJOR_ROOT_VERSION < 6
  Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::TGRSIDetectorHit(const TGRSIDetectorHit& rhs, bool copywave) : TObject() {
  ///Default Copy constructor
  rhs.Copy(*this);
  if(copywave) {
    rhs.CopyWave(*this);
  }
  ClearTransients();
 // if(!fPPG)
 //   fPPG = TPPG::Get();//static_cast<TPPG*>(gDirectory->Get("TPPG")); //There Might be a better way to do this
#if MAJOR_ROOT_VERSION < 6
  Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::~TGRSIDetectorHit() {
  //Default destructor
}

void TGRSIDetectorHit::Streamer(TBuffer& R__b) {
  /// Stream an object of class TGRSIDetectorHit.
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(TGRSIDetectorHit::Class(),this);
  } else {
    fBitflags = 0;
    R__b.WriteClassBuffer(TGRSIDetectorHit::Class(),this);
  }
}

Double_t TGRSIDetectorHit::GetTime(const UInt_t& correction_flag,Option_t* opt) const {
  if(IsTimeSet())
    return fTime;

  TChannel* chan = GetChannel();
  if(!chan) {
    Error("GetTime","No TChannel exists for address 0x%08x",GetAddress());
    return 10.*(static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform()));
  }

	switch(chan->GetDigitizerType()) {
		Double_t dTime;
		case TMnemonic::kGRF16:
			dTime = (GetTimeStamp()&(~0x3ffff))*10. + (GetCfd() + gRandom->Uniform())/1.6;//CFD is in 10/16th of a nanosecond
			return dTime - 10.*(chan->GetTZero(GetEnergy()));
		case TMnemonic::kGRF4G:
			dTime = GetTimeStamp()*10. + (fCfd>>22) + ((fCfd & 0x3fffff) + gRandom->Uniform())/256.;
			return dTime - 10.*(chan->GetTZero(GetEnergy()));
		default:
		   dTime = static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform());
		   return 10.*(dTime - chan->GetTZero(GetEnergy()));
	}
	return 0.;
}


int TGRSIDetectorHit::GetCharge() const {
  TChannel *chan = GetChannel();
  if(!chan )
    return std::floor(Charge());
  if(fKValue>0){
    return std::floor(Charge()/((Float_t)fKValue));// this will use the integration value
  } else if(chan->UseCalFileIntegration()) {
    return std::floor(Charge())/((Float_t)chan->GetIntegration());// this will use the integration value
  }                                                               // in the TChannel if it exists.
  return std::floor(Charge());// this will use no integration value
}

double TGRSIDetectorHit::GetEnergy(Option_t* opt) const {
  if(TestBit(kIsEnergySet))
    return fEnergy;
  TChannel* chan = GetChannel();
  if(!chan) {
    //Error("GetEnergy","No TChannel exists for address 0x%08x",GetAddress());
    return SetEnergy((Double_t)(Charge()));
  }
  if(fKValue >0) {
    return SetEnergy(chan->CalibrateENG(Charge(),(int)fKValue));
  } else if(chan->UseCalFileIntegration()) {
    return SetEnergy(chan->CalibrateENG(Charge(),0));  // this will use the integration value
                                            // in the TChannel if it exists.
  }
  return SetEnergy(chan->CalibrateENG(Charge()));
}

void TGRSIDetectorHit::Copy(TObject& rhs) const {
  TObject::Copy(rhs);
  static_cast<TGRSIDetectorHit&>(rhs).fAddress        = fAddress;
  //static_cast<TGRSIDetectorHit&>(rhs).fPosition       = fPosition;
  static_cast<TGRSIDetectorHit&>(rhs).fCfd            = fCfd;
  static_cast<TGRSIDetectorHit&>(rhs).fTimeStamp      = fTimeStamp;
  static_cast<TGRSIDetectorHit&>(rhs).fCharge         = fCharge;
  static_cast<TGRSIDetectorHit&>(rhs).fKValue         = fKValue;
  static_cast<TGRSIDetectorHit&>(rhs).fEnergy         = fEnergy;
  static_cast<TGRSIDetectorHit&>(rhs).fTime           = fTime;
  static_cast<TGRSIDetectorHit&>(rhs).fChannel        = fChannel;

  static_cast<TGRSIDetectorHit&>(rhs).fBitflags       = 0;
  static_cast<TGRSIDetectorHit&>(rhs).fPPGStatus      = fPPGStatus;
  static_cast<TGRSIDetectorHit&>(rhs).fCycleTimeStamp = fCycleTimeStamp;
}

void TGRSIDetectorHit::CopyWave(TObject &rhs) const {
  static_cast<TGRSIDetectorHit&>(rhs).fWaveform       = fWaveform;
}

void TGRSIDetectorHit::Copy(TObject& rhs,bool copywave) const {
  Copy(rhs);
  if(copywave)
    CopyWave(rhs);
}


void TGRSIDetectorHit::Print(Option_t* opt) const {
  ///General print statement for a TGRSIDetectorHit.
  ///Currently prints nothing.
  //fPosition.Print();
}

const char *TGRSIDetectorHit::GetName() const {
  TChannel *channel = GetChannel();
  if(!channel)
     return Class()->ClassName();
  else
     return channel->GetName();
}


void TGRSIDetectorHit::Clear(Option_t* opt) {
  ///General clear statement for a TGRSIDetectorHit.
  fAddress = 0xffffffff;    // -1
  //fPosition.SetXYZ(0,0,1);  // unit vector along the beam.
  fWaveform.clear();        // reset size to zero.
  fCharge         = 0;
  fKValue         =0;
  fCfd            = -1;
  fTimeStamp      = 0;
  //fDetector       = -1;
  // fSegment        = -1;
  fEnergy         = 0.;
  fBitflags       = fBitflags&0xff00;
  fPPGStatus      = TPPG::kJunk;
  fCycleTimeStamp = 0;
  fChannel        = NULL;
}

Int_t TGRSIDetectorHit::GetDetector() const {

  TChannel* channel = GetChannel();
  if(!channel) {
    Error("GetDetector","No TChannel exists for address 0x%08x",GetAddress());
    return -1;
  }
  return channel->GetDetectorNumber(); //mnemonic.arrayposition;

}

Int_t TGRSIDetectorHit::GetSegment() const {
   TChannel *channel = GetChannel();
   if(!channel){
      Error("GetSegment","No TChannel exists for address %08x",GetAddress());
      return -1;
   }
  return channel->GetSegmentNumber();
}

/*void TGRSIDetectorHit::GetSegment() const { 
   static bool been_warned = 0; 
   if(!been_warned) { 
      Warning(DRED "GetSegment()" RESET_COLOR,DRED "Has been moved to GetSegmentNumber" RESET_COLOR); 
      been_warned = true;
   } 
}*/

Int_t TGRSIDetectorHit::GetCrystal() const {
  TChannel *channel = GetChannel();
  if(channel)
    return channel->GetCrystalNumber();
  return -1;
}

UShort_t TGRSIDetectorHit::GetArrayNumber() const {
  TChannel *channel = GetChannel();
  if(channel) {
    return (GetDetector()-1)*4 + GetCrystal();
  }
  return -1;
}
/*
UInt_t TGRSIDetectorHit::SetDetector(const UInt_t& det) {
  fDetector = det;
  SetFlag(kIsDetSet,true);
  return fDetector;
}

Short_t TGRSIDetectorHit::SetSegment(const Short_t &seg) {
   fSegment = seg;
   SetFlag(kIsSegSet,true);
   return fSegment;
}
*/
//TVector3 TGRSIDetectorHit::SetPosition(Double_t dist) {
  ///This should not be overridden. It's job is to call the correct
  ///position for the derived TGRSIDetector object.
//  SetFlag(kIsPositionSet,true);
//  fPosition = GetChannelPosition(dist); //Calls a general Hit GetPosition function
//  return fPosition;
//}

//TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) const{
  ///This should not be overridden and instead GetChannelPosition should
  ///be used in the derived class.
//  if(IsPosSet())
//    return fPosition;

//  return GetChannelPosition(dist); //Calls the derivative GetPosition function
  //We must do a check in here to make sure it is returning something reasonable
//}

//TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) {
  ///This should not be overridden and instead GetChannelPosition should
  ///be used in the derived class.
  //if(IsPosSet())
  //  return fPosition;

  //if(GetDetector()>0)
  //  return TGRSIDetectorHit::SetPosition(dist);

  //GetDetector();
  //if(IsDetSet())
  //  return TGRSIDetectorHit::SetPosition(dist);

//  printf("no position found for current hit\n");
//  return *GetBeamDirection();  //TVector3(0,0,1);
//}
//    return channel->GetSegmentNumber(); //mnemonic.arrayposition;
// }


bool TGRSIDetectorHit::CompareEnergy(TGRSIDetectorHit* lhs, TGRSIDetectorHit* rhs) {
  return (lhs->GetEnergy() > rhs->GetEnergy());
}

Long_t TGRSIDetectorHit::GetTimeStamp(Option_t* opt) const  { 
   TChannel* tmpChan = GetChannel();
   if(!tmpChan){
      return fTimeStamp;   
   }
   return fTimeStamp - tmpChan->GetTimeOffset();   
}

uint16_t TGRSIDetectorHit::GetPPGStatus() const {
  if(IsPPGSet())
    return fPPGStatus;

  if(!fPPG)
    return TPPG::kJunk;

  fPPGStatus = fPPG->GetStatus(this->GetTime());
  fCycleTimeStamp = GetTime() - fPPG->GetLastStatusTime(GetTime());
  SetBit(kIsPPGSet,true);
  return fPPGStatus;
}

uint16_t TGRSIDetectorHit::GetCycleTimeStamp() const {
  if(IsPPGSet())
    return fCycleTimeStamp;

  if(!fPPG)
    return 0;

  fPPGStatus = fPPG->GetStatus(this->GetTime());
  fCycleTimeStamp = GetTime() - fPPG->GetLastStatusTime(GetTime());
  SetBit(kIsPPGSet,true);
  return fCycleTimeStamp;
}

//void TGRSIDetectorHit::CopyFragment(const TFragment& frag) {
//  this->fAddress   = frag.ChannelAddress;
//  this->fCharge    = frag.GetCharge();
//  this->fCfd       = frag.GetCfd();
//  this->fTimeStamp = frag.GetTimeStamp();
//  this->fPosition  = TVector3(0,0,1);
//  this->fEnergy    = frag.GetEnergy();
//}

//void TGRSIDetectorHit::CopyWaveform(const TFragment &frag) {
//  if(frag.HasWave())
//    SetWaveform(frag.wavebuffer);
//}

// const here is rather dirty
void TGRSIDetectorHit::SetBit(enum EBitFlag flag, Bool_t set) const {
  if(set)
    fBitflags |= flag;
  else
    fBitflags &= (~flag);
}
