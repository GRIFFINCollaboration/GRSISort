#include "TPeak.h"
#include "TGraph.h"

#include "Math/Minimizer.h"

/// \cond CLASSIMP
ClassImp(TPeak)
/// \endcond

Bool_t TPeak::fLogLikelihoodFlag = true;
TPeak* TPeak::fLastFit           = nullptr;

TPeak::TPeak(Double_t cent, Double_t xlow, Double_t xhigh, TF1* background)
   : TGRSIFit("photopeakbg", TGRSIFunctions::PhotoPeakBG, xlow, xhigh, 10)
{
   Clear();
   Bool_t outOfRangeFlag = false;

   if(cent > xhigh) {
      std::cout << "centroid is higher than range" << std::endl;
      outOfRangeFlag = true;
   } else if(cent < xlow) {
      std::cout << "centroid is lower than range" << std::endl;
      outOfRangeFlag = true;
   }

   // This fixes things if your user is like me and screws up a lot.
   if(outOfRangeFlag) {
      if(xlow > cent) {
         std::swap(xlow, cent);
      }
      if(xlow > xhigh) {
         std::swap(xlow, xhigh);
      }
      if(cent > xhigh) {
         std::swap(cent, xhigh);
      }
      std::cout << "Something about your range was wrong. Assuming:" << std::endl;
      std::cout << "centroid: " << cent << " \t range: " << xlow << " to " << xhigh << std::endl;
   }

   SetRange(xlow, xhigh);
   // We also need to make initial guesses at parameters
   // We need nice ways to access parameters etc.
   // Need to make a TMultipeak-like thing (does a helper class come into play then?)

   // Set the fit function to be a radware style photo peak.
   // This function might be unnecessary. Will revist this later. rd.
   SetName(Form("Chan%d_%d_to_%d", static_cast<Int_t>(cent), static_cast<Int_t>(xlow),
                static_cast<Int_t>(xhigh)));   // Gives a default name to the peak

   // We need to set parameter names now.
   InitNames();
   SetParameter("centroid", cent);

   // Check to see if background is good.
   if(background != nullptr) {
      fBackground = background;
      fOwnBgFlag  = false;
   } else {
      fBackground = new TF1(
         Form("background%d_%d_to_%d", static_cast<Int_t>(cent), static_cast<Int_t>(xlow), static_cast<Int_t>(xhigh)),
         TGRSIFunctions::StepBG, xlow, xhigh, 10);
      TGRSIFit::AddToGlobalList(fBackground, kFALSE);
      fOwnBgFlag = true;
   }

   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);

   fResiduals = new TGraph;
}

TPeak::TPeak() : TGRSIFit("photopeakbg", TGRSIFunctions::PhotoPeakBG, 0, 1000, 10)
{
   InitNames();
   fBackground = new TF1("background", TGRSIFunctions::StepBG, 0, 1000, 10);
   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);
   TGRSIFit::AddToGlobalList(fBackground, kFALSE);

   fResiduals = new TGraph;
}

TPeak::~TPeak()
{
   if(fOwnBgFlag) {
      delete fBackground;
   }
	delete fResiduals;
}

TPeak::TPeak(const TPeak& copy) : TGRSIFit(), fBackground(nullptr), fResiduals(nullptr)
{
   copy.Copy(*this);
}

void TPeak::InitNames()
{
   SetParName(0, "Height");
   SetParName(1, "centroid");
   SetParName(2, "sigma");
   SetParName(3, "beta");
   SetParName(4, "R");
   SetParName(5, "step");
   SetParName(6, "A");
   SetParName(7, "B");
   SetParName(8, "C");
   SetParName(9, "bg_offset");
}

void TPeak::Copy(TObject& obj) const
{
   TGRSIFit::Copy(obj);

   if(static_cast<TPeak&>(obj).fBackground == nullptr) {
      static_cast<TPeak&>(obj).fBackground = new TF1(*fBackground);
   }
   if(static_cast<TPeak&>(obj).fResiduals == nullptr) {
      static_cast<TPeak&>(obj).fResiduals = new TGraph(*fResiduals);
   }

   static_cast<TPeak&>(obj).fArea  = fArea;
   static_cast<TPeak&>(obj).fDArea = fDArea;

   static_cast<TPeak&>(obj).fChi2 = fChi2;
   static_cast<TPeak&>(obj).fNdf  = fNdf;

   *(static_cast<TPeak&>(obj).fBackground) = *fBackground;
   // We are making a direct copy of the background so the ownership is that of the copy
   static_cast<TPeak&>(obj).fOwnBgFlag    = true;
   *(static_cast<TPeak&>(obj).fResiduals) = *fResiduals;

   static_cast<TPeak&>(obj).SetHist(GetHist());
}

