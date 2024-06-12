#include "TPeakFitter.h"
#include "Math/Minimizer.h"
#include "TH1.h"
#include "TClass.h"

#include "Globals.h"

/// \cond CLASSIMP
ClassImp(TPeakFitter)
   /// \endcond

   TPeakFitter::TPeakFitter() : TObject()
{
   fBGToFit = new TF1("fbg", this, &TPeakFitter::DefaultBackgroundFunction, fRangeLow, fRangeHigh, 4, "TPeakFitter", "DefaultBackgroundFunction");
   fBGToFit->FixParameter(3, 0);
   fTotalFitFunction = nullptr;
   fBGToFit->SetLineColor(kRed + fIndex);
}

TPeakFitter::TPeakFitter(const Double_t& range_low, const Double_t& range_high) : TPeakFitter()
{
   fRangeLow  = range_low;
   fRangeHigh = range_high;
}

void TPeakFitter::Print(Option_t* opt) const
{
   /// Print information from the fit, opt is passed along to each individual peaks Print function.
   if(fTotalFitFunction != nullptr) {
      double xMin, xMax;
      fTotalFitFunction->GetRange(xMin, xMax);
      std::cout << "Fitting range: " << xMin << " to " << xMax << std::endl;
   } else {
      std::cout << "Range: " << fRangeLow << " to " << fRangeHigh << std::endl;
   }
   Int_t counter = 0;
   if(!fPeaksToFit.empty()) {
      std::cout << "Peaks: " << std::endl;
      for(auto i : fPeaksToFit) {
         std::cout << "Peak #" << counter++ << " - ";
         i->Print(opt);
      }
      std::cout << "Chi2/Ndf = " << fPeaksToFit.front()->GetChi2() << "/" << fPeaksToFit.front()->GetNDF() << " = " << fPeaksToFit.front()->GetReducedChi2() << std::endl;
   } else {
      std::cout << "Could not find peak to print" << std::endl;
   }
   if(fBGToFit) {
      std::cout << "BG: " << std::endl;
      fBGToFit->Print(opt);
   } else {
      std::cout << "Could not find a BG to print" << std::endl;
   }
}

void TPeakFitter::PrintParameters() const
{
   /// Print the range of the fit and the parameters of each peak on a single line.
   std::cout << "Range: " << fRangeLow << " to " << fRangeHigh << " - ";
   int i = 0;
   for(auto peak : fPeaksToFit) {
      std::cout << peak->IsA()->GetName() << " #" << i++ << " ";
      peak->PrintParameters();
   }
   std::cout << std::endl;
}

Int_t TPeakFitter::GetNParameters() const
{
   Int_t n_par = 0;
   for(auto i : fPeaksToFit)
      n_par += i->GetNParameters();
   if(fBGToFit)
      n_par += fBGToFit->GetNpar();

   return n_par;
}

void TPeakFitter::SetRange(const Double_t& low, const Double_t& high)
{
   fRangeLow  = low;
   fRangeHigh = high;
}

