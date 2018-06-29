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
#include "TBgo.h"
#include "TGRSIDetector.h"
#include "TGRSIRunInfo.h"
#include "TTransientBits.h"
#include "TSpline.h"

class TLaBrBgo : public TBgo {
public:
   TLaBrBgo();
   TLaBrBgo(const TLaBrBgo&);
   virtual ~TLaBrBgo();

   TLaBrBgo& operator=(const TLaBrBgo&); //!<!

   /// \cond CLASSIMP
   ClassDef(TLaBrBgo, 1) // LaBrBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
