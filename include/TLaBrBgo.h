#ifndef TLABRBGO_H
#define TLABRBGO_H

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
#include "TFragment.h"
#include "TChannel.h"
#include "TBgo.h"
#include "TLaBrBgoHit.h"

class TLaBrBgo : public TBgo {
public:
   TLaBrBgo();
   TLaBrBgo(const TLaBrBgo&);
   virtual ~TLaBrBgo();

   TLaBrBgoHit* GetLaBrBgoHit(const Int_t& i);
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override { return GetLaBrBgoHit(idx); }
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan) override; //!<!
#endif

   TLaBrBgo& operator=(const TLaBrBgo&); //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TLaBrBgo, 1) // LaBrBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
