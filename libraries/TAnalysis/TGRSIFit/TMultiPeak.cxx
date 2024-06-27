#include "TMultiPeak.h"

#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

#include <algorithm>

Bool_t TMultiPeak::fLogLikelihoodFlag = false;

TMultiPeak::TMultiPeak(Double_t xlow, Double_t xhigh, const std::vector<Double_t>& centroids, Option_t*)
   : TGRSIFit("multipeakbg", this, &TMultiPeak::MultiPhotoPeakBG, xlow, xhigh, centroids.size() * 6 + 5, "TMultiPeak", "MultiPhotoPeakBG")
{
   std::cout << "Warning, the class TMultiPeak is deprecated!" << std::endl;
   Clear();
   // We make the background first so we can send it to the TPeaks.
   fBackground = new TF1(Form("MPbackground_%d_to_%d", static_cast<Int_t>(xlow), static_cast<Int_t>(xhigh)), this, &TMultiPeak::MultiStepBG, xlow, xhigh, centroids.size() * 6 + 5, "TMuliPeak", "MultiStepBG");
   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);
   TGRSIFit::AddToGlobalList(fBackground, kFALSE);

   for(double cent : centroids) {
      Bool_t out_of_range_flag = false;
      if(cent > xhigh) {
         std::cout << "centroid " << cent << " is higher than range" << std::endl;
         out_of_range_flag = true;
      } else if(cent < xlow) {
         std::cout << "centroid " << cent << " is lower than range" << std::endl;
         out_of_range_flag = true;
      }
      if(out_of_range_flag) {
         std::cout << "ignoring peak at " << cent << ", make a new multi peak with the corrected energy" << std::endl;
      } else {
         auto* peak = new TPeak(cent, xlow, xhigh, fBackground);
         peak->AddToGlobalList(kFALSE);
         fPeakVec.push_back(peak);
      }
   }
   SetRange(xlow, xhigh);

   SetName(Form("MultiPeak_%d_to_%d", static_cast<Int_t>(xlow),
                static_cast<Int_t>(xhigh)));   // Gives a default name to the peak
   SortPeaks();                                // Defaults to sorting by TPeak::CompareEnergy
   InitNames();
}

TMultiPeak::TMultiPeak() : TGRSIFit("multipeakbg", this, &TMultiPeak::MultiPhotoPeakBG, 0, 1000, 10, "TMultiPeak", "MultiPhotoPeakBG")
{
   std::cout << "Warning, the class TMultiPeak is deprecated!" << std::endl;
   // I don't think this constructor should be used, RD.
   InitNames();
   fBackground = new TF1("background", this, &TMultiPeak::MultiStepBG, 1000, 10, 10, "TMultiPeak", "MultiStepBG");   // This is a weird nonsense line.
   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);
   TGRSIFit::AddToGlobalList(fBackground, kFALSE);
}

TMultiPeak::~TMultiPeak()
{
	delete fBackground;

   for(auto& peak : fPeakVec) {
		delete peak;
   }
}

void TMultiPeak::SortPeaks(Bool_t (*SortFunction)(const TPeak*, const TPeak*))
{
   std::sort(fPeakVec.begin(), fPeakVec.end(), SortFunction);
}

void TMultiPeak::InitNames()
{
   SetParName(0, "N_Peaks");
   SetParName(1, "A");
   SetParName(2, "B");
   SetParName(3, "C");
   SetParName(4, "bg_offset");

   for(int i = 0; i < static_cast<int>(fPeakVec.size()); ++i) {
      SetParName(6 * i + 5, Form("Height_%i", i));
      SetParName(6 * i + 6, Form("Centroid_%i", i));
      SetParName(6 * i + 7, Form("Sigma_%i", i));
      SetParName(6 * i + 8, Form("Beta_%i", i));
      SetParName(6 * i + 9, Form("R_%i", i));
      SetParName(6 * i + 10, Form("Step_%i", i));
   }
   FixParameter(0, fPeakVec.size());
}

TMultiPeak::TMultiPeak(const TMultiPeak& copy) : TGRSIFit(), fBackground(nullptr)
{
   copy.Copy(*this);
}

void TMultiPeak::Copy(TObject& obj) const
{
   TGRSIFit::Copy(obj);
   auto* mpobj = static_cast<TMultiPeak*>(&obj);
   if((mpobj->fBackground) == nullptr) {
      mpobj->fBackground = new TF1(*(fBackground));
   } else {
      *(mpobj->fBackground) = *fBackground;
   }

   TGRSIFit::AddToGlobalList(fBackground, kFALSE);

   // Copy all of the TPeaks.
   for(auto* i : fPeakVec) {
      auto* peak = new TPeak(*i);
      peak->AddToGlobalList(kFALSE);
      mpobj->fPeakVec.push_back(peak);
   }
}

