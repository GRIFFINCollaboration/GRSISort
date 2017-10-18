#include "TGenericDet.h"
#include "TMnemonic.h"

#include "TClass.h"
#include <cmath>
#include "TMath.h"

#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TGenericDet)
/// \endcond


TGenericDet::TGenericDet()
{
   Clear();
}

TGenericDet::~TGenericDet() = default;

TGenericDet& TGenericDet::operator=(const TGenericDet& rhs)
{
   rhs.Copy(*this);
   return *this;
}

TGenericDet::TGenericDet(const TGenericDet& rhs) : TGRSIDetector()
{
   rhs.Copy(*this);
}

void TGenericDet::Copy(TObject& rhs) const
{
   TGRSIDetector::Copy(rhs);
   static_cast<TGenericDet&>(rhs).fGenericHits   = fGenericHits;
   static_cast<TGenericDet&>(rhs).fTimeStamp = fTimeStamp;
   static_cast<TGenericDet&>(rhs).fEnergies = fEnergies;
   static_cast<TGenericDet&>(rhs).fEnergy = fEnergy;
   
 // if(fEnergies.size())std::cout<<std::endl<<"COPY "<<fGenericHits[0].GetEnergy();
}

void TGenericDet::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	
   /// This function creates TGenericDetHits for each fragment and stores them in separate front and back vectors
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   TGRSIDetectorHit dethit(*frag);
   fTimeStamp = frag->GetTimeStamp();

   fGenericHits.push_back(std::move(dethit));
   
   fEnergy=frag->GetEnergy();
   fEnergies.push_back(frag->GetEnergy());
   
   //std::cout<<std::endl<<fGenericHits[0].GetEnergy();
}

TGRSIDetectorHit* TGenericDet::GetHit(const int& idx)
{
   return GetGenericDetHit(idx);
}

TGRSIDetectorHit* TGenericDet::GetGenericDetHit(const int& i)
{
   if(i < GetMultiplicity()) {
      return &fGenericHits.at(i);
   }
   std::cerr<<"Generic Detector hits are out of range"<<std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

void TGenericDet::Print(Option_t*) const
{
   printf("%s\tnot yet written.\n", __PRETTY_FUNCTION__);
}

void TGenericDet::Clear(Option_t* opt)
{
   TGRSIDetector::Clear(opt);
   fGenericHits.clear();
   fEnergies.clear();
}
