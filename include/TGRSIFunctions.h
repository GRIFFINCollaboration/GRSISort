#ifndef __TGRSIFUNCTIONS_H
#define __TGRSIFUNCTIONS_H

#include <iostream>
#include <fstream>
#include <string>

#include "TMath.h"
#include "TROOT.h"

namespace TGRSIFunctions {

   Double_t PolyBg(Double_t*, Double_t*,Int_t);
   Double_t StepFunction(Double_t*, Double_t*);
   Double_t PhotoPeak(Double_t*, Double_t*);

}



#endif