Bool_t TMultiPeak::InitParams(TH1* fithist)
{
   // Makes initial guesses at parameters for the fit. Uses the histogram to make the initial guesses
   if((fithist == nullptr) && (GetHist() != nullptr)) {
      fithist = GetHist();
   }

   if(fithist == nullptr) {
      std::cout << "No histogram is associated yet, no initial guesses made" << std::endl;
      return false;
   }

   FixParameter(0, fPeakVec.size());
   // This is the range for the fit.
   Double_t xlow = 0;
	Double_t xhigh = 0;
   GetRange(xlow, xhigh);
   Int_t binlow  = fithist->GetXaxis()->FindBin(xlow);
   Int_t binhigh = fithist->GetXaxis()->FindBin(xhigh);

   // Initialize background
   SetParLimits(1, 0.0, fithist->GetBinContent(binlow) * 100.0);
   SetParameter("A", fithist->GetBinContent(binlow));
   SetParameter("B", (fithist->GetBinContent(binlow) - fithist->GetBinContent(binhigh)) / (xlow - xhigh));
   SetParameter("C", 0.0000);
   SetParameter("bg_offset", (xhigh + xlow) / 2.0);

   FixParameter(3, 0);

   // We need to initialize parameters for every peak in the fit
   for(int i = 0; i < static_cast<int>(fPeakVec.size()); ++i) {
      Double_t centroid = fPeakVec.at(i)->GetCentroid();
      Int_t    bin      = fithist->GetXaxis()->FindBin(centroid);
      SetParLimits(6 * i + 5, 0, fithist->GetBinContent(bin) * 5.);
      SetParLimits(6 * i + 6, centroid - 4, centroid + 4);
      // SetParLimits(6*i+7,0.1,xhigh-xlow);//This will be linked to other peaks eventually.
      SetParLimits(6 * i + 7, 0.1, 1.5);   // This will be linked to other peaks eventually.
      SetParLimits(6 * i + 8, 0.000001, 10);
      SetParLimits(6 * i + 9, 0.000001, 100);
      SetParLimits(6 * i + 10, 0.0, 1.0E2);
      // Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0

      // Now set the actual paramter to start the fit from these points
      SetParameter(Form("Height_%i", i), fithist->GetBinContent(bin));
      SetParameter(Form("Centroid_%i", i), centroid);
      //  SetParameter("sigma",(xhigh-xlow)*0.5); // slightly more robust starting value for sigma -JKS
      //  SetParameter("sigma",1.0/binWidth); // slightly more robust starting value for sigma -JKS
      SetParameter(Form("Sigma_%i", i), TMath::Sqrt(9.0 + 4. * GetParameter(Form("Centroid_%i", i)) / 1000.));
      SetParameter(Form("Beta_%i", i), GetParameter(Form("Sigma_%i", i)) / 2.0);
      SetParameter(Form("R_%i", i), 1.0);
      SetParameter(Form("Step_%i", i), 1.0);

      // Fix beta and R. These will be released if they are needed (or can be asked to be released).
      FixParameter(6 * i + 8, GetParameter(Form("Beta_%i", i)));
      FixParameter(6 * i + 9, 0.00);
   }

   SetInitialized();
   return true;
}

