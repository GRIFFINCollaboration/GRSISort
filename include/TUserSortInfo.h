#ifndef TUSERSORTINFO_H
#define TUSERSORTINFO_H

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
      TUserSortInfo(const TGRSIRunInfo* info) : TGRSISortInfo(info) {};
      virtual ~TUserSortInfo() {};

      void Print(Option_t* opt = "") const;
      void Clear(Option_t* opt = "");

   ClassDef(TUserSortInfo,1);
};

#endif
