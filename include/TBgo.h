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
#include "TGRSIDetector.h"
#include "TGRSIRunInfo.h"
#include "TTransientBits.h"
#include "TSpline.h"

class TBgo : public TGRSIDetector {
public:
   TBgo();
   TBgo(const TBgo&);
   virtual ~TBgo();

public:
   TBgoHit* GetBgoHit(const Int_t& i);
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override { return GetBgoHit(idx); }
   Short_t   GetMultiplicity() const override { return fBgoHits.size(); }
	const std::vector<TBgoHit>& GetHitVector() const { return fBgoHits; }

   static TVector3 GetPosition(int DetNbr, int CryNbr = 5, double distance = 110.0); //!<!
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan) override; //!<!
#endif
   void ClearTransients() override
   {
      for(auto hit : fBgoHits) hit.ClearTransients();
   }
   void ResetFlags() const;

   TBgo& operator=(const TBgo&); //!<!

protected:
   std::vector<TBgoHit> fBgoHits; //  The set of crystal hits
private:
   static TVector3 gScintPosition[17];                      //!<! Position of each BGO scintillator

public:
   virtual void Copy(TObject&) const override;            //!<!
   virtual void Clear(Option_t* opt = "all") override;    //!<!
   virtual void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TBgo, 1) // Bgo Physics structure
   /// \endcond
};
/*! @} */
#endif
