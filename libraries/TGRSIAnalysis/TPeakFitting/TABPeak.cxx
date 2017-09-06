#include "TABPeak.h"
#include "TH1.h"

/// \cond CLASSIMP
ClassImp(TABPeak)
/// \endcond

TABPeak::TABPeak() : TSinglePeak(){ }

TABPeak::TABPeak(Double_t centroid) : TSinglePeak() {

   fFitFunction = new TF1("ab_fit",this,&TABPeak::FitFunction,0,1,6,"TABPeak","FitFunction");
   InitParNames();
   fFitFunction->SetParameter(1,centroid);
   SetListOfBGPar(std::vector<bool> {0,0,0,0,0,1});
   fFitFunction->SetLineColor(kMagenta);
}

void TABPeak::InitParNames(){
   fFitFunction->SetParName(0, "Height");
   fFitFunction->SetParName(1, "centroid");
   fFitFunction->SetParName(2, "sigma");
   fFitFunction->SetParName(3, "rel_height");
   fFitFunction->SetParName(4, "rel_sigma");
   fFitFunction->SetParName(5, "step");
}

void TABPeak::InitializeParameters(TH1* fit_hist){
   // Makes initial guesses at parameters for the fit. Uses the histogram to
   Double_t xlow, xhigh, low, high;
   fFitFunction->GetRange(xlow, xhigh);
   Int_t bin     = fit_hist->FindBin(fFitFunction->GetParameter(1));
   fFitFunction->SetParLimits(0, 0, fit_hist->GetMaximum()*1.5);
   fFitFunction->GetParLimits(1, low, high);
   fFitFunction->GetParLimits(2, low, high);
   fFitFunction->SetParLimits(2, 0.1, 8); // sigma should be less than the window width - JKS
   fFitFunction->SetParLimits(3, 0.000001, 1.0);
   fFitFunction->SetParLimits(4, 1.0, 100); 
   // Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
   fFitFunction->SetParLimits(5, 0.0, 1.0E2);
   
   // Make initial guesses
   // Actually set the parameters in the photopeak function
   // Fixing has to come after setting
   // Might have to include bin widths eventually
   // The centroid should already be set by this point in the ctor
   fFitFunction->SetParameter("Height", fit_hist->GetBinContent(bin));
   fFitFunction->SetParameter("centroid", fFitFunction->GetParameter(1));
   fFitFunction->SetParameter("sigma", TMath::Sqrt(2.25 + 1.33 * fFitFunction->GetParameter("centroid") / 1000. +                                  0.9*TMath::Power(fFitFunction->GetParameter("centroid")/1000.,2)) / 2.35);
   fFitFunction->SetParameter("rel_sigma", fFitFunction->GetParameter(2)*2);
   fFitFunction->SetParameter("rel_height", fFitFunction->GetParameter(0)/2.);
   fFitFunction->SetParameter("step", 1.0);
}

Double_t TABPeak::Centroid() const{
   return fFitFunction->GetParameter("centroid");
}

Double_t TABPeak::CentroidErr() const{
   return fFitFunction->GetParError(1);
}

Double_t TABPeak::FitFunction(Double_t *dim, Double_t *par){
   
   Double_t x           = dim[0]; // channel number used for fitting
   Double_t height      = par[0]; // height of photopeak
   Double_t c           = par[1]; // Peak Centroid of non skew gaus
   Double_t sigma       = par[2]; // standard deviation of gaussian
   Double_t rel_height  = par[3]; // relative height of 2-hit peak
   Double_t rel_sigma   = par[4]; // relative sigma of 2-hit peak
   Double_t step        = par[5]; // Size of the step function;

   Double_t gauss1      = height * TMath::Gaus(x, c, sigma);
   Double_t gauss2      = height * rel_height * TMath::Gaus(x,c,rel_sigma*sigma);
   Double_t step_func   = TMath::Abs(step) * height / 100.0 * TMath::Erfc((x - c) / (TMath::Sqrt(2.0) * sigma));
   
   return gauss1 + gauss2 + step_func;
}

void TABPeak::Print(Option_t * opt) const{
   std::cout << "Addback-like peak:" << std::endl;
   TSinglePeak::Print(opt);
}


