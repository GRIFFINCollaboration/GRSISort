#include "TRWPeak.h"

/// \cond CLASSIMP
ClassImp(TRWPeak)
/// \endcond

TRWPeak::TRWPeak() : TSinglePeak(){ }

TRWPeak::TRWPeak(Double_t centroid) : TSinglePeak() {

   fTotalFunction = new TF1("rw_total",this,&TRWPeak::TotalFunction,0,1,6,"TRWPeak","TotalFunction");
   InitParNames();
   fTotalFunction->SetParameter(1,centroid);
   SetListOfBGPar(std::vector<bool> {0,0,0,0,0,1});
   fTotalFunction->SetLineColor(kMagenta);
}

void TRWPeak::InitParNames(){
   fTotalFunction->SetParName(0, "Height");
   fTotalFunction->SetParName(1, "centroid");
   fTotalFunction->SetParName(2, "sigma");
   fTotalFunction->SetParName(3, "beta");
   fTotalFunction->SetParName(4, "R");
   fTotalFunction->SetParName(5, "step");
}

void TRWPeak::InitializeParameters(TH1* fit_hist){
   // Makes initial guesses at parameters for the fit. Uses the histogram to
   Double_t xlow, xhigh, low, high;
   fTotalFunction->GetRange(xlow, xhigh);
   Int_t bin     = fit_hist->FindBin(fTotalFunction->GetParameter(1));
   fTotalFunction->SetParLimits(0, 0, fit_hist->GetMaximum()*2.);
   fTotalFunction->GetParLimits(1, low, high);
   fTotalFunction->GetParLimits(2, low, high);
   fTotalFunction->SetParLimits(2, 0.01, 10.); // sigma should be less than the window width - JKS
   fTotalFunction->SetParLimits(3, 0.000001, 10);
   fTotalFunction->SetParLimits(4, 0.000001, 100); // this is a percentage. no reason for it to go to 500% - JKS
   // Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
   fTotalFunction->SetParLimits(5, 0.0, 1.0E2);
   
   // Make initial guesses
   // Actually set the parameters in the photopeak function
   // Fixing has to come after setting
   // Might have to include bin widths eventually
   // The centroid should already be set by this point in the ctor
   fTotalFunction->SetParameter("Height", fit_hist->GetBinContent(bin));
   fTotalFunction->SetParameter("centroid", fTotalFunction->GetParameter(1));
 //  fTotalFunction->SetParameter("sigma", TMath::Sqrt(9.0 + 2. * fTotalFunction->GetParameter("centroid") / 1000.) / 2.35);
   fTotalFunction->SetParameter("sigma", TMath::Sqrt(5 + 1.33 * fTotalFunction->GetParameter("centroid") / 1000. +  0.9*TMath::Power(fTotalFunction->GetParameter("centroid")/1000.,2)) / 2.35);
   fTotalFunction->SetParameter("beta", fTotalFunction->GetParameter("sigma") / 2.0);
   fTotalFunction->SetParameter("R", 0.001);
   fTotalFunction->SetParameter("step", 0.1);
   fTotalFunction->FixParameter(3, fTotalFunction->GetParameter("beta"));
   fTotalFunction->FixParameter(4, 0.00);
}

Double_t TRWPeak::Centroid() const{
   return fTotalFunction->GetParameter("centroid");
}

Double_t TRWPeak::CentroidErr() const{
   return fTotalFunction->GetParError(1);
}

Double_t TRWPeak::PeakFunction(Double_t *dim, Double_t *par){
   
   Double_t x      = dim[0]; // channel number used for fitting
   Double_t height = par[0]; // height of photopeak
   Double_t c      = par[1]; // Peak Centroid of non skew gaus
   Double_t sigma  = par[2]; // standard deviation of gaussian
   Double_t beta   = par[3]; // Skewness parameter
   Double_t R      = par[4]; // relative height of skewed gaussian

   Double_t gauss      = height * (1.0 - R / 100.0) * TMath::Gaus(x, c, sigma);
   
   if(beta == 0.0)
      return gauss;
   else
      return gauss + R * height / 100.0 * (TMath::Exp((x - c) / beta)) *
          (TMath::Erfc(((x - c) / (TMath::Sqrt(2.0) * sigma)) + sigma / (TMath::Sqrt(2.0) * beta)));
}

Double_t TRWPeak::BackgroundFunction(Double_t *dim, Double_t *par){
   
   Double_t x      = dim[0]; // channel number used for fitting
   Double_t height = par[0]; // height of photopeak
   Double_t c      = par[1]; // Peak Centroid of non skew gaus
   Double_t sigma  = par[2]; // standard deviation of gaussian
   Double_t step   = par[5]; // Size of the step function;

   Double_t step_func  = TMath::Abs(step) * height / 100.0 * TMath::Erfc((x - c) / (TMath::Sqrt(2.0) * sigma));
   
   return step_func;
}


void TRWPeak::Print(Option_t * opt) const{
   std::cout << "RadWare-like peak:" << std::endl;
   TSinglePeak::Print(opt);
}

