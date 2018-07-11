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
#include "TBgo.h"
#include "TGRSIDetector.h"
#include "TGRSIRunInfo.h"
#include "TTransientBits.h"
#include "TSpline.h"

class TTdrCloverBgo : public TBgo {
public:
   TTdrCloverBgo();
   TTdrCloverBgo(const TTdrCloverBgo&);
   virtual ~TTdrCloverBgo();

   TTdrCloverBgo& operator=(const TTdrCloverBgo&); //!<!

   /// \cond CLASSIMP
   ClassDef(TTdrCloverBgo, 1) // TdrCloverBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
