#ifndef TBGO_H
#define TBGO_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>
#include <functional>
//#include <tuple>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TBgoHit.h"
#include "TDetector.h"
#include "TTransientBits.h"
#include "TSpline.h"

class TBgo : public TDetector {
public:
   TBgo();
   TBgo(const TBgo&);
   TBgo(TBgo&&);
   ~TBgo() override;

   TBgoHit* GetBgoHit(const Int_t& index) { return static_cast<TBgoHit*>(GetHit(index)); }

   static TVector3 GetPosition(int DetNbr, int CryNbr = 5, double distance = 110.0);   //!<!
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan) override;   //!<!
#endif
   void BuildHits() override {}   // no need to build any hits, everything already done in AddFragment

   TBgo& operator=(const TBgo&);   //!<!
   TBgo& operator=(TBgo&&);        //!<!

private:
   static TVector3 gScintPosition[17];   //!<! Position of each BGO scintillator

public:
   void Copy(TObject&) const override;              //!<!
   void Clear(Option_t* opt = "all") override;      //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TBgo, 1)   // Bgo Physics structure
   /// \endcond
};
/*! @} */
#endif
