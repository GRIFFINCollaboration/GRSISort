#ifndef TGRIFFINBGO_H
#define TGRIFFINBGO_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>
#include <functional>
//#include <tuple>

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"
#include "TBgo.h"
#include "TGriffinBgoHit.h"

class TGriffinBgo : public TBgo {
public:
   TGriffinBgo();
   TGriffinBgo(const TGriffinBgo&);
   virtual ~TGriffinBgo();

   TGriffinBgoHit* GetGriffinBgoHit(const Int_t& i);
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override { return GetGriffinBgoHit(idx); }
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan) override; //!<!
#endif

   TGriffinBgo& operator=(const TGriffinBgo&); //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TGriffinBgo, 1) // GriffinBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
