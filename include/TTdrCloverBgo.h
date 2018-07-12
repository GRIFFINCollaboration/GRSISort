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

class TTdrCloverBgo : public TBgo {
public:
   TTdrCloverBgo();
   TTdrCloverBgo(const TTdrCloverBgo&);
   virtual ~TTdrCloverBgo();

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
