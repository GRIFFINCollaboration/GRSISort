#include "TSourceCalibration.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#if __cplusplus >= 201703L
#include <filesystem>
#endif

#include "TSystem.h"
#include "TGTableLayout.h"
#include "TCanvas.h"
#include "TLinearFitter.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TPolyMarker.h"
#include "TObject.h"

#include "TChannel.h"
#include "GRootCommands.h"
#include "combinations.h"
#include "TRWPeak.h"
#include "Globals.h"

std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> Match(std::vector<std::tuple<double, double, double, double>> peaks, std::vector<std::tuple<double, double, double, double>> sources, int verboseLevel)
{
   if(verboseLevel > 1) { std::cout << "Matching " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl; }
   std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> result;
   std::sort(peaks.begin(), peaks.end());
   std::sort(sources.begin(), sources.end());

   auto maxSize = peaks.size();
   if(sources.size() > maxSize) { maxSize = sources.size(); }

   // Peaks are the fitted points.
   // source are the known values

   std::vector<bool> filled(maxSize);
   std::fill(filled.begin(), filled.end(), true);

   TLinearFitter fitter(1, "1 ++ x");

   // intermediate vectors and map
   std::vector<double> peakValues(peaks.size());
   for(size_t i = 0; i < peaks.size(); ++i) { peakValues[i] = std::get<0>(peaks[i]); }
   std::vector<double> sourceValues(sources.size());
   for(size_t i = 0; i < sources.size(); ++i) { sourceValues[i] = std::get<0>(sources[i]); }
   std::map<double, double> tmpMap;

   for(size_t num_data_points = peakValues.size(); num_data_points > 0; num_data_points--) {
      if(verboseLevel > 2) { std::cout << num_data_points << " data points:" << std::endl; }
      double best_chi2 = DBL_MAX;
      for(auto peak_values : combinations(peakValues, num_data_points)) {
         // Add a (0,0) point to the calibration.
         peak_values.push_back(0);
         for(auto source_values : combinations(sourceValues, num_data_points)) {
            source_values.push_back(0);

            if(peakValues.size() > 3) {
               double max_err = 0.02;
               double pratio  = peak_values.front() / peak_values.at(peak_values.size() - 2);
               double sratio  = source_values.front() / source_values.at(source_values.size() - 2);
               if(verboseLevel > 3) { std::cout << "ratio: " << pratio << " - " << sratio << " = " << std::abs(pratio - sratio) << std::endl; }
               if(std::abs(pratio - sratio) > max_err) {
                  if(verboseLevel > 3) { std::cout << "skipping" << std::endl; }
                  continue;
               }
            }

            fitter.ClearPoints();
            fitter.AssignData(source_values.size(), 1, peak_values.data(), source_values.data());
            fitter.Eval();

            if(fitter.GetChisquare() < best_chi2) {
               tmpMap.clear();
               for(size_t i = 0; i < num_data_points; i++) {
                  tmpMap[peak_values[i]] = source_values[i];
               }
               best_chi2 = fitter.GetChisquare();
            }
         }
      }

      // Remove one peak value from the best fit, make sure that we reproduce (0,0) intercept.
      if(tmpMap.size() > 2) {
         std::vector<double> peak_values;
         std::vector<double> source_values;
         for(auto& item : tmpMap) {
            peak_values.push_back(item.first);
            source_values.push_back(item.second);
         }

         for(size_t skipped_point = 0; skipped_point < source_values.size(); skipped_point++) {
            std::swap(peak_values[skipped_point], peak_values.back());
            std::swap(source_values[skipped_point], source_values.back());

            fitter.ClearPoints();
            fitter.AssignData(source_values.size() - 1, 1, peak_values.data(), source_values.data());
            fitter.Eval();

            if(std::abs(fitter.GetParameter(0)) > 10) {
               if(verboseLevel > 3) {
                  std::cout << fitter.GetParameter(0) << " too big, clearing map with " << tmpMap.size() << " points: ";
                  for(auto iter : tmpMap) { std::cout << iter.first << " - " << iter.second << "; "; }
                  std::cout << std::endl;
               }
               tmpMap.clear();
               break;
            }

            std::swap(peak_values[skipped_point], peak_values.back());
            std::swap(source_values[skipped_point], source_values.back());
         }
      }

      // copy all values from the vectors to the result map
      if(!tmpMap.empty()) {
         for(auto iter : tmpMap) {
            result[*(std::find_if(peaks.begin(), peaks.end(), [&iter](auto& item) { return iter.first == std::get<0>(item); }))] =
               *(std::find_if(sources.begin(), sources.end(), [&iter](auto& item) { return iter.second == std::get<0>(item); }));
            // apparently c++14 is needed to use auto in a lambda so for now we spell it out
            //result[*(std::find_if(peaks.begin(),   peaks.end(),   [&iter](std::tuple<double, double, double, double>& item) { return iter.first  == std::get<0>(item); }))] =
            //       *(std::find_if(sources.begin(), sources.end(), [&iter](std::tuple<double, double, double, double>& item) { return iter.second == std::get<0>(item); }));
         }
         if(verboseLevel > 2) {
            std::cout << "Matched " << num_data_points << " data points from " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl;
            std::cout << "Returning map with " << result.size() << " points: ";
            for(auto iter : result) { std::cout << std::get<0>(iter.first) << " - " << std::get<0>(iter.second) << "; "; }
            std::cout << std::endl;
         }
         break;
      }
   }

   return result;
}

std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> SmartMatch(std::vector<std::tuple<double, double, double, double>> peaks, std::vector<std::tuple<double, double, double, double>> sources, int verboseLevel)
{
   if(verboseLevel > 1) { std::cout << "Matching " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl; }
   std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> result;
   std::sort(peaks.begin(), peaks.end());
   std::sort(sources.begin(), sources.end(), [](const std::tuple<double, double, double, double>& a, const std::tuple<double, double, double, double>& b) { return std::get<2>(a) > std::get<2>(b); });

   auto maxSize = peaks.size();
   if(sources.size() > maxSize) { maxSize = sources.size(); }

   // Peaks are the fitted points.
   // source are the known values

   std::vector<bool> filled(maxSize);
   std::fill(filled.begin(), filled.end(), true);

   TLinearFitter fitter(1, "1 ++ x");

   // intermediate vectors and map
   std::vector<double> peakValues(peaks.size());
   for(size_t i = 0; i < peaks.size(); ++i) { peakValues[i] = std::get<0>(peaks[i]); }
   std::vector<double>      sourceValues(sources.size());
   std::map<double, double> tmpMap;

   for(size_t num_data_points = peakValues.size(); num_data_points > 0; num_data_points--) {
      if(verboseLevel > 2) { std::cout << num_data_points << " data points:" << std::endl; }
      double best_chi2 = DBL_MAX;
      for(auto peak_values : combinations(peakValues, num_data_points)) {
         // Add a (0,0) point to the calibration.
         peak_values.push_back(0);
         // instead of going through all possible combinations of the peaks with the source energies
         // we pick the num_data_points most intense lines and try them
         // we don't do the same with the peaks as there might be an intense background peak in the data (511 etc.)
         sourceValues.resize(num_data_points);
         for(size_t i = 0; i < sourceValues.size(); ++i) { sourceValues[i] = std::get<0>(sources[i]); }
         std::sort(sourceValues.begin(), sourceValues.end());
         sourceValues.push_back(0);

         if(peakValues.size() > 3) {
            double max_err = 0.02;
            double pratio  = peak_values.front() / peak_values.at(peak_values.size() - 2);
            double sratio  = sourceValues.front() / sourceValues.at(sourceValues.size() - 2);
            if(verboseLevel > 3) { std::cout << "ratio: " << pratio << " - " << sratio << " = " << std::abs(pratio - sratio) << std::endl; }
            if(std::abs(pratio - sratio) > max_err) {
               if(verboseLevel > 3) { std::cout << "skipping" << std::endl; }
               continue;
            }
         }

         fitter.ClearPoints();
         fitter.AssignData(sourceValues.size(), 1, peak_values.data(), sourceValues.data());
         fitter.Eval();

         if(fitter.GetChisquare() < best_chi2) {
            tmpMap.clear();
            for(size_t i = 0; i < num_data_points; i++) {
               tmpMap[peak_values[i]] = sourceValues[i];
            }
            best_chi2 = fitter.GetChisquare();
         }
      }

      // Remove one peak value from the best fit, make sure that we reproduce (0,0) intercept.
      if(tmpMap.size() > 2) {
         std::vector<double> peak_values;
         std::vector<double> source_values;
         for(auto& item : tmpMap) {
            peak_values.push_back(item.first);
            source_values.push_back(item.second);
         }

         for(size_t skipped_point = 0; skipped_point < source_values.size(); skipped_point++) {
            std::swap(peak_values[skipped_point], peak_values.back());
            std::swap(source_values[skipped_point], source_values.back());

            fitter.ClearPoints();
            fitter.AssignData(source_values.size() - 1, 1, peak_values.data(), source_values.data());
            fitter.Eval();

            if(std::abs(fitter.GetParameter(0)) > 10) {
               if(verboseLevel > 3) {
                  std::cout << fitter.GetParameter(0) << " too big, clearing map with " << tmpMap.size() << " points: ";
                  for(auto iter : tmpMap) { std::cout << iter.first << " - " << iter.second << "; "; }
                  std::cout << std::endl;
               }
               tmpMap.clear();
               break;
            }

            std::swap(peak_values[skipped_point], peak_values.back());
            std::swap(source_values[skipped_point], source_values.back());
         }
      }

      // copy all values from the vectors to the result map
      if(!tmpMap.empty()) {
         // apparently c++14 is needed to use auto in a lambda so for now we spell it out
         //	for(auto it : tmpMap) result[*(std::find_if(peaks.begin(),   peaks.end(),   [&it] (auto& item) { return it.first  == std::get<0>(item); }))] =
         //		                          *(std::find_if(sources.begin(), sources.end(), [&it] (auto& item) { return it.second == std::get<0>(item); }));
         for(auto iter : tmpMap) {
            result[*(std::find_if(peaks.begin(), peaks.end(), [&iter](std::tuple<double, double, double, double>& item) { return iter.first == std::get<0>(item); }))] =
               *(std::find_if(sources.begin(), sources.end(), [&iter](std::tuple<double, double, double, double>& item) { return iter.second == std::get<0>(item); }));
         }
         if(verboseLevel > 2) {
            std::cout << "Smart matched " << num_data_points << " data points from " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl;
            std::cout << "Returning map with " << result.size() << " points: ";
            for(auto iter : result) { std::cout << std::get<0>(iter.first) << " - " << std::get<0>(iter.second) << "; "; }
            std::cout << std::endl;
         }
         break;
      }
   }

   return result;
}

double Polynomial(double* x, double* par)   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay, readability-non-const-parameter)
{
   double result = par[1];
   for(int i = 1; i <= par[0]; ++i) {
      result += par[i + 1] * TMath::Power(x[0], i);
   }
   return result;
}

double Efficiency(double* x, double* par)   // NOLINT(readability-non-const-parameter)
{
   double sum = 0.0;
   for(int i = 0; i < 9; ++i) {
      sum += par[i] * TMath::Power(TMath::Log(x[0]), i);
   }
   return TMath::Exp(sum);
}

bool FilledBin(TH2* matrix, const int& bin)
{
   return (matrix->Integral(bin, bin, 1, matrix->GetNbinsY()) > 1000);
}

//////////////////////////////////////// TChannelTab ////////////////////////////////////////
TChannelTab::TChannelTab(TGTab* parent, TH1* projection, TGCompositeFrame* frame, const double& sigma, const double& threshold, const int& degree, const std::vector<std::tuple<double, double, double, double>>& sourceEnergy)
   : fParent(parent), fFrame(frame), fProjection(projection), fDegree(degree)
{
   BuildInterface();
   FindPeaks(sigma, threshold, sourceEnergy);
   Calibrate(degree, true);
}