Bool_t TPeak::InitParams(TH1* fitHist)
{
   // Makes initial guesses at parameters for the fit. Uses the histogram to
   if(fitHist == nullptr) {
      return false;
   }
   Double_t xlow = 0.;
	Double_t xhigh = 0.;
   GetRange(xlow, xhigh);
   Int_t bin     = fitHist->FindBin(GetParameter("centroid"));
   Int_t binlow  = fitHist->GetXaxis()->FindBin(xlow);
   Int_t binhigh = fitHist->GetXaxis()->FindBin(xhigh);
   SetParLimits(0, 0, fitHist->GetMaximum());
   Double_t low = 0.;
	Double_t high = 0.;
   GetParLimits(1, low, high);
   if(low == high && low == 0.) {
      SetParLimits(1, xlow, xhigh);
   }
   GetParLimits(2, low, high);
   if(low == high && low == 0.) {
      SetParLimits(2, 0.5, (xhigh - xlow));   // sigma should be less than the window width - JKS
   }
   SetParLimits(3, 0.000001, 10);
   SetParLimits(4, 0.000001, 100);   // this is a percentage. no reason for it to go to 500% - JKS
   // Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
   SetParLimits(5, 0.0, 1.0E2);
   SetParLimits(6, 0.0, fitHist->GetBinContent(bin) * 100.);
   SetParLimits(9, xlow, xhigh);

   if((fitHist == nullptr) && (GetHist() != nullptr)) {
      fitHist = GetHist();
   }

   if(fitHist == nullptr) {
      std::cout << "No histogram is associated yet, no initial guesses made" << std::endl;
      return false;
   }
   // Make initial guesses
   // Actually set the parameters in the photopeak function
   // Fixing has to come after setting
   // Might have to include bin widths eventually
   // The centroid should already be set by this point in the ctor
   SetParameter("Height", fitHist->GetBinContent(bin));
   SetParameter("centroid", GetParameter("centroid"));
   SetParameter("sigma", TMath::Sqrt(9.0 + 4. * GetParameter("centroid") / 1000.) / 2.35);
   SetParameter("beta", GetParameter("sigma") / 2.0);
   SetParameter("R", 0.001);
   SetParameter("step", 1.0);
   SetParameter("A", fitHist->GetBinContent(binhigh));
   SetParameter("B", (fitHist->GetBinContent(binlow) - fitHist->GetBinContent(binhigh)) / (xlow - xhigh));
   SetParameter("C", 0.0000);
   SetParameter("bg_offset", GetParameter("centroid"));
   FixParameter(8, 0.00);
   FixParameter(3, GetParameter("beta"));
   FixParameter(4, 0.00);
   SetInitialized();
   return true;
}

