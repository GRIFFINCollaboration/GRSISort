#ifndef TBGOHIT_H
#define TBGOHIT_H

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

#include "TGRSIDetectorHit.h"

class TBgoHit : public TGRSIDetectorHit {
public:
   TBgoHit();
   TBgoHit(const TBgoHit&);
   TBgoHit(const TFragment& frag) : TGRSIDetectorHit(frag) {}
   ~TBgoHit() override;

   /////////////////////////		/////////////////////////////////////
   UShort_t GetArrayNumber() const { return -1; } //!<!

   void Clear(Option_t* opt = "") override;       //!<!
	using TGRSIDetectorHit::Copy;
   void Copy(TObject&) const override;            //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TBgoHit, 1)
   /// \endcond
};
/*! @} */
#endif
