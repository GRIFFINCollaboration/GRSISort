#include "TDetectorHit.h"
#include "TGRSIOptions.h"

#include <iostream>

#include "TClass.h"

TVector3 TDetectorHit::fBeamDirection(0, 0, 1);

TDetectorHit::TDetectorHit(const int& address)
{
   /// Default constructor
   Clear();
   // this needs to happen here, after we call Clear
   // otherwise if will be cleared as well
   fAddress = address;   // NOLINT(cppcoreguidelines-prefer-member-initializer)
}

TDetectorHit::TDetectorHit(const TDetectorHit& rhs, bool copywave) : TObject(rhs)
{
   /// Default Copy constructor
   rhs.Copy(*this);
   // if we can get the commandline options, we respect whether ExtracWaves has been set or not
   // otherwise we rely on the copywave flag (defaults to true)
   if((TGRSIOptions::Get() != nullptr && TGRSIOptions::Get()->ExtractWaves()) ||
      (TGRSIOptions::Get() == nullptr && copywave)) {
      rhs.CopyWave(*this);
   }
   ClearTransients();
}

TDetectorHit::~TDetectorHit() = default;

void TDetectorHit::Streamer(TBuffer& R__b)
{
   /// Stream an object of class TDetectorHit.
   if(R__b.IsReading()) {
      R__b.ReadClassBuffer(TDetectorHit::Class(), this);
   } else {
      fBitFlags = 0;
      R__b.WriteClassBuffer(TDetectorHit::Class(), this);
   }
}

Double_t TDetectorHit::GetTime(const ETimeFlag&, Option_t*) const
{
   if(IsTimeSet()) {
      return fTime;
   }
   TChannel* tmpChan = GetChannel();
   if(tmpChan == nullptr) {
      return SetTime(static_cast<Double_t>(static_cast<double>(GetTimeStamp()) + gRandom->Uniform()));
   }

   return SetTime(tmpChan->GetTime(GetTimeStamp(), GetCfd(), GetEnergy()));
}

Float_t TDetectorHit::GetCharge() const
{
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      return Charge();
   }
   if(fKValue > 0 && !channel->UseCalFileIntegration()) {
      return Charge() / (static_cast<Float_t>(fKValue));   // this will use the integration value
   }
   if(channel->UseCalFileIntegration()) {
      return Charge() / (static_cast<Float_t>(channel->GetIntegration()));   // this will use the integration value
   }   // in the TChannel if it exists.
   return Charge();   // this will use no integration value
}

double TDetectorHit::GetEnergy(Option_t*) const
{
   if(TestHitBit(EBitFlag::kIsEnergySet)) {
      return fEnergy;
   }
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      return SetEnergy(static_cast<Double_t>(Charge()));
   }
   if(channel->UseCalFileIntegration()) {
      double energy = channel->CalibrateENG(Charge(), 0);
      return SetEnergy(energy +
                       GetEnergyNonlinearity(energy));   // this will use the integration value
                                                         // in the TChannel if it exists.
   }
   if(fKValue > 0) {
      double energy = channel->CalibrateENG(Charge(), static_cast<int>(fKValue));
      return SetEnergy(energy + GetEnergyNonlinearity(energy));
   }
   double energy = channel->CalibrateENG(Charge());
   return SetEnergy(energy + GetEnergyNonlinearity(energy));
}

Double_t TDetectorHit::GetEnergyNonlinearity(double energy) const
{
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      return 0.;
   }
   return -(channel->GetEnergyNonlinearity(energy));
}

void TDetectorHit::Copy(TObject& rhs) const
{
   TObject::Copy(rhs);
   static_cast<TDetectorHit&>(rhs).fAddress = fAddress;
   // static_cast<TDetectorHit&>(rhs).fPosition       = fPosition;
   static_cast<TDetectorHit&>(rhs).fCfd       = fCfd;
   static_cast<TDetectorHit&>(rhs).fTimeStamp = fTimeStamp;
   static_cast<TDetectorHit&>(rhs).fCharge    = fCharge;
   static_cast<TDetectorHit&>(rhs).fKValue    = fKValue;
   static_cast<TDetectorHit&>(rhs).fEnergy    = fEnergy;
   static_cast<TDetectorHit&>(rhs).fTime      = fTime;
   static_cast<TDetectorHit&>(rhs).fChannel   = fChannel;

   static_cast<TDetectorHit&>(rhs).fBitFlags       = 0;
   static_cast<TDetectorHit&>(rhs).fPPGStatus      = fPPGStatus;
   static_cast<TDetectorHit&>(rhs).fCycleTimeStamp = fCycleTimeStamp;
}

void TDetectorHit::CopyWave(TObject& rhs) const
{
   static_cast<TDetectorHit&>(rhs).fWaveform = fWaveform;
}

