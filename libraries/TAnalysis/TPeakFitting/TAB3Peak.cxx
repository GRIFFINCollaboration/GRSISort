#include "TAB3Peak.h"
#include "TH1.h"

/// \cond CLASSIMP
ClassImp(TAB3Peak)
   /// \endcond

   TAB3Peak::TAB3Peak() : TSinglePeak()
{}

TAB3Peak::TAB3Peak(Double_t centroid) : TSinglePeak()
{
   Centroid(centroid);
}

void TAB3Peak::Centroid(const Double_t& centroid)
{
   fTotalFunction = new TF1("ab_fit", this, &TAB3Peak::TotalFunction, 0, 1, 8, "TAB3Peak", "TotalFunction");
   fPeakFunction  = new TF1("ab_peak", this, &TAB3Peak::PeakFunction, 0, 1, 7, "TAB3Peak", "PeakFunction");
   InitParNames();
   fTotalFunction->SetParameter(1, centroid);
   SetListOfBGPar(std::vector<bool>{0, 0, 0, 0, 0, 0, 0, 1});
   fTotalFunction->SetLineColor(kMagenta);
}

void TAB3Peak::InitParNames()
{
   fTotalFunction->SetParName(0, "Height");
   fTotalFunction->SetParName(1, "centroid");
   fTotalFunction->SetParName(2, "sigma");
   fTotalFunction->SetParName(3, "rel_height1");
   fTotalFunction->SetParName(4, "rel_sigma1");
   fTotalFunction->SetParName(5, "rel_height2");
   fTotalFunction->SetParName(6, "rel_sigma2");
   fTotalFunction->SetParName(7, "step");
}

void TAB3Peak::InitializeParameters(TH1* fit_hist, const double& rangeLow, const double& rangeHigh)
{
   /// Makes initial guesses at parameters for the fit base on the histogram.
   // Make initial guesses
   // Actually set the parameters in the photopeak function
   // Fixing has to come after setting
   // Might have to include bin widths eventually
   // The centroid should already be set by this point in the ctor
   Int_t bin = fit_hist->FindBin(fTotalFunction->GetParameter(1));
   if(!ParameterSetByUser(0)) {
      fTotalFunction->SetParameter("Height", fit_hist->GetBinContent(bin));
      fTotalFunction->SetParLimits(0, 0, fit_hist->GetMaximum() * 1.5);
   }
   if(!ParameterSetByUser(1)) {
      fTotalFunction->SetParLimits(1, rangeLow, rangeHigh);
   }
   if(!ParameterSetByUser(2)) {
      fTotalFunction->SetParameter("sigma", TMath::Sqrt(2.25 + 1.33 * fTotalFunction->GetParameter("centroid") / 1000. + 0.9 * TMath::Power(fTotalFunction->GetParameter("centroid") / 1000., 2)) / 2.35);
      fTotalFunction->SetParLimits(2, 0.1, 8);
   }
   if(!ParameterSetByUser(3)) {
      fTotalFunction->SetParameter("rel_height1", 0.25);
      fTotalFunction->SetParLimits(3, 0.000001, 1.0);
   }
   if(!ParameterSetByUser(4)) {
      fTotalFunction->SetParameter("rel_sigma1", 2.);
      fTotalFunction->SetParLimits(4, 1.0, 100);
   }
   if(!ParameterSetByUser(5)) {
      fTotalFunction->SetParameter("rel_height2", 0.25);
      fTotalFunction->SetParLimits(5, 0.000001, 1.0);
   }
   if(!ParameterSetByUser(6)) {
      fTotalFunction->SetParameter("rel_sigma2", 6.);
      fTotalFunction->SetParLimits(6, 1.0, 100);
   }
   if(!ParameterSetByUser(7)) {
      // Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
      fTotalFunction->SetParameter("step", 1.0);
      fTotalFunction->SetParLimits(7, 0.0, 1.0E2);
   }
}

Double_t TAB3Peak::Centroid() const
{
   return fTotalFunction->GetParameter("centroid");
}

Double_t TAB3Peak::CentroidErr() const
{
   return fTotalFunction->GetParError(1);
}

Double_t TAB3Peak::Width() const
{
   return fTotalFunction->GetParameter("sigma") * fTotalFunction->GetParameter("rel_sigma2");
}

Double_t TAB3Peak::Sigma() const
{
   return fTotalFunction->GetParameter("sigma");
}

Double_t TAB3Peak::PeakFunction(Double_t* dim, Double_t* par)
{
   return OneHitPeakFunction(dim, par) + TwoHitPeakFunction(dim, par) + ThreeHitPeakFunction(dim, par);
}

Double_t TAB3Peak::OneHitPeakFunction(Double_t* dim, Double_t* par)
{
   Double_t x      = dim[0];   // channel number used for fitting
   Double_t height = par[0];   // height of photopeak
   Double_t c      = par[1];   // Peak Centroid of non skew gaus
   Double_t sigma  = par[2];   // standard deviation of gaussian

   return height * TMath::Gaus(x, c, sigma);
}

