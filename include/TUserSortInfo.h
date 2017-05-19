#ifndef TUSERSORTINFO_H
#define TUSERSORTINFO_H

/** \addtogroup Sorting
 *  @{
 */

#include <map>

#include "TList.h"
#include "TString.h"
#include "TCollection.h"

#include "Globals.h"
#include "TGRSIRunInfo.h"
#include "TGRSISortInfo.h"

class TUserSortInfo : public TGRSISortInfo {
public:
   TUserSortInfo(){};
   TUserSortInfo(const TGRSIRunInfo* info) : TGRSISortInfo(info){};
   virtual ~TUserSortInfo(){};

   void Print(Option_t* opt = "") const;
   void Clear(Option_t* opt = "");

   /// \cond CLASSIMP
   ClassDef(TUserSortInfo, 1);
   /// \endcond
};
/*! @} */
#endif
