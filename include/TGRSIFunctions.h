#ifndef TGRSIFUNCTIONS_H
#define TGRSIFUNCTIONS_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include "TMath.h"
#include "TROOT.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"

#ifdef HAS_MATHMORE
#include "Math/SpecFuncMathMore.h"
#endif

#include "Globals.h"

namespace TGRSIFunctions {

// Function to check parameter erros to see if any parameter is near its limit
bool CheckParameterErrors(const TFitResultPtr& fit_res, std::string opt = "");

// Fitting Functions
Double_t PolyBg(Double_t* x, Double_t* par, Int_t order);
Double_t StepBG(Double_t* dim, Double_t* par);
Double_t StepFunction(Double_t* dim, Double_t* par);
Double_t PhotoPeak(Double_t* dim, Double_t* par);
Double_t PhotoPeakBG(Double_t* dim, Double_t* par);
Double_t MultiPhotoPeakBG(Double_t* dim, Double_t* par);
Double_t Gaus(Double_t* dim, Double_t* par);
Double_t SkewedGaus(Double_t* dim, Double_t* par);
Double_t MultiSkewedGausWithBG(Double_t* dim, Double_t* par);
Double_t Bateman(std::vector<Double_t>& dim, std::vector<Double_t>& par, UInt_t nChain = 1, Double_t SecondsPerBin = 1.0);
Double_t PhotoEfficiency(Double_t* dim, Double_t* par);

// STEFFEN ADDED THESE
Double_t LanGausHighRes(Double_t* x, Double_t* pars);
Double_t LanGaus(Double_t* x, Double_t* pars);
Double_t MultiGausWithBG(Double_t* dim, Double_t* par);
// Double_t TripleAlphaGausWithBG(Double_t *x,Double_t *pars);
Double_t SkewedGaus2(Double_t* x, Double_t* par);
Double_t MultiSkewedGausWithBG2(Double_t* dim, Double_t* par);

// CSI FIT FUNCTION
Double_t CsIFitFunction(Double_t* i, Double_t* p);

// Common corrections
Double_t DeadTimeCorrect(Double_t* dim, Double_t deadtime, Double_t binWidth = 1.0);
Double_t DeadTimeAffect(Double_t function, Double_t deadtime, Double_t binWidth = 1.0);

// Timing functions
Double_t ConvolutedDecay(Double_t* x, Double_t* par);
Double_t ConvolutedDecay2(Double_t* x, Double_t* par);

#ifdef HAS_MATHMORE
// Angular correlation fitting
Double_t LegendrePolynomial(Double_t* x, Double_t* p);
#endif

// functions used for angular correlations
double RacahW(double a, double b, double c, double d, double e, double f);
double ClebschGordan(double j1, double m1, double j2, double m2, double j, double m);
double F(double k, double jf, double L1, double L2, double ji);
double A(double k, double ji, double jf, double L1, double L2, double delta);
double B(double k, double ji, double jf, double L1, double L2, double delta);
double CalculateA2(double j1, double j2, double j3, double l1a, double l1b, double l2a, double l2b, double delta1, double delta2);
double CalculateA4(double j1, double j2, double j3, double l1a, double l1b, double l2a, double l2b, double delta1, double delta2);

}   // namespace TGRSIFunctions
/*! @} */
#endif
