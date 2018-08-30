#include "TTdrSiLi.h"
#include "TTdrSiLiHit.h"
#include "Globals.h"

/// \cond CLASSIMP
ClassImp(TTdrSiLiHit)
/// \endcond

TTdrSiLiHit::TTdrSiLiHit()
   : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TTdrSiLiHit::TTdrSiLiHit(const TTdrSiLiHit& rhs) : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

TTdrSiLiHit::~TTdrSiLiHit() = default;

void TTdrSiLiHit::Copy(TObject& rhs) const
{
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TTdrSiLiHit&>(rhs).fFilter = fFilter;
}

bool TTdrSiLiHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TTdrSiLiHit::Clear(Option_t* opt)
{
   TGRSIDetectorHit::Clear(opt); // clears the base (address, position and waveform)
   fFilter = 0;
}

void TTdrSiLiHit::Print(Option_t*) const
{
   printf("TdrSiLi Detector: %i\n", GetDetector());
   printf("TdrSiLi Energy:   %lf\n", GetEnergy());
   printf("TdrSiLi hit time:   %f\n", GetTime());
}

TVector3 TTdrSiLiHit::GetPosition(Double_t) const
{
   return TTdrSiLi::GetPosition(GetDetector());
}

TVector3 TTdrSiLiHit::GetPosition() const
{
   return GetPosition(GetDefaultDistance());
}
