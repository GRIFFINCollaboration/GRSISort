#ifndef TTDRTIGRESSBGO_H
#define TTDRTIGRESSBGO_H

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

class TTdrTigressBgo : public TBgo {
public:
   TTdrTigressBgo();
   TTdrTigressBgo(const TTdrTigressBgo&);
   virtual ~TTdrTigressBgo();

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan) override; //!<!
#endif

   TTdrTigressBgo& operator=(const TTdrTigressBgo&); //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTdrTigressBgo, 1) // TdrTigressBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
