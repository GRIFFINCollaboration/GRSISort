#include "TDetectorHit.h"

#include <iostream>

#include "TClass.h"

/// \cond CLASSIMP
ClassImp(TDetectorHit)
/// \endcond

TPPG* TDetectorHit::fPPG = nullptr;

TVector3 TDetectorHit::fBeamDirection(0, 0, 1);

TDetectorHit::TDetectorHit(const int& Address) : TObject()
{
   /// Default constructor
   Clear();
   fAddress = Address;

#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TDetectorHit::TDetectorHit(const TDetectorHit& rhs, bool copywave) : TObject(rhs)
{
   /// Default Copy constructor
   rhs.Copy(*this);
   if(copywave) {
      rhs.CopyWave(*this);
   }
   ClearTransients();

#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TDetectorHit::~TDetectorHit()
{
   // Default destructor
}

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
		return SetTime(static_cast<Double_t>(((GetTimeStamp()) + gRandom->Uniform()) * GetTimeStampUnit()));
	}

	return SetTime(static_cast<Double_t>(((GetTimeStamp()) + gRandom->Uniform()) * GetTimeStampUnit() - tmpChan->GetTimeOffset()));
}

Float_t TDetectorHit::GetCharge() const
{
   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      return Charge();
   }
   if(fKValue > 0 && !channel->UseCalFileIntegration()) {
      return Charge() / (static_cast<Float_t>(fKValue)); // this will use the integration value
   }
   if(channel->UseCalFileIntegration()) {
      return Charge() / (static_cast<Float_t>(channel->GetIntegration())); // this will use the integration value
   }                                                                       // in the TChannel if it exists.
   return Charge();                                                        // this will use no integration value
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
                       GetEnergyNonlinearity(energy)); // this will use the integration value
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

void TDetectorHit::Print(Option_t*) const
{
   /// General print statement for a TDetectorHit.
   /// Currently prints nothing.
	printf("==== %s @ 0x%p ====\n", ClassName(), (void*)this);
	printf("\t%s\n", GetName());
	printf("\tCharge:    %.2f\n", Charge());
	printf("\tTime:      %.2f\n", GetTime());
	std::cout<<"\tTimestamp: "<<GetTimeStamp()<<" in "<<GetTimeStampUnit()<<" ns = "<<GetTimeStampNs()<<"\n";
	printf("============================\n");
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
	fAddress = 0xffffffff; // -1
	fCharge    = 0;
	fKValue    = 0;
	fCfd       = -1;
	fTimeStamp = 0;
	fWaveform.clear(); // reset size to zero.
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
	return channel->GetDetectorNumber(); // mnemonic.arrayposition;
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

Long64_t TDetectorHit::GetTimeStampNs(Option_t*) const
{
	TChannel* tmpChan = GetChannel();
	if(tmpChan == nullptr) {
		return fTimeStamp * GetTimeStampUnit();
	}
	return fTimeStamp * GetTimeStampUnit() - tmpChan->GetTimeOffset();
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
	fCycleTimeStamp = GetTimeStampNs() - TPPG::Get()->GetLastStatusTime(GetTimeStampNs());
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
	fCycleTimeStamp = GetTimeStampNs() - TPPG::Get()->GetLastStatusTime(GetTimeStampNs());
	SetHitBit(EBitFlag::kIsPPGSet, true);
	return fCycleTimeStamp;
}

double TDetectorHit::GetTimeSinceTapeMove() const
{
	/// returns time in ns, minus the time of the last tape move
	return GetTime() - TPPG::Get()->GetLastStatusTime(GetTimeStampNs(), EPpgPattern::kTapeMove);
}

// const here is rather dirty
void TDetectorHit::SetHitBit(enum EBitFlag flag, Bool_t set) const
{
	fBitFlags.SetBit(flag, set);
}
