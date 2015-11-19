#ifndef TGRSIFUNCTIONS_H
#define TGRSIFUNCTIONS_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <iostream>
#include <fstream>
#include <string>

#include "TMath.h"
#include "TROOT.h"

namespace TGRSIFunctions {

//Fitting Functions
   Double_t PolyBg(Double_t *dim, Double_t *par,Int_t order);
   Double_t StepBG(Double_t *dim, Double_t *par);
   Double_t StepFunction(Double_t *dim, Double_t *par);
   Double_t PhotoPeak(Double_t *dim, Double_t *par);
   Double_t PhotoPeakBG(Double_t *dim, Double_t *par);
   Double_t MultiPhotoPeakBG(Double_t *dim, Double_t *par);
   Double_t Gaus(Double_t *dim, Double_t *par);
   Double_t SkewedGaus(Double_t *dim, Double_t *par);
   Double_t MultiSkewedGausWithBG(Double_t *dim, Double_t *par);
   Double_t Bateman(Double_t *dim, Double_t *par, UInt_t nChain = 1, Double_t SecondsPerBin = 1.0);

// STEFFEN ADDED THESE
   Double_t LanGausHighRes(Double_t *x,Double_t *pars);
   Double_t LanGaus(Double_t *x,Double_t *pars);
   Double_t MultiGausWithBG(Double_t *x,Double_t *pars);
   //Double_t TripleAlphaGausWithBG(Double_t *x,Double_t *pars);
   Double_t SkewedGaus2(Double_t *dim, Double_t *par);
   Double_t MultiSkewedGausWithBG2(Double_t *dim, Double_t *par);

// CSI FIT FUNCTION
   Double_t CsIFitFunction(Double_t *i,Double_t *p);

//Common corrections
   Double_t DeadTimeCorrect(Double_t *dim, Double_t deadtime, Double_t binWidth = 1.0);
   Double_t DeadTimeAffect(Double_t function, Double_t deadtime, Double_t binWidth = 1.0);
}
/*! @} */
#endif
