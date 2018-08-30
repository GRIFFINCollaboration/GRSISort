#ifndef TTDRTIGRESSBGOHIT_H
#define TTDRTIGRESSBGOHIT_H

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

class TTdrTigressBgoHit : public TBgoHit {
public:
   TTdrTigressBgoHit();
   TTdrTigressBgoHit(const TTdrTigressBgoHit& hit) : TBgoHit(static_cast<const TBgoHit&>(hit)) {}
   TTdrTigressBgoHit(const TFragment& frag) : TBgoHit(frag) {}
   ~TTdrTigressBgoHit() override;

   /////////////////////////		/////////////////////////////////////
   inline UShort_t GetArrayNumber() const override { return (4 * (GetDetector() - 1) + GetCrystal() + 1); } //!<! each crystal has one single signal (OR of all segments)

   /// \cond CLASSIMP
   ClassDefOverride(TTdrTigressBgoHit, 1)
   /// \endcond
};
/*! @} */
#endif