Bool_t TPeak::Fit(TH1* fitHist, Option_t* opt)
{
   TString options = opt;
   options.ToLower();
   bool quiet   = options.Contains("q");
   bool verbose = options.Contains("v");
   if(quiet && verbose) {
      std::cout << "Don't know how to be quiet and verbose at once (" << opt << "), going to be verbose!" << std::endl;
      quiet = false;
   }
   bool retryFit = options.Contains("retryfit");
   options.ReplaceAll("retryfit", "");
   if(!verbose && !quiet) { options.Append("q"); }

   if(fitHist == nullptr && GetHist() == nullptr) {
      std::cout << "No hist passed, trying something... ";
      fitHist = fHistogram;
   }
   if(fitHist == nullptr) {
      std::cout << "No histogram associated with Peak" << std::endl;
      return false;
   }
   if(!IsInitialized()) {
      InitParams(fitHist);
   }
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Combination");
   TVirtualFitter::SetMaxIterations(100000);
   TVirtualFitter::SetPrecision(1e-3);

   SetHist(fitHist);

   // Now that it is initialized, let's fit it.
   // Just in case the range changed, we should reset the centroid and bg energy limits
   // check first if the parameter has been fixed!
   std::vector<double> lowerLimit(GetNpar());
   std::vector<double> upperLimit(GetNpar());
   for(int i = 0; i < GetNpar(); ++i) {
      GetParLimits(i, lowerLimit[i], upperLimit[i]);
      if(i < 2) {   // height, and centroid
         FixParameter(i, GetParameter(i));
      }
   }

   TFitResultPtr fitres;
   // Log likelihood is the proper fitting technique UNLESS the data is a result of an addition or subtraction.
   if(GetLogLikelihoodFlag()) {
      fitres = fitHist->Fit(this, Form("%sRLSN", options.Data()));   // The RS needs to always be there
   } else {
      fitres = fitHist->Fit(this, Form("%sRSN", options.Data()));   // The RS needs to always be there
   }

   // Check fit exited successfully before continuing
   if(static_cast<int>(fitres) == -1) { return false; }

   for(int i = 0; i < GetNpar(); ++i) {
      SetParLimits(i, lowerLimit[i], upperLimit[i]);
   }

   // Log likelihood is the proper fitting technique UNLESS the data is a result of an addition or subtraction.
   if(GetLogLikelihoodFlag()) {
      fitres = fitHist->Fit(this, Form("%sRLS", options.Data()));   // The RS needs to always be there
   } else {
      fitres = fitHist->Fit(this, Form("%sRS", options.Data()));   // The RS needs to always be there
   }

   // Check fit exited successfully before continuing
   if(static_cast<int>(fitres) == -1) { return false; }

   // After performing this fit I want to put something here that takes the fit result (good,bad,etc)
   // for printing out. RD

   if(fitres->ParError(2) != fitres->ParError(2)) {   // Check to see if nan
      if(fitres->Parameter(3) < 1) {
         InitParams(fitHist);
         FixParameter(4, 0);
         FixParameter(3, 1);
         if(verbose) { std::cout << "Beta may have broken the fit, retrying with R=0" << std::endl; }
         // Leaving the log-likelihood argument out so users are not constrained to just using that. - JKS
         fitHist->GetListOfFunctions()->Last()->Delete();
         if(GetLogLikelihoodFlag()) {
            fitres = fitHist->Fit(this, Form("%sRLS", options.Data()));   // The RS needs to always be there
         } else {
            fitres = fitHist->Fit(this, Form("%sRS", options.Data()));
         }
      }
   }

   // check parameter errors
   if(!TGRSIFunctions::CheckParameterErrors(fitres, options.Data())) {
      if(retryFit) {
         // fit again with all parameters released
         if(!quiet) { std::cout << GREEN << "Re-fitting with released parameters (without any limits)" << RESET_COLOR << std::endl; }
         for(int i = 0; i < GetNpar(); ++i) {
            ReleaseParameter(i);
         }
         if(GetLogLikelihoodFlag()) {
            fitres = fitHist->Fit(this, Form("%sRLS", options.Data()));   // The RS needs to always be there
         } else {
            fitres = fitHist->Fit(this, Form("%sRS", options.Data()));
         }
      } else {
         // re-try using minos instead of minuit
         if(!quiet) { std::cout << YELLOW << "Re-fitting with \"E\" option to get better error estimation using Minos technique." << RESET_COLOR << std::endl; }
         if(GetLogLikelihoodFlag()) {
            fitres = fitHist->Fit(this, Form("%sERLS", options.Data()));   // The RS needs to always be there
         } else {
            fitres = fitHist->Fit(this, Form("%sERS", options.Data()));
         }
      }
   }
   TGRSIFunctions::CheckParameterErrors(fitres);

   Double_t binWidth = fitHist->GetBinWidth(GetParameter("centroid"));
   Double_t width    = GetParameter("sigma");
   if(verbose) {
      std::cout << "Chi^2/NDF = " << fitres->Chi2() / fitres->Ndf() << std::endl;
   }
   fChi2 = fitres->Chi2();
   fNdf  = fitres->Ndf();
   Double_t xlow = 0.;
	Double_t xhigh = 0.;
   GetRange(xlow, xhigh);
   Double_t int_low  = xlow - 10. * width;   // making the integration bounds a bit smaller, but still large enough. -JKS
   Double_t int_high = xhigh + 10. * width;

   // Make a function that does not include the background
   // Integrate the background.
   // TPeak* tmppeak = new TPeak(*this);
   auto* tmppeak = new TPeak;
   Copy(*tmppeak);
   tmppeak->SetParameter("step", 0.0);
   tmppeak->SetParameter("A", 0.0);
   tmppeak->SetParameter("B", 0.0);
   tmppeak->SetParameter("C", 0.0);
   tmppeak->SetParameter("bg_offset", 0.0);
   tmppeak->SetRange(int_low, int_high);   // This will help get the true area of the gaussian 200 ~ infinity in a gaus
   tmppeak->SetName("tmppeak");

   // SOMETHING IS WRONG WITH THESE UNCERTAINTIES
   // This is where we will do integrals and stuff.
   fArea = (tmppeak->Integral(int_low, int_high)) / binWidth;
   // Set the background values in the covariance matrix to 0, while keeping their covariance errors
   TMatrixDSym CovMat = fitres->GetCovarianceMatrix();
   CovMat(5, 5)       = 0.0;
   CovMat(6, 6)       = 0.0;
   CovMat(7, 7)       = 0.0;
   CovMat(8, 8)       = 0.0;
   CovMat(9, 9)       = 0.0;
   fDArea             = (tmppeak->IntegralError(int_low, int_high, tmppeak->GetParameters(), CovMat.GetMatrixArray())) / binWidth;

   if(verbose) {
      std::cout << "Integral: " << fArea << " +/- " << fDArea << std::endl;
   }
   // Set the background for drawing later
   fBackground->SetParameters(GetParameters());
   // To DO: put a flag in signalling that the errors are not to be trusted if we have a bad cov matrix
   Copy(*fitHist->GetListOfFunctions()->Last());

   // always print result of the fit even if not verbose
   if(!quiet) { Print("+"); }
   delete tmppeak;
   fLastFit = this;
   return true;
}