TChannelTab::TChannelTab(const TChannelTab& rhs)
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fParent            = rhs.fParent;
   fFrame             = rhs.fFrame;
   fTopFrame          = rhs.fTopFrame;
   fLeftFrame         = rhs.fLeftFrame;
   fRightFrame        = rhs.fRightFrame;
   fProjectionCanvas  = rhs.fProjectionCanvas;
   fCalibrationCanvas = rhs.fCalibrationCanvas;
   fStatusBar         = rhs.fStatusBar;

   fProjection = rhs.fProjection;
   fData       = rhs.fData;
   fEfficiency = rhs.fEfficiency;
   fSigma      = rhs.fSigma;
   fThreshold  = rhs.fThreshold;
   fDegree     = rhs.fDegree;
   fPeaks.clear();
}

TChannelTab::~TChannelTab()
{
   for(auto* peak : fPeaks) {
      delete peak;
   }
   fPeaks.clear();
   delete fTopFrame;
   delete fProjectionCanvas;
   delete fCalibrationCanvas;
}

void TChannelTab::BuildInterface()
{
   // top frame with two canvases amd status bar
   fTopFrame          = new TGHorizontalFrame(fFrame, 1200, 450);
   fProjectionCanvas  = new TRootEmbeddedCanvas("ChannelProjectionCanvas", fTopFrame, 600, 400);
   fCalibrationCanvas = new TRootEmbeddedCanvas("ChannelCalibrationCanvas", fTopFrame, 600, 400);

   fTopFrame->AddFrame(fProjectionCanvas, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandY | kLHintsExpandX, 2, 2, 2, 2));
   fTopFrame->AddFrame(fCalibrationCanvas, new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsExpandY | kLHintsExpandX, 2, 2, 2, 2));

   fFrame->AddFrame(fTopFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

   fStatusBar               = new TGStatusBar(fFrame, 1200, 50);
   std::array<int, 5> parts = {35, 15, 20, 15, 15};
   fStatusBar->SetParts(parts.data(), parts.size());

   fFrame->AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));
}

void TChannelTab::MakeConnections()
{
   fProjectionCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TChannelTab", this, "ProjectionStatus(Int_t,Int_t,Int_t,TObject*)");
   fCalibrationCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TChannelTab", this, "CalibrationStatus(Int_t,Int_t,Int_t,TObject*)");
}

void TChannelTab::Disconnect()
{
   fProjectionCanvas->GetCanvas()->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this, "ProjectionStatus(Int_t,Int_t,Int_t,TObject*)");
   fCalibrationCanvas->GetCanvas()->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this, "CalibrationStatus(Int_t,Int_t,Int_t,TObject*)");
}

void TChannelTab::ProjectionStatus(Int_t event, Int_t px, Int_t py, TObject* selected)
{
   // looks like 51 is hovering over the object, 52 is moving the cursor over the object, and 53 is moving the cursort away from the object
   // kButton1 = left mouse button, kButton2 = right mouse button
   // enum EEventType {
   //   kNoEvent       =  0,
   //   kButton1Down   =  1, kButton2Down   =  2, kButton3Down   =  3, kKeyDown  =  4,
   //   kWheelUp       =  5, kWheelDown     =  6, kButton1Shift  =  7, kButton1ShiftMotion  =  8,
   //   kButton1Up     = 11, kButton2Up     = 12, kButton3Up     = 13, kKeyUp    = 14,
   //   kButton1Motion = 21, kButton2Motion = 22, kButton3Motion = 23, kKeyPress = 24,
   //   kArrowKeyPress = 25, kArrowKeyRelease = 26,
   //   kButton1Locate = 41, kButton2Locate = 42, kButton3Locate = 43, kESC      = 27,
   //   kMouseMotion   = 51, kMouseEnter    = 52, kMouseLeave    = 53,
   //   kButton1Double = 61, kButton2Double = 62, kButton3Double = 63
   //};
   fStatusBar->SetText(selected->GetName(), 0);
   fStatusBar->SetText(selected->GetObjectInfo(px, py), 1);
   if(selected == fProjection && event == kButton1Down) {
      if(fVerboseLevel > 1) { std::cout << "Adding new marker at " << px << ", " << py << std::endl; }
      auto* polym = static_cast<TPolyMarker*>(fProjection->GetListOfFunctions()->FindObject("TPolyMarker"));
      if(polym == nullptr) {
         std::cerr << "No peaks defined yet?" << std::endl;
         return;
      }
      polym->SetNextPoint(px, py);
      double range = 4 * fSigma * fProjection->GetXaxis()->GetBinWidth(1);
      GPeak* peak  = PhotoPeakFit(fProjection, px - range, px + range, "qretryfit");
      //fPeakFitter.SetRange(px - range, px + range);
      //auto peak = new TRWPeak(px);
      //fPeakFitter.AddPeak(peak);
      //fPeakFitter.Fit(fProjection, "qretryfit");
      if(peak->Area() > 0) {
         fPeaks.push_back(peak);
         if(fVerboseLevel > 1) { std::cout << "Fitted peak " << px - range << " - " << px + range << " -> centroid " << peak->Centroid() << std::endl; }
      } else {
         std::cout << "Ignoring peak at " << peak->Centroid() << " with negative area " << peak->Area() << std::endl;
      }
      fProjection->GetListOfFunctions()->Remove(peak);
      fProjection->Sumw2(false);   // turn errors off, makes the histogram look like a histogram when using normal plotting (hist and samefunc doesn't work for some reason)
   }
}

void TChannelTab::CalibrationStatus(Int_t, Int_t px, Int_t py, TObject* selected)
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": px " << px << ", py " << py << ", object " << selected->GetName() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fStatusBar->SetText(selected->GetName(), 3);
   fStatusBar->SetText(selected->GetObjectInfo(px, py), 4);
}

void TChannelTab::Calibrate(const int& degree, const bool& force)
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fData == nullptr) { return; }

   if(degree != fDegree || force) {
      if(fVerboseLevel > 2) {
         std::cout << __PRETTY_FUNCTION__ << ": degree (" << degree << "/" << fDegree << ") has changed, fitting " << fData->GetN() << " peaks with ploynomial of " << degree << " degree" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      }
      if(fData->GetN() > degree) {
         fDegree = degree;
      } else {
         if(fVerboseLevel > 1) { std::cout << "Only got " << fData->GetN() << " data points, reducing degree of polynomial from " << degree << " to " << fData->GetN() - 1 << std::endl; }
         fDegree = fData->GetN() - 1;
      }
      TF1* calibration = new TF1("fitfunction", ::Polynomial, 0., 10000., fDegree + 2);
      calibration->FixParameter(0, fDegree);
      fData->Fit(calibration, "Q");
      TString text = Form("%.6f + %.6f*x", calibration->GetParameter(1), calibration->GetParameter(2));
      for(int par = 2; par <= fDegree; ++par) {
         text.Append(Form(" + %.6f*x^%d", calibration->GetParameter(par + 1), par));
      }
      fStatusBar->SetText(text.Data(), 2);
      delete calibration;
   }

   fCalibrationCanvas->GetCanvas()->cd();
   fData->Draw("a*");
}

