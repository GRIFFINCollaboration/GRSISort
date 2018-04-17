
#include "TTdrTigress.h"
#include "TTdrTigressHit.h"
#include "Globals.h"
#include <cmath>
#include <iostream>

/// \cond CLASSIMP
ClassImp(TTdrTigressHit)
/// \endcond

TTdrTigressHit::TTdrTigressHit()
   : TGRSIDetectorHit()
{
// Default Ctor. Ignores TObject Streamer in ROOT < 6.
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TTdrTigressHit::TTdrTigressHit(const TTdrTigressHit& rhs) : TGRSIDetectorHit()
{
   // Copy Ctor. Ignores TObject Streamer in ROOT < 6.
   Clear();
   rhs.Copy(*this);
}

TTdrTigressHit::TTdrTigressHit(const TFragment& frag) : TGRSIDetectorHit(frag)
{
   SetNPileUps(frag.GetNumberOfPileups());
}

TTdrTigressHit::~TTdrTigressHit() = default;

void TTdrTigressHit::Copy(TObject& rhs) const
{
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TTdrTigressHit&>(rhs).fFilter = fFilter;
   // We should copy over a 0 and let the hit recalculate, this is safest
   static_cast<TTdrTigressHit&>(rhs).fTdrTigressHitBits      = 0;
   static_cast<TTdrTigressHit&>(rhs).fCrystal             = fCrystal;
   static_cast<TTdrTigressHit&>(rhs).fBremSuppressed_flag = fBremSuppressed_flag;
}

void TTdrTigressHit::Copy(TObject& obj, bool waveform) const
{
   Copy(obj);
   if(waveform) {
      CopyWave(obj);
   }
}

bool TTdrTigressHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TTdrTigressHit::Clear(Option_t* opt)
{
   // Clears the information stored in the TTdrTigressHit.
   TGRSIDetectorHit::Clear(opt); // clears the base (address, position and waveform)
   fFilter              = 0;
   fTdrTigressHitBits      = 0;
   fCrystal             = 0xFFFF;
   fBremSuppressed_flag = false;
}

void TTdrTigressHit::Print(Option_t*) const
{
   // Prints the Detector Number, Crystal Number, Energy, Time and Angle.
   printf("TdrTigress Detector: %i\n", GetDetector());
   printf("TdrTigress Crystal:  %i\n", GetCrystal());
   printf("TdrTigress Energy:   %lf\n", GetEnergy());
   printf("TdrTigress hit time:   %lf\n", GetTime());
   printf("TdrTigress hit TV3 theta: %.2f\tphi%.2f\n", GetPosition().Theta() * 180 / (3.141597),
          GetPosition().Phi() * 180 / (3.141597));
}

TVector3 TTdrTigressHit::GetPosition(double dist) const
{
   return TTdrTigress::GetPosition(GetDetector(), GetCrystal(), dist);
}

TVector3 TTdrTigressHit::GetPosition() const
{
   return GetPosition(GetDefaultDistance());
}

bool TTdrTigressHit::CompareEnergy(const TTdrTigressHit* lhs, const TTdrTigressHit* rhs)
{
   return (lhs->GetEnergy() > rhs->GetEnergy());
}

void TTdrTigressHit::Add(const TTdrTigressHit* hit)
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

void TTdrTigressHit::SetTdrTigressFlag(enum ETdrTigressHitBits flag, Bool_t set)
{
   fTdrTigressHitBits.SetBit(flag, set);
}

UShort_t TTdrTigressHit::NPileUps() const
{
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fTdrTigressHitBits.TestBits(ETdrTigressHitBits::kTotalPU1) + fTdrTigressHitBits.TestBits(ETdrTigressHitBits::kTotalPU2));
}

UShort_t TTdrTigressHit::PUHit() const
{
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fTdrTigressHitBits.TestBits(ETdrTigressHitBits::kPUHit1) +
                               (fTdrTigressHitBits.TestBits(ETdrTigressHitBits::kPUHit2) >> static_cast<std::underlying_type<ETdrTigressHitBits>::type>(ETdrTigressHitBits::kPUHitOffset)));
}

void TTdrTigressHit::SetNPileUps(UChar_t npileups)
{
   SetTdrTigressFlag(ETdrTigressHitBits::kTotalPU1, (npileups & static_cast<std::underlying_type<ETdrTigressHitBits>::type>(ETdrTigressHitBits::kTotalPU1)) != 0);
   SetTdrTigressFlag(ETdrTigressHitBits::kTotalPU2, (npileups & static_cast<std::underlying_type<ETdrTigressHitBits>::type>(ETdrTigressHitBits::kTotalPU2)) != 0);
}

void TTdrTigressHit::SetPUHit(UChar_t puhit)
{
   if(puhit > 2) {
      puhit = 3;
   }
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.

   SetTdrTigressFlag(ETdrTigressHitBits::kPUHit1, ((puhit<<static_cast<std::underlying_type<ETdrTigressHitBits>::type>(ETdrTigressHitBits::kPUHitOffset)) & static_cast<std::underlying_type<ETdrTigressHitBits>::type>(ETdrTigressHitBits::kPUHit1)) != 0);
   SetTdrTigressFlag(ETdrTigressHitBits::kPUHit2, ((puhit<<static_cast<std::underlying_type<ETdrTigressHitBits>::type>(ETdrTigressHitBits::kPUHitOffset)) & static_cast<std::underlying_type<ETdrTigressHitBits>::type>(ETdrTigressHitBits::kPUHit2)) != 0);
}