void TPeak::Clear(Option_t*)
{
   // Clear the TPeak including functions and histogram, does not
   // currently clear inherited members such as name.
   // want to make a clear that doesn't clear everything
   fArea  = 0.0;
   fDArea = 0.0;
   fChi2  = 0.0;
   fNdf   = 0.0;
   TGRSIFit::Clear();
   // Do deep clean stuff maybe? require an option?
}

void TPeak::Print(Option_t* opt) const
{
   // Prints TPeak properties. To see More properties use the option "+"
   std::cout << "Name:        " << GetName() << std::endl;
   std::cout << "Centroid:    " << GetParameter("centroid") << " +/- " << GetParError(GetParNumber("centroid")) << std::endl;
   std::cout << "Area: 	      " << fArea << " +/- " << fDArea << std::endl;
   std::cout << "FWHM:        " << GetParameter("sigma") * 2.3548 << " +/- " << GetParError(GetParNumber("sigma")) * 2.3548 << std::endl;
   std::cout << "Chi^2/NDF:   " << fChi2 / fNdf << std::endl;
   if(strchr(opt, '+') != nullptr) {
      TF1::Print();
      TGRSIFit::Print(opt);   // Polymorphise this a bit better
   }
}

void TPeak::DrawBackground(Option_t* opt) const
{
   fBackground->Draw(opt);
}

void TPeak::DrawResiduals()
{
   if(GetHist() == nullptr) {
      std::cout << "No hist set" << std::endl;
      return;
   }
   if(fChi2 < 0.000000001) {
      std::cout << "No fit performed" << std::endl;
      return;
   }

   Double_t xlow = 0.;
	Double_t xhigh = 0.;
   GetRange(xlow, xhigh);
   Int_t nbins  = GetHist()->GetXaxis()->GetNbins();
   auto* res    = new Double_t[nbins];
   auto* bin    = new Double_t[nbins];
   Int_t points = 0;
   fResiduals->Clear();

   for(int i = 1; i <= nbins; i++) {
      if(GetHist()->GetBinCenter(i) <= xlow || GetHist()->GetBinCenter(i) >= xhigh) {
         continue;
      }
      res[points] = (GetHist()->GetBinContent(i) - Eval(GetHist()->GetBinCenter(i))) +
                    GetParameter("Height") / 2;   /// GetHist()->GetBinError(i));// + GetParameter("Height") + 10.;
      bin[points] = GetHist()->GetBinCenter(i);
      fResiduals->SetPoint(i, bin[i], res[i]);

      points++;
   }

   fResiduals->Draw();

   delete[] res;
   delete[] bin;
}

bool TPeak::GoodStatus()
{
   if(GetHist() == nullptr) {
      std::cout << "No hist set" << std::endl;
      return false;
   }
   if(fChi2 < 0.000000001) {
      std::cout << "No fit performed" << std::endl;
      return false;
   }
   return true;
}

Double_t TPeak::GetIntegralArea()
{
   if(!GoodStatus()) { return 0.; }

   Double_t width = GetParameter("sigma");
   Double_t xlow, xhigh;
   Double_t int_low, int_high;
   GetRange(xlow, xhigh);
   int_low  = xlow - 10. * width;   // making the integration bounds a bit smaller, but still large enough. -JKS
   int_high = xhigh + 10. * width;
   return GetIntegralArea(int_low, int_high);
}