void TChannelTab::FindPeaks(const double& sigma, const double& threshold, const std::vector<std::tuple<double, double, double, double>>& sourceEnergy, const bool& force, const bool& fast)
{
   /// This functions finds the peaks in the histogram, fits them, and adds the fits to the list of peaks.
   /// This list is then used to find all peaks that lie on a straight line.

   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::flush << " " << fProjection->GetName() << ": got " << fPeaks.size() << " peaks" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   if(fPeaks.empty() || fData == nullptr || sigma != fSigma || threshold != fThreshold || force) {
      if(fVerboseLevel > 2) {
         std::cout << __PRETTY_FUNCTION__ << ": no peaks " << fPeaks.size() << ", sigma (" << sigma << "/" << fSigma << "), or threshold (" << threshold << "/" << fThreshold << ") have changed" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      }
      fSigma     = sigma;
      fThreshold = threshold;
      fPeaks.clear();
      TSpectrum spectrum;
      spectrum.Search(fProjection, fSigma, "", fThreshold);
      if(fVerboseLevel > 2) { std::cout << __PRETTY_FUNCTION__ << ": found " << spectrum.GetNPeaks() << " peaks" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      //fPeakFitter.RemoveAllPeaks();
      for(int i = 0; i < spectrum.GetNPeaks(); i++) {
         double range = 4 * fSigma * fProjection->GetXaxis()->GetBinWidth(1);
         GPeak* peak  = PhotoPeakFit(fProjection, spectrum.GetPositionX()[i] - range, spectrum.GetPositionX()[i] + range, "qretryfit");
         //fPeakFitter.SetRange(spectrum.GetPositionX()[i] - range, spectrum.GetPositionX()[i] + range);
         //auto peak = new TRWPeak(spectrum.GetPositionX()[i]);
         //fPeakFitter.AddPeak(peak);
         //fPeakFitter.Fit(fProjection, "qretryfit");
         if(peak->Area() > 0) {
            fPeaks.push_back(peak);
            if(fVerboseLevel > 2) {
               std::cout << "Fitted peak " << spectrum.GetPositionX()[i] - range << " - " << spectrum.GetPositionX()[i] + range << " -> centroid " << peak->Centroid() << ", area " << peak->Area() << std::endl;
            }
         } else if(fVerboseLevel > 2) {
            std::cout << "Ignoring peak at " << peak->Centroid() << " with negative area " << peak->Area() << std::endl;
         }
         //fProjection->GetListOfFunctions()->Remove(peak);
      }
      if(fVerboseLevel > 2) { std::cout << __PRETTY_FUNCTION__ << ": added " << fPeaks.size() << " peaks" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      fProjection->Sumw2(false);                                                                                            // turn errors off, makes the histogram look like a histogram when using normal plotting (hist and samefunc doesn't work for some reason)

      fProjectionCanvas->GetCanvas()->cd();
      fProjection->Draw();

      // get a list of peaks positions and areas
      std::vector<std::tuple<double, double, double, double>> peaks;
      for(auto* peak : fPeaks) {
         peaks.emplace_back(peak->Centroid(), peak->CentroidErr(), peak->Area(), peak->AreaErr());
      }

      if(fast) {
         auto map = SmartMatch(peaks, sourceEnergy, fVerboseLevel);
         Add(map);
      } else {
         auto map = Match(peaks, sourceEnergy, fVerboseLevel);
         Add(map);
      }
   }
   if(fVerboseLevel > 2) {
      std::cout << __PRETTY_FUNCTION__ << ": found " << fData->GetN() << " peaks";   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      Double_t* x = fData->GetX();
      Double_t* y = fData->GetY();
      for(int i = 0; i < fData->GetN(); ++i) {
         std::cout << " - " << x[i] << ", " << y[i];
      }
      std::cout << std::endl;
   }
}

void TChannelTab::Add(std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> map)
{
   delete fData;
   fData = new TGraphErrors(map.size());
   fData->SetLineColor(2);
   fData->SetMarkerColor(2);
   delete fEfficiency;
   fEfficiency = new TGraphErrors(map.size());
   fEfficiency->SetLineColor(2);
   fEfficiency->SetMarkerColor(2);
   int i = 0;
   for(auto iter = map.begin(); iter != map.end();) {
      // more readable variable names
      auto peakPos      = std::get<0>(iter->first);
      auto peakPosErr   = std::get<1>(iter->first);
      auto peakArea     = std::get<2>(iter->first);
      auto peakAreaErr  = std::get<3>(iter->first);
      auto energy       = std::get<0>(iter->second);
      auto energyErr    = std::get<1>(iter->second);
      auto intensity    = std::get<2>(iter->second);
      auto intensityErr = std::get<3>(iter->second);
      // drop this peak if the uncertainties in area or position are too large
      if(peakPosErr > 0.1 * peakPos || peakAreaErr > peakArea) {
         if(fVerboseLevel > 1) {
            std::cout << "Dropping peak with position " << peakPos << " +- " << peakPosErr << ", area " << peakArea << " +- " << peakAreaErr << ", energy " << energy << ", intensity " << intensity << std::endl;
         }
         map.erase(iter++);
      } else {
         fData->SetPoint(i, peakPos, energy);
         fData->SetPointError(i, peakPosErr, energyErr);
         // we use the source energy for the x-values, and the ratio of peak area and intensity for y
         fEfficiency->SetPoint(i, energy, peakArea / intensity);
         fEfficiency->SetPointError(i, energyErr, peakArea / intensity * TMath::Sqrt(TMath::Power(peakAreaErr / peakArea, 2) + TMath::Power(intensityErr / intensity, 2)));
         if(fVerboseLevel > 2) {
            std::cout << "Using peak with position " << peakPos << " +- " << peakPosErr << ", area " << peakArea << " +- " << peakAreaErr << ", energy " << energy << ", intensity " << intensity << std::endl;
         }
         ++iter;
         ++i;
      }
   }
   // if we dropped a peak, we need to resize the graph, if we haven't dropped any this doesn't do anything
   fData->Set(map.size());
   fEfficiency->Set(map.size());
   // remove poly markers that weren't used for the fit
   TList*              functions = fProjection->GetListOfFunctions();
   auto*               polym     = static_cast<TPolyMarker*>(functions->FindObject("TPolyMarker"));
   double*             oldX      = polym->GetX();
   double*             oldY      = polym->GetY();
   int                 size      = polym->GetN();
   std::vector<double> newX;
   std::vector<double> newY;
   for(i = 0; i < size; ++i) {
      for(auto point : map) {
         if(TMath::Abs(oldX[i] - std::get<0>(point.first)) < fSigma) {
            newX.push_back(oldX[i]);
            newY.push_back(oldY[i]);
            break;
         }
      }
   }
   polym->SetPolyMarker(newX.size(), newX.data(), newY.data());
   // remove fit functions for unused peaks
   TIter    iter(functions);
   TObject* item = nullptr;
   while((item = iter.Next()) != nullptr) {
      if(item->IsA() == TF1::Class() || item->IsA() == GPeak::Class()) {   // if the item is a TF1 or GPeak we see if we can find the centroid in the map of used peaks
         double centroid = 0.;
         if(item->IsA() == TF1::Class()) {
            centroid = static_cast<TF1*>(item)->GetParameter(1);
         } else {
            centroid = static_cast<GPeak*>(item)->Centroid();
         }
         bool found = false;
         for(auto point : map) {
            if(TMath::Abs(centroid - std::get<0>(point.first)) < fSigma) {
               found = true;
               break;
            }
         }
         // remove the TF1 if it wasn't found in the map
         if(!found) {
            functions->Remove(item);
         }
      }
   }
}

//////////////////////////////////////// TSourceTab ////////////////////////////////////////
TSourceTab::TSourceTab(TSourceCalibration* parent, TNucleus* nucleus, TH2* matrix, TGCompositeFrame* frame, double sigma, double threshold, int degree, std::vector<std::tuple<double, double, double, double>> sourceEnergy, TGHProgressBar* progressBar)
   : fFrame(frame), fChannelTab(new TGTab(frame, 1200, 500)), fProgressBar(progressBar), fNucleus(nucleus), fParent(parent), fMatrix(matrix), fSigma(sigma), fThreshold(threshold), fDegree(degree), fSourceEnergy(std::move(sourceEnergy))
{
   fChannel.resize(fMatrix->GetNbinsX(), nullptr);
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": creating channels for bin 1 to " << fMatrix->GetNbinsX() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   for(int bin = 1; bin <= fMatrix->GetNbinsX(); ++bin) {
      CreateChannelTab(bin);
   }

   for(auto iter = fChannel.begin(); iter != fChannel.end(); ++iter) {
      if(*iter == nullptr) {
         fChannel.erase(iter--);   // erase iterator and then go to the element before this one (and then the loop goes to the next one)
      }
   }

   fFrame->AddFrame(fChannelTab, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
}

TSourceTab::~TSourceTab()
{
   delete fChannelTab;
   for(auto* channel : fChannel) {
      delete channel;
   }
}

void TSourceTab::CreateChannelTab(int bin)
{
   auto*       proj  = fMatrix->ProjectionY(Form("%s_%s", fNucleus->GetName(), fMatrix->GetXaxis()->GetBinLabel(bin)), bin, bin);
   const char* label = fMatrix->GetXaxis()->GetBinLabel(bin);
   if(proj->GetEntries() > 1000) {
      fChannel[bin - 1] = new TChannelTab(fChannelTab, proj, fChannelTab->AddTab(Form("%s_%s", fNucleus->GetName(), label)),
                                          fSigma, fThreshold, fDegree, fSourceEnergy);
      fProgressBar->Increment(1);
   } else {
      fChannel[bin - 1] = nullptr;
      if(fVerboseLevel > 1) {
         std::cout << "Skipping projection of bin " << bin << " = " << proj->GetName() << ", only " << proj->GetEntries() << " entries" << std::endl;
      }
   }
   if(fVerboseLevel > 1) {
      std::cout << __PRETTY_FUNCTION__ << " bin " << bin << " done" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
}

void TSourceTab::MakeConnections()
{
   fChannelTab->Connect("Selected(Int_t)", "TSourceCalibration", fParent, "SelectedTab(Int_t)");
   for(auto& channel : fChannel) {
      channel->MakeConnections();
   }
}

void TSourceTab::Disconnect()
{
   fChannelTab->Disconnect("Selected(Int_t)", fParent, "SelectedTab(Int_t)");
   for(auto& channel : fChannel) {
      channel->Disconnect();
   }
}

//////////////////////////////////////// TSourceCalibration ////////////////////////////////////////
TSourceCalibration::TSourceCalibration(double sigma, double threshold, int degree, int count...)
   : TGMainFrame(nullptr, 1200, 600), fDefaultSigma(sigma), fDefaultThreshold(threshold), fDefaultDegree(degree)
{
   TH1::AddDirectory(false);   // turns off warnings about multiple histograms with the same name because ROOT doesn't manage them anymore

   va_list args;
   va_start(args, count);   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   for(int i = 0; i < count; ++i) {
      fMatrices.push_back(va_arg(args, TH2*));   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      if(fMatrices.back() == nullptr) {
         std::cout << "Error, got nullptr as matrix input?" << std::endl;
         fMatrices.pop_back();
      }
   }
   va_end(args);   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fVerboseLevel > 0) {
      std::cout << __PRETTY_FUNCTION__ << ": using " << count << "/" << fMatrices.size() << " matrices:" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      for(auto* mat : fMatrices) {
         std::cout << mat << std::flush << " = " << mat->GetName() << std::endl;
      }
   }

   fOldErrorLevel        = gErrorIgnoreLevel;
   gErrorIgnoreLevel     = kError;
   gPrintViaErrorHandler = true;   // redirects all printf's to root's normal message system

   // check matrices (# of filled bins and bin labels) and resize some vectors for later use
   // use the first matrix to get a reference for everything
   fActualSourceId.resize(fMatrices.size());
   if(fVerboseLevel > 1) { std::cout << "resized fActualSourceId to " << fActualSourceId.size() << std::endl; }
   fActualSourceId[0] = 0;
   fActualChannelId.resize(fMatrices.size());
   fData.resize(fMatrices.size());
   fEfficiency.resize(fMatrices.size());
   fNofBins = 0;
   for(int bin = 1; bin <= fMatrices[0]->GetNbinsX(); ++bin) {
      if(FilledBin(fMatrices[0], bin)) {
         fActualChannelId[0].push_back(fNofBins);   // at this point fNofBins is the index at which this projection will end up
         fChannelToIndex[bin] = fNofBins;           // this map simply stores which bin ends up at which index
         fChannelLabel.push_back(fMatrices[0]->GetXaxis()->GetBinLabel(bin));
         if(fVerboseLevel > 1) { std::cout << bin << ". bin: fNofBins " << fNofBins << ", fChannelToIndex[" << bin << "] " << fChannelToIndex[bin] << ", fActualChannelId[0].back() " << fActualChannelId[0].back() << std::endl; }
         ++fNofBins;
      } else {
         if(fVerboseLevel > 1) { std::cout << "skipping bin " << bin << std::endl; }
      }
   }
   fData[0].resize(fNofBins, nullptr);
   fEfficiency[0].resize(fNofBins, nullptr);
   // now loop over all other matrices and check vs. the first one
   for(size_t i = 1; i < fMatrices.size(); ++i) {
      fActualSourceId[i] = static_cast<int>(i);
      int tmpBins        = 0;
      for(int bin = 1; bin <= fMatrices[i]->GetNbinsX(); ++bin) {
         if(FilledBin(fMatrices[i], bin)) {   // good bin in the current matrix
            // current index is tmpBins, so we check if the bin agrees with what we have
            if(fChannelToIndex.find(bin) == fChannelToIndex.end() || tmpBins != fChannelToIndex[bin]) {   // found a full bin, but the corresponding bin in the first matrix isn't full or a different bin
               std::stringstream str;
               str << "Mismatch in " << i << ". matrix (" << fMatrices[i]->GetName() << "), bin " << bin << " is " << tmpBins << ". filled bin, but should be " << (fChannelToIndex.find(bin) == fChannelToIndex.end() ? "not filled" : Form("%d", fChannelToIndex[bin])) << std::endl;
               throw std::invalid_argument(str.str());
            }
            if(strcmp(fMatrices[0]->GetXaxis()->GetBinLabel(bin), fMatrices[i]->GetXaxis()->GetBinLabel(bin)) != 0) {   // bin is full and matches the bin in the first matrix so we check the labels
               std::stringstream str;
               str << i << ". matrix, " << bin << ". bin: label (" << fMatrices[i]->GetXaxis()->GetBinLabel(bin) << ") doesn't match bin label of the first matrix (" << fMatrices[0]->GetXaxis()->GetBinLabel(bin) << ")" << std::endl;
               throw std::invalid_argument(str.str());
            }
            ++tmpBins;
         } else {
            if(fChannelToIndex.find(bin) != fChannelToIndex.end()) {   //found an empty bin that was full in the first matrix
               std::stringstream str;
               str << "Mismatch in " << i << ". matrix (" << fMatrices[i]->GetName() << "), bin " << bin << " is empty, but should be " << fChannelToIndex[bin] << ". filled bin" << std::endl;
               throw std::invalid_argument(str.str());
            }
         }
      }
      // we looped over all bins without finding any issues, so we can just copy the first matrix
      fActualChannelId[i] = fActualChannelId[0];
      fData[i].resize(fNofBins, nullptr);
      fEfficiency[i].resize(fNofBins, nullptr);
   }

   if(fVerboseLevel > 0) { std::cout << fMatrices.size() << " matrices with " << fMatrices[0]->GetNbinsX() << " x-bins, fNofBins " << fNofBins << ", fActualChannelId[0].size() " << fActualChannelId[0].size() << std::endl; }

   fOutput = new TFile("TSourceCalibration.root", "recreate");
   if(!fOutput->IsOpen()) {
      throw std::runtime_error("Unable to open output file \"TSourceCalibration.root\"!");
   }

   // build the first screen
   BuildFirstInterface();
   MakeFirstConnections();

   // Set a name to the main frame
   SetWindowName("SourceCalibration");

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(GetDefaultSize());

   // Map main frame
   MapWindow();
}

TSourceCalibration::~TSourceCalibration()
{
   fOutput->Close();
   delete fOutput;

   gErrorIgnoreLevel = fOldErrorLevel;
}

void TSourceCalibration::BuildFirstInterface()
{
   /// Build initial interface with histogram <-> source assignment

   auto* layoutManager = new TGTableLayout(this, fMatrices.size() + 1, 2, true, 5);
   if(fVerboseLevel > 1) { std::cout << "created table layout manager with 2 columns, " << fMatrices.size() + 1 << " rows" << std::endl; }
   SetLayoutManager(layoutManager);

   // The matrices and source selection boxes
   size_t i = 0;
   for(i = 0; i < fMatrices.size(); ++i) {
      fMatrixNames.push_back(new TGLabel(this, fMatrices[i]->GetName()));
      if(fVerboseLevel > 2) { std::cout << "Text height " << fMatrixNames.back()->GetFont()->TextHeight() << std::endl; }
      fSource.push_back(nullptr);
      fSourceEnergy.emplace_back();
      fSourceBox.push_back(new TGComboBox(this, "Select source", kSourceBox + fSourceBox.size()));

      int index = 0;
#if __cplusplus >= 201703L
      // For some reasons getenv("GRSISYS") with strcat does not work (adds the "sub"-path twice).
      // Have to do this by copying the getenv result into a c++-string.
      if(std::getenv("GRSISYS") == nullptr) {
         throw std::runtime_error("Failed to get environment variable $GRSISYS");
      }
      std::string path(std::getenv("GRSISYS"));
      path += "/libraries/TAnalysis/SourceData/";
      for(const auto& file : std::filesystem::directory_iterator(path)) {
         if(file.is_regular_file() && file.path().extension().compare(".sou") == 0) {
            fSourceBox.back()->AddEntry(file.path().stem().c_str(), index);
            if(std::strstr(fMatrices[i]->GetName(), file.path().stem().c_str()) != nullptr) {
               fSourceBox.back()->Select(index);
               SetSource(kSourceBox + fSourceBox.size() - 1, index);
            }
         }
      }
#else
      fSourceBox.back()->AddEntry("22Na", index);
      if(std::strstr(fMatrices[i]->GetName(), "22Na") != nullptr) {
         fSourceBox.back()->Select(index);
         SetSource(kSourceBox + fSourceBox.size() - 1, index);
      }
      ++index;
      fSourceBox.back()->AddEntry("56Co", index);
      if(std::strstr(fMatrices[i]->GetName(), "56Co") != nullptr) {
         fSourceBox.back()->Select(index);
         SetSource(kSourceBox + fSourceBox.size() - 1, index);
      }
      ++index;
      fSourceBox.back()->AddEntry("60Co", index);
      if(std::strstr(fMatrices[i]->GetName(), "60Co") != nullptr) {
         fSourceBox.back()->Select(index);
         SetSource(kSourceBox + fSourceBox.size() - 1, index);
      }
      ++index;
      fSourceBox.back()->AddEntry("133Ba", index);
      if(std::strstr(fMatrices[i]->GetName(), "133Ba") != nullptr) {
         fSourceBox.back()->Select(index);
         SetSource(kSourceBox + fSourceBox.size() - 1, index);
      }
      ++index;
      fSourceBox.back()->AddEntry("152Eu", index);
      if(std::strstr(fMatrices[i]->GetName(), "152Eu") != nullptr) {
         fSourceBox.back()->Select(index);
         SetSource(kSourceBox + fSourceBox.size() - 1, index);
      }
      ++index;
      fSourceBox.back()->AddEntry("241Am", index);
      if(std::strstr(fMatrices[i]->GetName(), "241Am") != nullptr) {
         fSourceBox.back()->Select(index);
         SetSource(kSourceBox + fSourceBox.size() - 1, index);
      }
#endif

      fSourceBox.back()->SetMinHeight(200);

      //fMatrixNames.back()->Resize(600, fLineHeight);
      fSourceBox.back()->Resize(100, fLineHeight);
      if(fVerboseLevel > 2) { std::cout << "Attaching " << i << ". label to 0, 1, " << i << ", " << i + 1 << ", and box to 1, 2, " << i << ", " << i + 1 << std::endl; }
      AddFrame(fMatrixNames.back(), new TGTableLayoutHints(0, 1, i, i + 1, kLHintsRight | kLHintsCenterY));
      AddFrame(fSourceBox.back(), new TGTableLayoutHints(1, 2, i, i + 1, kLHintsLeft | kLHintsCenterY));
   }

   // The buttons
   if(fVerboseLevel > 1) { std::cout << "Attaching start button to 0, 2, " << i << ", " << i + 1 << std::endl; }
   fStartButton = new TGTextButton(this, "Accept && Continue", kStartButton);
   if(fVerboseLevel > 1) { std::cout << "start button " << fStartButton << std::endl; }
   AddFrame(fStartButton, new TGTableLayoutHints(0, 2, i, i + 1, kLHintsCenterX | kLHintsCenterY));
   Layout();
}

void TSourceCalibration::MakeFirstConnections()
{
   /// Create connections for the histogram <-> source assignment interface

   // Connect the selection of the source
   for(auto* box : fSourceBox) {
      box->Connect("Selected(Int_t, Int_t)", "TSourceCalibration", this, "SetSource(Int_t, Int_t)");
   }

   //Connect the clicking of buttons
   fStartButton->Connect("Clicked()", "TSourceCalibration", this, "Start()");
}

void TSourceCalibration::DisconnectFirst()
{
   /// Disconnect all signals from histogram <-> source assignment interface
   for(auto* box : fSourceBox) {
      box->Disconnect("Selected(Int_t, Int_t)", this, "SetSource(Int_t, Int_t)");
   }

   fStartButton->Disconnect("Clicked()", this, "Start()");
}

void TSourceCalibration::DeleteElement(TGFrame* element)
{
   HideFrame(element);
   RemoveFrame(element);
   //delete element;
   //element = nullptr;
}

void TSourceCalibration::DeleteFirst()
{
   for(auto* name : fMatrixNames) {
      DeleteElement(name);
   }
   fMatrixNames.clear();
   for(auto* box : fSourceBox) {
      DeleteElement(box);
   }
   fSourceBox.clear();
   DeleteElement(fStartButton);
   if(fVerboseLevel > 2) { std::cout << "Deleted start button " << fStartButton << std::endl; }
}

void TSourceCalibration::SetSource(Int_t windowId, Int_t entryId)
{
   int index = windowId - kSourceBox;
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": windowId " << windowId << ", entryId " << entryId << " => " << index << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   delete fSource[index];
   auto* nucleus = new TNucleus(fSourceBox[index]->GetListBox()->GetEntry(entryId)->GetTitle());
   TIter iter(nucleus->GetTransitionList());
   fSourceEnergy[index].clear();
   while(auto* transition = static_cast<TTransition*>(iter.Next())) {
      fSourceEnergy[index].push_back(std::make_tuple(transition->GetEnergy(), transition->GetEnergyUncertainty(), transition->GetIntensity(), transition->GetIntensityUncertainty()));
   }
   fSource[index] = nucleus;
}

void TSourceCalibration::HandleTimer()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": fEmitter " << fEmitter << ", fStartButton " << fStartButton << ", fAcceptAllButton " << fAcceptAllButton << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fEmitter == fStartButton) {
      SecondWindow();
   } else if(fEmitter == fAcceptAllButton) {
      FinalWindow();
   }
}

void TSourceCalibration::Start()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": fEmitter " << fEmitter << ", fStartButton " << fStartButton << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fEmitter == nullptr) {                                                                                                                    // we only want to do this once at the beginning (after fEmitter was initialized to nullptr)
      fEmitter = fStartButton;
      TTimer::SingleShot(100, "TSourceCalibration", this, "HandleTimer()");
   }
}