Bool_t TMultiPeak::Fit(TH1* fithist, Option_t* opt)
{
   TString optstr = opt;
   if((fithist == nullptr) && (GetHist() == nullptr)) {
      std::cout << "No hist passed, trying something...";
      fithist = fHistogram;
   }
   if(fithist == nullptr) {
      std::cout << "No histogram associated with Peak" << std::endl;
      return false;
   }
   if(!IsInitialized()) {
      InitParams(fithist);
   }

   TVirtualFitter::SetMaxIterations(100000);
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Combination");
   SetHist(fithist);

   TString options(opt);
   bool    print_flag = true;
   if(options.Contains("Q")) {
      print_flag = false;
   }

   // Now that it is initialized, let's fit it.

   TFitResultPtr fitres;
   // Log likelihood is the proper fitting technique UNLESS the data is a result of an addition or subtraction.
   if(GetLogLikelihoodFlag()) {
      fitres = fithist->Fit(this, Form("%sLRSQ", opt));   // The RS needs to always be there
   } else {
      fitres = fithist->Fit(this, Form("%sRSQ", opt));   // The RS needs to always be there
   }

   std::vector<Double_t> sigma_list(static_cast<int>(GetParameter(0) + 0.5));
   for(int i = 0; i < static_cast<int>(GetParameter(0) + 0.5); ++i) {
      // Get Median sigma
      sigma_list[i] = GetParameter(6 * i + 7);
   }
   std::sort(sigma_list.begin(), sigma_list.end(), std::greater<Double_t>());
   Double_t median = sigma_list.at(static_cast<int>(sigma_list.size() / 2.));

   Double_t range_low = 0.;
	Double_t range_high = 0.;
   for(int i = 0; i < static_cast<int>(GetParameter(0) + 0.5); ++i) {
      GetParLimits(6 * i + 7, range_low, range_high);
      if(range_low != range_high) {
         SetParLimits(6 * i + 7, median * .95, median * 1.05);
      }
   }

   // Refit
   if(GetLogLikelihoodFlag()) {
      fitres = fithist->Fit(this, Form("%sLRS", opt));   // The RS needs to always be there
   } else {
      fitres = fithist->Fit(this, Form("%sRS", opt));   // The RS needs to always be there
   }

   // After performing this fit I want to put something here that takes the fit result (good,bad,etc)
   // for printing out. RD

   // This removes the background parts of the fit form the integral error, while maintaining the covariance between the
   // fits and the background.
   TMatrixDSym CovMat = fitres->GetCovarianceMatrix();
   CovMat(0, 0)       = 0.0;
   CovMat(1, 1)       = 0.0;
   CovMat(2, 2)       = 0.0;
   CovMat(3, 3)       = 0.0;
   CovMat(4, 4)       = 0.0;

   // This copies the parameters background but the background function doesn't have peaks
   CopyParameters(fBackground);
   // We now make a copy of the covariance matrix that has completel 0 diagonals so that we can remove the other peaks
   // form the integral error.
   // This is done by adding back the peak of interest on the diagonal when it is integrated.
   TMatrixDSym emptyCovMat = CovMat;
   for(size_t i = 0; i < fPeakVec.size(); ++i) {
      emptyCovMat(6 * i + 5, 6 * i + 5)   = 0.0;
      emptyCovMat(6 * i + 6, 6 * i + 6)   = 0.0;
      emptyCovMat(6 * i + 7, 6 * i + 7)   = 0.0;
      emptyCovMat(6 * i + 8, 6 * i + 8)   = 0.0;
      emptyCovMat(6 * i + 9, 6 * i + 9)   = 0.0;
      emptyCovMat(6 * i + 10, 6 * i + 10) = 0.0;
   }

   if(print_flag) {
      std::cout << "Chi^2/NDF = " << fitres->Chi2() / fitres->Ndf() << std::endl;
   }
   // We will now set the parameters of each of the peaks based on the fits.
   for(int i = 0; i < static_cast<int>(fPeakVec.size()); ++i) {
      auto* tmpMp = new TMultiPeak(*this);
      tmpMp->ClearParameters();   // We need to clear all of the parameters so that we can add the ones we want back in
      Double_t    binWidth  = fithist->GetBinWidth(GetParameter(Form("Centroid_%i", i)));
      TPeak*      peak      = fPeakVec.at(i);
      TMatrixDSym tmpCovMat = emptyCovMat;
      peak->SetParameter("Height", GetParameter(Form("Height_%i", i)));
      peak->SetParameter("centroid", GetParameter(Form("Centroid_%i", i)));
      peak->SetParameter("sigma", GetParameter(Form("Sigma_%i", i)));
      peak->SetParameter("beta", GetParameter(Form("Beta_%i", i)));
      peak->SetParameter("R", GetParameter(Form("R_%i", i)));
      peak->SetParameter("step", GetParameter(Form("Step_%i", i)));
      peak->SetParameter("A", 0.0);
      peak->SetParameter("B", 0.0);
      peak->SetParameter("C", 0.0);
      peak->SetParameter("bg_offset", 0.0);
      peak->SetChi2(fitres->Chi2());
      peak->SetNdf(fitres->Ndf());

      // Set the important diagonals for the integral of the covariance matrix
      tmpCovMat(6 * i + 5, 6 * i + 5) = CovMat(6 * i + 5, 6 * i + 5);
      tmpCovMat(6 * i + 6, 6 * i + 6) = CovMat(6 * i + 6, 6 * i + 6);
      tmpCovMat(6 * i + 7, 6 * i + 7) = CovMat(6 * i + 7, 6 * i + 7);
      tmpCovMat(6 * i + 8, 6 * i + 8) = CovMat(6 * i + 8, 6 * i + 8);
      tmpCovMat(6 * i + 9, 6 * i + 9) = CovMat(6 * i + 9, 6 * i + 9);

      tmpMp->SetParameter("N_Peaks", GetParameter("N_Peaks"));
      tmpMp->SetParameter(Form("Height_%i", i), GetParameter(Form("Height_%i", i)));
      tmpMp->SetParameter(Form("Centroid_%i", i), GetParameter(Form("Centroid_%i", i)));
      tmpMp->SetParameter(Form("Sigma_%i", i), GetParameter(Form("Sigma_%i", i)));
      tmpMp->SetParameter(Form("Beta_%i", i), GetParameter(Form("Beta_%i", i)));
      tmpMp->SetParameter(Form("R_%i", i), GetParameter(Form("R_%i", i)));

      Double_t width = GetParameter(Form("Sigma_%i", i));
      Double_t xlow = 0.;
		Double_t xhigh = 0.;
      GetRange(xlow, xhigh);
      Double_t int_low  = xlow - 10. * width;   // making the integration bounds a bit smaller, but still large enough. -JKS
      Double_t int_high = xhigh + 10. * width;

      // Make a function that does not include the background
      // Intgrate the background.
      tmpMp->SetRange(int_low, int_high);   // This will help get the true area of the gaussian 200 ~ infinity in a gaus
      //   peak->SetName("tmppeak");

      // This is where we will do integrals and stuff.
      peak->SetArea((tmpMp->Integral(int_low, int_high)) / binWidth);
      peak->SetAreaErr((tmpMp->IntegralError(int_low, int_high, tmpMp->GetParameters(), tmpCovMat.GetMatrixArray())) /
                       binWidth);
      peak->SetParameter("centroid", GetParameter(Form("Centroid_%i", i)));
      peak->SetParError(peak->GetParNumber("centroid"), GetParError(GetParNumber(Form("Centroid_%i", i))));
      peak->SetParameter("sigma", GetParameter(GetParNumber(Form("Sigma_%i", i))));
      peak->SetParError(peak->GetParNumber("sigma"), GetParError(GetParNumber(Form("Sigma_%i", i))));
      if(print_flag) {
         std::cout << "Integral: " << peak->GetArea() << " +- " << peak->GetAreaErr() << std::endl;
      }
   }

   return true;
}

