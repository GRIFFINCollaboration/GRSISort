
#include "TGriffin.h"
#include "TGriffinHit.h"
#include "Globals.h"
#include <cmath>
#include <iostream>

/// \cond CLASSIMP
ClassImp(TGriffinHit)
/// \endcond

TGriffinHit::TGriffinHit()
   : TGRSIDetectorHit()
{
// Default Ctor. Ignores TObject Streamer in ROOT < 6.
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TGriffinHit::TGriffinHit(const TGriffinHit& rhs) : TGRSIDetectorHit()
{
   // Copy Ctor. Ignores TObject Streamer in ROOT < 6.
   Clear();
   rhs.Copy(*this);
}

TGriffinHit::TGriffinHit(const TFragment& frag) : TGRSIDetectorHit(frag)
{
   SetNPileUps(frag.GetNumberOfPileups());
}

TGriffinHit::~TGriffinHit() = default;

void TGriffinHit::Copy(TObject& rhs) const
{
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TGriffinHit&>(rhs).fFilter = fFilter;
   // We should copy over a 0 and let the hit recalculate, this is safest
   static_cast<TGriffinHit&>(rhs).fGriffinHitBits      = 0;
   static_cast<TGriffinHit&>(rhs).fPPG                 = fPPG;
   static_cast<TGriffinHit&>(rhs).fBremSuppressed_flag = fBremSuppressed_flag;
}

void TGriffinHit::Copy(TObject& obj, bool waveform) const
{
   Copy(obj);
   if(waveform) {
      CopyWave(obj);
   }
}

bool TGriffinHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TGriffinHit::Clear(Option_t* opt)
{
   // Clears the information stored in the TGriffinHit.
   TGRSIDetectorHit::Clear(opt); // clears the base (address, position and waveform)
   fFilter              = 0;
   fGriffinHitBits      = 0;
   fPPG                 = nullptr;
   fBremSuppressed_flag = false;
}

void TGriffinHit::Print(Option_t*) const
{
   // Prints the Detector Number, Crystal Number, Energy, Time and Angle.
   printf("Griffin Detector: %i\n", GetDetector());
   printf("Griffin Crystal:  %i\n", GetCrystal());
   printf("Griffin Energy:   %lf\n", GetEnergy());
   printf("Griffin hit time:   %lf\n", GetTime());
   printf("Griffin hit TV3 theta: %.2f\tphi%.2f\n", GetPosition().Theta() * 180 / (3.141597),
          GetPosition().Phi() * 180 / (3.141597));
}

TVector3 TGriffinHit::GetPosition(double dist) const
{
   return TGriffin::GetPosition(GetDetector(), GetCrystal(), dist);
}

TVector3 TGriffinHit::GetPosition() const
{
   return GetPosition(GetDefaultDistance());
}

bool TGriffinHit::CompareEnergy(const TGriffinHit* lhs, const TGriffinHit* rhs)
{
   return (lhs->GetEnergy() > rhs->GetEnergy());
}

void TGriffinHit::Add(const TGRSIDetectorHit* hit)
{
	const TGriffinHit* griffinHit = dynamic_cast<const TGriffinHit*>(hit);
	if(griffinHit == nullptr) {
		throw std::runtime_error("trying to add non-griffin hit to griffin hit!");
	}
   // add another griffin hit to this one (for addback),
   // using the time and position information of the one with the higher energy
   if(!CompareEnergy(this, griffinHit)) {
      SetCfd(griffinHit->GetCfd());
      SetTime(griffinHit->GetTime());
      // SetPosition(griffinHit->GetPosition());
      SetAddress(griffinHit->GetAddress());
   } else {
      SetTime(GetTime());
   }
   SetEnergy(GetEnergy() + griffinHit->GetEnergy());
   // this has to be done at the very end, otherwise GetEnergy() might not work
   SetCharge(0);
   // Add all of the pileups.This should be changed when the max number of pileups changes
   if((NPileUps() + griffinHit->NPileUps()) < 4) {
      SetNPileUps(NPileUps() + griffinHit->NPileUps());
   } else {
      SetNPileUps(3);
   }
   if((PUHit() + griffinHit->PUHit()) < 4) {
      SetPUHit(PUHit() + griffinHit->PUHit());
   } else {
      SetPUHit(3);
   }
   // KValue is somewhate meaningless in addback, so I am using it as an indicator that a piledup griffinHit was added-back RD
   if(GetKValue() > griffinHit->GetKValue()) {
      SetKValue(griffinHit->GetKValue());
   }
}

void TGriffinHit::SetGriffinFlag(enum EGriffinHitBits flag, Bool_t set)
{
   fGriffinHitBits.SetBit(flag, set);
}

UShort_t TGriffinHit::NPileUps() const
{
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fGriffinHitBits.TestBits(EGriffinHitBits::kTotalPU1) + fGriffinHitBits.TestBits(EGriffinHitBits::kTotalPU2));
}

UShort_t TGriffinHit::PUHit() const
{
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fGriffinHitBits.TestBits(EGriffinHitBits::kPUHit1) +
                               (fGriffinHitBits.TestBits(EGriffinHitBits::kPUHit2) >> static_cast<std::underlying_type<EGriffinHitBits>::type>(EGriffinHitBits::kPUHitOffset)));
}

void TGriffinHit::SetNPileUps(UChar_t npileups)
{
   SetGriffinFlag(EGriffinHitBits::kTotalPU1, (npileups & static_cast<std::underlying_type<EGriffinHitBits>::type>(EGriffinHitBits::kTotalPU1)) != 0);
   SetGriffinFlag(EGriffinHitBits::kTotalPU2, (npileups & static_cast<std::underlying_type<EGriffinHitBits>::type>(EGriffinHitBits::kTotalPU2)) != 0);
}

void TGriffinHit::SetPUHit(UChar_t puhit)
{
   if(puhit > 2) {
      puhit = 3;
   }
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.

   SetGriffinFlag(EGriffinHitBits::kPUHit1, ((puhit<<static_cast<std::underlying_type<EGriffinHitBits>::type>(EGriffinHitBits::kPUHitOffset)) & static_cast<std::underlying_type<EGriffinHitBits>::type>(EGriffinHitBits::kPUHit1)) != 0);
   SetGriffinFlag(EGriffinHitBits::kPUHit2, ((puhit<<static_cast<std::underlying_type<EGriffinHitBits>::type>(EGriffinHitBits::kPUHitOffset)) & static_cast<std::underlying_type<EGriffinHitBits>::type>(EGriffinHitBits::kPUHit2)) != 0);
}

Double_t TGriffinHit::GetNoCTEnergy(Option_t*) const
{
   TChannel* chan = GetChannel();
   if(chan == nullptr) {
      Error("GetEnergy", "No TChannel exists for address 0x%08x", GetAddress());
      return 0.;
   }
   return chan->CalibrateENG(Charge(), GetKValue());
}

Double_t TGriffinHit::GetEnergyNonlinearity(double energy) const
{
   // return 0.0;
   return -(TGriffin::GetEnergyNonlinearity(GetArrayNumber(), energy));
}