void TSourceCalibration::SecondWindow()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   // check that all sources have been set
   for(size_t i = 0; i < fSource.size(); ++i) {
      if(fSource[i] == nullptr) {
         std::cerr << "Source " << i << " not set!" << std::endl;
         return;
      }
   }
   // now check that we don't have the same source twice (which wouldn't make sense)
   for(size_t i = 0; i < fSource.size(); ++i) {
      for(size_t j = i + 1; j < fSource.size(); ++j) {
         if(*(fSource[i]) == *(fSource[j])) {
            std::cerr << "Duplicate sources: " << i << " - " << fSource[i]->GetName() << " and " << j << " - " << fSource[j]->GetName() << std::endl;
            return;
         }
      }
   }

   // disconnect signals of first screen and remove all elements
   DisconnectFirst();
   RemoveAll();
   DeleteFirst();

   SetLayoutManager(new TGVerticalLayout(this));

   // create intermediate progress bar
   fProgressBar = new TGHProgressBar(this, TGProgressBar::kFancy, 600);
   fProgressBar->SetRange(0., static_cast<Float_t>(fMatrices.size() * fNofBins));
   fProgressBar->Percent(true);
   if(fVerboseLevel > 2) { std::cout << "Set range of progress bar to 0. - " << fProgressBar->GetMax() << " = " << fMatrices.size() * fNofBins << " = " << fMatrices.size() << "*" << fNofBins << std::endl; }
   AddFrame(fProgressBar, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(TGDimension(600, fLineHeight));

   // Map main frame
   MapWindow();

   // create second screen and its connections
   BuildSecondInterface();
   MakeSecondConnections();

   // remove progress bar
   DeleteElement(fProgressBar);

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(TGDimension(1200, 600));

   // Map main frame
   MapWindow();
   if(fVerboseLevel > 2) { std::cout << __PRETTY_FUNCTION__ << " done" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

void TSourceCalibration::BuildSecondInterface()
{
   SetLayoutManager(new TGVerticalLayout(this));
   fTab = new TGTab(this, 1200, 600);

   fSourceTab.resize(fSource.size());
   for(size_t i = 0; i < fSource.size(); ++i) {
      fSourceTab[i] = new TSourceTab(this, fSource[i], fMatrices[i], fTab->AddTab(fSource[i]->GetName()), fDefaultSigma, fDefaultThreshold, fDefaultDegree, SourceEnergy(i), fProgressBar);
   }

   if(fVerboseLevel > 1) { std::cout << fMatrices.size() << " matrices with " << fMatrices[0]->GetNbinsX() << " x-bins, fNofBins " << fNofBins << ", fActualChannelId[0].size() " << fActualChannelId[0].size() << ", fSourceTab[0]->ChannelTab()->GetNumberOfTabs() " << fSourceTab[0]->ChannelTab()->GetNumberOfTabs() << std::endl; }

   for(int i = 0; i < fSourceTab[0]->ChannelTab()->GetNumberOfTabs(); ++i) {
      if(fVerboseLevel > 2) { std::cout << i << ": " << fSourceTab[0]->ChannelTab()->GetTabTab(i)->GetText()->GetString() << std::endl; }
   }

   AddFrame(fTab, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 0, 0, 0, 0));

   // bottom frame with navigation button group, text entries, etc.
   fBottomFrame = new TGHorizontalFrame(this, 1200, 50);

   fLeftFrame       = new TGVerticalFrame(fBottomFrame, 600, 50);
   fNavigationGroup = new TGHButtonGroup(fLeftFrame, "");
   fPreviousButton  = new TGTextButton(fNavigationGroup, "Previous");
   if(fVerboseLevel > 2) { std::cout << "prev button " << fPreviousButton << std::endl; }
   fPreviousButton->SetEnabled(false);
   fFindPeaksButton     = new TGTextButton(fNavigationGroup, "Find Peaks");
   fFindPeaksFastButton = new TGTextButton(fNavigationGroup, "Find Peaks Fast");
   if(fVerboseLevel > 2) { std::cout << "find peaks button " << fFindPeaksButton << std::endl; }
   fCalibrateButton = new TGTextButton(fNavigationGroup, "Calibrate");
   if(fVerboseLevel > 2) { std::cout << "cal button " << fCalibrateButton << std::endl; }
   fDiscardButton = new TGTextButton(fNavigationGroup, "Discard");
   if(fVerboseLevel > 2) { std::cout << "discard button " << fDiscardButton << std::endl; }
   fAcceptButton = new TGTextButton(fNavigationGroup, "Accept");
   if(fVerboseLevel > 2) { std::cout << "accept button " << fAcceptButton << std::endl; }
   if(fMatrices.size() == 1) {
      fAcceptAllButton = new TGTextButton(fNavigationGroup, "Accept All && Finish");
   } else {
      fAcceptAllButton = new TGTextButton(fNavigationGroup, "Accept All");
   }
   if(fVerboseLevel > 2) { std::cout << "accept all button " << fAcceptAllButton << std::endl; }
   fNextButton = new TGTextButton(fNavigationGroup, "Next");
   if(fVerboseLevel > 2) { std::cout << "next button " << fNextButton << std::endl; }

   fLeftFrame->AddFrame(fNavigationGroup, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

   fBottomFrame->AddFrame(fLeftFrame, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 2, 2, 2, 2));

   fRightFrame     = new TGVerticalFrame(fBottomFrame, 600, 50);
   fParameterFrame = new TGGroupFrame(fRightFrame, "Parameters", kHorizontalFrame);
   fParameterFrame->SetLayoutManager(new TGMatrixLayout(fParameterFrame, 0, 2, 5));
   fSigmaLabel     = new TGLabel(fParameterFrame, "Sigma");
   fSigmaEntry     = new TGNumberEntry(fParameterFrame, fDefaultSigma, 5, kSigmaEntry, TGNumberFormat::EStyle::kNESRealTwo, TGNumberFormat::EAttribute::kNEAPositive);
   fThresholdLabel = new TGLabel(fParameterFrame, "Threshold");
   fThresholdEntry = new TGNumberEntry(fParameterFrame, fDefaultThreshold, 5, kThresholdEntry, TGNumberFormat::EStyle::kNESRealThree, TGNumberFormat::EAttribute::kNEAPositive, TGNumberFormat::ELimit::kNELLimitMinMax, 0., 1.);
   fDegreeLabel    = new TGLabel(fParameterFrame, "Degree of polynomial");
   fDegreeEntry    = new TGNumberEntry(fParameterFrame, fDefaultDegree, 2, kDegreeEntry, TGNumberFormat::EStyle::kNESInteger, TGNumberFormat::EAttribute::kNEAPositive);

   fParameterFrame->AddFrame(fSigmaLabel);
   fParameterFrame->AddFrame(fSigmaEntry);
   fParameterFrame->AddFrame(fThresholdLabel);
   fParameterFrame->AddFrame(fThresholdEntry);
   fParameterFrame->AddFrame(fDegreeLabel);
   fParameterFrame->AddFrame(fDegreeEntry);

   fRightFrame->AddFrame(fParameterFrame, new TGLayoutHints(kLHintsRight | kLHintsExpandY, 2, 2, 2, 2));

   fBottomFrame->AddFrame(fRightFrame, new TGLayoutHints(kLHintsBottom | kLHintsExpandY, 2, 2, 2, 2));

   AddFrame(fBottomFrame, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));
   if(fVerboseLevel > 2) { std::cout << "Second interface done" << std::endl; }
}

