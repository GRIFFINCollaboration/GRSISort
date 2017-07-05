#include "TGRSIDetectorHit.h"

#include <iostream>

#include "TClass.h"

/// \cond CLASSIMP
ClassImp(TGRSIDetectorHit)
   /// \endcond

   TPPG* TGRSIDetectorHit::fPPG = nullptr;

TVector3 TGRSIDetectorHit::fBeamDirection(0, 0, 1);

TGRSIDetectorHit::TGRSIDetectorHit(const int& Address) : TObject()
{
   /// Default constructor
   Clear();
   fAddress = Address;

   Class()->IgnoreTObjectStreamer(kTRUE);
}

TGRSIDetectorHit::TGRSIDetectorHit(const TGRSIDetectorHit& rhs, bool copywave) : TObject(rhs)
{
   /// Default Copy constructor
   rhs.Copy(*this);
   if(copywave) {
      rhs.CopyWave(*this);
   }
   ClearTransients();

   Class()->IgnoreTObjectStreamer(kTRUE);
}

TGRSIDetectorHit::~TGRSIDetectorHit()
{
   // Default destructor
}

void TGRSIDetectorHit::Streamer(TBuffer& R__b)
{
   /// Stream an object of class TGRSIDetectorHit.
   if(R__b.IsReading()) {
      R__b.ReadClassBuffer(TGRSIDetectorHit::Class(), this);
   } else {
      fBitflags = 0;
      R__b.WriteClassBuffer(TGRSIDetectorHit::Class(), this);
   }
}

Double_t TGRSIDetectorHit::GetTime(const UInt_t&, Option_t*) const
{
   if(IsTimeSet()) {
      return fTime;
   }

   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      Error("GetTime", "No TChannel exists for address 0x%08x", GetAddress());
      return SetTime(10. * (static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform())));
   }
   switch(channel->GetDigitizerType()) {
      Double_t dTime;
   case TMnemonic::kGRF16:
      dTime = (GetTimeStamp() & (~0x3ffff)) * 10. +
              (GetCfd() + gRandom->Uniform()) / 1.6; // CFD is in 10/16th of a nanosecond
      return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
   case TMnemonic::kGRF4G:
      dTime = GetTimeStamp() * 10. + (fCfd >> 22) + ((fCfd & 0x3fffff) + gRandom->Uniform()) / 256.;
      return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
   default:
      dTime = static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform());
      return SetTime(10. * (dTime - channel->GetTZero(GetEnergy())));
   }
   return 0.;
}

Float_t TGRSIDetectorHit::GetCharge() const
{
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      return Charge();
   }
   if(fKValue > 0) {
      return Charge() / (static_cast<Float_t>(fKValue)); // this will use the integration value
   }
   if(channel->UseCalFileIntegration()) {
      return Charge() / (static_cast<Float_t>(channel->GetIntegration())); // this will use the integration value
   }                                                                       // in the TChannel if it exists.
   return Charge();                                                        // this will use no integration value
}

double TGRSIDetectorHit::GetEnergy(Option_t*) const
{
   if(TestHitBit(kIsEnergySet)) {
      return fEnergy;
   }
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      // Error("GetEnergy","No TChannel exists for address 0x%08x",GetAddress());
      return SetEnergy(static_cast<Double_t>(Charge()));
   }
   if(fKValue > 0) {
      double energy = channel->CalibrateENG(Charge(), static_cast<int>(fKValue));
      return SetEnergy(energy + GetEnergyNonlinearity(energy));
   }
   if(channel->UseCalFileIntegration()) {
      double energy = channel->CalibrateENG(Charge(), 0);
      return SetEnergy(channel->CalibrateENG(energy) +
                       GetEnergyNonlinearity(energy)); // this will use the integration value
                                                       // in the TChannel if it exists.
   }
   double energy = channel->CalibrateENG(Charge());
   return SetEnergy(energy + GetEnergyNonlinearity(energy));
}

void TGRSIDetectorHit::Copy(TObject& rhs) const
{
   TObject::Copy(rhs);
   static_cast<TGRSIDetectorHit&>(rhs).fAddress = fAddress;
   // static_cast<TGRSIDetectorHit&>(rhs).fPosition       = fPosition;
   static_cast<TGRSIDetectorHit&>(rhs).fCfd       = fCfd;
   static_cast<TGRSIDetectorHit&>(rhs).fTimeStamp = fTimeStamp;
   static_cast<TGRSIDetectorHit&>(rhs).fCharge    = fCharge;
   static_cast<TGRSIDetectorHit&>(rhs).fKValue    = fKValue;
   static_cast<TGRSIDetectorHit&>(rhs).fEnergy    = fEnergy;
   static_cast<TGRSIDetectorHit&>(rhs).fTime      = fTime;
   static_cast<TGRSIDetectorHit&>(rhs).fChannel   = fChannel;

   static_cast<TGRSIDetectorHit&>(rhs).fBitflags       = 0;
   static_cast<TGRSIDetectorHit&>(rhs).fPPGStatus      = fPPGStatus;
   static_cast<TGRSIDetectorHit&>(rhs).fCycleTimeStamp = fCycleTimeStamp;
}