void TMultiPeak::Clear(Option_t* opt)
{
   TGRSIFit::Clear(opt);
   for(auto& peak : fPeakVec) {
		delete peak;
		peak = nullptr;
   }
   fPeakVec.clear();
}

void TMultiPeak::Print(Option_t* opt) const
{
   /// Prints TMultiPeak properties. To see More properties use the option "+"
   std::cout << "Name:        " << GetName() << std::endl;
   std::cout << "Number of Peaks: " << fPeakVec.size() << std::endl;
   TF1::Print();
   for(int i = 0; i < static_cast<int>(fPeakVec.size()); ++i) {
      std::cout << "Peak: " << i << std::endl;
      fPeakVec.at(i)->Print(opt);
      std::cout << std::endl;
   }
}

Double_t TMultiPeak::MultiPhotoPeakBG(Double_t* dim, Double_t* par)
{
   // Limits need to be imposed or error states may occour.
   //
   // General background.
   int    npeaks = static_cast<int>(par[0] + 0.5);
   double result = TGRSIFunctions::PolyBg(dim, &par[1], 2);   // polynomial background. uses par[1->4]
   for(int i = 0; i < npeaks; ++i) {                          // par[0] is number of peaks
		std::array<Double_t, 6> tmp_par;
      tmp_par[0] = par[6 * i + 5];    // height of photopeak
      tmp_par[1] = par[6 * i + 6];    // Peak Centroid of non skew gaus
      tmp_par[2] = par[6 * i + 7];    // standard deviation  of gaussian
      tmp_par[3] = par[6 * i + 8];    //"skewedness" of the skewed gaussian
      tmp_par[4] = par[6 * i + 9];    // relative height of skewed gaussian
      tmp_par[5] = par[6 * i + 10];   // Size of step in background
      result += TGRSIFunctions::PhotoPeak(dim, tmp_par.data()) + TGRSIFunctions::StepFunction(dim, tmp_par.data());
   }
   return result;
}