void TSourceCalibration::MakeSecondConnections()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fNavigationGroup->Connect("Clicked(Int_t)", "TSourceCalibration", this, "Navigate(Int_t)");
   // we don't need to connect the sigma, threshold, and degree number entries, those are automatically read when we start the calibration
   for(auto* sourceTab : fSourceTab) {
      sourceTab->MakeConnections();
   }
}

void TSourceCalibration::DisconnectSecond()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fNavigationGroup->Disconnect("Clicked(Int_t)", this, "Navigate(Int_t)");
   for(auto* sourceTab : fSourceTab) {
      sourceTab->Disconnect();
   }
}

void TSourceCalibration::Navigate(Int_t id)
{
   // we get the current source tab id and use it to get the channel tab from the right source tab
   // since the current id only refers to the position within the open tabs we need to keep track of the actual id it relates to
   // for this we created a vector with all ids at the beginning and now remove the position correspoding to the current id when we remove the tab
   int   currentSourceId  = fTab->GetCurrent();
   int   actualSourceId   = fActualSourceId[currentSourceId];
   auto* currentTab       = fSourceTab[actualSourceId]->ChannelTab();
   int   currentChannelId = currentTab->GetCurrent();
   int   nofTabs          = currentTab->GetNumberOfTabs();
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": id " << id << ", source tab id " << currentSourceId << ", actual source tab id " << actualSourceId << ", channel tab id " << currentTab->GetCurrent() << ", # of tabs " << nofTabs << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   switch(id) {
   case 1:   // previous
      currentTab->SetTab(currentChannelId - 1);
      SelectedTab(currentChannelId - 1);
      break;
   case 2:   // find peaks
      FindPeaks();
      break;
   case 3:   // find peaks fast
      FindPeaksFast();
      break;
   case 4:   // calibrate
      Calibrate();
      break;
   case 5:   // discard
      // select the next (or if we are on the last tab, the previous) tab
      if(currentChannelId < nofTabs - 1) {
         currentTab->SetTab(currentChannelId + 1);
      } else {
         currentTab->SetTab(currentChannelId - 1);
      }
      // remove the original active tab
      currentTab->RemoveTab(currentChannelId);
      break;
   case 6:   // accept
      AcceptChannel(currentChannelId);
      break;
   case 7:   // accept all (no argument = -1 = all)
      AcceptChannel();
      break;
   case 8:   // next
      currentTab->SetTab(currentChannelId + 1);
      SelectedTab(currentChannelId + 1);
      break;
   default:
      break;
   }
}

void TSourceCalibration::SelectedTab(Int_t id)
{
   /// Simple function that enables and disables the previous and next buttons depending on which tab was selected
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": id " << id << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(id == 0) {
      fPreviousButton->SetEnabled(false);
   } else {
      fPreviousButton->SetEnabled(true);
   }

   if(id == fSourceTab[fTab->GetCurrent()]->ChannelTab()->GetNumberOfTabs() - 1) {
      fNextButton->SetEnabled(false);
   } else {
      fNextButton->SetEnabled(true);
   }
}

void TSourceCalibration::AcceptChannel(const int& channelId)
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": channelId " << channelId << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   // select the next (or if we are on the last tab, the previous) tab
   int   currentSourceId = fTab->GetCurrent();
   int   actualSourceId  = fActualSourceId[currentSourceId];
   auto* currentTab      = fSourceTab[actualSourceId]->ChannelTab();
   int   nofTabs         = currentTab->GetNumberOfTabs();
   int   minChannel      = 0;
   int   maxChannel      = nofTabs - 1;
   if(channelId >= 0) {
      minChannel = channelId;
      maxChannel = channelId;
   }
   // we need to loop backward, because removing the first channel would make the second one the new first and so on
   if(fVerboseLevel > 2) { std::cout << __PRETTY_FUNCTION__ << ": accepting channels " << maxChannel << " to " << minChannel << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   for(int currentChannelId = maxChannel; currentChannelId >= minChannel; --currentChannelId) {
      int actualChannelId = fActualChannelId[currentSourceId][currentChannelId];
      if(fVerboseLevel > 3) { std::cout << __PRETTY_FUNCTION__ << ": currentChannelId " << currentChannelId << ", currentSourceId " << currentSourceId << ", actualChannelId " << actualChannelId << ", actualSourceId " << actualSourceId << ", fData.size() " << fData.size() << ", fData[" << actualSourceId << "].size() " << fData[actualSourceId].size() << ", fSourceTab.size() " << fSourceTab.size() << ", fActualChannelId[" << currentSourceId << "].size() " << fActualChannelId[currentSourceId].size() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      if(minChannel == maxChannel) {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // we don't need to select the tab if we close all
         if(currentChannelId < nofTabs - 1) {
            currentTab->SetTab(currentChannelId + 1);
         } else {
            currentTab->SetTab(currentChannelId - 1);
         }
      }
      // remove the original active tab
      currentTab->RemoveTab(currentChannelId);
      // get the graph from the closed tab
      fData[actualSourceId][actualChannelId]       = fSourceTab[actualSourceId]->Data(currentChannelId);
      fEfficiency[actualSourceId][actualChannelId] = fSourceTab[actualSourceId]->Efficiency(currentChannelId);
      fActualChannelId[currentSourceId].erase(fActualChannelId[currentSourceId].begin() + currentChannelId);
   }
   if(fVerboseLevel > 2) { std::cout << __PRETTY_FUNCTION__ << ": # of channel tabs " << fActualChannelId[currentSourceId].size() << ", # of source tabs " << fActualSourceId.size() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   // if this was the last tab, we close the whole source tab and remove that vector from the actual ids too
   if(!fActualChannelId.empty() && fActualChannelId[currentSourceId].empty()) {
      if(fVerboseLevel > 2) { std::cout << "Last tab closed, closing source tab " << currentSourceId << "/" << fActualSourceId.size() << "/" << fActualChannelId.size() << std::endl; }
      std::cout << "Last tab closed, closing source tab " << currentSourceId << "/" << fActualSourceId.size() << "/" << fActualChannelId.size() << std::endl;
      fTab->RemoveTab();
      fActualSourceId.erase(fActualSourceId.begin() + currentSourceId);
      fActualChannelId.erase(fActualChannelId.begin() + currentSourceId);
   }
   // check if this changes which buttons are enabled or not (not using the variables from the beginning of this block because they might have changed by now!)
   SelectedTab(fSourceTab[fTab->GetCurrent()]->ChannelTab()->GetCurrent());
   // if this was also the last source vector we initiate the last screen
   if(fActualSourceId.empty()) {
      if(fVerboseLevel > 2) { std::cout << "last source tab done - going to final screen" << std::endl; }
      fEmitter = fAcceptAllButton;
      TTimer::SingleShot(100, "TSourceCalibration", this, "HandleTimer()");
   }
}

void TSourceCalibration::FinalWindow()
{
   // disconnect signals of first screen and remove all elements
   DisconnectSecond();
   fParameterFrame->RemoveAll();
   fRightFrame->RemoveAll();
   fLeftFrame->RemoveAll();
   fBottomFrame->RemoveAll();
   RemoveAll();
   DeleteSecond();

   if(fMatrices.size() == 1) {   // single matrix => don't need the last screen
      fFinalData.resize(fData[0].size());
      fFinalEfficiency.resize(fEfficiency[0].size());
      for(size_t i = 0; i < fData[0].size(); ++i) {   // should both be the same size, so just one loop
         fFinalData[i]       = new TCalibrationGraphSet(fData[0][i]);
         fFinalEfficiency[i] = new TCalibrationGraphSet(fEfficiency[0][i]);
         TF1* calibration    = new TF1("fitfunction", ::Polynomial, 0., 10000., Degree() + 2);
         calibration->FixParameter(0, Degree());
         fFinalData[i]->Fit(calibration, "Q");
      }
      for(size_t id = 0; id < fChannelLabel.size(); ++id) {
         UpdateChannel(id);
      }
      WriteCalibration();
      std::cout << "Closing window" << std::endl;
      CloseWindow();
      std::cout << "all done" << std::endl;
      exit(0);
   }

   // create last screen and its connections
   BuildThirdInterface();
   MakeThirdConnections();

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(TGDimension(600, 600));

   // Map main frame
   MapWindow();
   if(fVerboseLevel > 1) { std::cout << "final screen done" << std::endl; }
}

void TSourceCalibration::DeleteSecond()
{
   //for(auto tab : fSourceTab) {
   //delete tab;
   //}
   fSourceTab.clear();
   DeleteElement(fBottomFrame);
   DeleteElement(fLeftFrame);
   DeleteElement(fNavigationGroup);
   DeleteElement(fPreviousButton);
   DeleteElement(fFindPeaksButton);
   DeleteElement(fFindPeaksFastButton);
   DeleteElement(fCalibrateButton);
   DeleteElement(fDiscardButton);
   DeleteElement(fAcceptButton);
   DeleteElement(fAcceptAllButton);
   DeleteElement(fNextButton);
   DeleteElement(fRightFrame);
   DeleteElement(fParameterFrame);
   DeleteElement(fSigmaLabel);
   DeleteElement(fSigmaEntry);
   DeleteElement(fThresholdLabel);
   DeleteElement(fThresholdEntry);
   DeleteElement(fDegreeLabel);
   DeleteElement(fDegreeEntry);
}

void TSourceCalibration::FindPeaks()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fSourceTab[fTab->GetCurrent()]->FindPeaks(Sigma(), Threshold(), true, false);
   Calibrate();
}

void TSourceCalibration::FindPeaksFast()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fSourceTab[fTab->GetCurrent()]->FindPeaks(Sigma(), Threshold(), true, true);
   Calibrate();
}

