#ifndef TGRIFFINBGO_H
#define TGRIFFINBGO_H

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

class TGriffinBgo : public TBgo {
public:
   TGriffinBgo();
   TGriffinBgo(const TGriffinBgo&);
   virtual ~TGriffinBgo();

   TGriffinBgo& operator=(const TGriffinBgo&); //!<!

   /// \cond CLASSIMP
   ClassDef(TGriffinBgo, 1) // GriffinBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