Double_t TAB3Peak::TwoHitPeakFunction(Double_t* dim, Double_t* par)
{
   Double_t x          = dim[0];   // channel number used for fitting
   Double_t height     = par[0];   // height of photopeak
   Double_t c          = par[1];   // Peak Centroid of non skew gaus
   Double_t sigma      = par[2];   // standard deviation of gaussian
   Double_t rel_height = par[3];   // relative height of 2-hit peak
   Double_t rel_sigma  = par[4];   // relative sigma of 2-hit peak

   return height * rel_height * TMath::Gaus(x, c, rel_sigma * sigma);
}

Double_t TAB3Peak::ThreeHitPeakFunction(Double_t* dim, Double_t* par)
{
   Double_t x          = dim[0];   // channel number used for fitting
   Double_t height     = par[0];   // height of photopeak
   Double_t c          = par[1];   // Peak Centroid of non skew gaus
   Double_t sigma      = par[2];   // standard deviation of gaussian
   Double_t rel_height = par[5];   // relative height of 2-hit peak
   Double_t rel_sigma  = par[6];   // relative sigma of 2-hit peak

   return height * rel_height * TMath::Gaus(x, c, rel_sigma * sigma);
}

Double_t TAB3Peak::OneHitPeakOnGlobalFunction(Double_t* dim, Double_t* par)
{
   return OneHitPeakFunction(dim, par) + fGlobalBackground->EvalPar(dim, &par[fTotalFunction->GetNpar()]);
}

Double_t TAB3Peak::TwoHitPeakOnGlobalFunction(Double_t* dim, Double_t* par)
{
   return TwoHitPeakFunction(dim, par) + fGlobalBackground->EvalPar(dim, &par[fTotalFunction->GetNpar()]);
}

Double_t TAB3Peak::ThreeHitPeakOnGlobalFunction(Double_t* dim, Double_t* par)
{
   return ThreeHitPeakFunction(dim, par) + fGlobalBackground->EvalPar(dim, &par[fTotalFunction->GetNpar()]);
}

Double_t TAB3Peak::BackgroundFunction(Double_t* dim, Double_t* par)
{
   Double_t x      = dim[0];   // channel number used for fitting
   Double_t height = par[0];   // height of photopeak
   Double_t c      = par[1];   // Peak Centroid of non skew gaus
   Double_t sigma  = par[2];   // standard deviation of gaussian
   Double_t step   = par[7];   // Size of the step function;

   Double_t step_func = TMath::Abs(step) * height / 100.0 * TMath::Erfc((x - c) / (TMath::Sqrt(2.0) * sigma));

   return step_func;
}

void TAB3Peak::DrawComponents(Option_t* opt)
{
   // We need to draw this on top of the global background. Probably easiest to make another temporary TF1?
   if(!fGlobalBackground)
      return;

   Double_t low, high;
   fGlobalBackground->GetRange(low, high);
   if(fOneHitOnGlobal) fOneHitOnGlobal->Delete();
   if(fTwoHitOnGlobal) fTwoHitOnGlobal->Delete();
   if(fThreeHitOnGlobal) fThreeHitOnGlobal->Delete();
   // Make a copy of the total function, and then tack on the global background parameters.
   fOneHitOnGlobal   = new TF1("draw_component1", this, &TAB3Peak::OneHitPeakOnGlobalFunction, low, high, fTotalFunction->GetNpar() + fGlobalBackground->GetNpar(), "TAB3Peak", "OneHitPeakOnGlobalFunction");
   fTwoHitOnGlobal   = new TF1("draw_component2", this, &TAB3Peak::TwoHitPeakOnGlobalFunction, low, high, fTotalFunction->GetNpar() + fGlobalBackground->GetNpar(), "TAB3Peak", "TwoHitPeakOnGlobalFunction");
   fThreeHitOnGlobal = new TF1("draw_component2", this, &TAB3Peak::ThreeHitPeakOnGlobalFunction, low, high, fTotalFunction->GetNpar() + fGlobalBackground->GetNpar(), "TAB3Peak", "ThreeHitPeakOnGlobalFunction");
   for(int i = 0; i < fTotalFunction->GetNpar(); ++i) {
      fOneHitOnGlobal->SetParameter(i, fTotalFunction->GetParameter(i));
      fTwoHitOnGlobal->SetParameter(i, fTotalFunction->GetParameter(i));
      fThreeHitOnGlobal->SetParameter(i, fTotalFunction->GetParameter(i));
   }
   for(int i = 0; i < fGlobalBackground->GetNpar(); ++i) {
      fOneHitOnGlobal->SetParameter(i + fTotalFunction->GetNpar(), fGlobalBackground->GetParameter(i));
      fTwoHitOnGlobal->SetParameter(i + fTotalFunction->GetNpar(), fGlobalBackground->GetParameter(i));
      fThreeHitOnGlobal->SetParameter(i + fTotalFunction->GetNpar(), fGlobalBackground->GetParameter(i));
   }
   // Draw a copy of this function
   fOneHitOnGlobal->SetLineColor(fTotalFunction->GetLineColor());
   fTwoHitOnGlobal->SetLineColor(fTotalFunction->GetLineColor());
   fThreeHitOnGlobal->SetLineColor(fTotalFunction->GetLineColor());
   fOneHitOnGlobal->SetLineStyle(8);
   fTwoHitOnGlobal->SetLineStyle(3);
   fThreeHitOnGlobal->SetLineStyle(2);
   fOneHitOnGlobal->Draw(opt);
   fTwoHitOnGlobal->Draw(opt);
   fThreeHitOnGlobal->Draw(opt);
}