void TSourceCalibration::Calibrate()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fSourceTab[fTab->GetCurrent()]->Calibrate(Degree(), true);
}

void TSourceCalibration::BuildThirdInterface()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   SetLayoutManager(new TGVerticalLayout(this));
   fTab         = new TGTab(this, 600, 600);
   auto* calTab = fTab->AddTab("Calibration");
   fFinalTabs.push_back(new TGTab(calTab, 600, 600));
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 11)
   fFinalTabs.back()->SetScrollingEnabled();
#endif
   auto* effTab = fTab->AddTab("Efficiency");
   fFinalTabs.push_back(new TGTab(effTab, 600, 600));
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 11)
   fFinalTabs.back()->SetScrollingEnabled();
#endif
   if(fVerboseLevel > 2) { std::cout << "# of tabs before adding: " << fTab->GetNumberOfTabs() << std::endl; }
   // we're re-using the actual source id for the channels here
   fActualSourceId.resize(fNofBins);
   if(fVerboseLevel > 2) { std::cout << "resized fActualSourceId to " << fActualSourceId.size() << std::endl; }
   fFinalData.resize(fNofBins);
   fFinalEfficiency.resize(fNofBins);
   fCalibrationPad.resize(fNofBins, nullptr);
   fEfficiencyPad.resize(fNofBins, nullptr);
   fResidualPad.resize(fNofBins, nullptr);
   fEfficiencyResidualPad.resize(fNofBins, nullptr);
   fLegend.resize(fNofBins, nullptr);
   fEfficiencyLegend.resize(fNofBins, nullptr);
   fChi2Label.resize(fNofBins, nullptr);
   fEfficiencyChi2Label.resize(fNofBins, nullptr);
   int                tmpBin   = 0;
   std::array<int, 3> parts    = {20, 50, 30};
   std::array<int, 2> partsEff = {60, 40};
   for(int bin = 1; bin <= fMatrices[0]->GetNbinsX(); ++bin) {
      // check if we use this bin
      if(!FilledBin(fMatrices[0], bin)) {
         if(fVerboseLevel > 3) { std::cout << "skipping bin " << bin << ", tmpBin " << tmpBin << std::endl; }
         continue;
      }
      if(fVerboseLevel > 2) { std::cout << "using bin " << bin << ", tmpBin " << tmpBin << std::endl; }
      // copy all data into one graph (which we use for the calibration)
      fFinalData[tmpBin] = new TCalibrationGraphSet;
      fFinalData[tmpBin]->VerboseLevel(fVerboseLevel - 4);
      fFinalData[tmpBin]->SetName(Form("data%s", fMatrices[0]->GetXaxis()->GetBinLabel(bin)));
      if(fVerboseLevel > 2) { std::cout << "set name of fFinalData[" << tmpBin << "] to " << fMatrices[0]->GetXaxis()->GetBinLabel(bin) << " = " << fFinalData[tmpBin]->GetName() << std::endl; }
      fFinalEfficiency[tmpBin] = new TCalibrationGraphSet;
      fFinalEfficiency[tmpBin]->VerboseLevel(fVerboseLevel - 4);
      fFinalEfficiency[tmpBin]->SetName(Form("eff%s", fMatrices[0]->GetXaxis()->GetBinLabel(bin)));
      if(fVerboseLevel > 2) {
         std::cout << "set name of fFinalEfficiency[" << tmpBin << "] to " << fMatrices[0]->GetXaxis()->GetBinLabel(bin) << " = " << fFinalEfficiency[tmpBin]->GetName() << std::endl;
         std::cout << "fFinalData[" << tmpBin << "] " << fFinalData[tmpBin] << ": " << (fFinalData[tmpBin] != nullptr ? fFinalData[tmpBin]->GetN() : -1) << " data points after creation" << std::endl;
      }
      for(size_t source = 0; source < fSource.size(); ++source) {
         if(fData[source][tmpBin]->GetN() > 0) {
            int index = fFinalData[tmpBin]->Add(fData[source][tmpBin], fSource[source]->GetName());
            if(index >= 0) {
               fFinalData[tmpBin]->SetLineColor(index, static_cast<Color_t>(source + 1));   //+1 for the color so that we start with 1 = black instead of 0 = white
               fFinalData[tmpBin]->SetMarkerColor(index, static_cast<Color_t>(source + 1));
            }
         }
         if(fEfficiency[source][tmpBin]->GetN() > 0) {
            int index = fFinalEfficiency[tmpBin]->Add(fEfficiency[source][tmpBin], fSource[source]->GetName());
            if(index >= 0) {
               fFinalEfficiency[tmpBin]->SetLineColor(index, static_cast<Color_t>(source + 1));   //+1 for the color so that we start with 1 = black instead of 0 = white
               fFinalEfficiency[tmpBin]->SetMarkerColor(index, static_cast<Color_t>(source + 1));
            }
         }
      }

      // calibration and residual graphs
      // create tab and status bar
      fFinalTab.push_back(fFinalTabs[0]->AddTab(Form("%s", fMatrices[0]->GetXaxis()->GetBinLabel(bin))));
      fFinalCanvas.push_back(new TRootEmbeddedCanvas("CalibrationCanvas", fFinalTab.back(), 600, 400));
      fFinalTab.back()->AddFrame(fFinalCanvas.back(), new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsExpandY | kLHintsExpandX, 2, 2, 2, 2));
      fStatusBar.push_back(new TGStatusBar(fFinalTab.back(), 600, 50));
      fStatusBar.back()->SetParts(parts.data(), parts.size());
      fFinalTab.back()->AddFrame(fStatusBar.back(), new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

      if(fVerboseLevel > 2) { std::cout << "fFinalData[" << tmpBin << "] " << fFinalData[tmpBin] << ": " << (fFinalData[tmpBin] != nullptr ? fFinalData[tmpBin]->GetN() : -1) << " data points after adding" << std::endl; }
      fActualSourceId[tmpBin] = tmpBin;
      // plot the graphs without their fit functions
      fFinalCanvas.back()->GetCanvas()->cd();
      fCalibrationPad[tmpBin] = new TPad(Form("cal_%s", fChannelLabel[tmpBin]), Form("calibration for %s", fChannelLabel[tmpBin]), 0.2, 0., 1., 1.);
      fCalibrationPad[tmpBin]->SetNumber(1);
      fCalibrationPad[tmpBin]->Draw();
      fCalibrationPad[tmpBin]->cd();
      fCalibrationPad[tmpBin]->AddExec("zoom", "TSourceCalibration::ZoomY()");
      //fCalibrationPad[tmpBin]->GetListOfExecs()->Print();
      fLegend[tmpBin] = new TLegend(0.8, 0.3, 0.95, 0.3 + static_cast<double>(fMatrices.size()) * 0.05);   // x1, y1, x2, y2
      //fFinalData[tmpBin]->DrawCalibration("*", fLegend[tmpBin]);
      //fLegend[tmpBin]->Draw();
      fFinalCanvas.back()->GetCanvas()->cd();
      fResidualPad[tmpBin] = new TPad(Form("res_%s", fChannelLabel[tmpBin]), Form("residual for %s", fChannelLabel[tmpBin]), 0.0, 0., 0.2, 1.);
      fResidualPad[tmpBin]->SetNumber(2);
      fResidualPad[tmpBin]->Draw();
      //fResidualPad[tmpBin]->cd();
      fResidualPad[tmpBin]->AddExec("zoom", "TSourceCalibration::ZoomY()");
      //fResidualPad[tmpBin]->GetListOfExecs()->Print();
      //fFinalData[tmpBin]->DrawResidual("*");
      FitFinal(tmpBin);   // also creates the residual and chi^2 label
      //fChi2Label[tmpBin]->Draw();

      // efficiency and residual graphs
      // create tab and status bar
      if(fVerboseLevel > 2) { std::cout << __PRETTY_FUNCTION__ << ": " << tmpBin << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      fEfficiencyTabs.push_back(fFinalTabs[1]->AddTab(Form("%s", fMatrices[0]->GetXaxis()->GetBinLabel(bin))));
      fEfficiencyCanvas.push_back(new TRootEmbeddedCanvas("EfficiencyCanvas", fEfficiencyTabs.back(), 600, 400));
      fEfficiencyTabs.back()->AddFrame(fEfficiencyCanvas.back(), new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsExpandY | kLHintsExpandX, 2, 2, 2, 2));
      fEfficiencyStatusBar.push_back(new TGStatusBar(fEfficiencyTabs.back(), 600, 50));
      fEfficiencyStatusBar.back()->SetParts(partsEff.data(), partsEff.size());
      fEfficiencyTabs.back()->AddFrame(fEfficiencyStatusBar.back(), new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

      // plot the graphs without their fit functions
      if(fVerboseLevel > 2) { std::cout << __PRETTY_FUNCTION__ << ": " << tmpBin << "/" << fEfficiencyPad.size() << "/" << fEfficiencyResidualPad.size() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      fEfficiencyCanvas.back()->GetCanvas()->cd();
      fEfficiencyPad[tmpBin] = new TPad(Form("eff_%s", fChannelLabel[tmpBin]), Form("efficiency for %s", fChannelLabel[tmpBin]), 0.2, 0., 1., 1.);
      fEfficiencyPad[tmpBin]->SetNumber(1);
      fEfficiencyPad[tmpBin]->Draw();
      fEfficiencyPad[tmpBin]->cd();
      fEfficiencyLegend[tmpBin] = new TLegend(0.8, 0.3, 0.95, 0.3 + static_cast<double>(fMatrices.size()) * 0.05);   // x1, y1, x2, y2
      FitEfficiency(tmpBin);                                                                    // also scales different source to first source, creates residual, and chi^2 label
      fFinalEfficiency[tmpBin]->DrawCalibration("*", fEfficiencyLegend[tmpBin]);
      fEfficiencyLegend[tmpBin]->Draw();
      fEfficiencyChi2Label[tmpBin]->Draw();
      fEfficiencyCanvas.back()->GetCanvas()->cd();
      fEfficiencyResidualPad[tmpBin] = new TPad(Form("eff_res_%s", fChannelLabel[tmpBin]), Form("residual for %s", fChannelLabel[tmpBin]), 0.0, 0., 0.2, 1.);
      fEfficiencyResidualPad[tmpBin]->SetNumber(2);
      fEfficiencyResidualPad[tmpBin]->Draw();
      fEfficiencyResidualPad[tmpBin]->cd();
      fFinalEfficiency[tmpBin]->DrawResidual("*");

      // write graphs to output file
      fOutput->cd();
      if(fVerboseLevel > 2) { std::cout << "writing " << tmpBin << std::endl; }
      fFinalData[tmpBin]->Write(Form("calGraph_%s", fChannelLabel[tmpBin]), TObject::kOverwrite);
      if(fVerboseLevel > 2) { std::cout << "wrote data " << tmpBin << std::endl; }
      fFinalEfficiency[tmpBin]->Write(Form("eff_%s", fChannelLabel[tmpBin]), TObject::kOverwrite);
      if(fVerboseLevel > 2) { std::cout << "wrote efficiency " << tmpBin << std::endl; }
      ++tmpBin;
   }
   if(fVerboseLevel > 2) { std::cout << "# of tabs after adding: " << fTab->GetNumberOfTabs() << std::endl; }

   calTab->AddFrame(fFinalTabs[0], new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 0, 0, 0, 0));
   effTab->AddFrame(fFinalTabs[1], new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 0, 0, 0, 0));
   AddFrame(fTab, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 0, 0, 0, 0));

   // bottom frame with navigation button group, text entries, etc.
   fBottomFrame = new TGHorizontalFrame(this, 600, 50);

   fLeftFrame       = new TGVerticalFrame(fBottomFrame, 300, 50);
   fNavigationGroup = new TGHButtonGroup(fLeftFrame, "");
   fPreviousButton  = new TGTextButton(fNavigationGroup, "Previous Channel");
   if(fVerboseLevel > 2) { std::cout << "prev button 2 " << fPreviousButton << std::endl; }
   fPreviousButton->SetEnabled(false);
   fCalibrateButton = new TGTextButton(fNavigationGroup, "Calibrate Channel");
   if(fVerboseLevel > 2) { std::cout << "cal button 2 " << fCalibrateButton << std::endl; }
   fAcceptButton = new TGTextButton(fNavigationGroup, "Accept Channel");
   if(fVerboseLevel > 2) { std::cout << "accept button 2 " << fAcceptButton << std::endl; }
   fAcceptAllButton = new TGTextButton(fNavigationGroup, "Accept All && Finish");
   if(fVerboseLevel > 2) { std::cout << "accept all button 2 " << fAcceptAllButton << std::endl; }
   fNextButton = new TGTextButton(fNavigationGroup, "Next Channel");
   if(fVerboseLevel > 2) { std::cout << "next button 2 " << fNextButton << std::endl; }

   fLeftFrame->AddFrame(fNavigationGroup, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

   fBottomFrame->AddFrame(fLeftFrame, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 2, 2, 2, 2));

   // new right frame (only degree option left, no sigma or threshold)
   fRightFrame  = new TGVerticalFrame(fBottomFrame, 300, 50);
   fDegreeLabel = new TGLabel(fRightFrame, "Degree of polynomial");
   fDegreeEntry = new TGNumberEntry(fRightFrame, fDefaultDegree, 2, kDegreeEntry, TGNumberFormat::EStyle::kNESInteger, TGNumberFormat::EAttribute::kNEAPositive);
   fRightFrame->AddFrame(fDegreeEntry);
   fRightFrame->AddFrame(fDegreeLabel);

   fBottomFrame->AddFrame(fRightFrame, new TGLayoutHints(kLHintsBottom | kLHintsExpandY, 2, 2, 2, 2));

   AddFrame(fBottomFrame, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));
}

