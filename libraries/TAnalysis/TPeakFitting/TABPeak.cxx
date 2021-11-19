#include "TABPeak.h"
#include "TH1.h"

/// \cond CLASSIMP
ClassImp(TABPeak)
/// \endcond

TABPeak::TABPeak() : TSinglePeak() {}

TABPeak::TABPeak(Double_t centroid) : TSinglePeak()
{
	Centroid(centroid);
}

void TABPeak::Centroid(const Double_t& centroid)
{
   fTotalFunction = new TF1("ab_fit",this,&TABPeak::TotalFunction,0,1,6,"TABPeak","TotalFunction");
   InitParNames();
   fTotalFunction->SetParameter(1, centroid);
   SetListOfBGPar(std::vector<bool> {0,0,0,0,0,1});
   fTotalFunction->SetLineColor(kMagenta);
}

void TABPeak::InitParNames()
{
   fTotalFunction->SetParName(0, "Height");
   fTotalFunction->SetParName(1, "centroid");
   fTotalFunction->SetParName(2, "sigma");
   fTotalFunction->SetParName(3, "rel_height");
   fTotalFunction->SetParName(4, "rel_sigma");
   fTotalFunction->SetParName(5, "step");
}

void TABPeak::InitializeParameters(TH1* fit_hist)
{
   /// Makes initial guesses at parameters for the fit base on the histogram.
   // Make initial guesses
   // Actually set the parameters in the photopeak function
   // Fixing has to come after setting
   // Might have to include bin widths eventually
   // The centroid should already be set by this point in the ctor
	Int_t bin     = fit_hist->FindBin(fTotalFunction->GetParameter(1));
	if(!ParameterSetByUser(0)) {
		fTotalFunction->SetParLimits(0, 0, fit_hist->GetMaximum()*1.5);
		fTotalFunction->SetParameter("Height", fit_hist->GetBinContent(bin));
	}
	if(!ParameterSetByUser(2)) {
		fTotalFunction->SetParLimits(2, 0.1, 8);
		fTotalFunction->SetParameter("sigma", TMath::Sqrt(2.25 + 1.33 * fTotalFunction->GetParameter("centroid") / 1000. +0.9*TMath::Power(fTotalFunction->GetParameter("centroid")/1000.,2)) / 2.35);
	}
	if(!ParameterSetByUser(3)) {
		fTotalFunction->SetParLimits(3, 0.000001, 1.0);
		fTotalFunction->SetParameter("rel_height", 0.25);
	}
	if(!ParameterSetByUser(4)) {
		fTotalFunction->SetParLimits(4, 1.0, 100); 
		fTotalFunction->SetParameter("rel_sigma", 2.);
	}
	if(!ParameterSetByUser(5)) {
		// Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
		fTotalFunction->SetParLimits(5, 0.0, 1.0E2);
		fTotalFunction->SetParameter("step", 1.0);
	}
}

Double_t TABPeak::Centroid() const
{
	return fTotalFunction->GetParameter("centroid");
}

Double_t TABPeak::CentroidErr() const
{
	return fTotalFunction->GetParError(1);
}

Double_t TABPeak::Width() const
{
	return fTotalFunction->GetParameter("sigma")*fTotalFunction->GetParameter("rel_sigma");
}

Double_t TABPeak::PeakFunction(Double_t *dim, Double_t *par)
{
	return OneHitPeakFunction(dim,par) + TwoHitPeakFunction(dim,par);
}

Double_t TABPeak::OneHitPeakFunction(Double_t *dim, Double_t *par)
{
	Double_t x           = dim[0]; // channel number used for fitting
	Double_t height      = par[0]; // height of photopeak
	Double_t c           = par[1]; // Peak Centroid of non skew gaus
	Double_t sigma       = par[2]; // standard deviation of gaussian

	return height * TMath::Gaus(x, c, sigma);
}

Double_t TABPeak::TwoHitPeakFunction(Double_t *dim, Double_t *par)
{
	Double_t x           = dim[0]; // channel number used for fitting
	Double_t height      = par[0]; // height of photopeak
	Double_t c           = par[1]; // Peak Centroid of non skew gaus
	Double_t sigma       = par[2]; // standard deviation of gaussian
	Double_t rel_height  = par[3]; // relative height of 2-hit peak
	Double_t rel_sigma   = par[4]; // relative sigma of 2-hit peak

	return height * rel_height * TMath::Gaus(x,c,rel_sigma*sigma);
}

Double_t TABPeak::OneHitPeakOnGlobalFunction(Double_t *dim, Double_t *par)
{
	return OneHitPeakFunction(dim,par) + fGlobalBackground->EvalPar(dim, &par[fTotalFunction->GetNpar()]);
}

Double_t TABPeak::TwoHitPeakOnGlobalFunction(Double_t *dim, Double_t *par)
{
	return TwoHitPeakFunction(dim,par) + fGlobalBackground->EvalPar(dim, &par[fTotalFunction->GetNpar()]);
}

Double_t TABPeak::BackgroundFunction(Double_t *dim, Double_t *par)
{
	Double_t x           = dim[0]; // channel number used for fitting
	Double_t height      = par[0]; // height of photopeak
	Double_t c           = par[1]; // Peak Centroid of non skew gaus
	Double_t sigma       = par[2]; // standard deviation of gaussian
	Double_t step        = par[5]; // Size of the step function;

	Double_t step_func   = TMath::Abs(step) * height / 100.0 * TMath::Erfc((x - c) / (TMath::Sqrt(2.0) * sigma));

	return step_func;
}

void TABPeak::DrawComponents(Option_t * opt)
{
	//We need to draw this on top of the global background. Probably easiest to make another temporary TF1?
	if(!fGlobalBackground) return;

	Double_t low, high;
	fGlobalBackground->GetRange(low,high);
	if(fOneHitOnGlobal) fOneHitOnGlobal->Delete();
	if(fTwoHitOnGlobal) fTwoHitOnGlobal->Delete();
	//Make a copy of the total function, and then tack on the global background parameters.
	fOneHitOnGlobal = new TF1("draw_component1", this, &TABPeak::OneHitPeakOnGlobalFunction,low,high,fTotalFunction->GetNpar()+fGlobalBackground->GetNpar(),"TABPeak","OneHitPeakOnGlobalFunction");
	fTwoHitOnGlobal = new TF1("draw_component2", this, &TABPeak::TwoHitPeakOnGlobalFunction,low,high,fTotalFunction->GetNpar()+fGlobalBackground->GetNpar(),"TABPeak","TwoHitPeakOnGlobalFunction");
	for(int i = 0; i < fTotalFunction->GetNpar(); ++i) {
		fOneHitOnGlobal->SetParameter(i,fTotalFunction->GetParameter(i));
		fTwoHitOnGlobal->SetParameter(i,fTotalFunction->GetParameter(i));
	}
	for(int i = 0; i < fGlobalBackground->GetNpar(); ++i) {
		fOneHitOnGlobal->SetParameter(i+fTotalFunction->GetNpar(),fGlobalBackground->GetParameter(i));
		fTwoHitOnGlobal->SetParameter(i+fTotalFunction->GetNpar(),fGlobalBackground->GetParameter(i));
	}
	//Draw a copy of this function
	fOneHitOnGlobal->SetLineColor(fTotalFunction->GetLineColor());
	fTwoHitOnGlobal->SetLineColor(fTotalFunction->GetLineColor());
	fOneHitOnGlobal->SetLineStyle(8);
	fTwoHitOnGlobal->SetLineStyle(3);
	fOneHitOnGlobal->Draw(opt);
	fTwoHitOnGlobal->Draw(opt);
}

