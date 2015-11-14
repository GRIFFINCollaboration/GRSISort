#if !defined (__CINT__) && !defined (__CLING__)

#include "TTip.h"
#include "TTipData.h"

bool TTipData::fIsSet   = false;

TTipData::TTipData()  {  
  Clear();
}

TTipData::~TTipData()  {  }


void TTipData::Clear(Option_t *opt)  {
  fIsSet = false;

  fTip_Frag.clear();
  fTip_Name.clear();
}

void TTipData::Print(Option_t *opt) const  {
  // not yet written.
  printf("not yet written.\n");
}
#endif
