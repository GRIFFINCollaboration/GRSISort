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
   virtual ~TBgo();

public:
   TBgoHit* GetBgoHit(const Int_t& i) { return static_cast<TBgoHit*>(GetHit(i)); }

   static TVector3 GetPosition(int DetNbr, int CryNbr = 5, double distance = 110.0); //!<!
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan) override; //!<!
#endif

   TBgo& operator=(const TBgo&); //!<!

protected:
   std::vector<TBgoHit*> fBgoHits; //  The set of crystal hits
private:
   static TVector3 gScintPosition[17];                      //!<! Position of each BGO scintillator

public:
   virtual void Copy(TObject&) const override;            //!<!
   virtual void Clear(Option_t* opt = "all") override;    //!<!
   virtual void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TBgo, 2) // Bgo Physics structure
   /// \endcond
};
/*! @} */
#endif
