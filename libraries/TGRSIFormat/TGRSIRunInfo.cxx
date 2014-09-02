
#include "TGRSIRunInfo.h"

ClassImp(TGRSIRunInfo)

TGRSIRunInfo *TGRSIRunInfo::fGRSIRunInfo = new TGRSIRunInfo();  

int  TGRSIRunInfo::fRunNumber    = 0;
int  TGRSIRunInfo::fSubRunNumber = -1;

bool TGRSIRunInfo:: fTigress     = false;
bool TGRSIRunInfo:: fSharc       = false;
bool TGRSIRunInfo:: fTriFoil     = false;
bool TGRSIRunInfo:: fRf          = false;
bool TGRSIRunInfo:: fCSM         = false;
bool TGRSIRunInfo:: fSpice       = false;
bool TGRSIRunInfo:: fTip         = false;

bool TGRSIRunInfo:: fGriffin     = false;
bool TGRSIRunInfo:: fSceptar     = false;
bool TGRSIRunInfo:: fPaces       = false;
bool TGRSIRunInfo:: fDante       = false;
bool TGRSIRunInfo:: fZeroDegree  = false;
bool TGRSIRunInfo:: fDescant     = false;

TGRSIRunInfo *TGRSIRunInfo::Get() {
   if(!fGRSIRunInfo)
      fGRSIRunInfo = new TGRSIRunInfo();
   return fGRSIRunInfo;
}

TGRSIRunInfo::TGRSIRunInfo() { }

TGRSIRunInfo::~TGRSIRunInfo() { }

void TGRSIRunInfo::Print(Option_t *opt) { }

void TGRSIRunInfo::Clear(Option_t *opt) { }


void TGRSIRunInfo::SetRunInfo() {



}


void TGRSIRunInfo::SetAnalysisTreeBranches(TTree*) {



}





