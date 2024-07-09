#include "TABPeak.h"
#include "TH1.h"

void TABPeak::Centroid(const Double_t& centroid)
{
   SetFitFunction(new TF1("ab_fit", this, &TABPeak::TotalFunction, 0, 1, 6, "TABPeak", "TotalFunction"));
   SetPeakFunction(new TF1("ab_peak", this, &TABPeak::PeakFunction, 0, 1, 5, "TABPeak", "PeakFunction"));
   InitParNames();
   GetFitFunction()->SetParameter(1, centroid);
   SetListOfBGPar(std::vector<bool>{false, false, false, false, false, true});
   GetFitFunction()->SetLineColor(kMagenta);
}

void TABPeak::InitParNames()
{
   GetFitFunction()->SetParName(0, "Height");
   GetFitFunction()->SetParName(1, "centroid");
   GetFitFunction()->SetParName(2, "sigma");
   GetFitFunction()->SetParName(3, "rel_height");
   GetFitFunction()->SetParName(4, "rel_sigma");
   GetFitFunction()->SetParName(5, "step");
}

void TABPeak::InitializeParameters(TH1* fit_hist, const double& rangeLow, const double& rangeHigh)
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
      GetFitFunction()->SetParLimits(0, 0, fit_hist->GetMaximum() * 1.5);
   }
   if(!ParameterSetByUser(1)) {
      GetFitFunction()->SetParLimits(1, rangeLow, rangeHigh);
   }
   if(!ParameterSetByUser(2)) {
      GetFitFunction()->SetParameter("sigma", TMath::Sqrt(2.25 + 1.33 * GetFitFunction()->GetParameter("centroid") / 1000. + 0.9 * TMath::Power(GetFitFunction()->GetParameter("centroid") / 1000., 2)) / 2.35);
      GetFitFunction()->SetParLimits(2, 0.1, 8);
   }
   if(!ParameterSetByUser(3)) {
      GetFitFunction()->SetParameter("rel_height", 0.25);
      GetFitFunction()->SetParLimits(3, 0.000001, 1.0);
   }
   if(!ParameterSetByUser(4)) {
      GetFitFunction()->SetParameter("rel_sigma", 2.);
      GetFitFunction()->SetParLimits(4, 1.0, 100);
   }
   if(!ParameterSetByUser(5)) {
      // Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
      GetFitFunction()->SetParameter("step", 1.0);
      GetFitFunction()->SetParLimits(5, 0.0, 1.0E2);
   }
}

Double_t TABPeak::Centroid() const
{
   return GetFitFunction()->GetParameter("centroid");
}

Double_t TABPeak::CentroidErr() const
{
   return GetFitFunction()->GetParError(1);
}

Double_t TABPeak::Width() const
{
   return GetFitFunction()->GetParameter("sigma") * GetFitFunction()->GetParameter("rel_sigma");
}

Double_t TABPeak::Sigma() const
{
   return GetFitFunction()->GetParameter("sigma");
}

Double_t TABPeak::PeakFunction(Double_t* dim, Double_t* par)
{
   return OneHitPeakFunction(dim, par) + TwoHitPeakFunction(dim, par);
}

Double_t TABPeak::OneHitPeakFunction(Double_t* dim, Double_t* par)
{
   Double_t x        = dim[0];   // channel number used for fitting
   Double_t height   = par[0];   // height of photopeak
   Double_t centroid = par[1];   // Peak Centroid of non skew gaus
   Double_t sigma    = par[2];   // standard deviation of gaussian

   return height * TMath::Gaus(x, centroid, sigma);
}

Double_t TABPeak::TwoHitPeakFunction(Double_t* dim, Double_t* par)
{
   Double_t x          = dim[0];   // channel number used for fitting
   Double_t height     = par[0];   // height of photopeak
   Double_t centroid   = par[1];   // Peak Centroid of non skew gaus
   Double_t sigma      = par[2];   // standard deviation of gaussian
   Double_t rel_height = par[3];   // relative height of 2-hit peak
   Double_t rel_sigma  = par[4];   // relative sigma of 2-hit peak

   return height * rel_height * TMath::Gaus(x, centroid, rel_sigma * sigma);
}

Double_t TABPeak::OneHitPeakOnGlobalFunction(Double_t* dim, Double_t* par)
{
   return OneHitPeakFunction(dim, par) + GetGlobalBackground()->EvalPar(dim, &par[GetFitFunction()->GetNpar()]);
}

Double_t TABPeak::TwoHitPeakOnGlobalFunction(Double_t* dim, Double_t* par)
{
   return TwoHitPeakFunction(dim, par) + GetGlobalBackground()->EvalPar(dim, &par[GetFitFunction()->GetNpar()]);
}

Double_t TABPeak::BackgroundFunction(Double_t* dim, Double_t* par)
{
   Double_t x        = dim[0];   // channel number used for fitting
   Double_t height   = par[0];   // height of photopeak
   Double_t centroid = par[1];   // Peak Centroid of non skew gaus
   Double_t sigma    = par[2];   // standard deviation of gaussian
   Double_t step     = par[5];   // Size of the step function;

   Double_t step_func = TMath::Abs(step) * height / 100.0 * TMath::Erfc((x - centroid) / (TMath::Sqrt(2.0) * sigma));

   return step_func;
}

void TABPeak::DrawComponents(Option_t* opt)
{
   // We need to draw this on top of the global background. Probably easiest to make another temporary TF1?
   if(GetGlobalBackground() == nullptr) return;

   Double_t low  = 0;
   Double_t high = 0;
   GetGlobalBackground()->GetRange(low, high);
   if(fOneHitOnGlobal != nullptr) { fOneHitOnGlobal->Delete(); }
   if(fTwoHitOnGlobal != nullptr) { fTwoHitOnGlobal->Delete(); }
   // Make a copy of the total function, and then tack on the global background parameters.
   fOneHitOnGlobal = new TF1("draw_component1", this, &TABPeak::OneHitPeakOnGlobalFunction, low, high, GetFitFunction()->GetNpar() + GetGlobalBackground()->GetNpar(), "TABPeak", "OneHitPeakOnGlobalFunction");
   fTwoHitOnGlobal = new TF1("draw_component2", this, &TABPeak::TwoHitPeakOnGlobalFunction, low, high, GetFitFunction()->GetNpar() + GetGlobalBackground()->GetNpar(), "TABPeak", "TwoHitPeakOnGlobalFunction");
   for(int i = 0; i < GetFitFunction()->GetNpar(); ++i) {
      fOneHitOnGlobal->SetParameter(i, GetFitFunction()->GetParameter(i));
      fTwoHitOnGlobal->SetParameter(i, GetFitFunction()->GetParameter(i));
   }
   for(int i = 0; i < GetGlobalBackground()->GetNpar(); ++i) {
      fOneHitOnGlobal->SetParameter(i + GetFitFunction()->GetNpar(), GetGlobalBackground()->GetParameter(i));
      fTwoHitOnGlobal->SetParameter(i + GetFitFunction()->GetNpar(), GetGlobalBackground()->GetParameter(i));
   }
   // Draw a copy of this function
   fOneHitOnGlobal->SetLineColor(GetFitFunction()->GetLineColor());
   fTwoHitOnGlobal->SetLineColor(GetFitFunction()->GetLineColor());
   fOneHitOnGlobal->SetLineStyle(8);
   fTwoHitOnGlobal->SetLineStyle(3);
   fOneHitOnGlobal->Draw(opt);
   fTwoHitOnGlobal->Draw(opt);
}
