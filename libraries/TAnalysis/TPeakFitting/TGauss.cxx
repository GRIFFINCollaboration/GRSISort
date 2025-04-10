#include "TGauss.h"

TGauss::TGauss(Double_t centroid, Double_t relativeLimit)
{
   Centroid(centroid);
   if(relativeLimit >= 0) {
      GetFitFunction()->SetParLimits(1, (1. - relativeLimit) * centroid, (1. + relativeLimit) * centroid);
   }
}

void TGauss::Centroid(const Double_t& centroid)
{
   if(TPeakFitter::VerboseLevel() >= EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   SetFitFunction(new TF1("gauss_total", this, &TGauss::TotalFunction, 0, 1, 3, "TGauss", "TotalFunction"));
   SetPeakFunction(new TF1("gauss_peak", this, &TGauss::PeakFunction, 0, 1, 3, "TGauss", "TotalFunction"));   // peak = total function
   if(TPeakFitter::VerboseLevel() >= EVerbosity::kSubroutines) { std::cout << "Set the fit and peak functions" << std::endl; }
   InitParNames();
   GetFitFunction()->SetParameter(1, centroid);
   if(TPeakFitter::VerboseLevel() >= EVerbosity::kSubroutines) { std::cout << "Set the centroid to " << centroid << std::endl; }
   SetListOfBGPar(std::vector<bool>{false, false, false, false, false, true});
   GetFitFunction()->SetLineColor(kMagenta);
   if(TPeakFitter::VerboseLevel() >= EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": done" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

void TGauss::InitParNames()
{
   if(TPeakFitter::VerboseLevel() >= EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   GetFitFunction()->SetParName(0, "height");
   GetFitFunction()->SetParName(1, "centroid");
   GetFitFunction()->SetParName(2, "sigma");
   if(TPeakFitter::VerboseLevel() >= EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": done" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

void TGauss::InitializeParameters(TH1* fit_hist, const double& rangeLow, const double& rangeHigh)
{
   /// Makes initial guesses at parameters for the fit base on the histogram.
   // Make initial guesses
   // Actually set the parameters in the photopeak function
   // Fixing has to come after setting
   // Might have to include bin widths eventually
   // The centroid should already be set by this point in the ctor
   // We need to set the limits after setting the parameter otherwise we might get a warning
   // that the parameter (which is zero at this time) is outside the limits.
   Int_t bin = fit_hist->FindBin(GetFitFunction()->GetParameter(1));
   if(!ParameterSetByUser(0)) {
      GetFitFunction()->SetParameter("height", fit_hist->GetBinContent(bin));
      GetFitFunction()->SetParLimits(0, 0, fit_hist->GetMaximum() * 2.);
   }
   if(!ParameterSetByUser(1)) {
      GetFitFunction()->SetParLimits(1, rangeLow, rangeHigh);
   }
   if(!ParameterSetByUser(2)) {
      GetFitFunction()->SetParameter("sigma", TMath::Sqrt(5 + 1.33 * GetFitFunction()->GetParameter("centroid") / 1000. + 0.9 * TMath::Power(GetFitFunction()->GetParameter("centroid") / 1000., 2)) / 2.35);
      GetFitFunction()->SetParLimits(2, 0.01, 10.);
   }
}

Double_t TGauss::Centroid() const
{
   return GetFitFunction()->GetParameter("centroid");
}

Double_t TGauss::CentroidErr() const
{
   return GetFitFunction()->GetParError(1);
}

Double_t TGauss::PeakFunction(Double_t* dim, Double_t* par)
{
   Double_t x      = dim[0];   // channel number used for fitting
   Double_t height = par[0];   // height of photopeak
   Double_t c      = par[1];   // centroid of gaussian
   Double_t sigma  = par[2];   // standard deviation of gaussian

   Double_t gauss = height * TMath::Gaus(x, c, sigma);

   return gauss;
}