void TDetectorHit::Copy(TObject& rhs, bool copywave) const
{
   Copy(rhs);
   if(copywave) {
      CopyWave(rhs);
   }
}

//void TDetectorHit::CopyFragment(const TFragment& frag)
//{
//	frag.Copy(*this);
//}

void TDetectorHit::Print(Option_t*) const
{
   /// General print statement for a TDetectorHit.
   Print(std::cout);
}

void TDetectorHit::Print(std::ostream& out) const
{
   /// Print detector hit to stream out.
   std::ostringstream str;
   str << "==== " << ClassName() << " @ " << this << " ====" << std::endl;
   str << "\t" << GetName() << std::endl;
   str << "\tCharge:    " << Charge() << std::endl;
   str << "\tTime:      " << GetTime() << std::endl;
   str << "\tTimestamp: " << GetTimeStamp() << " in " << GetTimeStampUnit() << " ns = " << GetTimeStampNs() << std::endl;
   str << "============================" << std::endl;
   out << str.str();
}

const char* TDetectorHit::GetName() const
{
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      return Class()->ClassName();
   }
   return channel->GetName();
}

void TDetectorHit::Clear(Option_t*)
{
   /// General clear statement for a TDetectorHit.
   fAddress   = 0xffffffff;   // -1
   fCharge    = 0;
   fKValue    = 0;
   fCfd       = -1;
   fTimeStamp = 0;
   fWaveform.clear();   // reset size to zero.
   fTime           = 0.;
   fEnergy         = 0.;
   fPPGStatus      = EPpgPattern::kJunk;
   fCycleTimeStamp = 0;
   fChannel        = nullptr;
   fBitFlags       = 0;
}

Int_t TDetectorHit::GetDetector() const
{
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      return -1;
   }
   return channel->GetDetectorNumber();   // mnemonic.arrayposition;
}

Int_t TDetectorHit::GetSegment() const
{
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      return -1;
   }
   return channel->GetSegmentNumber();
}

Int_t TDetectorHit::GetCrystal() const
{
   TChannel* channel = GetChannel();
   if(channel != nullptr) {
      return channel->GetCrystalNumber();
   }
   return -1;
}

bool TDetectorHit::CompareEnergy(TDetectorHit* lhs, TDetectorHit* rhs)
{
   return (lhs->GetEnergy() > rhs->GetEnergy());
}

int TDetectorHit::GetChannelNumber() const
{
   TChannel* chan = TChannel::GetChannel(fAddress, true);
   if(chan == nullptr) {
      return 0;
   }
   return chan->GetNumber();
}

Long64_t TDetectorHit::GetTimeStampNs(Option_t*) const
{
   TChannel* tmpChan = GetChannel();
   if(tmpChan == nullptr) {
      return GetTimeStamp();   // GetTimeStampUnit returns 1 of there is no channel
   }
   return GetTimeStamp() * GetTimeStampUnit() * static_cast<Long64_t>((1.0 - tmpChan->GetTimeDrift()) - static_cast<double>(tmpChan->GetTimeOffset()));
}

Int_t TDetectorHit::GetTimeStampUnit() const
{
   TChannel* chan = GetChannel();
   if(chan == nullptr) {
      return 1;
   }
   return chan->GetTimeStampUnit();
}

EPpgPattern TDetectorHit::GetPPGStatus() const
{
   if(IsPPGSet()) {
      return fPPGStatus;
   }

   if(TPPG::Get() == nullptr) {
      return EPpgPattern::kJunk;
   }

   fPPGStatus      = TPPG::Get()->GetStatus(GetTimeStampNs());
   fCycleTimeStamp = GetTimeStampNs() - static_cast<Long64_t>(TPPG::Get()->GetLastStatusTime(GetTimeStampNs()));
   SetHitBit(EBitFlag::kIsPPGSet, true);
   return fPPGStatus;
}

Long64_t TDetectorHit::GetCycleTimeStamp() const
{
   if(IsPPGSet()) {
      return fCycleTimeStamp;
   }

   if(TPPG::Get() == nullptr) {
      return 0;
   }

   fPPGStatus      = TPPG::Get()->GetStatus(GetTimeStampNs());
   fCycleTimeStamp = GetTimeStampNs() - static_cast<Long64_t>(TPPG::Get()->GetLastStatusTime(GetTimeStampNs()));
   SetHitBit(EBitFlag::kIsPPGSet, true);
   return fCycleTimeStamp;
}

double TDetectorHit::GetTimeSinceTapeMove() const
{
   /// returns time in ns, minus the time of the last tape move
   return GetTime() - static_cast<double>(TPPG::Get()->GetLastStatusTime(GetTimeStampNs(), EPpgPattern::kTapeMove));
}

// const here is rather dirty
void TDetectorHit::SetHitBit(enum EBitFlag flag, Bool_t set) const
{
   fBitFlags.SetBit(flag, set);
}