void TSourceCalibration::MakeThirdConnections()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fTab->Connect("Selected(Int_t)", "TSourceCalibration", this, "SelectedFinalMainTab(Int_t)");
   fFinalTabs[0]->Connect("Selected(Int_t)", "TSourceCalibration", this, "SelectedFinalTab(Int_t)");
   fFinalTabs[1]->Connect("Selected(Int_t)", "TSourceCalibration", this, "SelectedFinalTab(Int_t)");
   for(auto* canvas : fFinalCanvas) {
      canvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TSourceCalibration", this, "CalibrationStatus(Int_t,Int_t,Int_t,TObject*)");
   }
   fNavigationGroup->Connect("Clicked(Int_t)", "TSourceCalibration", this, "NavigateFinal(Int_t)");
}

void TSourceCalibration::DisconnectThird()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   TGTab::Disconnect("Selected(Int_t)", "TSourceCalibration", this, "SelectedFinalMainTab(Int_t)");
   fFinalTabs[0]->Disconnect("Selected(Int_t)", this, "SelectedFinalTab(Int_t)");
   fFinalTabs[1]->Disconnect("Selected(Int_t)", this, "SelectedFinalTab(Int_t)");
   for(auto* canvas : fFinalCanvas) {
      canvas->GetCanvas()->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this, "CalibrationStatus(Int_t,Int_t,Int_t,TObject*)");
   }
   fNavigationGroup->Disconnect("Clicked(Int_t)", this, "NavigateFinal(Int_t)");
}

void TSourceCalibration::CalibrationStatus(Int_t, Int_t px, Int_t py, TObject* selected)
{
   if(fVerboseLevel > 3) { std::cout << __PRETTY_FUNCTION__ << ": px " << px << ", py " << py << ", object " << selected->GetName() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fStatusBar[fTab->GetCurrent()]->SetText(selected->GetName(), 0);
   fStatusBar[fTab->GetCurrent()]->SetText(selected->GetObjectInfo(px, py), 2);
}

void TSourceCalibration::NavigateFinal(Int_t id)
{
   // we get the current source tab id and use it to get the channel tab from the right source tab
   // since the current id only refers to the position within the open tabs we need to keep track of the actual id it relates to
   // for this we created a vector with all ids at the beginning and now remove the position correspoding to the current id when we remove the tab

   int currentChannelId = fFinalTabs[fTab->GetCurrent()]->GetCurrent();
   int actualChannelId  = fActualSourceId[currentChannelId];
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": id " << id << ", channel tab id " << currentChannelId << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   switch(id) {
   case 1:   // previous
      fFinalTabs[fTab->GetCurrent()]->SetTab(currentChannelId - 1);
      SelectedFinalTab(currentChannelId - 1);
      break;
   case 2:   // calibrate
      FitFinal(actualChannelId);
      break;
   case 3:   // accept
      AcceptFinalChannel(currentChannelId);
      break;
   case 4:   // accept all (no argument = -1 = all)
      AcceptFinalChannel();
      break;
   case 5:   // next
      fFinalTabs[fTab->GetCurrent()]->SetTab(currentChannelId + 1);
      SelectedFinalTab(currentChannelId + 1);
      break;
   default:
      break;
   }
}

void TSourceCalibration::SelectedFinalTab(Int_t id)
{
   /// Simple function that enables and disables the previous and next buttons depending on which tab was selected
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": id " << id << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(id == 0) {
      fPreviousButton->SetEnabled(false);
   } else {
      fPreviousButton->SetEnabled(true);
   }

   if(id == fFinalTabs[fTab->GetCurrent()]->GetNumberOfTabs() - 1) {
      fNextButton->SetEnabled(false);
   } else {
      fNextButton->SetEnabled(true);
   }
}

void TSourceCalibration::SelectedFinalMainTab(Int_t)
{
   /// Simple function that enables and disables the previous and next buttons depending on which tab was selected
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": main current " << fTab->GetCurrent() << " current " << fFinalTabs[fTab->GetCurrent()]->GetCurrent() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fFinalTabs[fTab->GetCurrent()]->GetCurrent() == 0) {
      fPreviousButton->SetEnabled(false);
   } else {
      fPreviousButton->SetEnabled(true);
   }

   if(fFinalTabs[fTab->GetCurrent()]->GetCurrent() == fFinalTabs[fTab->GetCurrent()]->GetNumberOfTabs() - 1) {
      fNextButton->SetEnabled(false);
   } else {
      fNextButton->SetEnabled(true);
   }
}

void TSourceCalibration::AcceptFinalChannel(const int& channelId)
{
   // select the next (or if we are on the last tab, the previous) tab
   int nofTabs    = fFinalTabs[0]->GetNumberOfTabs();
   int minChannel = 0;
   int maxChannel = nofTabs - 1;
   if(channelId >= 0) {
      minChannel = channelId;
      maxChannel = channelId;
   }
   // we need to loop backward, because removing the first channel would make the second one the new first and so on
   for(int currentChannelId = maxChannel; currentChannelId >= minChannel; --currentChannelId) {
      int actualChannelId = fActualSourceId[currentChannelId];
      UpdateChannel(actualChannelId);
      if(currentChannelId < nofTabs - 1) {
         fTab->SetTab(currentChannelId + 1);
      } else {
         fTab->SetTab(currentChannelId - 1);
      }
      // remove the original active tab
      fFinalTabs[0]->RemoveTab(currentChannelId);
      fFinalTabs[1]->RemoveTab(currentChannelId);
      //fTab->Layout();
      fActualSourceId.erase(fActualSourceId.begin() + currentChannelId);
      if(fVerboseLevel > 2) { std::cout << "Erased " << currentChannelId << ", fActualSourceId.size() " << fActualSourceId.size() << std::endl; }
   }
   // if this was the last tab, we're done
   if(fActualSourceId.empty()) {
      WriteCalibration();
      CloseWindow();
      std::cout << "Done" << std::endl;
      exit(0);
   }
}

