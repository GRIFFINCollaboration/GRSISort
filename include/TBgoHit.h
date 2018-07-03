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

public:
   /////////////////////////		/////////////////////////////////////
   int GetCrystal() const override;
   inline UShort_t GetArrayNumber() const override { return (4 * (GetDetector() - 1) + (GetCrystal() + 1)); } //!<!
   Double_t GetCorrectedTime() const; ///< Returns a time value using the CFD, corrected using GValue

private:
public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Copy(TObject&) const override;            //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TBgoHit, 1)
   /// \endcond
};
/*! @} */
#endif