TFitResultPtr TPeakFitter::Fit(TH1* fit_hist, Option_t* opt)
{
   /// Fit the histogram. Recognized options are "q" for a quiet fit, "retryfit" to retry a fit without parameter limits
   /// if one of the parameters got close to its limit. All options (apart from "retryfit") are passed on to the actual
   /// call to TH1::Fit.
   if(fPeaksToFit.empty()) {
      std::cout << "No peaks provided!" << std::endl;
      return TFitResultPtr();
   }

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

   // store original range
   int firstBin = fit_hist->GetXaxis()->GetFirst();
   int lastBin  = fit_hist->GetXaxis()->GetLast();
   // set range to fit range of peak fitter
   fit_hist->GetXaxis()->SetRangeUser(fRangeLow, fRangeHigh);

   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Combination");
   TVirtualFitter::SetMaxIterations(100000);
   TVirtualFitter::SetPrecision(1e-4);
   if(fTotalFitFunction == nullptr) {
      fTotalFitFunction = new TF1("total_fit", this, &TPeakFitter::FitFunction, fRangeLow, fRangeHigh, GetNParameters(), "TPeakFitter", "FitFunction");
   }
   fTotalFitFunction->SetLineColor(kMagenta + fIndex);
   fTotalFitFunction->SetRange(fRangeLow, fRangeHigh);
   // We need to initialize all of the parameters based on the peak parameters
   if(fit_hist != nullptr && (!fInitFlag || fit_hist != fLastHistFit)) {
      if(verbose) std::cout << "Initializing Fit...." << std::endl;
      InitializeBackgroundParameters(fit_hist);
      InitializeParameters(fit_hist);
      fInitFlag    = true;
      fLastHistFit = fit_hist;
   }
   UpdateFitterParameters();
   if(verbose) PrintParameters();
   // Do a first fit to get closer on the parameters
   fit_hist->Fit(fTotalFitFunction, "RNQ0");
   // Now try the fit with the options provided
   fit_hist->Fit(fTotalFitFunction, Form("RNQO%s", options.Data()));
   // Now do a final fit with integrated bins
   TFitResultPtr fit_res = fit_hist->Fit(fTotalFitFunction, Form("SRI%s", options.Data()));

   // check the parameter errors
   if(!TGRSIFunctions::CheckParameterErrors(fit_res, options.Data())) {
      if(retryFit) {
         // fit again with all parameters released
         if(!quiet) std::cout << GREEN << "Re-fitting with released parameters (without any limits)" << RESET_COLOR << std::endl;
         for(int i = 0; i < fTotalFitFunction->GetNpar(); ++i) {
            if(i == 1) continue;   // skipping centroid, which should always be parameter 1
            fTotalFitFunction->ReleaseParameter(i);
         }
         fit_res = fit_hist->Fit(fTotalFitFunction, Form("SRI%s", options.Data()));
         TGRSIFunctions::CheckParameterErrors(fit_res, options.Data());
      } else {
         // re-try using minos instead of minuit (only thing from opt that might play a role it 'q' or 'v')
         if(!quiet) std::cout << YELLOW << "Re-fitting with \"E\" option to get better error estimation using Minos technique." << RESET_COLOR << std::endl;
         fit_res = fit_hist->Fit(fTotalFitFunction, Form("SRIE%s", options.Data()));
      }
   }

   if(fit_res.Get() != nullptr) {
      fTotalFitFunction->SetFitResult(*fit_res);   // Not sure if this is needed

      // Once we do the fit, we want to update all of the Peak parameters.
      UpdatePeakParameters(fit_res, fit_hist);
   } else if(!quiet) {
      std::cout << RED << "Failed to get fit result, be aware that the results and especially the error estimates might be wrong!" << RESET_COLOR << std::endl;
   }
   fit_hist->Draw("hist");
   fTotalFitFunction->Draw("same");
   fPeaksToFit.front()->DrawBackground("same");

   // always print the result of the fit even if not verbose
   if(!quiet) {
      std::cout << "****************" << std::endl
                << "Summary of Fit: " << std::endl;
      Print();
   }
   // restore old range
   fit_hist->GetXaxis()->SetRange(firstBin, lastBin);
   // reset the default back to minuit in case we switched to minos in between
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Combination");

   return fit_res;
}

