#ifndef TTIP_H
#define TTIP_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TTip
///
/// The TTip class defines the observables and algorithms used
/// when analyzing TIP data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>
#include <iostream>
#include <set>
#include <cstdio>

#include "TObject.h"
#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TTipHit.h"

class TTip : public TGRSIDetector {
public:
   ~TTip() override;
   TTip();
   TTip(const TTip& rhs);

   TTipHit* GetTipHit(const int& i); //!<!
   TGRSIDetectorHit* GetHit(const int& i);
   Short_t GetMultiplicity() const { return fTipHits.size(); } //!<!

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
   void Copy(TObject& rhs) const override;

   void ClearTransients() override
   {
      for(const auto& hit : fTipHits) {
         hit.ClearTransients();
      }
   }

   TTip& operator=(const TTip&); //!<!

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

private:
   std::vector<TTipHit> fTipHits; //   The set of detector hits

public:
   /// \cond CLASSIMP
   ClassDefOverride(TTip, 2);
   /// \endcond
};
/*! @} */
#endif
