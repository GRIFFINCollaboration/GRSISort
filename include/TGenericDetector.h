#ifndef TGENERICDET_H
#define TGENERICDET_H

/** \addtogroup Detectors
 *  @{
 */

#include <iostream>

#include "TGRSIDetector.h"
#include "TChannel.h"

class TGenericDetector : public TGRSIDetector {
public:

   TGenericDetector();
   TGenericDetector(const TGenericDetector&);
   ~TGenericDetector() override;

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif

   TGRSIDetectorHit* GetHit(const int& idx = 0) override;
   TGRSIDetectorHit* GeTGenericDetectorHit(const int& i);

   Short_t GetMultiplicity() const override { return fGenericHits.size(); }

   void ClearTransients() override
   {
      for(const auto& hit : fGenericHits) {
         hit.ClearTransients();
      }
   }

   void Copy(TObject&) const override;
   TGenericDetector& operator=(const TGenericDetector&);                    //
   void Clear(Option_t* opt = "all") override;    //!<!
   void Print(Option_t* opt = "") const override; //!<!

private:
   std::vector<TGRSIDetectorHit> fGenericHits;
   void  ClearStatus() {  }

   /// \cond CLASSIMP
   ClassDefOverride(TGenericDetector, 1)
   /// \endcond
};
/*! @} */
#endif