void TPeakFitter::UpdatePeakParameters(const TFitResultPtr& fit_res, TH1* fit_hist)
{
   // We enter this function once we have performed a fit and need to tell the peaks
   // what their new parameters should be.
   Int_t param_counter = 0;

   TF1* global_bg = new TF1("global_bg", this, &TPeakFitter::BackgroundFunction, fRangeLow, fRangeHigh, fTotalFitFunction->GetNpar(), "TPeakFitter", "BackgroundFunction");
   global_bg->SetParameters(fTotalFitFunction->GetParameters());

   // Start by looping through all of the peaks in the fitter.
   for(auto p_it : fPeaksToFit) {
      TF1* peak_func = p_it->GetFitFunction();
      // We need to make a clone of the main fit function, and set all of the non-peak parameters to 0.
      // This will allow us to do the proper integrals. We have to do the same with the covariance matrix
      // We get the covariance matrix that we are using to update the individual peaks
      TF1*        total_function_copy  = new TF1(*fTotalFitFunction);
      TMatrixDSym covariance_matrix    = fit_res->GetCovarianceMatrix();
      bool        goodCovarianceMatrix = true;
      if(covariance_matrix.GetNrows() < peak_func->GetNpar() || covariance_matrix.GetNcols() < peak_func->GetNpar()) {
         goodCovarianceMatrix = false;
      }
      // Now we need to remove all of the parts of the covariance peak that has nothing to do with this particular peak
      // This would be the diagonals of the background, and all of the other peaks.
      Int_t              param_to_zero_counter = 0;
      std::vector<Int_t> param_to_zero_list;
      for(auto other_p_it : fPeaksToFit) {
         if(other_p_it != p_it) {
            for(int i = 0; i < other_p_it->GetFitFunction()->GetNpar(); ++i) {
               param_to_zero_list.push_back(param_to_zero_counter);
               ++param_to_zero_counter;
            }
         } else {
            for(int i = 0; i < peak_func->GetNpar(); ++i) {
               if(p_it->IsBackgroundParameter(i)) {
                  param_to_zero_list.push_back(param_to_zero_counter);
               }
               ++param_to_zero_counter;
            }
            Double_t low_range, high_range;
            fTotalFitFunction->GetRange(low_range, high_range);
            peak_func->SetRange(low_range, high_range);
         }
      }
      // We have now taken care to zero all of the non-peak parameters in the peak list, so we want to remove the total background contribution
      for(int i = param_to_zero_counter; i < fTotalFitFunction->GetNpar(); ++i) {
         param_to_zero_list.push_back(i);
      }

      // zero other non-peak portions of matrix
      for(auto i : param_to_zero_list) {
         for(auto j : param_to_zero_list) {
            if(goodCovarianceMatrix) covariance_matrix(i, j) = 0.0;
         }
         total_function_copy->SetParameter(i, 0.0);
      }
      if(peak_func) {
         for(int i = 0; i < peak_func->GetNpar(); ++i) {
            peak_func->SetParameter(i, fTotalFitFunction->GetParameter(param_counter));
            peak_func->SetParError(i, fTotalFitFunction->GetParError(param_counter));
            Double_t low, high;
            fTotalFitFunction->GetParLimits(param_counter, low, high);
            peak_func->SetParLimits(i, low, high);
            ++param_counter;
            // Lets do some integrals meow.
         }
         p_it->SetArea(total_function_copy->Integral(p_it->Centroid() - p_it->Width() * 5., p_it->Centroid() + p_it->Width() * 5., 1e-8) / fit_hist->GetBinWidth(1));
         if(goodCovarianceMatrix) p_it->SetAreaErr(total_function_copy->IntegralError(p_it->Centroid() - p_it->Width() * 5., p_it->Centroid() + p_it->Width() * 5., total_function_copy->GetParameters(), covariance_matrix.GetMatrixArray(), 1E-5) / fit_hist->GetBinWidth(1));
         else std::cout << "Not setting area error because we don't have a good covariance matrix!" << std::endl;
         // std::cout<<"Integrating from: "<<p_it->Centroid()-p_it->Width()*5.<<" to "<<p_it->Centroid()+p_it->Width()*5.<<std::endl;
      }
      total_function_copy->Delete();
   }
   if(fBGToFit) {
      for(int i = 0; i < fBGToFit->GetNpar(); ++i) {
         fBGToFit->SetParameter(i, fTotalFitFunction->GetParameter(param_counter));
         fBGToFit->SetParError(i, fTotalFitFunction->GetParError(param_counter));
         ++param_counter;
      }
   }
   // We now have a copy of the global background. Copy and send to every peak
   for(auto p_it : fPeaksToFit) {
      p_it->SetGlobalBackground(new TF1(*global_bg));
      p_it->SetChi2(fTotalFitFunction->GetChisquare());
      p_it->SetNDF(fTotalFitFunction->GetNDF());
   }
   global_bg->Delete();
}

void TPeakFitter::InitializeParameters(TH1* fit_hist)
{
   for(auto p_it : fPeaksToFit) {
      p_it->InitializeParameters(fit_hist, fRangeLow, fRangeHigh);
   }
}

void TPeakFitter::UpdateFitterParameters()
{
   /// This functions gets the parameters and their limits from the peak functions and
   /// sets them for the total fit function
   Int_t param_counter = 0;
   Int_t peak_counter  = 0;
   for(auto p_it : fPeaksToFit) {
      TF1* peak_func = p_it->GetFitFunction();
      if(peak_func) {
         for(int i = 0; i < peak_func->GetNpar(); ++i) {
            fTotalFitFunction->SetParName(param_counter, Form("%s_%i", peak_func->GetParName(i), peak_counter));
            fTotalFitFunction->SetParameter(param_counter, peak_func->GetParameter(i));
            fTotalFitFunction->SetParError(param_counter, peak_func->GetParError(i));
            Double_t limit_low, limit_high;
            peak_func->GetParLimits(i, limit_low, limit_high);
            fTotalFitFunction->SetParLimits(param_counter, limit_low, limit_high);
            ++param_counter;
         }
         ++peak_counter;
      }
   }
   if(fBGToFit) {
      for(int i = 0; i < fBGToFit->GetNpar(); ++i) {
         fTotalFitFunction->SetParName(param_counter, fBGToFit->GetParName(i));
         fTotalFitFunction->SetParameter(param_counter, fBGToFit->GetParameter(i));
         fTotalFitFunction->SetParError(param_counter, fBGToFit->GetParError(i));
         Double_t limit_low, limit_high;
         fBGToFit->GetParLimits(i, limit_low, limit_high);
         fTotalFitFunction->SetParLimits(param_counter, limit_low, limit_high);
         ++param_counter;
      }
   }
}

Double_t TPeakFitter::FitFunction(Double_t* dim, Double_t* par)
{
   // I want to use the EvalPar command here in order to ge the individual peaks
   Double_t sum           = 0;
   Int_t    params_so_far = 0;
   for(auto p_it : fPeaksToFit) {
      TF1* peak_func = p_it->GetFitFunction();
      sum += peak_func->EvalPar(dim, &par[params_so_far]);
      params_so_far += peak_func->GetNpar();
   }
   sum += fBGToFit->EvalPar(dim, &par[params_so_far]);

   return sum;
}

