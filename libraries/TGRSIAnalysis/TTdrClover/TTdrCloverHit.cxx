
#include "TTdrClover.h"
#include "TTdrCloverHit.h"
#include "Globals.h"
#include <cmath>
#include <iostream>

/// \cond CLASSIMP
ClassImp(TTdrCloverHit)
/// \endcond

TTdrCloverHit::TTdrCloverHit()
   : TGRSIDetectorHit()
{
// Default Ctor. Ignores TObject Streamer in ROOT < 6.
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TTdrCloverHit::TTdrCloverHit(const TTdrCloverHit& rhs) : TGRSIDetectorHit()
{
   // Copy Ctor. Ignores TObject Streamer in ROOT < 6.
   Clear();
   rhs.Copy(*this);
}

TTdrCloverHit::TTdrCloverHit(const TFragment& frag) : TGRSIDetectorHit(frag)
{
   SetNPileUps(frag.GetNumberOfPileups());
}

TTdrCloverHit::~TTdrCloverHit() = default;

void TTdrCloverHit::Copy(TObject& rhs) const
{
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TTdrCloverHit&>(rhs).fFilter = fFilter;
   // We should copy over a 0 and let the hit recalculate, this is safest
   static_cast<TTdrCloverHit&>(rhs).fTdrCloverHitBits      = 0;
   static_cast<TTdrCloverHit&>(rhs).fCrystal             = fCrystal;
   static_cast<TTdrCloverHit&>(rhs).fBremSuppressed_flag = fBremSuppressed_flag;
}

void TTdrCloverHit::Copy(TObject& obj, bool waveform) const
{
   Copy(obj);
   if(waveform) {
      CopyWave(obj);
   }
}

bool TTdrCloverHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TTdrCloverHit::Clear(Option_t* opt)
{
   // Clears the information stored in the TTdrCloverHit.
   TGRSIDetectorHit::Clear(opt); // clears the base (address, position and waveform)
   fFilter              = 0;
   fTdrCloverHitBits      = 0;
   fCrystal             = 0xFFFF;
   fBremSuppressed_flag = false;
}

void TTdrCloverHit::Print(Option_t*) const
{
   // Prints the Detector Number, Crystal Number, Energy, Time and Angle.
   printf("TdrClover Detector: %i\n", GetDetector());
   printf("TdrClover Crystal:  %i\n", GetCrystal());
   printf("TdrClover Energy:   %lf\n", GetEnergy());
   printf("TdrClover hit time:   %lf\n", GetTime());
   printf("TdrClover hit TV3 theta: %.2f\tphi%.2f\n", GetPosition().Theta() * 180 / (3.141597),
          GetPosition().Phi() * 180 / (3.141597));
}

TVector3 TTdrCloverHit::GetPosition(double dist) const
{
   return TTdrClover::GetPosition(GetDetector(), GetCrystal(), dist);
}

TVector3 TTdrCloverHit::GetPosition() const
{
   return GetPosition(GetDefaultDistance());
}

bool TTdrCloverHit::CompareEnergy(const TTdrCloverHit* lhs, const TTdrCloverHit* rhs)
{
   return (lhs->GetEnergy() > rhs->GetEnergy());
}

void TTdrCloverHit::Add(const TTdrCloverHit* hit)
{
   // add another griffin hit to this one (for addback),
   // using the time and position information of the one with the higher energy
   if(!CompareEnergy(this, hit)) {
      SetCfd(hit->GetCfd());
      SetTime(hit->GetTime());
      // SetPosition(hit->GetPosition());
      SetAddress(hit->GetAddress());
   } else {
      SetTime(GetTime());
   }
   SetEnergy(GetEnergy() + hit->GetEnergy());
   // this has to be done at the very end, otherwise GetEnergy() might not work
   SetCharge(0);
   // Add all of the pileups.This should be changed when the max number of pileups changes
   if((NPileUps() + hit->NPileUps()) < 4) {
      SetNPileUps(NPileUps() + hit->NPileUps());
   } else {
      SetNPileUps(3);
   }
   if((PUHit() + hit->PUHit()) < 4) {
      SetPUHit(PUHit() + hit->PUHit());
   } else {
      SetPUHit(3);
   }
   // KValue is somewhate meaningless in addback, so I am using it as an indicator that a piledup hit was added-back RD
   if(GetKValue() > hit->GetKValue()) {
      SetKValue(hit->GetKValue());
   }
}

void TTdrCloverHit::SetTdrCloverFlag(enum ETdrCloverHitBits flag, Bool_t set)
{
   fTdrCloverHitBits.SetBit(flag, set);
}

UShort_t TTdrCloverHit::NPileUps() const
{
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fTdrCloverHitBits.TestBits(ETdrCloverHitBits::kTotalPU1) + fTdrCloverHitBits.TestBits(ETdrCloverHitBits::kTotalPU2));
}

UShort_t TTdrCloverHit::PUHit() const
{
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fTdrCloverHitBits.TestBits(ETdrCloverHitBits::kPUHit1) +
                               (fTdrCloverHitBits.TestBits(ETdrCloverHitBits::kPUHit2) >> static_cast<std::underlying_type<ETdrCloverHitBits>::type>(ETdrCloverHitBits::kPUHitOffset)));
}

void TTdrCloverHit::SetNPileUps(UChar_t npileups)
{
   SetTdrCloverFlag(ETdrCloverHitBits::kTotalPU1, (npileups & static_cast<std::underlying_type<ETdrCloverHitBits>::type>(ETdrCloverHitBits::kTotalPU1)) != 0);
   SetTdrCloverFlag(ETdrCloverHitBits::kTotalPU2, (npileups & static_cast<std::underlying_type<ETdrCloverHitBits>::type>(ETdrCloverHitBits::kTotalPU2)) != 0);
}

void TTdrCloverHit::SetPUHit(UChar_t puhit)
{
   if(puhit > 2) {
      puhit = 3;
   }
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.

   SetTdrCloverFlag(ETdrCloverHitBits::kPUHit1, ((puhit<<static_cast<std::underlying_type<ETdrCloverHitBits>::type>(ETdrCloverHitBits::kPUHitOffset)) & static_cast<std::underlying_type<ETdrCloverHitBits>::type>(ETdrCloverHitBits::kPUHit1)) != 0);
   SetTdrCloverFlag(ETdrCloverHitBits::kPUHit2, ((puhit<<static_cast<std::underlying_type<ETdrCloverHitBits>::type>(ETdrCloverHitBits::kPUHitOffset)) & static_cast<std::underlying_type<ETdrCloverHitBits>::type>(ETdrCloverHitBits::kPUHit2)) != 0);
}