Double_t TPeak::GetIntegralArea(Double_t int_low, Double_t int_high)
{
   if(!GoodStatus()) { return 0.; }

   // pull appropriate properties from peak and histogram
   TH1* hist = GetHist();

   // use those properties to integrate the histogram
   Int_t    binlow        = hist->FindBin(int_low);
   Int_t    binhigh       = hist->FindBin(int_high);
   Double_t binWidth      = hist->GetBinWidth(binlow);
   Double_t hist_integral = hist->Integral(binlow, binhigh);
   Double_t xlow          = hist->GetXaxis()->GetBinLowEdge(binlow);
   Double_t xhigh         = hist->GetXaxis()->GetBinUpEdge(binhigh);

   // ... and then integrate the background
   Double_t bg_area = (Background()->Integral(xlow, xhigh)) / binWidth;

   // calculate the peak area and error
   Double_t peakarea = hist_integral - bg_area;

   return peakarea;
}

Double_t TPeak::GetIntegralAreaErr(Double_t int_low, Double_t int_high)
{
   if(!GoodStatus()) { return 0.; }

   // pull appropriate properties from peak and histogram
   TH1* hist = GetHist();

   // use those properties to integrate the histogram
   Int_t    binlow        = hist->FindBin(int_low);
   Int_t    binhigh       = hist->FindBin(int_high);
   Double_t binWidth      = hist->GetBinWidth(binlow);
   Double_t hist_integral = hist->Integral(binlow, binhigh);
   Double_t xlow          = hist->GetXaxis()->GetBinLowEdge(binlow);
   Double_t xhigh         = hist->GetXaxis()->GetBinUpEdge(binhigh);

   // ... and then integrate the background
   Double_t bg_area = (Background()->Integral(xlow, xhigh)) / binWidth;

   // calculate the peak error
   Double_t peakerr = sqrt(hist_integral + bg_area);

   return peakerr;
}

Double_t TPeak::GetIntegralAreaErr()
{
   if(!GoodStatus()) { return 0.; }

   Double_t width = GetParameter("sigma");
   Double_t xlow = 0.;
	Double_t xhigh = 0.;
   GetRange(xlow, xhigh);
   Double_t int_low  = xlow - 10. * width;   // making the integration bounds a bit smaller, but still large enough. -JKS
   Double_t int_high = xhigh + 10. * width;
   return GetIntegralAreaErr(int_low, int_high);
}

void TPeak::CheckArea(Double_t int_low, Double_t int_high)
{
   if(!GoodStatus()) { return; }

   // calculate the peak area and error
   Double_t peakarea = GetIntegralArea(int_low, int_high);
   Double_t peakerr  = GetIntegralAreaErr(int_low, int_high);

   // now print properties
   std::cout << "TPeak integral: 	        " << fArea << " +/- " << fDArea << std::endl;
   std::cout << "Histogram - BG integral:  " << peakarea << " +/- " << peakerr << std::endl;
   if(std::abs(peakarea - fArea) < (fDArea + peakerr)) {
      std::cout << DGREEN << "Areas are consistent." << RESET_COLOR << std::endl;
   } else if(std::abs(peakarea - fArea) < 2 * (fDArea + peakerr)) {
      std::cout << DYELLOW << "Areas are consistent within 2 sigma." << RESET_COLOR << std::endl;
   } else {
      std::cout << DRED << "Areas are inconsistent." << RESET_COLOR << std::endl;
   }
}

void TPeak::CheckArea()
{
   if(!GoodStatus()) { return; }

   // calculate the peak area and error
   Double_t peakarea = GetIntegralArea();
   Double_t peakerr  = GetIntegralAreaErr();

   // now print properties
   std::cout << "TPeak integral: 	        " << fArea << " +/- " << fDArea << std::endl;
   std::cout << "Histogram - BG integral:  " << peakarea << " +/- " << peakerr << std::endl;
   if(std::abs(peakarea - fArea) < (fDArea + peakerr)) {
      std::cout << DGREEN << "Areas are consistent." << RESET_COLOR << std::endl;
   } else if(std::abs(peakarea - fArea) < 2 * (fDArea + peakerr)) {
      std::cout << DYELLOW << "Areas are consistent within 2 sigma." << RESET_COLOR << std::endl;
   } else {
      std::cout << DRED << "Areas are inconsistent." << RESET_COLOR << std::endl;
   }
}
