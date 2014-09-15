#ifndef __TGRSIFUNCTIONS_H
#define __TGRSIFUNCTIONS_H

#include <iostream>
#include <fstream>
#include <string>

#include "TMath.h"
#include "TROOT.h"

namespace TGRSIFunctions {

//Fitting Functions
   Double_t PolyBg(Double_t *dim, Double_t *par,Int_t order);
   Double_t StepFunction(Double_t *dim, Double_t *par);
   Double_t PhotoPeak(Double_t *dim, Double_t *par);
   Double_t Gaus(Double_t *dim, Double_t *par);
   Double_t SkewedGaus(Double_t *dim, Double_t *par);
   Double_t Bateman(Double_t *dim, Double_t *par, Int_t nChain = 1, Double_t SecondsPerBin = 1.0);

//Common corrections
   Double_t DeadTimeCorrect(Double_t *dim, Double_t deadtime, Double_t binWidth = 1.0);
   Double_t DeadTimeAffect(Double_t function, Double_t deadtime, Double_t binWidth = 1.0);

}




#endif
