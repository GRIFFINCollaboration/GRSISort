#ifndef TGRSIDETECTOR_H
#define TGRSIDETECTOR_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <vector>

#include "TObject.h"
#include "TVector3.h"

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"
#include "TDetector.h"

class TGRSIDetectorHit;

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIDetector
///
/// This is an abstract class that contains the basic info
/// about a detector. This is where the hits are built and
/// the data is filled.
///
/////////////////////////////////////////////////////////////////

class TGRSIDetector : public TDetector {
public:
   TGRSIDetector();
   TGRSIDetector(const TGRSIDetector&);
   ~TGRSIDetector() override;

public:
// virtual TGRSIDetectorHit* GetHit(const Int_t idx = 0) { AbstractMethod("GetHit()"); return 0;}
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override
   {
      AbstractMethod("AddFragment()");
   } //!<!
#endif
   void BuildHits() override {}

   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!

protected:
#ifndef __CINT__
// void CopyFragment(std::shared_ptr<const TFragment> frag); //not implemented anywhere???
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIDetector, 1) // Abstract class for detector systems
   /// \endcond
};
/*! @} */
#endif
