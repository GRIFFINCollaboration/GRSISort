#ifndef TTDRCLOVERBGOHIT_H
#define TTDRCLOVERBGOHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>
#if !defined(__CINT__) && !defined(__CLING__)
#include <tuple>
#endif

#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TPulseAnalyzer.h"

#include "TBgoHit.h"

class TTdrCloverBgoHit : public TBgoHit {
public:
   TTdrCloverBgoHit();
   TTdrCloverBgoHit(const TTdrCloverBgoHit& hit) : TBgoHit(static_cast<const TBgoHit&>(hit)) {}
   TTdrCloverBgoHit(const TFragment& frag) : TBgoHit(frag) {}
   ~TTdrCloverBgoHit() override;

   /////////////////////////		/////////////////////////////////////
   inline UShort_t GetArrayNumber() const override { return (4 * (GetDetector() - 1) + GetCrystal() + 1); } //!<! each crystal has one single signal (OR of all segments)

   /// \cond CLASSIMP
   ClassDefOverride(TTdrCloverBgoHit, 1)
   /// \endcond
};
/*! @} */
#endif
