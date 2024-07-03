#include "TRWPeak.h"

void TRWPeak::Centroid(const Double_t& centroid)
{
   SetFitFunction(new TF1("rw_total", this, &TRWPeak::TotalFunction, 0, 1, 6, "TRWPeak", "TotalFunction"));
   SetPeakFunction(new TF1("rw_peak", this, &TRWPeak::PeakFunction, 0, 1, 5, "TRWPeak", "PeakFunction"));
   InitParNames();
   GetFitFunction()->SetParameter(1, centroid);
   SetListOfBGPar(std::vector<bool>{false, false, false, false, false, true});
   GetFitFunction()->SetLineColor(kMagenta);
}

void TRWPeak::InitParNames()
{
   GetFitFunction()->SetParName(0, "Height");
   GetFitFunction()->SetParName(1, "centroid");
   GetFitFunction()->SetParName(2, "sigma");
   GetFitFunction()->SetParName(3, "beta");
   GetFitFunction()->SetParName(4, "R");
   GetFitFunction()->SetParName(5, "step");
}

void TRWPeak::InitializeParameters(TH1* fit_hist, const double& rangeLow, const double& rangeHigh)
{
   /// Makes initial guesses at parameters for the fit base on the histogram.
   // Make initial guesses
   // Actually set the parameters in the photopeak function
   // Fixing has to come after setting
   // Might have to include bin widths eventually
   // The centroid should already be set by this point in the ctor
   Int_t bin = fit_hist->FindBin(GetFitFunction()->GetParameter(1));
   if(!ParameterSetByUser(0)) {
      GetFitFunction()->SetParameter("Height", fit_hist->GetBinContent(bin));
      GetFitFunction()->SetParLimits(0, 0, fit_hist->GetMaximum() * 2.);
   }
   if(!ParameterSetByUser(1)) {
      GetFitFunction()->SetParLimits(1, rangeLow, rangeHigh);
   }
   if(!ParameterSetByUser(2)) {
      GetFitFunction()->SetParameter("sigma", TMath::Sqrt(5 + 1.33 * GetFitFunction()->GetParameter("centroid") / 1000. + 0.9 * TMath::Power(GetFitFunction()->GetParameter("centroid") / 1000., 2)) / 2.35);
      GetFitFunction()->SetParLimits(2, 0.01, 10.);
   }
   if(!ParameterSetByUser(3)) {
      GetFitFunction()->SetParameter("beta", GetFitFunction()->GetParameter("sigma") / 2.0);
      // GetFitFunction()->SetParLimits(3, 0.000001, 10);
      GetFitFunction()->FixParameter(3, GetFitFunction()->GetParameter("beta"));
   }
   if(!ParameterSetByUser(4)) {
      GetFitFunction()->SetParameter("R", 0.001);
      GetFitFunction()->SetParLimits(4, 0.000001, 100);   // this is a percentage. no reason for it to go to 500% - JKS
      GetFitFunction()->FixParameter(4, 0.00);
   }
   // Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
   if(!ParameterSetByUser(5)) {
      GetFitFunction()->SetParameter("step", 0.1);
      GetFitFunction()->SetParLimits(5, 0.0, 1.0E2);
   }
}

Double_t TRWPeak::Centroid() const
{
   return GetFitFunction()->GetParameter("centroid");
}

Double_t TRWPeak::CentroidErr() const
{
   return GetFitFunction()->GetParError(1);
}

Double_t TRWPeak::PeakFunction(Double_t* dim, Double_t* par)
{
   Double_t x        = dim[0];   // channel number used for fitting
   Double_t height   = par[0];   // height of photopeak
   Double_t centroid = par[1];   // Peak Centroid of non skew gaus
   Double_t sigma    = par[2];   // standard deviation of gaussian
   Double_t beta     = par[3];   // Skewness parameter
   Double_t relative = par[4];   // relative height of skewed gaussian

   Double_t gauss = height * (1.0 - relative / 100.0) * TMath::Gaus(x, centroid, sigma);

   if(beta == 0.0) { return gauss; }

   return gauss + relative * height / 100.0 * (TMath::Exp((x - centroid) / beta)) *
                  (TMath::Erfc(((x - centroid) / (TMath::Sqrt(2.0) * sigma)) + sigma / (TMath::Sqrt(2.0) * beta)));
}

Double_t TRWPeak::BackgroundFunction(Double_t* dim, Double_t* par)
{
   Double_t x        = dim[0];   // channel number used for fitting
   Double_t height   = par[0];   // height of photopeak
   Double_t centroid = par[1];   // Peak Centroid of non skew gaus
   Double_t sigma    = par[2];   // standard deviation of gaussian
   Double_t step     = par[5];   // Size of the step function;

   Double_t step_func = TMath::Abs(step) * height / 100.0 * TMath::Erfc((x - centroid) / (TMath::Sqrt(2.0) * sigma));

   return step_func;
}
