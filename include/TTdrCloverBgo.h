#ifndef TTDRCLOVERBGO_H
#define TTDRCLOVERBGO_H

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
#include "TTdrCloverBgoHit.h"

class TTdrCloverBgo : public TBgo {
public:
   TTdrCloverBgo();
   TTdrCloverBgo(const TTdrCloverBgo&);
   virtual ~TTdrCloverBgo();

   TTdrCloverBgoHit* GetTdrCloverBgoHit(const Int_t& i);
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override { return GetTdrCloverBgoHit(idx); }
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan) override; //!<!
#endif

   TTdrCloverBgo& operator=(const TTdrCloverBgo&); //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTdrCloverBgo, 1) // TdrCloverBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
