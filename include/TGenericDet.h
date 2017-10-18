#ifndef TGENERICDET_H
#define TGENERICDET_H

/** \addtogroup Detectors
 *  @{
 */

#include <iostream>
#include <TRandom2.h>

#include "TGRSIDetector.h"
#include "TChannel.h"

class TGenericDet : public TGRSIDetector {
public:

   TGenericDet();
   TGenericDet(const TGenericDet&);
   ~TGenericDet() override;

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif

   TGRSIDetectorHit* GetHit(const int& idx = 0) override;
   TGRSIDetectorHit* GetGenericDetHit(const int& i);

   Short_t GetMultiplicity() const override { return fGenericHits.size(); }

   void ClearTransients() override
   {
      for(const auto& hit : fGenericHits) {
         hit.ClearTransients();
      }
   }

   void Copy(TObject&) const override;
   TGenericDet& operator=(const TGenericDet&);                    //
   void Clear(Option_t* opt = "all") override;    //!<!
   void Print(Option_t* opt = "") const override; //!<!

private:
   std::vector<TGRSIDetectorHit> fGenericHits; //!<!
   std::vector<double> fEnergies; //!<!
   double fEnergy;
   Long_t fTimeStamp;

   void  ClearStatus() {  }

   /// \cond CLASSIMP
   ClassDefOverride(TGenericDet, 1)
   /// \endcond
};
/*! @} */
#endif
