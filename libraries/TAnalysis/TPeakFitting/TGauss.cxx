#include "TGauss.h"

/// \cond CLASSIMP
ClassImp(TGauss)
/// \endcond

TGauss::TGauss() : TSinglePeak() { }

TGauss::TGauss(Double_t centroid, Double_t relativeLimit) : TSinglePeak()
{
	Centroid(centroid);
	if(relativeLimit >= 0) {
		fTotalFunction->SetParLimits(1, (1.-relativeLimit)*centroid, (1.+relativeLimit)*centroid);
	}
}

void TGauss::Centroid(const Double_t& centroid)
{
   fTotalFunction = new TF1("gauss_total",this,&TGauss::TotalFunction,0,1,3,"TGauss","TotalFunction");
   fPeakFunction =  new TF1("gauss_peak",this,&TGauss::PeakFunction,0,1,3,"TGauss","TotalFunction"); // peak = total function
   InitParNames();
   fTotalFunction->SetParameter(1, centroid);
   SetListOfBGPar(std::vector<bool> {0,0,0,0,0,1});
   fTotalFunction->SetLineColor(kMagenta);
}

void TGauss::InitParNames()
{
   fTotalFunction->SetParName(0, "height");
   fTotalFunction->SetParName(1, "centroid");
   fTotalFunction->SetParName(2, "sigma");
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
   Int_t bin     = fit_hist->FindBin(fTotalFunction->GetParameter(1));
   if(!ParameterSetByUser(0)) {
		fTotalFunction->SetParameter("height", fit_hist->GetBinContent(bin));
		fTotalFunction->SetParLimits(0, 0, fit_hist->GetMaximum()*2.);
	}
	// no sense checking whether the centroid has been set, this always gets set in the constructor
	fTotalFunction->SetParLimits(1, rangeLow, rangeHigh);
	if(!ParameterSetByUser(2)) {
		fTotalFunction->SetParameter("sigma", TMath::Sqrt(5 + 1.33 * fTotalFunction->GetParameter("centroid") / 1000. +  0.9*TMath::Power(fTotalFunction->GetParameter("centroid")/1000.,2)) / 2.35);
		fTotalFunction->SetParLimits(2, 0.01, 10.);
	}
}

Double_t TGauss::Centroid() const
{
   return fTotalFunction->GetParameter("centroid");
}

Double_t TGauss::CentroidErr() const
{
   return fTotalFunction->GetParError(1);
}

Double_t TGauss::PeakFunction(Double_t *dim, Double_t *par)
{
   Double_t x      = dim[0]; // channel number used for fitting
   Double_t height = par[0]; // height of photopeak
   Double_t c      = par[1]; // centroid of gaussian
   Double_t sigma  = par[2]; // standard deviation of gaussian

   Double_t gauss      = height * TMath::Gaus(x, c, sigma);
   
	return gauss;
}
