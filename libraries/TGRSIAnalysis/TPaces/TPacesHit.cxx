#include "TPaces.h"
#include "TPacesHit.h"
#include "Globals.h"

/// \cond CLASSIMP
ClassImp(TPacesHit)
/// \endcond

TPacesHit::TPacesHit()
   : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TPacesHit::TPacesHit(const TPacesHit& rhs) : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

TPacesHit::~TPacesHit() = default;

void TPacesHit::Copy(TObject& rhs) const
{
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TPacesHit&>(rhs).fFilter = fFilter;
}

bool TPacesHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TPacesHit::Clear(Option_t* opt)
{
   TGRSIDetectorHit::Clear(opt); // clears the base (address, position and waveform)
   fFilter = 0;
}

void TPacesHit::Print(Option_t*) const
{
   printf("Paces Detector: %i\n", GetDetector());
   printf("Paces Energy:   %lf\n", GetEnergy());
   printf("Paces hit time:   %f\n", GetTime());
}

TVector3 TPacesHit::GetPosition(Double_t) const
{
   return TPaces::GetPosition(GetDetector());
}

TVector3 TPacesHit::GetPosition() const
{
   return GetPosition(GetDefaultDistance());
}
