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

#include "TDetectorHit.h"

class TBgoHit : public TDetectorHit {
public:
   TBgoHit();
   TBgoHit(const TBgoHit&);
   TBgoHit(TBgoHit&&);
   // this is problematic as this means we're slicing the TFragment object to fit the
   // TDetectorHit argument of the base constructor
   explicit TBgoHit(const TFragment& frag) : TDetectorHit(frag) {}
   ~TBgoHit() override;

   TBgoHit& operator=(const TBgoHit&);
   TBgoHit& operator=(TBgoHit&&);

   /////////////////////////		/////////////////////////////////////
   inline UShort_t GetArrayNumber() const override { return (20 * (GetDetector() - 1) + 5 * GetCrystal() + GetSegment()); }   //!<!

   using TObject::Copy;
   void Copy(TObject&, bool copywave) const override;   //!<!
   void Clear(Option_t* opt = "all") override;          //!<!
   void Print(Option_t* opt = "") const override;       //!<!
   void Print(std::ostream& out) const override;        //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TBgoHit, 1) // NOLINT
   /// \endcond
};
/*! @} */
#endif