void TSourceCalibration::FitFinal(const int& channelId)
{
   /// This function fit's the final data of the given channel. It requires all other elements to have been created already.
   TF1* calibration = new TF1("fitfunction", ::Polynomial, 0., 10000., Degree() + 2);
   calibration->FixParameter(0, Degree());
   if(fVerboseLevel > 1) { std::cout << "fFinalData[" << channelId << "] " << fFinalData[channelId] << ": " << (fFinalData[channelId] != nullptr ? fFinalData[channelId]->GetN() : -1) << " data points being fit" << std::endl; }
   fFinalData[channelId]->Fit(calibration, "Q");
   TString text = Form("%.6f + %.6f*x", calibration->GetParameter(1), calibration->GetParameter(2));
   for(int i = 2; i <= Degree(); ++i) {
      text.Append(Form(" + %.6f*x^%d", calibration->GetParameter(i + 1), i));
   }
   fStatusBar[channelId]->SetText(text.Data(), 1);
   if(fVerboseLevel > 2) { std::cout << "re-calculating residuals and clearing fields" << std::endl; }
   // re-calculate the residuals
   fFinalData[channelId]->SetResidual(true);

   fLegend[channelId]->Clear();
   fCalibrationPad[channelId]->cd();
   fFinalData[channelId]->DrawCalibration("*", fLegend[channelId]);
   fLegend[channelId]->Draw();
   if(fVerboseLevel > 2) { std::cout << "set chi2 label" << std::endl; }
   // calculate the corners of the chi^2 label from the minimum and maximum x/y-values of the graph
   // we position it in the top left corner about 50% of the width and 10% of the height of the graph
   double left           = fFinalData[channelId]->GetMinimumX();
   double right          = left + (fFinalData[channelId]->GetMaximumX() - left) * 0.5;
   double top            = fFinalData[channelId]->GetMaximumY();
   double bottom         = top - (top - fFinalData[channelId]->GetMinimumY()) * 0.1;
   fChi2Label[channelId] = new TPaveText(left, bottom, right, top);
   if(fVerboseLevel > 2) {
      std::cout << "fChi2Label[" << channelId << "] created " << fChi2Label[channelId] << " (" << left << " - " << right << ", " << bottom << " - " << top << ", from " << fFinalData[channelId]->GetMinimumX() << "-" << fFinalData[channelId]->GetMaximumX() << ", " << fFinalData[channelId]->GetMinimumY() << "-" << fFinalData[channelId]->GetMaximumY() << ") on gPad " << gPad->GetName() << std::endl;
      fFinalData[channelId]->Print("e");
   }
   fChi2Label[channelId]->Clear();
   fChi2Label[channelId]->AddText(Form("#chi^{2}/NDF = %f", calibration->GetChisquare() / calibration->GetNDF()));
   fChi2Label[channelId]->SetFillColor(10);
   fChi2Label[channelId]->Draw();
   if(fVerboseLevel > 2) {
      std::cout << "fChi2Label[" << channelId << "] set to:" << std::endl;
      fChi2Label[channelId]->GetListOfLines()->Print();
      std::cout << "Text size " << fChi2Label[channelId]->GetTextSize() << std::endl;
   }

   fResidualPad[channelId]->cd();
   fFinalData[channelId]->DrawResidual("*");

   fFinalCanvas[channelId]->GetCanvas()->Modified();

   delete calibration;

   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << " done" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

void TSourceCalibration::FitEfficiency(const int& channelId)
{
   // start with scaling all graphs to each other
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": " << channelId << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fFinalEfficiency[channelId]->Scale();
   auto* efficiency = new TF1("fitfunction", ::Efficiency, 0., 10000., 9);
   if(fVerboseLevel > 2) { std::cout << "fFinalEfficiency[" << channelId << "] " << fFinalEfficiency[channelId] << ": " << (fFinalEfficiency[channelId] != nullptr ? fFinalEfficiency[channelId]->GetN() : 0) << " data points" << std::endl; }
   fFinalEfficiency[channelId]->Fit(efficiency, "Q");
   //TString text = Form("%.6f + %.6f*x", calibration->GetParameter(1), calibration->GetParameter(2));
   //for(int i = 2; i <= Degree(); ++i) {
   //	text.Append(Form(" + %.6f*x^%d", calibration->GetParameter(i+1), i));
   //}
   //fStatusBar[channelId]->SetText(text.Data(), 1);
   // re-calculate the residuals
   fFinalEfficiency[channelId]->SetResidual(true);

   //fLegend[channelId]->Clear();
   //fCalibrationPad[channelId]->cd();
   //fFinalData[channelId]->DrawCalibration("*", fLegend[channelId]);
   //fLegend[channelId]->Draw();
   // calculate the corners of the chi^2 label from the minimum and maximum x/y-values of the graph
   // we position it in the top right corner about 50% of the width and 10% of the height of the graph
   double right = fFinalEfficiency[channelId]->GetMaximumX();
   double left  = right - (right - fFinalEfficiency[channelId]->GetMinimumX()) * 0.5;
   // y is switched around because the first point is actually the highest (typically???)
   // and the last point is the lowest
   double top                      = fFinalEfficiency[channelId]->GetMinimumY();
   double bottom                   = top - (top - fFinalEfficiency[channelId]->GetMaximumY()) * 0.1;
   fEfficiencyChi2Label[channelId] = new TPaveText(left, bottom, right, top);
   if(fVerboseLevel > 2) {
      std::cout << "fEfficiencyChi2Label[" << channelId << "] created " << fChi2Label[channelId] << " (" << left << " - " << right << ", " << bottom << " - " << top << ", from " << fFinalEfficiency[channelId]->GetMinimumX() << "-" << fFinalEfficiency[channelId]->GetMaximumX() << ", " << fFinalEfficiency[channelId]->GetMinimumY() << "-" << fFinalEfficiency[channelId]->GetMaximumY() << ")" << std::endl;
      fFinalEfficiency[channelId]->Print();
   }
   fEfficiencyChi2Label[channelId]->Clear();
   TText* chi2 = fEfficiencyChi2Label[channelId]->AddText(Form("#chi^{2}/NDF = %f", efficiency->GetChisquare() / efficiency->GetNDF()));
   chi2->SetTextSize(12);
   fEfficiencyChi2Label[channelId]->SetFillColor(10);
   fEfficiencyChi2Label[channelId]->Draw();
   if(fVerboseLevel > 2) {
      std::cout << "fEfficiencyChi2Label[" << channelId << "] set to " << fEfficiencyChi2Label[channelId]->GetLabel() << std::endl;
      fEfficiencyChi2Label[channelId]->GetListOfLines()->Print();
   }

   delete efficiency;
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << " done" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

void TSourceCalibration::UpdateChannel(const int& channelId)
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": channelId " << channelId << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   // write the actual parameters of the fit
   std::stringstream str;
   str << std::hex << fChannelLabel[channelId];
   int address = 0;
   str >> address;
   if(fVerboseLevel > 2) { std::cout << "Got address " << hex(address, 4) << " from label " << fChannelLabel[channelId] << std::endl; }
   TF1* calibration = fFinalData[channelId]->FitFunction();
   if(calibration == nullptr) {
      std::cout << "Failed to find calibration in fFinalData[" << channelId << "]" << std::endl;
      fFinalData[channelId]->TotalGraph()->GetListOfFunctions()->Print();
      return;
   }
   std::vector<Float_t> parameters;
   for(int i = 0; i <= calibration->GetParameter(0); ++i) {
      parameters.push_back(static_cast<Float_t>(calibration->GetParameter(i + 1)));
   }
   TChannel* channel = TChannel::GetChannel(address, false);
   if(channel == nullptr) {
      std::cerr << "Failed to get channel for address " << hex(address, 4) << std::endl;
      //TChannel::WriteCalFile();
      if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": done" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      return;
   }
   channel->SetENGCoefficients(parameters);
   channel->DestroyEnergyNonlinearity();
   double* x = fFinalData[channelId]->GetX();
   double* y = fFinalData[channelId]->GetY();
   if(fVerboseLevel > 2) { std::cout << "Going to add " << fFinalData[channelId]->GetN() << " points to nonlinearity graph" << std::endl; }
   for(int i = 0; i < fFinalData[channelId]->GetN(); ++i) {
      // nonlinearity expects y to be the true source energy minus the calibrated energy of the peak
      // the source energy is y, the peak is x
      channel->AddEnergyNonlinearityPoint(y[i], y[i] - calibration->Eval(x[i]));
      if(fVerboseLevel > 3) { std::cout << "Added " << channel->GetEnergyNonlinearity().GetN() << ". point " << y[i] << ", " << y[i] - calibration->Eval(x[i]) << " = " << y[i] << " - " << calibration->Eval(x[i]) << std::endl; }
   }
}

void TSourceCalibration::WriteCalibration()
{
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   std::stringstream str;
   for(auto* source : fSource) {
      str << source->GetName();
   }
   str << ".cal";
   TChannel::WriteCalFile(str.str());
   if(fVerboseLevel > 1) { std::cout << __PRETTY_FUNCTION__ << ": wrote " << str.str() << " with " << TChannel::GetNumberOfChannels() << " channels" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fVerboseLevel > 2) { TChannel::WriteCalFile(); }
}

void TSourceCalibration::ZoomX()
{
   /// finds corresponding graph (res_xxx for cal_xxx and vice versa) and sets it's x-axis range to be the same as the selected graphs
   // find the histogram in the current pad
   TH1* hist1 = nullptr;
   for(const auto&& obj : *(gPad->GetListOfPrimitives())) {
      if(obj->InheritsFrom(TGraph::Class())) {
         hist1 = static_cast<TGraph*>(obj)->GetHistogram();
         break;
      }
   }
   if(hist1 == nullptr) {
      std::cout << "Failed to find histogram for pad " << gPad->GetName() << std::endl;
      return;
   }

   // extract base name and channel from pad name
   std::string padName     = gPad->GetName();
   std::string baseName    = padName.substr(0, 3);
   std::string channelName = padName.substr(4);

   // find the corresponding partner pad to the selected one
   std::string newName;
   if(baseName == "cal") {
      newName = "res_" + channelName;
   } else if(baseName == "res") {
      newName = "cal_" + channelName;
   } else {
      std::cout << "Unknown combination of pad " << gPad->GetName() << " and histogram " << hist1->GetName() << std::endl;
      return;
   }
   auto* newObj = gPad->GetCanvas()->FindObject(newName.c_str());
   if(newObj == nullptr) {
      std::cout << "Failed to find " << newName << std::endl;
      return;
   }

   if(newObj->IsA() != TPad::Class()) {
      std::cout << newObj << " = " << newObj->GetName() << ", " << newObj->GetTitle() << " is a " << newObj->ClassName() << " not a TPad" << std::endl;
      return;
   }
   auto* pad2 = static_cast<TPad*>(newObj);
   if(pad2 == nullptr) {
      std::cout << "Failed to find partner pad " << newName << std::endl;
      return;
   }
   // find the histogram in the partner pad
   TH1* hist2 = nullptr;
   for(const auto&& obj : *(pad2->GetListOfPrimitives())) {
      if(obj->InheritsFrom(TGraph::Class())) {
         hist2 = static_cast<TGraph*>(obj)->GetHistogram();
         break;
      }
   }
   if(hist2 == nullptr) {
      std::cout << "Failed to find histogram for pad " << newName << std::endl;
      return;
   }

   TAxis*   axis1  = hist1->GetXaxis();
   Int_t    binmin = axis1->GetFirst();
   Int_t    binmax = axis1->GetLast();
   Double_t xmin   = axis1->GetBinLowEdge(binmin);
   Double_t xmax   = axis1->GetBinLowEdge(binmax);
   TAxis*   axis2  = hist2->GetXaxis();
   Int_t    newmin = axis2->FindBin(xmin);
   Int_t    newmax = axis2->FindBin(xmax);
   axis2->SetRange(newmin, newmax);
   pad2->Modified();
   pad2->Update();
}

void TSourceCalibration::ZoomY()
{
   /// finds corresponding graph (res_xxx for cal_xxx and vice versa) and sets it's y-axis range to be the same as the selected graphs
   // find the histogram in the current pad
   TH1* hist1 = nullptr;
   for(const auto&& obj : *(gPad->GetListOfPrimitives())) {
      if(obj->InheritsFrom(TGraph::Class())) {
         hist1 = static_cast<TGraph*>(obj)->GetHistogram();
         break;
      }
   }
   if(hist1 == nullptr) {
      std::cout << "Failed to find histogram for pad " << gPad->GetName() << std::endl;
      return;
   }

   // extract base name and channel from pad name
   std::string padName     = gPad->GetName();
   std::string baseName    = padName.substr(0, 3);
   std::string channelName = padName.substr(4);

   // find the corresponding partner pad to the selected one
   std::string newName;
   if(baseName == "cal") {
      newName = "res_" + channelName;
   } else if(baseName == "res") {
      newName = "cal_" + channelName;
   } else {
      std::cout << "Unknown combination of pad " << gPad->GetName() << " and histogram " << hist1->GetName() << std::endl;
      return;
   }
   auto* newObj = gPad->GetCanvas()->FindObject(newName.c_str());
   if(newObj == nullptr) {
      std::cout << "Failed to find " << newName << std::endl;
      return;
   }

   if(newObj->IsA() != TPad::Class()) {
      std::cout << newObj << " = " << newObj->GetName() << ", " << newObj->GetTitle() << " is a " << newObj->ClassName() << " not a TPad" << std::endl;
      return;
   }
   auto* pad2 = static_cast<TPad*>(newObj);
   if(pad2 == nullptr) {
      std::cout << "Failed to find partner pad " << newName << std::endl;
      return;
   }
   // find the histogram in the partner pad
   TH1* hist2 = nullptr;
   for(const auto&& obj : *(pad2->GetListOfPrimitives())) {
      if(obj->InheritsFrom(TGraph::Class())) {
         hist2 = static_cast<TGraph*>(obj)->GetHistogram();
         break;
      }
   }
   if(hist2 == nullptr) {
      std::cout << "Failed to find histogram for pad " << newName << std::endl;
      return;
   }

   hist2->SetMinimum(hist1->GetMinimum());
   hist2->SetMaximum(hist1->GetMaximum());

   pad2->Modified();
   pad2->Update();
}
