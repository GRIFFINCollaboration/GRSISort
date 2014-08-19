

#include "TGRSIOptions.h"

ClassImp(TGRSIOptions)

TGRSIOptions *TGRSIOptions::fTGRSIOptions = 0;
bool TGRSIOptions::fCloseAfterSort = false;


TGRSIOptions *TGRSIOptions::Get()   {
   if(!fTGRSIOptions)
      fTGRSIOptions = new TGRSIOptions;
   return fTGRSIOptions;
}

TGRSIOptions::TGRSIOptions() { 
}

TGRSIOptions::~TGRSIOptions() {  }


void TGRSIOptions::Print(Option_t *opt) {   }

void TGRSIOptions::Clear(Option_t *opt) {   }