void TGRSIDetectorHit::CopyWave(TObject& rhs) const
{
   static_cast<TGRSIDetectorHit&>(rhs).fWaveform = fWaveform;
}

void TGRSIDetectorHit::Copy(TObject& rhs, bool copywave) const
{
   Copy(rhs);
   if(copywave) {
      CopyWave(rhs);
   }
}

void TGRSIDetectorHit::Print(Option_t*) const
{
   /// General print statement for a TGRSIDetectorHit.
   /// Currently prints nothing.
   // fPosition.Print();
}

const char* TGRSIDetectorHit::GetName() const
{
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      return Class()->ClassName();
   }
   return channel->GetName();
}

void TGRSIDetectorHit::Clear(Option_t*)
{
   /// General clear statement for a TGRSIDetectorHit.
   fAddress = 0xffffffff; // -1
   // fPosition.SetXYZ(0,0,1);  // unit vector along the beam.
   fWaveform.clear(); // reset size to zero.
   fCharge    = 0;
   fKValue    = 0;
   fCfd       = -1;
   fTimeStamp = 0;
   // fDetector       = -1;
   // fSegment        = -1;
   fEnergy         = 0.;
   fBitflags       = 0;
   fPPGStatus      = TPPG::kJunk;
   fCycleTimeStamp = 0;
   fChannel        = nullptr;
}

Int_t TGRSIDetectorHit::GetDetector() const
{

   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      Error("GetDetector", "No TChannel exists for address 0x%08x", GetAddress());
      return -1;
   }
   return channel->GetDetectorNumber(); // mnemonic.arrayposition;
}

Int_t TGRSIDetectorHit::GetSegment() const
{
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      Error("GetSegment", "No TChannel exists for address %08x", GetAddress());
      return -1;
   }
   return channel->GetSegmentNumber();
}

Int_t TGRSIDetectorHit::GetCrystal() const
{
   TChannel* channel = GetChannel();
   if(channel != nullptr) {
      return channel->GetCrystalNumber();
   }
   return -1;
}

UShort_t TGRSIDetectorHit::GetArrayNumber() const
{
   TChannel* channel = GetChannel();
   if(channel != nullptr) {
      return (GetDetector() - 1) * 4 + GetCrystal();
   }
   return -1;
}

bool TGRSIDetectorHit::CompareEnergy(TGRSIDetectorHit* lhs, TGRSIDetectorHit* rhs)
{
   return (lhs->GetEnergy() > rhs->GetEnergy());
}

Long64_t TGRSIDetectorHit::GetTimeStamp(Option_t*) const
{
   TChannel* tmpChan = GetChannel();
   if(tmpChan == nullptr) {
      return fTimeStamp;
   }
   return fTimeStamp - tmpChan->GetTimeOffset();
}

uint16_t TGRSIDetectorHit::GetPPGStatus() const
{
   if(IsPPGSet()) {
      return fPPGStatus;
   }

   if(TPPG::Get() == nullptr) {
      return TPPG::kJunk;
   }

   fPPGStatus      = TPPG::Get()->GetStatus(GetTimeStamp());
   fCycleTimeStamp = GetTimeStamp() - TPPG::Get()->GetLastStatusTime(GetTimeStamp());
   SetHitBit(kIsPPGSet, true);
   return fPPGStatus;
}

Long64_t TGRSIDetectorHit::GetCycleTimeStamp() const
{
   if(IsPPGSet()) {
      return fCycleTimeStamp;
   }

   if(TPPG::Get() == nullptr) {
      return 0;
   }

   fPPGStatus      = TPPG::Get()->GetStatus(GetTimeStamp());
   fCycleTimeStamp = GetTimeStamp() - TPPG::Get()->GetLastStatusTime(GetTimeStamp());
   SetHitBit(kIsPPGSet, true);
   return fCycleTimeStamp;
}

// const here is rather dirty
void TGRSIDetectorHit::SetHitBit(enum EBitFlag flag, Bool_t set) const
{
   fBitflags.SetBit(flag, set);
}
