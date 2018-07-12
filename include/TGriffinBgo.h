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

class TGriffinBgo : public TBgo {
public:
   TGriffinBgo();
   TGriffinBgo(const TGriffinBgo&);
   virtual ~TGriffinBgo();

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
