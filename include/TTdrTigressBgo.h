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
#include "TBgo.h"
#include "TGRSIDetector.h"
#include "TGRSIRunInfo.h"
#include "TTransientBits.h"
#include "TSpline.h"

class TTdrTigressBgo : public TBgo {
public:
   TTdrTigressBgo();
   TTdrTigressBgo(const TTdrTigressBgo&);
   virtual ~TTdrTigressBgo();

   TTdrTigressBgo& operator=(const TTdrTigressBgo&); //!<!

   /// \cond CLASSIMP
   ClassDef(TTdrTigressBgo, 1) // TdrTigressBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
