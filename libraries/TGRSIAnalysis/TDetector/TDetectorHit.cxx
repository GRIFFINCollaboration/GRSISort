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

   //TChannel* channel = GetChannel();
   //if(channel == nullptr) {
   //   Error("GetTime", "No TChannel exists for address 0x%08x", GetAddress());
      return SetTime(10. * (static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform())));
   //}
   //switch(channel->GetDigitizerType()) {
	//	Double_t dTime;
	//	case TMnemonic::EDigitizer::kGRF16:
	//	dTime = (GetTimeStamp() & (~0x3ffff)) * 10. +
	//	channel->CalibrateCFD((GetCfd() + gRandom->Uniform()) / 1.6); // CFD is in 10/16th of a nanosecond
	//	return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
	//	case TMnemonic::EDigitizer::kGRF4G:
	//	dTime = GetTimeStamp() * 10. + channel->CalibrateCFD((fCfd >> 22) + ((fCfd & 0x3fffff) + gRandom->Uniform()) / 256.);
	//	return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
	//	case TMnemonic::EDigitizer::kTIG10:
	//	dTime = (GetTimeStamp() & (~0x7fffff)) * 10. +
	//	channel->CalibrateCFD((GetCfd() + gRandom->Uniform()) / 1.6); // CFD is in 10/16th of a nanosecond
	//	//channel->CalibrateCFD((GetCfd() & (~0xf) + gRandom->Uniform()) / 1.6); // PBender suggests this.
	//	return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
   //   case TMnemonic::EDigitizer::kCaen:
   //   //10 bit CFD for 0-2ns => divide by 512
   //   dTime = GetTimeStamp() * 10. + channel->CalibrateCFD((GetCfd() + gRandom->Uniform()) / 512.);
   //   return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
	//	case TMnemonic::EDigitizer::kPixie:
	//	dTime = GetTimeStamp() * 10. + channel->CalibrateCFD(fCfd/3276.8);// CFD is reported as 15bit interpolation of 10 ns
	//	return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
	//	case TMnemonic::EDigitizer::kFastPixie:
	//	dTime = GetTimeStamp() * 10. + channel->CalibrateCFD(fCfd/6553.6);// CFD is reported as 16bit interpolation of 10 ns
	//	return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
	//	default:
	//	dTime = static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform());
	//	return SetTime(10. * (dTime - channel->GetTZero(GetEnergy())));
	//}
   //return 0.;
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
      // Error("GetEnergy","No TChannel exists for address 0x%08x",GetAddress());
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
	printf("==== %s @ 0x%p\n ====", ClassName(), (void*)this);
	printf("\t%s\n", GetName());
	printf("\tCharge: %.2f\n", Charge());
	printf("\tTime:   %.2f\n", GetTime());
	std::cout<<"\tTime:   "<<GetTimeStamp()<<"\n";
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
	// fPosition.SetXYZ(0,0,1);  // unit vector along the beam.
	fWaveform.clear(); // reset size to zero.
	fCharge    = 0;
	fKValue    = 0;
	fCfd       = -1;
	fTimeStamp = 0;
	// fDetector       = -1;
	// fSegment        = -1;
	fEnergy         = 0.;
	fBitFlags       = 0;
	fPPGStatus      = EPpgPattern::kJunk;
	fCycleTimeStamp = 0;
	fChannel        = nullptr;
}

Int_t TDetectorHit::GetDetector() const
{
	TChannel* channel = GetChannel();
	if(channel == nullptr) {
		Error("GetDetector", "No TChannel exists for address 0x%08x", GetAddress());
		return -1;
	}
	return channel->GetDetectorNumber(); // mnemonic.arrayposition;
}

Int_t TDetectorHit::GetSegment() const
{
	TChannel* channel = GetChannel();
	if(channel == nullptr) {
		Error("GetSegment", "No TChannel exists for address %08x", GetAddress());
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

UShort_t TDetectorHit::GetArrayNumber() const
{
	TChannel* channel = GetChannel();
	if(channel != nullptr) {
		return (GetDetector() - 1) * 4 + GetCrystal();
	}
	return -1;
}

bool TDetectorHit::CompareEnergy(TDetectorHit* lhs, TDetectorHit* rhs)
{
	return (lhs->GetEnergy() > rhs->GetEnergy());
}

Long64_t TDetectorHit::GetTimeStamp(Option_t*) const
{
	TChannel* tmpChan = GetChannel();
	if(tmpChan == nullptr) {
		return fTimeStamp;
	}
	return fTimeStamp - tmpChan->GetTimeOffset();
}

EPpgPattern TDetectorHit::GetPPGStatus() const
{
	if(IsPPGSet()) {
		return fPPGStatus;
	}

	if(TPPG::Get() == nullptr) {
		return EPpgPattern::kJunk;
	}

	fPPGStatus      = TPPG::Get()->GetStatus(GetTimeStamp());
	fCycleTimeStamp = GetTimeStamp() - TPPG::Get()->GetLastStatusTime(GetTimeStamp());
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

	fPPGStatus      = TPPG::Get()->GetStatus(GetTimeStamp());
	fCycleTimeStamp = GetTimeStamp() - TPPG::Get()->GetLastStatusTime(GetTimeStamp());
	SetHitBit(EBitFlag::kIsPPGSet, true);
	return fCycleTimeStamp;
}

double TDetectorHit::GetTimeSinceTapeMove() const
{
	/// returns time in ns, minus the time of the last tape move
	return GetTime() - 10.*TPPG::Get()->GetLastStatusTime(GetTimeStamp(), EPpgPattern::kTapeMove);
}

// const here is rather dirty
void TDetectorHit::SetHitBit(enum EBitFlag flag, Bool_t set) const
{
	fBitFlags.SetBit(flag, set);
}