Double_t TMultiPeak::MultiStepBG(Double_t* dim, Double_t* par)
{
   // Limits need to be imposed or error states may occour.
   //
   // General background.
   int    npeaks = static_cast<int>(par[0] + 0.5);
   double result = TGRSIFunctions::PolyBg(dim, &par[1], 2);   // polynomial background. uses par[1->4]
   for(int i = 0; i < npeaks; i++) {                          // par[0] is number of peaks
		std::array<Double_t, 6> tmp_par;
      tmp_par[0] = par[6 * i + 5];    // height of photopeak
      tmp_par[1] = par[6 * i + 6];    // Peak Centroid of non skew gaus
      tmp_par[2] = par[6 * i + 7];    // standard deviation  of gaussian
      tmp_par[3] = par[6 * i + 8];    //"skewedness" of the skewed gaussian
      tmp_par[4] = par[6 * i + 9];    // relative height of skewed gaussian
      tmp_par[5] = par[6 * i + 10];   // Size of step in background
      result += TGRSIFunctions::StepFunction(dim, tmp_par.data());
   }
   return result;
}

Double_t TMultiPeak::SinglePeakBG(Double_t* dim, Double_t* par)
{
   // Limits need to be imposed or error states may occour.
   //
   // General background.

   int    npeaks = static_cast<int>(par[0] + 0.5);
   double result = TGRSIFunctions::PolyBg(dim, &par[1], 2);   // polynomial background. uses par[1->4]
   for(int i = 0; i < npeaks; i++) {                          // par[0] is number of peaks
		std::array<Double_t, 6> tmp_par;
      tmp_par[0] = par[6 * i + 5];    // height of photopeak
      tmp_par[1] = par[6 * i + 6];    // Peak Centroid of non skew gaus
      tmp_par[2] = par[6 * i + 7];    // standard deviation  of gaussian
      tmp_par[3] = par[6 * i + 8];    //"skewedness" of the skewed gaussian
      tmp_par[4] = par[6 * i + 9];    // relative height of skewed gaussian
      tmp_par[5] = par[6 * i + 10];   // Size of step in background
      result += TGRSIFunctions::StepFunction(dim, tmp_par.data());
   }
   result += TGRSIFunctions::PhotoPeak(dim, &par[npeaks * 6 + 5]);
   return result;
}

TPeak* TMultiPeak::GetPeak(UInt_t idx)
{
   if(idx < fPeakVec.size()) {
      return fPeakVec.at(idx);
   }
   std::cout << "No matching peak at index " << idx << std::endl;

   return nullptr;
}

TPeak* TMultiPeak::GetPeakClosestTo(Double_t energy)
{
   size_t   closest_idx    = 0;
   Double_t closest_so_far = 1000000.;
   for(size_t i = 0; i < fPeakVec.size(); ++i) {
      if(std::abs(energy - fPeakVec.at(i)->GetCentroid()) < closest_so_far) {
         closest_so_far = std::abs(energy - fPeakVec.at(i)->GetCentroid());
         closest_idx    = i;
      }
   }

   return GetPeak(closest_idx);
}

void TMultiPeak::DrawPeaks()
{
   // Draws the individual TPeaks that make up the TMultiPeak. ROOT makes this a complicated process. The result on the
   // histogram might have memory issues.
   Double_t xlow = 0.;
	Double_t xhigh = 0.;
   GetRange(xlow, xhigh);
   Double_t npeaks = fPeakVec.size();
   for(size_t i = 0; i < fPeakVec.size(); ++i) {
      TPeak* peak = fPeakVec.at(i);
      // Should be good enough to draw between -2 and +2 fwhm
      Double_t centroid = peak->GetCentroid();
      Double_t range    = 2. * peak->GetFWHM();

      auto* sum = new TF1(Form("tmp%s", peak->GetName()), this, &TMultiPeak::SinglePeakBG, centroid - range, centroid + range,
                          fPeakVec.size() * 6 + 11, "TMultiPeak", "SinglePeakBG");

      for(int j = 0; j < GetNpar(); ++j) {
         sum->SetParameter(j, GetParameter(j));
      }
      for(int j = 0; j < 5; ++j) {
         sum->SetParameter(npeaks * 6 + 5 + j, peak->GetParameter(j));
      }

      sum->SetNpx(1000);
      sum->SetLineStyle(2);
      sum->SetLineColor(kMagenta);

      sum->SetRange(centroid - range, centroid + range);
      sum->DrawCopy("SAME");
      delete sum;
   }
}