Double_t TPeakFitter::BackgroundFunction(Double_t* dim, Double_t* par)
{
   Double_t sum           = 0;
   Int_t    params_so_far = 0;
   for(auto p_it : fPeaksToFit) {
      TF1* peak_func = p_it->GetBackgroundFunction();
      sum += peak_func->EvalPar(dim, &par[params_so_far]);
      params_so_far += peak_func->GetNpar();
   }
   sum += fBGToFit->EvalPar(dim, &par[params_so_far]);

   return sum;
}

void TPeakFitter::InitializeBackgroundParameters(TH1* fit_hist)
{
   Double_t lowLimit;
   Double_t highLimit;
   double   value;

   fBGToFit->SetParName(0, "A");
   fBGToFit->SetParName(1, "B");
   fBGToFit->SetParName(2, "C");
   fBGToFit->SetParName(3, "bg_offset");

   fBGToFit->GetParLimits(0, lowLimit, highLimit);
   value = fBGToFit->GetParameter(0);
   if(value == 0 && lowLimit == 0 && highLimit == 0) {
      fBGToFit->SetParLimits(0, 0.0, fit_hist->GetBinContent(fit_hist->FindBin(fRangeHigh)) * 100.);
      fBGToFit->SetParameter("A", fit_hist->GetBinContent(fit_hist->FindBin(fRangeHigh)));
   }
   fBGToFit->GetParLimits(1, lowLimit, highLimit);
   value = fBGToFit->GetParameter(1);
   if(value == 0 && lowLimit == 0 && highLimit == 0) {
      fBGToFit->SetParameter("B", fit_hist->GetBinWidth(1) * ((fit_hist->GetBinContent(fit_hist->FindBin(fRangeLow)) - fit_hist->GetBinContent(fit_hist->FindBin(fRangeHigh))) / (fRangeLow - fRangeHigh)));
   }
   fBGToFit->GetParLimits(2, lowLimit, highLimit);
   value = fBGToFit->GetParameter(2);
   if(value == 0 && lowLimit == 0 && highLimit == 0) {
      fBGToFit->SetParameter("C", 0.0000);
      fBGToFit->FixParameter(2, 0.00);
   }
   fBGToFit->GetParLimits(3, lowLimit, highLimit);
   value = fBGToFit->GetParameter(3);
   if(value == 0 && lowLimit == 0 && highLimit == 0) {
      fBGToFit->SetParLimits(3, fRangeLow, fRangeHigh);
      if(!fPeaksToFit.empty()) {
         fBGToFit->SetParameter("bg_offset", fPeaksToFit.front()->Centroid());
      } else {
         fBGToFit->SetParameter("bg_offset", (fRangeHigh + fRangeLow) / 2.);
      }
   }
}

Double_t TPeakFitter::DefaultBackgroundFunction(Double_t* dim, Double_t* par)
{
   Double_t x         = dim[0];
   Double_t A         = par[0];
   Double_t B         = par[1];
   Double_t C         = par[2];
   Double_t bg_offset = par[3];

   return A + B * (x - bg_offset) + C * TMath::Power(x - bg_offset, 2.);
}

void TPeakFitter::DrawPeaks(Option_t*) const
{
   // First we are going to draw the background
   TF1* bg_to_draw = new TF1;
   fTotalFitFunction->Copy(*bg_to_draw);
   bg_to_draw->SetLineColor(kRed + fIndex);

   for(auto p_it : fPeaksToFit) {
      TF1* peak_func           = p_it->GetFitFunction();
      TF1* total_function_copy = new TF1;
      fTotalFitFunction->Copy(*total_function_copy);
      // Now we need to remove all of the parts of the covariance peak that has nothing to do with this particular peak
      // This would be the diagonals of the background, and all of the other peaks.
      Int_t param_to_zero_counter = 0;
      for(auto other_p_it : fPeaksToFit) {
         if(other_p_it != p_it) {
            for(int i = 0; i < other_p_it->GetFitFunction()->GetNpar(); ++i) {
               total_function_copy->SetParameter(param_to_zero_counter, 0.0);
               ++param_to_zero_counter;
            }
         } else {
            for(int i = 0; i < peak_func->GetNpar(); ++i) {
               if(!p_it->IsBackgroundParameter(i)) {
                  bg_to_draw->SetParameter(param_to_zero_counter, 0.0);
               }
               ++param_to_zero_counter;
            }
         }
      }
      total_function_copy->Draw("same");
   }
   bg_to_draw->Draw("same");
}
