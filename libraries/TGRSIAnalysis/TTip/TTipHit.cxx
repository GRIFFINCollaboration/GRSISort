#include "TTip.h"
#include "TTipHit.h"
#include "TGRSIOptions.h"

////////////////////////////////////////////////////////////
//
// TTipHit
//
// The TTipHit class defines the observables and algorithms used
// when analyzing TIP hits. It includes detector positions, etc.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TTipHit)
/// \endcond

TTipHit::TTipHit()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TTipHit::TTipHit(const TFragment& frag) : TGRSIDetectorHit(frag)
{
   // SetVariables(frag);
   if(TGRSIOptions::AnalysisOptions()->IsWaveformFitting() && !IsCsI()) {
      SetWavefit(frag);
   } else if(TGRSIOptions::AnalysisOptions()->IsWaveformFitting() && IsCsI()) {
      SetPID(frag);
   }
}

TTipHit::~TTipHit() = default;

TTipHit::TTipHit(const TTipHit& rhs) : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

void TTipHit::Copy(TObject& rhs) const
{
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TTipHit&>(rhs).fFilter     = fFilter;
   static_cast<TTipHit&>(rhs).fPID        = fPID;
   static_cast<TTipHit&>(rhs).fTipChannel = fTipChannel;
   static_cast<TTipHit&>(rhs).fTimeFit    = fTimeFit;
   static_cast<TTipHit&>(rhs).fSig2Noise  = fSig2Noise;
   static_cast<TTipHit&>(rhs).fChiSq      = fChiSq;
}

bool TTipHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TTipHit::Clear(Option_t*)
{
   fFilter     = 0;
   fPID        = 0;
   fTipChannel = 0;
   fTimeFit    = 0;
}

void TTipHit::Print(Option_t*) const
{
   printf("Tip Detector: %i\n", GetDetector());
   printf("Tip hit energy: %.2f\n", GetEnergy());
   printf("Tip hit time:   %.f\n", GetTime());
}

void TTipHit::SetWavefit(const TFragment& frag)
{
   TPulseAnalyzer pulse(frag);
   if(pulse.IsSet()) {
      fTimeFit   = pulse.fit_newT0();
      fSig2Noise = pulse.get_sig2noise();
   }
}

void TTipHit::SetPID(const TFragment& frag)
{
   TPulseAnalyzer pulse(frag);
   if(pulse.IsSet()) {
      fPID     = pulse.CsIPID();
      fTimeFit = pulse.CsIt0();
      fChiSq   = pulse.GetCsIChiSq();
   }
}
