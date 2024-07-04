#include "TUserSortInfo.h"

#include <iostream>

ClassImp(TUserSortInfo)

void TUserSortInfo::Print(Option_t* opt) const
{
   TGRSISortInfo::Print(opt);
}

void TUserSortInfo::Clear(Option_t* opt)
{
   TGRSISortInfo::Clear(opt);
}
