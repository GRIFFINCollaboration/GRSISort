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
   ~TUserSortInfo() override= default;;

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   /// \cond CLASSIMP
   ClassDefOverride(TUserSortInfo, 1);
   /// \endcond
};
/*! @} */
#endif
