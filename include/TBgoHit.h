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
   virtual ~TBgoHit();

public:
   /////////////////////////		/////////////////////////////////////
   int GetCrystal() const;

private:
public:
   virtual void Clear(Option_t* opt = "");       //!<!
   virtual void Copy(TObject&) const;            //!<!
   virtual void Print(Option_t* opt = "") const; //!<!

   /// \cond CLASSIMP
   ClassDef(TBgoHit, 1)
   /// \endcond
};
/*! @} */
#endif
