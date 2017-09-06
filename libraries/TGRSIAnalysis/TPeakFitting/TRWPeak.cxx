#include "TRWPeak.h"

/// \cond CLASSIMP
ClassImp(TRWPeak)
/// \endcond

TRWPeak::TRWPeak() : TSinglePeak(){ }

TRWPeak::TRWPeak(Double_t centroid) : TSinglePeak() {

   fFitFunction = new TF1("rw_fit",this,&TRWPeak::FitFunction,0,1,6,"TRWPeak","FitFunction");
   InitParNames();
   fFitFunction->SetParameter(1,centroid);
   SetListOfBGPar(std::vector<bool> {0,0,0,0,0,1});
   fFitFunction->SetLineColor(kMagenta);
}

void TRWPeak::InitParNames(){
   fFitFunction->SetParName(0, "Height");
   fFitFunction->SetParName(1, "centroid");
   fFitFunction->SetParName(2, "sigma");
   fFitFunction->SetParName(3, "beta");
   fFitFunction->SetParName(4, "R");
   fFitFunction->SetParName(5, "step");
}

void TRWPeak::InitializeParameters(TH1* fit_hist){
   // Makes initial guesses at parameters for the fit. Uses the histogram to
   Double_t xlow, xhigh, low, high;
   fFitFunction->GetRange(xlow, xhigh);
   Int_t bin     = fit_hist->FindBin(fFitFunction->GetParameter(1));
 //  Int_t binlow  = fitHist->GetXaxis()->FindBin(xlow);
 //  Int_t binhigh = fitHist->GetXaxis()->FindBin(xhigh);
   // Double_t binWidth = fitHist->GetBinWidth(bin);
   fFitFunction->SetParLimits(0, 0, fit_hist->GetMaximum()*1.5);
   fFitFunction->GetParLimits(1, low, high);
 //  if(low == high && low == 0.) {
 //     SetParLimits(1, xlow, xhigh);
 //  }
   fFitFunction->GetParLimits(2, low, high);
 //  if(low == high && low == 0.) {
      fFitFunction->SetParLimits(2, 0.1, 8); // sigma should be less than the window width - JKS
 //  }
   fFitFunction->SetParLimits(3, 0.000001, 10);
   fFitFunction->SetParLimits(4, 0.000001, 100); // this is a percentage. no reason for it to go to 500% - JKS
   // Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
   fFitFunction->SetParLimits(5, 0.0, 1.0E2);
   
   // Make initial guesses
   // Actually set the parameters in the photopeak function
   // Fixing has to come after setting
   // Might have to include bin widths eventually
   // The centroid should already be set by this point in the ctor
   fFitFunction->SetParameter("Height", fit_hist->GetBinContent(bin));
   fFitFunction->SetParameter("centroid", fFitFunction->GetParameter(1));
 //  fFitFunction->SetParameter("sigma", TMath::Sqrt(9.0 + 2. * fFitFunction->GetParameter("centroid") / 1000.) / 2.35);
   fFitFunction->SetParameter("sigma", TMath::Sqrt(2.25 + 1.33 * fFitFunction->GetParameter("centroid") / 1000. +  0.9*TMath::Power(fFitFunction->GetParameter("centroid")/1000.,2)) / 2.35);
   fFitFunction->SetParameter("beta", fFitFunction->GetParameter("sigma") / 2.0);
   fFitFunction->SetParameter("R", 0.001);
   fFitFunction->SetParameter("step", 1.0);
   fFitFunction->FixParameter(3, fFitFunction->GetParameter("beta"));
   fFitFunction->FixParameter(4, 0.00);
}

Double_t TRWPeak::Centroid() const{
   return fFitFunction->GetParameter("centroid");
}

Double_t TRWPeak::CentroidErr() const{
   return fFitFunction->GetParError(1);
}

Double_t TRWPeak::FitFunction(Double_t *dim, Double_t *par){
   
   Double_t x      = dim[0]; // channel number used for fitting
   Double_t height = par[0]; // height of photopeak
   Double_t c      = par[1]; // Peak Centroid of non skew gaus
   Double_t sigma  = par[2]; // standard deviation of gaussian
   Double_t beta   = par[3]; // Skewness parameter
   Double_t R      = par[4]; // relative height of skewed gaussian
   Double_t step   = par[5]; // Size of the step function;

   Double_t gauss      = height * (1.0 - R / 100.0) * TMath::Gaus(x, c, sigma);
   Double_t step_func  = TMath::Abs(step) * height / 100.0 * TMath::Erfc((x - c) / (TMath::Sqrt(2.0) * sigma));
   
   if(beta == 0.0)
      return gauss + step_func;
   else
      return gauss + step_func + R * height / 100.0 * (TMath::Exp((x - c) / beta)) *
          (TMath::Erfc(((x - c) / (TMath::Sqrt(2.0) * sigma)) + sigma / (TMath::Sqrt(2.0) * beta)));
}

void TRWPeak::Print(Option_t * opt) const{
   std::cout << "RadWare-like peak:" << std::endl;
   TSinglePeak::Print(opt);
}

