#include "TGenericDetector.h"
#include "TMnemonic.h"

#include "TClass.h"
#include <cmath>
#include "TMath.h"

#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TGenericDetector)
/// \endcond


TGenericDetector::TGenericDetector()
{
   Clear();
}

TGenericDetector::~TGenericDetector() = default;

TGenericDetector& TGenericDetector::operator=(const TGenericDetector& rhs)
{
   rhs.Copy(*this);
   return *this;
}

TGenericDetector::TGenericDetector(const TGenericDetector& rhs) : TGRSIDetector()
{
   rhs.Copy(*this);
}

void TGenericDetector::Copy(TObject& rhs) const
{
   TGRSIDetector::Copy(rhs);
   static_cast<TGenericDetector&>(rhs).fGenericHits   = fGenericHits;   
}

void TGenericDetector::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	
   /// This function creates TGenericDetectorHits for each fragment and stores them in separate front and back vectors
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   TGRSIDetectorHit dethit(*frag);
   fGenericHits.push_back(std::move(dethit));
}

TGRSIDetectorHit* TGenericDetector::GetHit(const int& idx)
{
   return GeTGenericDetectorHit(idx);
}

TGRSIDetectorHit* TGenericDetector::GeTGenericDetectorHit(const int& i)
{
   if(i < GetMultiplicity()) {
      return &fGenericHits.at(i);
   }
   std::cerr<<"Generic Detector hits are out of range"<<std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

void TGenericDetector::Print(Option_t*) const
{
   printf("%s\tnot yet written.\n", __PRETTY_FUNCTION__);
}

void TGenericDetector::Clear(Option_t* opt)
{
   TGRSIDetector::Clear(opt);
   fGenericHits.clear();
}
