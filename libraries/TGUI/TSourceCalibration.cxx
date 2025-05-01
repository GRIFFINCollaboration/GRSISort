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
#include <thread>

#include "TSystem.h"
#include "TGTableLayout.h"
#include "TCanvas.h"
#include "TLinearFitter.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TPolyMarker.h"
#include "TObject.h"
#include "TFrame.h"
#include "TVirtualX.h"

#include "TChannel.h"
#include "GRootCommands.h"
#include "combinations.h"
#include "Globals.h"

std::map<double, std::tuple<double, double, double, double>> RoughCal(std::vector<double> peaks, std::vector<std::tuple<double, double, double, double>> sources, TSourceTab* sourceTab)
{
   /// This function tries to match a list of found peaks (channels) to a list of provided peaks (energies).
   /// It does so in slightly smarter way than the brute force method `Match`, by taking the reported intensity of the source peaks into account.

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << "\"Smart\" matching " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl; }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
      for(size_t i = 0; i < peaks.size() || i < sources.size(); ++i) {
         std::cout << i << ".: " << std::setw(8);
         if(i < peaks.size()) {
            std::cout << peaks[i];
         } else {
            std::cout << " ";
         }
         std::cout << " - " << std::setw(8);
         if(i < sources.size()) {
            std::cout << std::get<0>(sources[i]);
         } else {
            std::cout << " ";
         }
         std::cout << std::endl;
      }
   }

   std::map<double, std::tuple<double, double, double, double>> result;
   std::sort(peaks.begin(), peaks.end());
   std::sort(sources.begin(), sources.end(), [](const std::tuple<double, double, double, double>& a, const std::tuple<double, double, double, double>& b) { return std::get<2>(a) > std::get<2>(b); });

   auto maxSize = peaks.size();
   if(sources.size() > maxSize) { maxSize = sources.size(); }

   // Peaks are the fitted points.
   // source are the known values

   TLinearFitter fitter(1, "1 ++ x");

   // intermediate vectors and map
   std::vector<double>      sourceValues(sources.size());
   std::map<double, double> tmpMap;

   for(size_t num_data_points = std::min(peaks.size(), sourceValues.size()); num_data_points > 0; num_data_points--) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << num_data_points << " data points:" << std::endl; }
      double best_chi2 = DBL_MAX;
      int    iteration = 0;
      for(auto peak_values : combinations(peaks, num_data_points)) {
         // Add a (0,0) point to the calibration.
         peak_values.push_back(0);
         // instead of going through all possible combinations of the peaks with the source energies
         // we pick the num_data_points most intense lines and try them
         // we don't do the same with the peaks as there might be an intense background peak in the data (511 etc.)
         sourceValues.resize(num_data_points);
         for(size_t i = 0; i < sourceValues.size(); ++i) { sourceValues[i] = std::get<0>(sources[i]); }
         std::sort(sourceValues.begin(), sourceValues.end());
         sourceValues.push_back(0);

         if(TSourceCalibration::VerboseLevel() > EVerbosity::kAll) {
            for(size_t i = 0; i < peak_values.size(); ++i) {
               std::cout << i << ".: " << std::setw(8) << peak_values[i] << " - " << std::setw(8) << sourceValues[i] << std::endl;
            }
         }
         if(peaks.size() > 3) {
            double pratio = peak_values.front() / peak_values.at(peak_values.size() - 2);
            double sratio = sourceValues.front() / sourceValues.at(sourceValues.size() - 2);
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kAll) { std::cout << "ratio: " << pratio << " - " << sratio << " = " << std::abs(pratio - sratio) << std::endl; }
            if(std::abs(pratio - sratio) > 0.02) {
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kAll) { std::cout << "skipping" << std::endl; }
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
         sourceTab->Status(Form("%zu/%zu - %zu - %d", num_data_points, peaks.size(), maxSize, iteration), 1);
         ++iteration;
         if(iteration >= TSourceCalibration::MaxIterations()) { break; }
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
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
                  std::cout << fitter.GetParameter(0) << " too big an offset, clearing map with " << tmpMap.size() << " points: ";
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
            result[*(std::find_if(peaks.begin(), peaks.end(), [&iter](auto& item) { return iter.first == item; }))] =
               *(std::find_if(sources.begin(), sources.end(), [&iter](auto& item) { return iter.second == std::get<0>(item); }));
         }
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "Smart matched " << num_data_points << " data points from " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl;
            std::cout << "Returning map with " << result.size() << " points: ";
            for(auto iter : result) { std::cout << iter.first << " - " << std::get<0>(iter.second) << "; "; }
            std::cout << std::endl;
         }
         break;
      }
      sourceTab->Status(Form("%zu/%zu - %zu", num_data_points, peaks.size(), maxSize), 1);
   }

   return result;
}

std::map<TGauss*, std::tuple<double, double, double, double>> Match(std::vector<TGauss*> peaks, std::vector<std::tuple<double, double, double, double>> sources, TSourceTab* sourceTab)
{
   /// This function tries to match a list of found peaks (channels) to a list of provided peaks (energies).
   /// It does so in a brute force fashion where we try all combinations of channels and energies, do a linear fit through them, and keep the one with the best chi square.

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << "Matching " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl; }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "peaks:";
      for(auto* peak : peaks) {
         std::cout << " " << peak->Centroid();
      }
      std::cout << std::endl;
      std::cout << "energies:";
      for(auto source : sources) {
         std::cout << " " << std::get<0>(source);
      }
      std::cout << std::endl;
   }

   std::map<TGauss*, std::tuple<double, double, double, double>> result;
   std::sort(peaks.begin(), peaks.end(), [](const TGauss* a, const TGauss* b) { return a->Centroid() < b->Centroid(); });
   std::sort(sources.begin(), sources.end());

   auto maxSize = peaks.size();
   if(sources.size() > maxSize) { maxSize = sources.size(); }

   // Peaks are the fitted points.
   // source are the known values

   TLinearFitter fitter(1, "1 ++ x");

   // intermediate vectors and map
   std::vector<double> peakValues(peaks.size());
   for(size_t i = 0; i < peaks.size(); ++i) { peakValues[i] = peaks[i]->Centroid(); }
   std::vector<double> sourceValues(sources.size());
   for(size_t i = 0; i < sources.size(); ++i) { sourceValues[i] = std::get<0>(sources[i]); }
   std::map<double, double> tmpMap;

   for(size_t num_data_points = peakValues.size(); num_data_points > 0; num_data_points--) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << num_data_points << " data points:" << std::endl; }
      double best_chi2 = DBL_MAX;
      int    iteration = 0;
      for(auto peak_values : combinations(peakValues, num_data_points)) {
         // Add a (0,0) point to the calibration.
         peak_values.push_back(0);
         for(auto source_values : combinations(sourceValues, num_data_points)) {
            source_values.push_back(0);

            if(peakValues.size() > 3) {
               double pratio = peak_values.front() / peak_values.at(peak_values.size() - 2);
               double sratio = source_values.front() / source_values.at(source_values.size() - 2);
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kAll) { std::cout << "ratio: " << pratio << " - " << sratio << " = " << std::abs(pratio - sratio) << " "; }
               if(std::abs(pratio - sratio) > 0.02) {
                  if(TSourceCalibration::VerboseLevel() > EVerbosity::kAll) { std::cout << "skipping" << std::endl; }
                  continue;
               }
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kAll) { std::cout << std::endl; }
            }

            fitter.ClearPoints();
            fitter.AssignData(source_values.size(), 1, peak_values.data(), source_values.data());
            fitter.Eval();

            if(fitter.GetChisquare() < best_chi2) {
               tmpMap.clear();
               for(size_t i = 0; i < num_data_points; i++) {
                  tmpMap[peak_values[i]] = source_values[i];
               }
               if(TSourceCalibration::VerboseLevel() == EVerbosity::kAll) {
                  std::cout << "Chi^2 " << fitter.GetChisquare() << " better than " << best_chi2 << ":" << std::endl;
                  for(auto iter : tmpMap) {
                     std::cout << std::setw(10) << iter.first << " - " << iter.second << std::endl;
                  }
               }
               best_chi2 = fitter.GetChisquare();
            }
         }
         sourceTab->Status(Form("%zu/%zu - %zu - %d", num_data_points, peakValues.size(), maxSize, iteration), 1);
         ++iteration;
         if(iteration >= TSourceCalibration::MaxIterations()) { break; }
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
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
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
            result[*(std::find_if(peaks.begin(), peaks.end(), [&iter](auto& item) { return iter.first == item->Centroid(); }))] =
               *(std::find_if(sources.begin(), sources.end(), [&iter](auto& item) { return iter.second == std::get<0>(item); }));
            // apparently c++14 is needed to use auto in a lambda so for now we spell it out
            //result[*(std::find_if(peaks.begin(),   peaks.end(),   [&iter](std::tuple<double, double, double, double>& item) { return iter.first  == std::get<0>(item); }))] =
            //       *(std::find_if(sources.begin(), sources.end(), [&iter](std::tuple<double, double, double, double>& item) { return iter.second == std::get<0>(item); }));
         }
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "Matched " << num_data_points << " data points from " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl;
            std::cout << "Returning map with " << result.size() << " points: ";
            for(auto iter : result) { std::cout << iter.first->Centroid() << " - " << std::get<0>(iter.second) << "; "; }
            std::cout << std::endl;
         }
         break;
      }
      sourceTab->Status(Form("%zu/%zu - %zu", num_data_points, peakValues.size(), maxSize), 1);
   }

   return result;
}

std::map<TGauss*, std::tuple<double, double, double, double>> SmartMatch(std::vector<TGauss*> peaks, std::vector<std::tuple<double, double, double, double>> sources, TSourceTab* sourceTab)
{
   /// This function tries to match a list of found peaks (channels) to a list of provided peaks (energies).
   /// It does so in slightly smarter way than the brute force method `Match`, by taking the reported intensity of the source peaks into account.

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << "\"Smart\" matching " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl; }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
      for(size_t i = 0; i < peaks.size() || i < sources.size(); ++i) {
         std::cout << i << ".: " << std::setw(8);
         if(i < peaks.size()) {
            std::cout << peaks[i]->Centroid();
         } else {
            std::cout << " ";
         }
         std::cout << " - " << std::setw(8);
         if(i < sources.size()) {
            std::cout << std::get<0>(sources[i]);
         } else {
            std::cout << " ";
         }
         std::cout << std::endl;
      }
   }

   std::map<TGauss*, std::tuple<double, double, double, double>> result;
   std::sort(peaks.begin(), peaks.end(), [](const TGauss* a, const TGauss* b) { return a->Centroid() < b->Centroid(); });
   std::sort(sources.begin(), sources.end(), [](const std::tuple<double, double, double, double>& a, const std::tuple<double, double, double, double>& b) { return std::get<2>(a) > std::get<2>(b); });

   auto maxSize = peaks.size();
   if(sources.size() > maxSize) { maxSize = sources.size(); }

   // Peaks are the fitted points.
   // source are the known values

   TLinearFitter fitter(1, "1 ++ x");

   // intermediate vectors and map
   std::vector<double> peakValues(peaks.size());
   for(size_t i = 0; i < peaks.size(); ++i) { peakValues[i] = peaks[i]->Centroid(); }
   std::vector<double>      sourceValues(sources.size());
   std::map<double, double> tmpMap;

   for(size_t num_data_points = std::min(peakValues.size(), sourceValues.size()); num_data_points > 0; num_data_points--) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << num_data_points << " data points:" << std::endl; }
      double best_chi2 = DBL_MAX;
      int    iteration = 0;
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

         if(TSourceCalibration::VerboseLevel() > EVerbosity::kAll) {
            for(size_t i = 0; i < peak_values.size(); ++i) {
               std::cout << i << ".: " << std::setw(8) << peak_values[i] << " - " << std::setw(8) << sourceValues[i] << std::endl;
            }
         }
         if(peakValues.size() > 3) {
            double pratio = peak_values.front() / peak_values.at(peak_values.size() - 2);
            double sratio = sourceValues.front() / sourceValues.at(sourceValues.size() - 2);
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kAll) { std::cout << "ratio: " << pratio << " - " << sratio << " = " << std::abs(pratio - sratio) << std::endl; }
            if(std::abs(pratio - sratio) > 0.02) {
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kAll) { std::cout << "skipping" << std::endl; }
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
         sourceTab->Status(Form("%zu/%zu - %zu - %d", num_data_points, peakValues.size(), maxSize, iteration), 1);
         ++iteration;
         if(iteration >= TSourceCalibration::MaxIterations()) { break; }
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
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
                  std::cout << fitter.GetParameter(0) << " too big an offset, clearing map with " << tmpMap.size() << " points: ";
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
            result[*(std::find_if(peaks.begin(), peaks.end(), [&iter](auto& item) { return iter.first == item->Centroid(); }))] =
               *(std::find_if(sources.begin(), sources.end(), [&iter](auto& item) { return iter.second == std::get<0>(item); }));
         }
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "Smart matched " << num_data_points << " data points from " << peaks.size() << " peaks with " << sources.size() << " source energies" << std::endl;
            std::cout << "Returning map with " << result.size() << " points: ";
            for(auto iter : result) { std::cout << iter.first->Centroid() << " - " << std::get<0>(iter.second) << "; "; }
            std::cout << std::endl;
         }
         break;
      }
      sourceTab->Status(Form("%zu/%zu - %zu", num_data_points, peakValues.size(), maxSize), 1);
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

bool FilledBin(TH2* matrix, const int& bin)
{
   return (matrix->Integral(bin, bin, 1, matrix->GetNbinsY()) > 1000);
}

//////////////////////////////////////// TSourceTab ////////////////////////////////////////
TSourceTab::TSourceTab(TChannelTab* parent, TGCompositeFrame* frame, GH1D* projection, const char* sourceName, const double& sigma, const double& threshold, const double& peakRatio, std::vector<std::tuple<double, double, double, double>> sourceEnergy)
   : fParent(parent), fSourceFrame(frame), fProjection(projection), fSourceName(sourceName), fSigma(sigma), fThreshold(threshold), fPeakRatio(peakRatio), fSourceEnergy(std::move(sourceEnergy))
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << __PRETTY_FUNCTION__ << RESET_COLOR << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   BuildInterface();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << "interface built, finding peaks ... " << RESET_COLOR << std::endl; }
}

TSourceTab::TSourceTab(const TSourceTab& rhs)
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << __PRETTY_FUNCTION__ << RESET_COLOR << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fParent           = rhs.fParent;
   fSourceFrame      = rhs.fSourceFrame;
   fProjectionCanvas = rhs.fProjectionCanvas;
   fSourceStatusBar  = rhs.fSourceStatusBar;

   fProjection = rhs.fProjection;
   fData       = rhs.fData;
   fFwhm       = rhs.fFwhm;
   fSigma      = rhs.fSigma;
   fThreshold  = rhs.fThreshold;
   fPeakRatio  = rhs.fPeakRatio;
   // ? not sure what to do here, clearing the vectors seems unnecessary since we never filled them
   // but we also can't just simply copy them, we would need a deep copy, which might not make sense?
   // does is make sense in general to have assignment constructors for this class?
   fFits.clear();
   fBadFits.clear();
   fPeaks.clear();
}

TSourceTab::~TSourceTab()
{
   for(auto* fit : fFits) {
      delete fit;
   }
   fFits.clear();
   for(auto* fit : fBadFits) {
      delete fit;
   }
   fBadFits.clear();
   for(auto* peak : fPeaks) {
      delete peak;
   }
   fPeaks.clear();
   delete fSourceFrame;
   delete fProjectionCanvas;
   delete fSourceStatusBar;
   //delete fProjection;
   //delete fData;
   //delete fFwhm;
}

void TSourceTab::BuildInterface()
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << __PRETTY_FUNCTION__ << RESET_COLOR << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
                                                                                                                                                //if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Creating unique lock on graphics mutex to build interface " << std::endl; }
                                                                                                                                                //std::unique_lock<std::mutex> graphicsLock(fParent->Parent()->GraphicsMutex());
                                                                                                                                                //if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Got unique lock on graphics mutex!" << std::endl; }
   // frame with canvas and status bar
   fProjectionCanvas = new TRootEmbeddedCanvas(Form("ProjectionCanvas%s", fProjection->GetName()), fSourceFrame, TSourceCalibration::PanelWidth(), TSourceCalibration::PanelHeight());

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Created projection canvas!" << std::endl; }

   fSourceFrame->AddFrame(fProjectionCanvas, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Added projection canvas to source frame!" << std::endl; }

   fSourceStatusBar         = new TGStatusBar(fSourceFrame, TSourceCalibration::PanelWidth(), TSourceCalibration::StatusbarHeight());
   std::array<int, 3> parts = {35, 50, 15};
   fSourceStatusBar->SetParts(parts.data(), parts.size());

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Created status bar!" << std::endl; }

   fSourceFrame->AddFrame(fSourceStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Releasing unique lock on graphics mutex after building interface" << std::endl; }
}

void TSourceTab::MakeConnections()
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << __PRETTY_FUNCTION__ << RESET_COLOR << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
                                                                                                                                                //std::unique_lock<std::mutex> graphicsLock(fParent->Parent()->GraphicsMutex());
   fProjectionCanvas->Connect("ProcessedEvent(Event_t*)", "TSourceTab", this, "ProjectionStatus(Event_t*)");
   fProjectionCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TSourceTab", this, "ProjectionStatus(Int_t,Int_t,Int_t,TObject*)");
}

void TSourceTab::Disconnect()
{
   //std::unique_lock<std::mutex> graphicsLock(fParent->Parent()->GraphicsMutex());
   fProjectionCanvas->Disconnect("ProcessedEvent(Event_t*)", this, "ProjectionStatus(Event_t*)");
   fProjectionCanvas->GetCanvas()->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this, "ProjectionStatus(Int_t,Int_t,Int_t,TObject*)");
}

void TSourceTab::ProjectionStatus(Event_t* event)
{
   // enum EGEventType {
   // kGKeyPress      =  0, kKeyRelease,       =  1, kButtonPress      =  2, kButtonRelease   =  3,
   // kMotionNotify   =  4, kEnterNotify       =  5, kLeaveNotify      =  6, kFocusIn         =  7, kFocusOut      =  8,
   // kExpose         =  9, kConfigureNotify   = 10, kMapNotify        = 11, kUnmapNotify     = 12, kDestroyNotify = 13,
   // kClientMessage  = 14, kSelectionClear    = 15, kSelectionRequest = 16, kSelectionNotify = 17,
   // kColormapNotify = 18, kButtonDoubleClick = 19, kOtherEvent       = 20
   // };
   if(TSourceCalibration::VerboseLevel() == EVerbosity::kAll) {
      std::cout << __PRETTY_FUNCTION__ << ": code " << event->fCode << ", count " << event->fCount << ", state " << event->fState << ", type " << event->fType << std::endl;   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      if(event->fType == kClientMessage) {
         std::cout << "Client message: " << event->fUser[0] << ", " << event->fUser[1] << ", " << event->fUser[2] << std::endl;
      }
   }
   if(event->fType == kEnterNotify) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
         std::cout << "Entered source tab => changing gPad from " << gPad->GetName();
      }
      gPad = fProjectionCanvas->GetCanvas();
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
         std::cout << " to " << gPad->GetName() << std::endl;
      }
   }
}

void TSourceTab::ProjectionStatus(Int_t event, Int_t px, Int_t py, TObject* selected)
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
   Status(selected->GetName(), 0);
   Status(selected->GetObjectInfo(px, py), 1);
   //auto key = static_cast<char>(px);
   switch(event) {
   case kButton1Down:
      if(selected == fProjection) {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << "Adding new marker at " << px << ", " << py << " (pixel to user: " << fProjectionCanvas->GetCanvas()->PixeltoX(px) << ", " << fProjectionCanvas->GetCanvas()->PixeltoY(py - fProjectionCanvas->GetCanvas()->GetWh()) << ", abs pixel to user: " << fProjectionCanvas->GetCanvas()->AbsPixeltoX(px) << ", " << fProjectionCanvas->GetCanvas()->AbsPixeltoY(py) << ")" << std::endl; }
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << fPeaks.size() << " peaks:";
            for(auto* peak : fPeaks) {
               std::cout << " " << peak->Centroid();
            }
            std::cout << std::endl;
         }
         auto* polym = static_cast<TPolyMarker*>(fProjection->GetListOfFunctions()->FindObject("TPolyMarker"));
         if(polym == nullptr) {
            std::cerr << "No peaks defined yet?" << std::endl;
            return;
         }
         polym->SetNextPoint(fProjectionCanvas->GetCanvas()->AbsPixeltoX(px), fProjectionCanvas->GetCanvas()->AbsPixeltoX(py));
         double range = 4 * fSigma;   // * fProjection->GetXaxis()->GetBinWidth(1);
         auto*  pf    = new TPeakFitter(fProjectionCanvas->GetCanvas()->AbsPixeltoX(px) - range, fProjectionCanvas->GetCanvas()->AbsPixeltoX(px) + range);
         auto*  peak  = new TGauss(fProjectionCanvas->GetCanvas()->AbsPixeltoX(px));
         pf->AddPeak(peak);
         pf->Fit(fProjection, "qnretryfit");
         if(peak->Area() > 0) {
            fPeaks.push_back(peak);
            fFits.push_back(pf);
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Fitted peak " << fProjectionCanvas->GetCanvas()->AbsPixeltoX(px) - range << " - " << fProjectionCanvas->GetCanvas()->AbsPixeltoX(px) + range << " -> centroid " << peak->Centroid() << std::endl; }
         } else {
            std::cout << "Ignoring peak at " << peak->Centroid() << " with negative area " << peak->Area() << std::endl;
         }
         fProjection->GetListOfFunctions()->Remove(peak);
         fProjection->Sumw2(false);   // turn errors off, makes the histogram look like a histogram when using normal plotting (hist and samefunc doesn't work for some reason)

         std::sort(fPeaks.begin(), fPeaks.end(), [](const TGauss* a, const TGauss* b) { return a->Centroid() < b->Centroid(); });

         auto map = Match(fPeaks, fSourceEnergy, this);
         Add(map);

         // update status
         Status(Form("%d/%d", static_cast<int>(fData->GetN()), static_cast<int>(fPeaks.size())), 2);

         // update data and re-calibrate
         fParent->UpdateData();
         fParent->UpdateFwhm();
         fParent->Calibrate();
      }
      break;
   case kArrowKeyPress:
      //Move1DHistogram(px, fProjection);
      //fProjectionCanvas->GetCanvas()->Modified();
   case kArrowKeyRelease:
      break;
   case kKeyDown:
   case kKeyPress:
      //switch(key) {
      //	case 'u':
      //		fProjection->GetXaxis()->UnZoom();
      //		fProjection->GetYaxis()->UnZoom();
      //		break;
      //	case 'U':
      //		fProjection->GetYaxis()->UnZoom();
      //		break;
      //	default:
      //		std::cout << "Key press '" << key << "' not recognized!" << std::endl;
      //		break;
      //}
   case kKeyUp:
      break;
   case kButton1Motion:
      break;
   case kButton1Up:
      break;
   case kMouseMotion:
      break;
   case kMouseEnter:
      break;
   case kMouseLeave:
      break;
   default:
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
         std::cout << "unprocessed event " << event << " with px, py " << px << ", " << py << " selected object is a " << selected->ClassName() << " with name " << selected->GetName() << " and title " << selected->GetTitle() << " object info is " << selected->GetObjectInfo(px, py) << std::endl;
      }
      break;
   }
}

void TSourceTab::UpdateRegions()
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "UpdateRegions: clearing " << fRegions.size() << " regions" << std::endl; }
   fRegions.clear();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "projection has " << fProjection->ListOfRegions()->GetEntries() << " regions:" << std::endl;
      fProjection->ListOfRegions()->Print();
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) { fProjection->Print(); }
   for(auto* obj : *(fProjection->ListOfRegions())) {
      if(obj->InheritsFrom(TRegion::Class())) {
         auto* region = static_cast<TRegion*>(obj);
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "UpdateRegions: found " << fRegions.size() << ". region: " << std::min(region->GetX1(), region->GetX2()) << " - " << std::max(region->GetX1(), region->GetX2()) << std::endl; }
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) { region->Print(); }
         fRegions.emplace_back(std::min(region->GetX1(), region->GetX2()), std::max(region->GetX1(), region->GetX2()));
      } else if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
         std::cout << obj->GetName() << " is not a TRegion but a " << obj->ClassName() << std::endl;
      }
   }
}

bool TSourceTab::Good(TGauss* peak)
{
   return TSourceCalibration::AcceptBadFits() || (peak->CentroidErr() < 0.1 * peak->Centroid() && peak->AreaErr() < peak->Area() && !std::isnan(peak->CentroidErr()) && !std::isnan(peak->AreaErr()));
}

void TSourceTab::UpdateFits()
{
   /// This functions removes all fits named "gauss_total" from the histogram and adds instead the fit functions from all good and bad peaks to it.
   TList* functions = fProjection->GetListOfFunctions();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "Updating functions for histogram " << fProjection->GetName() << " by deleting " << functions->GetEntries() << " functions:" << std::endl;
      for(auto* obj : *functions) {
         std::cout << obj << ": " << obj->GetName() << "/" << obj->GetTitle() << " - " << obj->ClassName() << std::endl;
      }
   }
   // remove all old fits (all have the same name/title of "gauss_total")
   // alternative: clone the peaks with indices in their names, that would also allow us to add a function to replace a single fit
   TObject* fit = nullptr;
   while((fit = functions->FindObject("gauss_total")) != nullptr) {
      functions->Remove(fit);
   }
   // add all the fit functions of the good and the bad peaks
   for(auto* obj : fFits) {
      functions->Add(obj->GetFitFunction()->Clone("gauss_total"));
   }
   for(auto* obj : fBadFits) {
      functions->Add(obj->GetFitFunction()->Clone("gauss_total"));
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "And adding " << functions->GetEntries() << " functions instead:" << std::endl;
      for(auto* obj : *functions) {
         std::cout << obj << ": " << obj->GetName() << "/" << obj->GetTitle() << " - " << obj->ClassName() << std::endl;
      }
   }
}

void TSourceTab::Draw()
{
   fProjectionCanvas->GetCanvas()->cd();
   fProjection->Draw();                                                                                                                                                                                                                           // seems like hist + samefunc does not work. Could use hist + loop over list of functions (with same)
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": drew " << fProjection->GetName() << " on " << fProjectionCanvas->GetCanvas()->GetName() << "/" << gPad->GetName() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

void TSourceTab::InitialCalibration(const double& sigma, const double& threshold, const double& peakRatio, const bool& force, const bool& fast)
{
   /// This functions finds the peaks in the histogram, fits them, and adds the fits to the list of peaks.
   /// This list is then used to find all peaks that lie on a straight line.

   if(fPeakRatio != peakRatio) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << __PRETTY_FUNCTION__ << ": updating peak ratio from " << fPeakRatio << " to " << peakRatio << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      fPeakRatio = peakRatio;
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << __PRETTY_FUNCTION__ << std::flush << " " << fProjection->GetName() << ": got " << fPeaks.size() << " peaks" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   if(fPeaks.empty() || fData == nullptr || sigma != fSigma || threshold != fThreshold || force) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << __PRETTY_FUNCTION__ << ": # peaks " << fPeaks.size() << ", sigma (" << sigma << "/" << fSigma << "), or threshold (" << threshold << "/" << fThreshold << ") have changed?" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      }
      fSigma     = sigma;
      fThreshold = threshold;
      fPeaks.clear();
      fFits.clear();
      fBadFits.clear();
      // Remove all associated functions from projection.
      // These are the poly markers, fits, and the pave stat
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
         TList* functions = fProjection->GetListOfFunctions();
         std::cout << functions->GetEntries() << " functions in projection " << fProjection->GetName() << " before clearing" << std::endl;
         for(auto* obj : *functions) {
            std::cout << obj->GetName() << "/" << obj->GetTitle() << " - " << obj->ClassName() << std::endl;
         }
      }
      fProjection->GetListOfFunctions()->Clear();
      TSpectrum           spectrum;
      int                 nofPeaks = 0;
      std::vector<double> peakPos;
      UpdateRegions();
      if(fRegions.empty()) {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "No regions found, using whole spectrum" << std::endl; }
         spectrum.Search(fProjection, fSigma, "", fThreshold);
         nofPeaks = spectrum.GetNPeaks();
         if(nofPeaks > fPeakRatio * static_cast<double>(fSourceEnergy.size())) {
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "Reducing # of peaks from " << nofPeaks; }
            nofPeaks = static_cast<int>(fPeakRatio * static_cast<double>(fSourceEnergy.size()));
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << " to " << nofPeaks << std::endl; }
         }
         peakPos.insert(peakPos.end(), spectrum.GetPositionX(), spectrum.GetPositionX() + nofPeaks);
      } else {
         for(auto& region : fRegions) {
            fProjection->GetXaxis()->SetRangeUser(region.first, region.second);
            spectrum.Search(fProjection, fSigma, "", fThreshold);
            int tmpNofPeaks = spectrum.GetNPeaks();
            if(tmpNofPeaks > fPeakRatio * static_cast<double>(fSourceEnergy.size())) {
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "Reducing # of peaks from " << tmpNofPeaks; }
               tmpNofPeaks = static_cast<int>(fPeakRatio * static_cast<double>(fSourceEnergy.size()));
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << " to " << tmpNofPeaks << std::endl; }
            }
            peakPos.insert(peakPos.end(), spectrum.GetPositionX(), spectrum.GetPositionX() + tmpNofPeaks);
            nofPeaks += spectrum.GetNPeaks();
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": found " << spectrum.GetNPeaks() << " peaks in region " << region.first << " - " << region.second << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
         }
         fProjection->GetXaxis()->UnZoom();
      }
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": found " << nofPeaks << " peaks" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

      auto map = RoughCal(peakPos, fSourceEnergy, this);
      Add(map);

      // update status
      Status(Form("%d/%d", static_cast<int>(fData->GetN()), nofPeaks), 2);
   }
}

void TSourceTab::FindPeaks(const double& sigma, const double& threshold, const double& peakRatio, const bool& force, const bool& fast)
{
   /// This functions finds the peaks in the histogram, fits them, and adds the fits to the list of peaks.
   /// This list is then used to find all peaks that lie on a straight line.

   if(fPeakRatio != peakRatio) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << __PRETTY_FUNCTION__ << ": updating peak ratio from " << fPeakRatio << " to " << peakRatio << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      fPeakRatio = peakRatio;
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << __PRETTY_FUNCTION__ << std::flush << " " << fProjection->GetName() << ": got " << fPeaks.size() << " peaks" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   if(fPeaks.empty() || fData == nullptr || sigma != fSigma || threshold != fThreshold || force) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << __PRETTY_FUNCTION__ << ": # peaks " << fPeaks.size() << ", sigma (" << sigma << "/" << fSigma << "), or threshold (" << threshold << "/" << fThreshold << ") have changed?" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      }
      fSigma     = sigma;
      fThreshold = threshold;
      fPeaks.clear();
      fFits.clear();
      fBadFits.clear();
      // Remove all associated functions from projection.
      // These are the poly markers, fits, and the pave stat
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
         TList* functions = fProjection->GetListOfFunctions();
         std::cout << functions->GetEntries() << " functions in projection " << fProjection->GetName() << " before clearing" << std::endl;
         for(auto* obj : *functions) {
            std::cout << obj->GetName() << "/" << obj->GetTitle() << " - " << obj->ClassName() << std::endl;
         }
      }
      fProjection->GetListOfFunctions()->Clear();
      TSpectrum           spectrum;
      int                 nofPeaks = 0;
      std::vector<double> peakPos;
      UpdateRegions();
      if(fRegions.empty()) {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "No regions found, using whole spectrum" << std::endl; }
         spectrum.Search(fProjection, fSigma, "", fThreshold);
         nofPeaks = spectrum.GetNPeaks();
         if(nofPeaks > fPeakRatio * static_cast<double>(fSourceEnergy.size())) {
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "Reducing # of peaks from " << nofPeaks; }
            nofPeaks = static_cast<int>(fPeakRatio * static_cast<double>(fSourceEnergy.size()));
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << " to " << nofPeaks << std::endl; }
         }
         peakPos.insert(peakPos.end(), spectrum.GetPositionX(), spectrum.GetPositionX() + nofPeaks);
      } else {
         for(auto& region : fRegions) {
            fProjection->GetXaxis()->SetRangeUser(region.first, region.second);
            spectrum.Search(fProjection, fSigma, "", fThreshold);
            int tmpNofPeaks = spectrum.GetNPeaks();
            if(tmpNofPeaks > fPeakRatio * static_cast<double>(fSourceEnergy.size())) {
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "Reducing # of peaks from " << tmpNofPeaks; }
               tmpNofPeaks = static_cast<int>(fPeakRatio * static_cast<double>(fSourceEnergy.size()));
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << " to " << tmpNofPeaks << std::endl; }
            }
            peakPos.insert(peakPos.end(), spectrum.GetPositionX(), spectrum.GetPositionX() + tmpNofPeaks);
            nofPeaks += spectrum.GetNPeaks();
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": found " << spectrum.GetNPeaks() << " peaks in region " << region.first << " - " << region.second << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
         }
         fProjection->GetXaxis()->UnZoom();
      }
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": found " << nofPeaks << " peaks" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      for(int i = 0; i < nofPeaks; i++) {
         double range = TSourceCalibration::FitRange() * fSigma;
         // if we aren't already redirecting the output, we redirect it to /dev/null and do a quiet fit, otherwise we do a normal fit since the output will be redirected to a file
         auto* pf   = new TPeakFitter(peakPos[i] - range, peakPos[i] + range);
         auto* peak = new TGauss(peakPos[i]);
         pf->AddPeak(peak);
         if(TSourceCalibration::LogFile().empty()) {
            TRedirect redirect("/dev/null");
            pf->Fit(fProjection, "qnretryfit");
         } else {
            pf->Fit(fProjection, "nretryfit");
         }
         if(peak->Area() > 0) {
            // check if we accept bad fits (and if it was a bad fit)
            if(Good(peak)) {
               peak->SetLineColor(kRed);
               fPeaks.push_back(peak);
               fFits.push_back(pf);
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
                  std::cout << "Fitted peak " << peakPos[i] - range << " - " << peakPos[i] + range << " -> centroid " << peak->Centroid() << ", area " << peak->Area() << std::endl;
               }
            } else {
               peak->SetLineColor(kGray);
               fBadFits.push_back(pf);
               if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
                  std::cout << "We're ignoring bad fits, and this one has position " << peak->Centroid() << " +- " << peak->CentroidErr() << ", area " << peak->Area() << " +- " << peak->AreaErr() << std::endl;
               }
            }
         } else if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "Ignoring peak at " << peak->Centroid() << " with negative area " << peak->Area() << std::endl;
            //delete peak;
         }
      }
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": added " << fPeaks.size() << " peaks" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      fProjection->Sumw2(false);                                                                                                                                        // turn errors off, makes the histogram look like a histogram when using normal plotting (hist and samefunc doesn't work for some reason)

      UpdateFits();

      std::sort(fFits.begin(), fFits.end(), [](const TPeakFitter* a, const TPeakFitter* b) { return a->GetFitFunction()->GetParameter("centroid_0") < b->GetFitFunction()->GetParameter("centroid_0"); });
      std::sort(fBadFits.begin(), fBadFits.end(), [](const TPeakFitter* a, const TPeakFitter* b) { return a->GetFitFunction()->GetParameter("centroid_0") < b->GetFitFunction()->GetParameter("centroid_0"); });
      std::sort(fPeaks.begin(), fPeaks.end(), [](const TGauss* a, const TGauss* b) { return a->Centroid() < b->Centroid(); });

      if(fast) {
         auto map = SmartMatch(fPeaks, fSourceEnergy, this);
         Add(map);
      } else {
         auto map = Match(fPeaks, fSourceEnergy, this);
         Add(map);
      }

      // update status
      Status(Form("%d/%d", static_cast<int>(fData->GetN()), nofPeaks), 2);
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << __PRETTY_FUNCTION__ << ": found " << fData->GetN() << " peaks";   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      Double_t* x = fData->GetX();
      Double_t* y = fData->GetY();
      for(int i = 0; i < fData->GetN(); ++i) {
         std::cout << " - " << x[i] << ", " << y[i];
      }
      std::cout << std::endl;
   }
}

void TSourceTab::FindCalibratedPeaks(const TF1* calibration)
{
   /// This functions finds uses the existing calibration to find all peaks from the list of source energies.

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DCYAN << __PRETTY_FUNCTION__ << std::flush << " " << fProjection->GetName() << ": using calibration " << calibration << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   if(calibration == nullptr) {
      std::cerr << __PRETTY_FUNCTION__ << ": provided calibration is nullptr, this function can only be called after an initial calibration has been made!" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      return;
   }

   fPeaks.clear();
   fFits.clear();
   fBadFits.clear();
   // Remove all associated functions from projection.
   // These are the poly markers, fits, and the pave stat
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
      TList* functions = fProjection->GetListOfFunctions();
      std::cout << functions->GetEntries() << " functions in projection " << fProjection->GetName() << " before clearing" << std::endl;
      for(auto* obj : *functions) {
         std::cout << obj->GetName() << "/" << obj->GetTitle() << " - " << obj->ClassName() << std::endl;
      }
   }
   fProjection->GetListOfFunctions()->Clear();

   // loop over all source energies
   for(auto iter : fSourceEnergy) {
      if(std::get<2>(iter) < TSourceCalibration::MinIntensity()) {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "Skipping peak at " << std::get<0>(iter) << " keV with intensity " << std::get<2>(iter) << " below required minimum intensity " << TSourceCalibration::MinIntensity() << std::endl;
         }
         continue;
      }
      double range   = TSourceCalibration::FitRange() * fSigma;
      double peakPos = calibration->GetX(std::get<0>(iter));
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << "Trying to fit peak " << peakPos << " (" << std::get<0>(iter) << " keV) in range " << peakPos - range << " - " << peakPos + range << std::endl;
      }
      // if we aren't already redirecting the output, we redirect it to /dev/null and do a quiet fit, otherwise we do a normal fit since the output will be redirected to a file
      auto* pf   = new TPeakFitter(peakPos - range, peakPos + range);
      auto* peak = new TGauss(peakPos);
      pf->AddPeak(peak);
      if(TSourceCalibration::LogFile().empty()) {
         TRedirect redirect("/dev/null");
         pf->Fit(fProjection, "qnretryfit");
      } else {
         pf->Fit(fProjection, "nretryfit");
      }
      if(peak->Area() > 0) {
         // check if we accept bad fits (and if it was a bad fit)
         if(Good(peak)) {
            peak->SetLineColor(kRed);
            fPeaks.push_back(peak);
            fFits.push_back(pf);
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
               std::cout << "Fitted peak " << peakPos - range << " - " << peakPos + range << " -> centroid " << peak->Centroid() << ", area " << peak->Area() << std::endl;
            }
         } else {
            peak->SetLineColor(kGray);
            fBadFits.push_back(pf);
            if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
               std::cout << "We're ignoring bad fits, and this one has position " << peak->Centroid() << " +- " << peak->CentroidErr() << ", area " << peak->Area() << " +- " << peak->AreaErr() << std::endl;
            }
         }
      } else if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << "Ignoring peak at " << peak->Centroid() << " with negative area " << peak->Area() << std::endl;
      }
   }

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": added " << fPeaks.size() << " peaks (" << fSourceEnergy.size() << " source energies)" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fProjection->Sumw2(false);                                                                                                                                                                                         // turn errors off, makes the histogram look like a histogram when using normal plotting (hist and samefunc doesn't work for some reason)

   UpdateFits();

   std::sort(fFits.begin(), fFits.end(), [](const TPeakFitter* a, const TPeakFitter* b) { return a->GetFitFunction()->GetParameter("centroid_0") < b->GetFitFunction()->GetParameter("centroid_0"); });
   std::sort(fBadFits.begin(), fBadFits.end(), [](const TPeakFitter* a, const TPeakFitter* b) { return a->GetFitFunction()->GetParameter("centroid_0") < b->GetFitFunction()->GetParameter("centroid_0"); });
   std::sort(fPeaks.begin(), fPeaks.end(), [](const TGauss* a, const TGauss* b) { return a->Centroid() < b->Centroid(); });

   auto map = Match(fPeaks, fSourceEnergy, this);
   Add(map);

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << __PRETTY_FUNCTION__ << ": found " << fData->GetN() << " peaks";   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      Double_t* x = fData->GetX();
      Double_t* y = fData->GetY();
      for(int i = 0; i < fData->GetN(); ++i) {
         std::cout << " - " << x[i] << ", " << y[i];
      }
      std::cout << std::endl;
   }
}

void TSourceTab::Add(std::map<double, std::tuple<double, double, double, double>> map)
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << DCYAN << "Adding map with " << map.size() << " data points, fData = " << fData << std::endl;
   }
   if(fData == nullptr) {
      fData = new TGraphErrors(map.size());
   } else {
      fData->Set(map.size());
   }
   fData->SetLineColor(2);
   fData->SetMarkerColor(2);
   int i = 0;
   for(auto iter = map.begin(); iter != map.end();) {
      // more readable variable names
      auto peakPos = iter->first;
      auto energy  = std::get<0>(iter->second);
      fData->SetPoint(i, peakPos, energy);
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << "Using peak with position " << peakPos << ", energy " << energy << std::endl;
      }
      ++iter;
      ++i;
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << "Accepted " << map.size() << " peaks" << std::endl;
   }
   fData->Sort();
}

void TSourceTab::Add(std::map<TGauss*, std::tuple<double, double, double, double>> map)
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << DCYAN << "Adding map with " << map.size() << " data points, fData = " << fData << std::endl;
   }
   if(fData == nullptr) {
      fData = new TGraphErrors(map.size());
   } else {
      fData->Set(map.size());
   }
   fData->SetLineColor(2);
   fData->SetMarkerColor(2);
   if(fFwhm == nullptr) {
      fFwhm = new TGraphErrors(map.size());
   } else {
      fFwhm->Set(map.size());
   }
   fFwhm->SetLineColor(4);
   fFwhm->SetMarkerColor(4);
   int i = 0;
   for(auto iter = map.begin(); iter != map.end();) {
      // more readable variable names
      auto peakPos     = iter->first->Centroid();
      auto peakPosErr  = iter->first->CentroidErr();
      auto peakArea    = iter->first->Area();
      auto peakAreaErr = iter->first->AreaErr();
      auto fwhm        = iter->first->FWHM();
      auto fwhmErr     = iter->first->FWHMErr();
      auto energy      = std::get<0>(iter->second);
      auto energyErr   = std::get<1>(iter->second);
      // drop this peak if the uncertainties in area or position are too large
      if(!Good(iter->first)) {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
            std::cout << "Dropping peak with position " << peakPos << " +- " << peakPosErr << ", area " << peakArea << " +- " << peakAreaErr << ", energy " << energy << std::endl;
         }
         iter = map.erase(iter);
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "Erasing peak returned iterator " << std::distance(map.begin(), iter) << std::endl;
         }
      } else {
         fData->SetPoint(i, peakPos, energy);
         fData->SetPointError(i, peakPosErr, energyErr);
         fFwhm->SetPoint(i, peakPos, fwhm);
         fFwhm->SetPointError(i, peakPosErr, fwhmErr);
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "Using peak with position " << peakPos << " +- " << peakPosErr << ", area " << peakArea << " +- " << peakAreaErr << ", energy " << energy << std::endl;
         }
         ++iter;
         ++i;
      }
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << "Accepted " << map.size() << " peaks" << std::endl;
   }
   // if we dropped a peak, we need to resize the graph, if we haven't dropped any this doesn't do anything
   fData->Set(map.size());
   fFwhm->Set(map.size());
   fData->Sort();
   fFwhm->Sort();
   // split poly marker into those peaks that were used, and those that weren't
   TList* functions = fProjection->GetListOfFunctions();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
      std::cout << functions->GetEntries() << " functions in projection " << fProjection->GetName() << std::endl;
      for(auto* obj : *functions) {
         std::cout << obj->GetName() << "/" << obj->GetTitle() << " - " << obj->ClassName() << std::endl;
      }
   }
   auto* polym = static_cast<TPolyMarker*>(functions->FindObject("TPolyMarker"));
   if(polym != nullptr) {
      double*             oldX = polym->GetX();
      double*             oldY = polym->GetY();
      int                 size = polym->GetN();
      std::vector<double> newX;
      std::vector<double> newY;
      std::vector<double> unusedX;
      std::vector<double> unusedY;
      for(i = 0; i < size; ++i) {
         bool used = false;
         for(auto point : map) {
            if(TMath::Abs(oldX[i] - point.first->Centroid()) < fSigma) {
               newX.push_back(oldX[i]);
               newY.push_back(oldY[i]);
               used = true;
               break;
            }
         }
         if(!used) {
            unusedX.push_back(oldX[i]);
            unusedY.push_back(oldY[i]);
         }
      }
      polym->SetPolyMarker(newX.size(), newX.data(), newY.data());
      auto* unusedMarkers = new TPolyMarker(unusedX.size(), unusedX.data(), unusedY.data());
      unusedMarkers->SetMarkerStyle(23);   // triangle down
      unusedMarkers->SetMarkerColor(16);   // light grey
      functions->Add(unusedMarkers);
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << fProjection->GetTitle() << ": " << size << " peaks founds total, " << polym->GetN() << " used, and " << unusedMarkers->GetN() << " unused" << std::endl;
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
            std::cout << "Used: ";
            for(size_t m = 0; m < newX.size(); ++m) { std::cout << newX[m] << " - " << newY[m] << ";"; }
            std::cout << std::endl;
            std::cout << "Unused: ";
            for(size_t m = 0; m < unusedX.size(); ++m) { std::cout << unusedX[m] << " - " << unusedY[m] << ";"; }
            std::cout << std::endl;
         }
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
            std::cout << functions->GetEntries() << " functions in projection " << fProjection->GetTitle() << std::endl;
            for(auto* obj : *functions) {
               std::cout << obj->GetName() << "/" << obj->GetTitle() << " - " << obj->ClassName() << std::endl;
            }
         }
      }
   }

   // change color of fit functions for unused peaks
   TIter    iter(functions);
   TObject* item = nullptr;
   while((item = iter.Next()) != nullptr) {
      if(item->IsA() == TF1::Class() || item->IsA() == TGauss::Class()) {   // if the item is a TF1 or TGauss we see if we can find the centroid in the map of used peaks
         double centroid = 0.;
         if(item->IsA() == TF1::Class()) {
            centroid = static_cast<TF1*>(item)->GetParameter(1);
         } else {
            centroid = static_cast<TGauss*>(item)->Centroid();
         }
         bool found = false;
         for(auto point : map) {
            if(TMath::Abs(centroid - point.first->Centroid()) < fSigma) {
               found = true;
               break;
            }
         }
         // remove the TF1 if it wasn't found in the map
         if(!found) {
            //functions->Remove(item);
            if(item->IsA() == TF1::Class()) {
               static_cast<TF1*>(item)->SetLineColor(kGray);
            } else {
               static_cast<TGauss*>(item)->GetFitFunction()->SetLineColor(kGray);
            }
         }
      }
   }
}

void TSourceTab::ReplacePeak(const size_t& index, const double& channel)
{
   /// Replace the peak at the index with one centered at channel (calculated from an initial calibration and the source energy of this peak).
   /// This replaces the TGauss* in fPeaks, and updates the values of this index in fData and fFwhm.
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << DCYAN << __PRETTY_FUNCTION__ << ": index " << index << ", channel " << channel << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      for(size_t i = 0; i < fPeaks.size() && i < fFits.size(); ++i) {
         std::cout << i << ": " << fPeaks[i]->Centroid() << " +- " << fPeaks[i]->CentroidErr() << " - " << fFits[i]->GetFitFunction()->GetParameter("centroid_0") << std::endl;
      }
   }

   if(index >= fPeaks.size()) {
      std::cerr << "Trying to replace peak at index " << index << " but there are only " << fPeaks.size() << " peaks!" << std::endl;
      return;
   }

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Replacing old peak at index " << index << ", centroid " << fPeaks.at(index)->Centroid() << " with new peak at channel " << channel << std::endl; }

   // calculate the fitting range (low to high) and adjust it so we ignore the old peak
   double range = TSourceCalibration::FitRange() * fSigma;
   double low   = channel - range;
   double high  = channel + range;
   if(fPeaks.at(index)->Centroid() < channel) {
      low = (fPeaks.at(index)->Centroid() + channel) / 2.;
   } else {
      high = (fPeaks.at(index)->Centroid() + channel) / 2.;
   }
   // if we aren't already redirecting the output, we redirect it to /dev/null and do a quiet fit, otherwise we do a normal fit since the output will be redirected to a file
   auto* pf   = new TPeakFitter(low, high);
   auto* peak = new TGauss(channel);
   pf->AddPeak(peak);
   if(TSourceCalibration::LogFile().empty()) {
      //TRedirect redirect("/dev/null");
      pf->Fit(fProjection, "retryfit");
   } else {
      pf->Fit(fProjection, "retryfit");
   }
   if(peak->Area() > 0) {
      if(Good(peak)) {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << index << ". peak/fit out of " << fPeaks.size() << "/" << fFits.size() << " peaks/fits: " << std::flush << fPeaks.at(index)->Centroid() << "/" << fFits.at(index)->GetFitFunction()->GetParameter("centroid_0") << std::endl;
            std::cout << "Deleting " << index << ". peak " << fPeaks.at(index) << std::flush;
         }
         delete fPeaks.at(index);
         fPeaks[index] = peak;
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << " and setting it to " << fPeaks.at(index) << std::endl;
         }
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "Deleting " << index << ". fit " << fFits.at(index) << std::flush;
         }
         delete fFits.at(index);
         fFits[index] = pf;
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << " and setting it to " << fFits.at(index) << std::endl;
         }
         UpdateFits();
         fData->SetPoint(index, peak->Centroid(), fData->GetPointY(index));
         fData->SetPointError(index, peak->CentroidErr(), fData->GetErrorY(index));
         fFwhm->SetPoint(index, peak->Centroid(), peak->FWHM());
         fFwhm->SetPointError(index, peak->CentroidErr(), peak->FWHMErr());
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "Fitted peak " << low << " - " << high << " -> centroid " << peak->Centroid() << ", area " << peak->Area() << std::endl;
         }
      } else {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            std::cout << "We're ignoring bad fits, and this one has position " << peak->Centroid() << " +- " << peak->CentroidErr() << ", area " << peak->Area() << " +- " << peak->AreaErr() << std::endl;
         }
      }
   } else if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "Ignoring peak at " << peak->Centroid() << " with negative area " << peak->Area() << std::endl;
   }
}

void TSourceTab::RemovePoint(Int_t oldPoint)
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << DCYAN << __PRETTY_FUNCTION__ << ": oldPoint " << oldPoint << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   // first thing to do is check if the graph the point was removed from matches the id of this tab
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { Print(); }
   //auto erasedPeak = fPeaks.at(oldPoint);
   fPeaks.erase(fPeaks.begin() + oldPoint);
   auto* erasedFit = fFits.at(oldPoint);
   fFits.erase(fFits.begin() + oldPoint);
   fBadFits.push_back(erasedFit);
   fData->RemovePoint(oldPoint);

   UpdateFits();
   // we also need to move the marker of this peak to the poly-marker of unused peaks (16 is light gray aka the color of the unused markers)
   //if(item->IsA() == TPolyMarker::Class()) { item->Print(); }   // && static_cast<TPolyMarker*>(item)->GetMarkerColor() != 16) {
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { Print(); }
}

void TSourceTab::Status(const char* status, int position)
{
   fSourceStatusBar->SetText(status, position);
   fSourceStatusBar->MapWindow();
   fSourceFrame->MapSubwindows();
   gVirtualX->Update();
}

void TSourceTab::Print() const
{
   std::cout << "Projection " << fProjection;
   if(fProjection != nullptr) {
      std::cout << " = " << fProjection->GetName() << "/" << fProjection->GetTitle();
   } else {
      std::cout << " is null";
   }
   std::cout << std::endl;

   std::cout << "Data " << fData;
   if(fData != nullptr) {
      std::cout << " = " << fData->GetName() << "/" << fData->GetTitle();
   } else {
      std::cout << " is null";
   }
   std::cout << std::endl;

   std::cout << "Fwhm " << fFwhm;
   if(fFwhm != nullptr) {
      std::cout << " = " << fFwhm->GetName() << "/" << fFwhm->GetTitle();
   } else {
      std::cout << " is null";
   }
   std::cout << std::endl;

   std::cout << fFits.size() << " good fits:";
   for(size_t i = 0; i < fFits.size(); ++i) {
      std::cout << " " << std::setw(2) << i << " - " << std::setw(8) << fFits.at(i)->GetFitFunction()->GetParameter("centroid_0");
   }
   std::cout << std::endl;

   std::cout << fBadFits.size() << " bad fits:";
   for(size_t i = 0; i < fBadFits.size(); ++i) {
      std::cout << " " << std::setw(2) << i << " - " << std::setw(8) << fBadFits.at(i)->GetFitFunction()->GetParameter("centroid_0");
   }
   std::cout << std::endl;

   std::cout << fPeaks.size() << " peaks:";
   for(size_t i = 0; i < fPeaks.size(); ++i) {
      std::cout << " " << std::setw(2) << i << " - " << std::setw(8) << fPeaks.at(i)->Centroid();
   }
   std::cout << std::endl;
}

void TSourceTab::PrintCanvases() const
{
   std::cout << "TSourceTab " << fSourceName << " projection canvas:" << std::endl;
   fProjectionCanvas->GetCanvas()->Print();
   for(auto* obj : *(fProjectionCanvas->GetCanvas()->GetListOfPrimitives())) {
      obj->Print();
   }
}

void TSourceTab::PrintLayout() const
{
   std::cout << "TSourceTab frame:" << std::endl;
   fSourceFrame->Print();
   std::cout << "TSourceTab canvas:" << std::endl;
   fProjectionCanvas->Print();
   std::cout << "TSourceTab status bar:" << std::endl;
   fSourceStatusBar->Print();
}

//////////////////////////////////////// TChannelTab ////////////////////////////////////////
TChannelTab::TChannelTab(TSourceCalibration* parent, std::vector<TNucleus*> nuclei, std::vector<GH1D*> projections, std::string name, TGCompositeFrame* frame, double sigma, double threshold, int degree, std::vector<std::vector<std::tuple<double, double, double, double>>> sourceEnergies, TGHProgressBar* progressBar)
   : fChannelFrame(frame), fSourceTab(new TGTab(frame, TSourceCalibration::PanelWidth(), TSourceCalibration::PanelHeight() + 2 * TSourceCalibration::StatusbarHeight())), fCanvasTab(new TGTab(frame, TSourceCalibration::PanelWidth(), TSourceCalibration::PanelHeight() + 2 * TSourceCalibration::StatusbarHeight())), fProgressBar(progressBar), fParent(parent), fNuclei(std::move(nuclei)), fProjections(std::move(projections)), fName(std::move(name)), fSigma(sigma), fThreshold(threshold), fDegree(degree), fSourceEnergies(std::move(sourceEnergies))
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << DYELLOW << "========================================" << std::endl;
      std::cout << __PRETTY_FUNCTION__ << std::endl   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
                << " name = " << fName << ", fData = " << fData << std::endl;
      std::cout << "========================================" << std::endl;
   }

   BuildInterface();

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << DYELLOW << "----------------------------------------" << std::endl;
      std::cout << __PRETTY_FUNCTION__ << std::endl   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
                << " channel " << fName << " done" << std::endl;
      std::cout << "----------------------------------------" << std::endl;
   }
}

TChannelTab::~TChannelTab()
{
   delete fChannelFrame;
   delete fSourceTab;
   for(auto* channel : fSources) {
      delete channel;
   }
   delete fCanvasTab;
   delete fCalibrationFrame;
   delete fCalibrationCanvas;
   delete fResidualPad;
   delete fCalibrationPad;
   delete fChannelStatusBar;
   delete fLegend;
   delete fChi2Label;
   delete fFwhmFrame;
   delete fFwhmCanvas;
   delete fProgressBar;
   // delete all fProjections and all fNuclei?
   delete fData;
   delete fFwhm;
}

void TChannelTab::BuildInterface()
{
   {
      //std::unique_lock<std::mutex> graphicsLock(fParent->GraphicsMutex());
      fChannelFrame->SetLayoutManager(new TGHorizontalLayout(fChannelFrame));
   }

   fSources.resize(fNuclei.size(), nullptr);
   //if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << ": creating channels for bin 1 to " << fMatrix->GetNbinsX() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   for(size_t source = 0; source < fNuclei.size(); ++source) {
      CreateSourceTab(source);
   }

   for(auto iter = fSources.begin(); iter != fSources.end(); ++iter) {
      if(*iter == nullptr) {
         fSources.erase(iter--);   // erase iterator and then go to the element before this one (and then the loop goes to the next one)
      }
   }

   {
      //std::unique_lock<std::mutex> graphicsLock(fParent->GraphicsMutex());
      fChannelFrame->AddFrame(fSourceTab, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

      fCalibrationFrame = fCanvasTab->AddTab("Calibration");
      fCalibrationFrame->SetLayoutManager(new TGVerticalLayout(fCalibrationFrame));
      fCalibrationCanvas = new TRootEmbeddedCanvas(Form("CalibrationCanvas%s", fName.c_str()), fCalibrationFrame, TSourceCalibration::PanelWidth(), TSourceCalibration::PanelHeight());
      fCalibrationFrame->AddFrame(fCalibrationCanvas, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
      fChannelStatusBar        = new TGStatusBar(fCalibrationFrame, TSourceCalibration::PanelWidth(), TSourceCalibration::StatusbarHeight());
      std::array<int, 3> parts = {25, 35, 40};
      fChannelStatusBar->SetParts(parts.data(), parts.size());
      fCalibrationFrame->AddFrame(fChannelStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));
      fFwhmFrame = fCanvasTab->AddTab("FWHM");
      fFwhmFrame->SetLayoutManager(new TGVerticalLayout(fFwhmFrame));
      fFwhmCanvas = new TRootEmbeddedCanvas(Form("FwhmCanvas%s", fName.c_str()), fFwhmFrame, TSourceCalibration::PanelWidth(), TSourceCalibration::PanelHeight());
      fFwhmFrame->AddFrame(fFwhmCanvas, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
      //fChannelFrame->AddFrame(fCalibrationFrame, new TGLayoutHints(kLHintsRight | kLHintsBottom | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

      fInitFrame = fCanvasTab->AddTab("Initial");
      fInitFrame->SetLayoutManager(new TGVerticalLayout(fInitFrame));
      fInitCanvas = new TRootEmbeddedCanvas(Form("InitCanvas%s", fName.c_str()), fInitFrame, TSourceCalibration::PanelWidth(), TSourceCalibration::PanelHeight());
      fInitFrame->AddFrame(fInitCanvas, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

      fChannelFrame->AddFrame(fCanvasTab, new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
   }

   UpdateData();

   {
      //std::unique_lock<std::mutex> graphicsLock(fParent->GraphicsMutex());
      fCalibrationCanvas->GetCanvas()->cd();
      fCalibrationPad = new TPad(Form("cal_%s", fName.c_str()), Form("calibration for %s", fName.c_str()), 0.2, 0., 1., 1.);
      fCalibrationPad->SetNumber(1);
      fCalibrationPad->Draw();
      fCalibrationPad->AddExec("zoom", "TChannelTab::ZoomY()");

      fLegend = new TLegend(0.8, 0.3, 0.95, 0.3 + static_cast<double>(fNuclei.size()) * 0.05);   // x1, y1, x2, y2

      fCalibrationCanvas->GetCanvas()->cd();
      fResidualPad = new TPad(Form("res_%s", fName.c_str()), Form("residual for %s", fName.c_str()), 0.0, 0., 0.2, 1.);
      fResidualPad->SetNumber(2);
      fResidualPad->Draw();
      fResidualPad->AddExec("zoom", "TChannelTab::ZoomY()");
   }
   //Calibrate();   // also creates the residual and chi^2 label

   // check whether we want to use this initial calibration to find more peaks
   //if(TSourceCalibration::UseCalibratedPeaks()) {
   //	FindCalibratedPeaks();
   //}

   // get the fwhm graphs and plot them
   UpdateFwhm();
   //{
   //	std::unique_lock<std::mutex> graphicsLock(fParent->GraphicsMutex());
   //	fFwhmCanvas->GetCanvas()->cd();
   //	fFwhm->DrawCalibration("*");
   //	fLegend->Draw();
   //}
}

void TChannelTab::CreateSourceTab(size_t source)
{
   if(fProjections[source]->GetEntries() > 1000) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
         std::cout << DYELLOW << "Creating source tab " << source << ", using fParent " << fParent << std::flush;
         if(fParent != nullptr) {
            std::cout << ", peak ratio " << fParent->PeakRatio();
         }
         std::cout << std::endl;
      }
      TGCompositeFrame* tmpTab = nullptr;
      {
         //if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Creating unique lock on graphics mutex to add tab" << std::endl; }
         //std::unique_lock<std::mutex> graphicsLock(fParent->GraphicsMutex());
         tmpTab = fSourceTab->AddTab(Form("%s_%s", fNuclei[source]->GetName(), fName.c_str()));
         //if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Releasing unique lock on graphics mutex after adding tab" << std::endl; }
      }
      fSources[source] = new TSourceTab(this, tmpTab, fProjections[source], fNuclei[source]->GetName(), fSigma, fThreshold, fParent->PeakRatio(), fSourceEnergies[source]);
      {
         //if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Creating unique lock on graphics mutex to increment status bar" << std::endl; }
         //std::unique_lock<std::mutex> graphicsLock(fParent->GraphicsMutex());
         fProgressBar->Increment(1);
         //if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Releasing unique lock on graphics mutex after incrementing status bar" << std::endl; }
      }
   } else {
      fSources[source] = nullptr;
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
         std::cout << DYELLOW << "Skipping projection of source " << source << " = " << fProjections[source]->GetName() << ", only " << fProjections[source]->GetEntries() << " entries" << std::endl;
      }
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << __PRETTY_FUNCTION__ << " source " << source << " done" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
}

void TChannelTab::MakeConnections()
{
   //std::unique_lock<std::mutex> graphicsLock(fParent->GraphicsMutex());
   fSourceTab->Connect("Selected(Int_t)", "TChannelTab", this, "SelectedTab(Int_t)");
   fCalibrationCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TChannelTab", this, "CalibrationStatus(Int_t,Int_t,Int_t,TObject*)");
   fCalibrationCanvas->Connect("ProcessedEvent(Event_t*)", "TChannelTab", this, "SelectCanvas(Event_t*)");
   if(fData != nullptr) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
         std::cout << DYELLOW << __PRETTY_FUNCTION__ << ": connecting fData " << fData << ":" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
         fData->Print();
      }
      fData->Connect("RemovePoint(Int_t, Int_t)", "TChannelTab", this, "RemovePoint(Int_t, Int_t)");
   } else {
      std::cerr << DRED << "Failed to create connections for the data graph fData since it hasn't been created yet!" << std::endl;
   }
}

void TChannelTab::Disconnect()
{
   //std::unique_lock<std::mutex> graphicsLock(fParent->GraphicsMutex());
   fSourceTab->Disconnect("Selected(Int_t)", this, "SelectedTab(Int_t)");
   fCalibrationCanvas->GetCanvas()->Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this, "CalibrationStatus(Int_t,Int_t,Int_t,TObject*)");
   fData->Disconnect("RemovePoint(Int_t, Int_t)", this, "RemovePoint(Int_t, Int_t)");
}

void TChannelTab::SelectedTab(Int_t id)
{
   /// Simple function that enables and disables the previous and next buttons depending on which tab was selected.
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DYELLOW << __PRETTY_FUNCTION__ << ": id " << id << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fActiveSourceTab = id;
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "active source tab " << fActiveSourceTab << RESET_COLOR << std::endl; }
   if(fSources[fActiveSourceTab] == nullptr) {
      std::cout << "Failed to get fSources[" << fActiveSourceTab << "]" << RESET_COLOR << std::endl;
      return;
   }
   if(fSources[fActiveSourceTab]->ProjectionCanvas() == nullptr) {
      std::cout << "Failed to get fSources[" << fActiveSourceTab << "]->ProjectionCanvas()" << RESET_COLOR << std::endl;
      return;
   }
   if(fSources[fActiveSourceTab]->ProjectionCanvas()->GetCanvas() == nullptr) {
      std::cout << "Failed to get fSources[" << fActiveSourceTab << "]->ProjectionCanvas()->GetCanvas()" << RESET_COLOR << std::endl;
      return;
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DGREEN << "Changing gpad from  \"" << gPad->GetName() << "\" to \""; }
   gPad = fSources[fActiveSourceTab]->ProjectionCanvas()->GetCanvas();
   fSources[fActiveSourceTab]->ProjectionCanvas()->GetCanvas()->cd();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << gPad->GetName() << "\"" << RESET_COLOR << std::endl; }
}

void TChannelTab::SelectCanvas(Event_t* event)
{
   if(event->fType == kEnterNotify) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
         std::cout << "Entered channel tab => changing gPad from " << gPad->GetName();
      }
      gPad = fCalibrationCanvas->GetCanvas();
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
         std::cout << " to " << gPad->GetName() << std::endl;
      }
   }
}

void TChannelTab::RemovePoint(Int_t oldGraph, Int_t oldPoint)
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << DCYAN << __PRETTY_FUNCTION__ << ": oldGraph " << oldGraph << ", oldPoint " << oldPoint << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   if(0 <= oldGraph && oldGraph < static_cast<int>(fSources.size())) {
      if(oldPoint >= 0) {
         return fSources[oldGraph]->RemovePoint(oldPoint);
      }
      std::cout << "Can't remove negative point " << oldPoint << " from graph " << oldGraph << std::endl;
   }
   std::cout << "Graph the point was removed from was " << oldGraph << ", but we only have " << fSources.size() << " source tabs?" << std::endl;
}

void TChannelTab::CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected)
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
   fChannelStatusBar->SetText(selected->GetName(), 0);
   if(event == kKeyPress) {
      fChannelStatusBar->SetText(Form("%c", static_cast<char>(px)), 1);
   } else {
      auto* canvas = fCalibrationCanvas->GetCanvas();
      if(canvas == nullptr) {
         fChannelStatusBar->SetText("canvas nullptr");
         return;
      }
      auto* pad = canvas->GetSelectedPad();
      if(pad == nullptr) {
         fChannelStatusBar->SetText("pad nullptr");
         return;
      }
      //if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) {
      //	std::cout << "px " << px << ", py " << py << ";    gPad: px " << gPad->AbsPixeltoX(px) << ", py " << gPad->AbsPixeltoY(py) << ";    fCalibrationCanvas: px " << fCalibrationCanvas->GetCanvas()->AbsPixeltoX(px) << ", py " << fCalibrationCanvas->GetCanvas()->AbsPixeltoY(py) << ";    fResidualPad: px " << fResidualPad->AbsPixeltoX(px) << ", py " << fResidualPad->AbsPixeltoY(py) << ";    fCalibrationPad: px " << fCalibrationPad->AbsPixeltoX(px) << ", py " << fCalibrationPad->AbsPixeltoY(py) << ";    pad: px " << pad->AbsPixeltoX(px) << ", py " << pad->AbsPixeltoY(py) << std::endl;
      //}

      fChannelStatusBar->SetText(Form("x = %f, y = %f", pad->AbsPixeltoX(px), pad->AbsPixeltoY(py)), 1);
   }
}

void TChannelTab::UpdateData()
{
   /// Copy data from all sources into one graph (which we use for the calib && source < fSources.size()ration).
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DYELLOW << __PRETTY_FUNCTION__ << " fData = " << fData << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fData == nullptr) {
      fData = new TCalibrationGraphSet("ADC channel", "Energy [keV]");
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " created fData = " << fData << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   fData->Clear();

   fData->SetName(Form("data%s", fName.c_str()));
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "set name of fData using " << fName.c_str() << ": " << fData->GetName() << std::endl;
      std::cout << "fData " << fData << ": " << (fData != nullptr ? fData->GetN() : -2) << " data points after creation" << std::endl;
      std::cout << "Looping over " << fNuclei.size() << " sources, fSources.size() = " << fSources.size() << std::endl;
   }
   for(size_t source = 0; source < fNuclei.size() && source < fSources.size(); ++source) {
      if(fSources[source]->Data() != nullptr && fSources[source]->Data()->GetN() > 0) {
         int index = fData->Add(fSources[source]->Data(), fNuclei[source]->GetName());
         if(index >= 0) {
            fData->SetLineColor(index, static_cast<Color_t>(source + 1));   //+1 for the color so that we start with 1 = black instead of 0 = white
            fData->SetMarkerColor(index, static_cast<Color_t>(source + 1));
         }
      }
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "fData " << fData << ": " << (fData != nullptr ? fData->GetN() : -1) << " data points after adding " << fNuclei.size() << " graphs" << std::endl;
      fData->Print();
   }
}

void TChannelTab::UpdateFwhm()
{
   /// Copy data from all sources into one graph (which we use for the calib && source < fSources.size()ration).
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DYELLOW << __PRETTY_FUNCTION__ << " fFwhm = " << fFwhm << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fFwhm == nullptr) {
      fFwhm = new TCalibrationGraphSet("ADC channel", "FWHM [ADC channel]");
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " created fFwhm = " << fFwhm << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   fFwhm->Clear();

   fFwhm->SetName(Form("fwhm%s", fName.c_str()));
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "set name of fFwhm using " << fName.c_str() << ": " << fFwhm->GetName() << std::endl;
      std::cout << "fFwhm " << fFwhm << ": " << (fFwhm != nullptr ? fFwhm->GetN() : -2) << " data points after creation" << std::endl;
      std::cout << "Looping over " << fNuclei.size() << " sources, fSources.size() = " << fSources.size() << std::endl;
   }
   for(size_t source = 0; source < fNuclei.size() && source < fSources.size(); ++source) {
      if(fSources[source]->Fwhm() != nullptr && fSources[source]->Fwhm()->GetN() > 0) {
         int index = fFwhm->Add(fSources[source]->Fwhm(), fNuclei[source]->GetName());
         if(index >= 0) {
            fFwhm->SetLineColor(index, static_cast<Color_t>(source + 1));   //+1 for the color so that we start with 1 = black instead of 0 = white
            fFwhm->SetMarkerColor(index, static_cast<Color_t>(source + 1));
         }
      }
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "fFwhm " << fFwhm << ": " << (fFwhm != nullptr ? fFwhm->GetN() : -1) << " data points after adding " << fNuclei.size() << " graphs" << std::endl;
      fFwhm->Print();
   }
}

void TChannelTab::Calibrate(const int& degree, const bool& force)
{
   if(degree != fDegree || force) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << DYELLOW;
         if(force) {
            std::cout << "forced calibration" << std::endl;
         } else {
            std::cout << "changed degree of polynomial: " << degree << " != " << fDegree << std::endl;
         }
      }
      fDegree = degree;
      Calibrate();
   } else if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << degree << " == " << fDegree << " and not forcing (" << force << "): not fitting channel " << fName << std::endl;
   }
}

void TChannelTab::Calibrate()
{
   /// This function fit's the final data of the given channel. It requires all other elements to have been created already.
   if(TSourceCalibration::VerboseLevel() >= EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   TF1* calibration = new TF1("fitfunction", ::Polynomial, 0., 10000., fDegree + 2);
   calibration->FixParameter(0, fDegree);
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) {
      std::cout << DYELLOW << "fData " << fData << ": ";
      if(fData != nullptr) {
         std::cout << fData->GetN() << " data points being fit, ";
      }
      double min = 0.;
      double max = 0.;
      calibration->GetRange(min, max);
      std::cout << "range " << min << " - " << max << std::endl;
   }
   fData->Fit(calibration, "Q");
   TString text = Form("%.6f + %.6f*x", calibration->GetParameter(1), calibration->GetParameter(2));
   for(int i = 2; i <= fDegree; ++i) {
      text.Append(Form(" + %.6f*x^%d", calibration->GetParameter(i + 1), i));
   }
   fChannelStatusBar->SetText(text.Data(), 2);
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "re-calculating residuals and clearing fields" << std::endl; }
   // re-calculate the residuals
   fData->SetResidual(true);

   fLegend->Clear();
   fCalibrationPad->cd();
   fData->DrawCalibration("*", fLegend);
   fLegend->Draw();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "set chi2 label" << std::endl; }
   // calculate the corners of the chi^2 label from the minimum and maximum x/y-values of the graph
   // we position it in the top left corner about 50% of the width and 10% of the height of the graph
   if(fChi2Label == nullptr) {
      double left   = fData->GetMinimumX();
      double right  = left + (fData->GetMaximumX() - left) * 0.5;
      double top    = fData->GetMaximumY();
      double bottom = top - (top - fData->GetMinimumY()) * 0.1;

      fChi2Label = new TPaveText(left, bottom, right, top);
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << "fChi2Label created " << fChi2Label << " (" << left << " - " << right << ", " << bottom << " - " << top << ", from " << fData->GetMinimumX() << "-" << fData->GetMaximumX() << ", " << fData->GetMinimumY() << "-" << fData->GetMaximumY() << ") on gPad " << gPad->GetName() << std::endl;
         fData->Print("e");
      }
   } else {
      fChi2Label->Clear();
   }
   fChi2Label->AddText(Form("#chi^{2}/NDF = %f", calibration->GetChisquare() / calibration->GetNDF()));
   fChi2Label->SetFillColor(kWhite);
   fChi2Label->Draw();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "fChi2Label set to:" << std::endl;
      fChi2Label->GetListOfLines()->Print();
      std::cout << "Text size " << fChi2Label->GetTextSize() << std::endl;
   }

   fResidualPad->cd();
   fData->DrawResidual("*");

   fCalibrationCanvas->GetCanvas()->Modified();

   fData->FitFunction()->SetRange(0., 10000.);
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      double min = 0.;
      double max = 0.;
      calibration->GetRange(min, max);
      std::cout << DYELLOW << "calibration has range " << min << " - " << max;
      if(fData->FitFunction() != nullptr) {
         fData->FitFunction()->GetRange(min, max);
         std::cout << ",  fit function has range " << min << " - " << max;
      }
      std::cout << std::endl;
   }
   delete calibration;

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " done" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

void TChannelTab::Remove(const double& maxResidual)
{
   /// This function goes through the residuals of the calibration and removes each point that is above the maxResidual parameter.
   /// The function requires an initial calibration to have residuals available.

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DYELLOW << __PRETTY_FUNCTION__ << ", max. residual " << maxResidual << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   // loop over all points of the total residual graph and remove those that are above the maximum acceptable value
   // we only want to increment the point index if we do not remove the point, otherwise the same index needs to be checked again
   for(int point = 0; point < fData->TotalResidualGraph()->GetN();) {
      if(std::abs(fData->TotalResidualGraph()->GetX()[point]) > maxResidual) {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "std::abs(" << fData->TotalResidualGraph()->GetX()[point] << ") > " << maxResidual << ": removing point " << point << "/" << fData->TotalResidualGraph()->GetN() << std::endl; }
         fData->RemovePoint(point);
      } else {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "std::abs(" << fData->TotalResidualGraph()->GetX()[point] << ") <= " << maxResidual << ": not removing point " << point << "/" << fData->TotalResidualGraph()->GetN() << std::endl; }
         ++point;
      }
   }

   // update the data, and re-calibrate
   UpdateData();
   UpdateFwhm();
   Calibrate();

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "Remove(" << maxResidual << ") is done" << RESET_COLOR << std::endl; }
}

void TChannelTab::UpdateChannel()
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DYELLOW << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   // write the actual parameters of the fit
   std::stringstream str;
   str << std::hex << fName;
   int address = 0;
   str >> address;
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "Got address " << hex(address, 4) << " from name " << fName << std::endl; }
   TF1* calibration = fData->FitFunction();
   if(calibration == nullptr) {
      std::cout << "Failed to find calibration in fData" << std::endl;
      fData->TotalGraph()->GetListOfFunctions()->Print();
      return;
   }
   std::vector<Float_t> parameters;
   for(int i = 0; i <= calibration->GetParameter(0); ++i) {
      parameters.push_back(static_cast<Float_t>(calibration->GetParameter(i + 1)));
   }
   TChannel* channel = TChannel::GetChannel(address, false);
   if(channel == nullptr) {
      std::cerr << "Failed to get channel for address " << hex(address, 4) << std::endl;
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << ": done" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      return;
   }
   channel->SetENGCoefficients(parameters);
   channel->DestroyEnergyNonlinearity();
   if(fParent->WriteNonlinearities()) {
      double* x = fData->GetX();
      double* y = fData->GetY();
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "Going to add " << fData->GetN() << " points to nonlinearity graph" << std::endl; }
      for(int i = 0; i < fData->GetN(); ++i) {
         // nonlinearity expects y to be the true source energy minus the calibrated energy of the peak
         // the source energy is y, the peak is x
         channel->AddEnergyNonlinearityPoint(y[i], y[i] - calibration->Eval(x[i]));
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) { std::cout << "Added " << channel->GetEnergyNonlinearity().GetN() << ". point " << y[i] << ", " << y[i] - calibration->Eval(x[i]) << " = " << y[i] << " - " << calibration->Eval(x[i]) << std::endl; }
      }
   }
}

void TChannelTab::Initialize(const double& sigma, const double& threshold, const double& peakRatio, const bool& force, const bool& fast)
{
   // loop through sources and get rough calibration for each
   for(auto* source : fSources) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << DYELLOW << "Rough calibration in source tab:" << std::endl;
         source->Print();
      }
      source->InitialCalibration(sigma, threshold, peakRatio, force, fast);
      fProgressBar->Increment(1);
   }
   // get this rough calibration data and calibrate
   UpdateData();
   Calibrate();
   // copy the data to the initial data and draw it
   fInit = static_cast<TCalibrationGraphSet*>(fData->Clone(Form("init%s", fName.c_str())));
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "Cloned data " << fData->GetName() << " to " << fInit->GetName() << ":" << std::endl;
      fInit->Print();
   }
   auto* oldPad = gPad;
   fInitCanvas->GetCanvas()->cd();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "Switched from " << oldPad->GetName() << " to " << gPad->GetName() << std::endl; }
   fInit->DrawCalibration("*", fLegend);
   // calculate the corners of the label from the minimum and maximum x/y-values of the graph
   // we position it in the top left corner about 50% of the width and 10% of the height of the graph
   if(fCalLabel == nullptr) {
      double left   = fInit->GetMinimumX();
      double right  = left + (fInit->GetMaximumX() - left) * 0.5;
      double top    = fInit->GetMaximumY();
      double bottom = top - (top - fInit->GetMinimumY()) * 0.1;

      fCalLabel = new TPaveText(left, bottom, right, top);
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << "fCalLabel created " << fCalLabel << " (" << left << " - " << right << ", " << bottom << " - " << top << ", from " << fInit->GetMinimumX() << "-" << fInit->GetMaximumX() << ", " << fInit->GetMinimumY() << "-" << fInit->GetMaximumY() << ") on gPad " << gPad->GetName() << std::endl;
      }
   } else {
      fCalLabel->Clear();
   }
   std::stringstream str;
   if(fInit->FitFunction()->GetNpar() == 3) {
      str << "Fit function: " << fInit->FitFunction()->GetParameter(1) << " + " << fInit->FitFunction()->GetParameter(2) << " x";
   } else {
      str << "Unknown fit function with " << fInit->FitFunction()->GetNpar() << " parameters";
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << "Created label text \"" << str.str() << "\"" << std::endl;
   }

   fCalLabel->AddText(str.str().c_str());
   fCalLabel->SetFillColor(kWhite);
   fCalLabel->Draw();
   oldPad->cd();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << "Switched back to " << gPad->GetName() << std::endl; }
}

void TChannelTab::FindPeaks(const double& sigma, const double& threshold, const double& peakRatio, const bool& force, const bool& fast)
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << DYELLOW << "Finding peaks in source tab " << fActiveSourceTab << ". Old: " << fSourceTab->GetCurrent() << " = " << fSources[fSourceTab->GetCurrent()] << ", current tab element " << fSourceTab->GetCurrentTab() << " is enabled = " << fSourceTab->GetCurrentTab()->IsEnabled() << std::endl;
      fSourceTab->Print();
      for(int tab = 0; tab < fSourceTab->GetNumberOfTabs(); ++tab) {
         std::cout << "Tab " << tab << " = " << fSourceTab->GetTabTab(tab) << " = " << fSourceTab->GetTabTab(tab)->GetText()->GetString() << (fSourceTab->GetTabTab(tab)->IsActive() ? " is active" : " is inactive") << (fSourceTab->GetTabTab(tab)->IsEnabled() ? " is enabled" : " is not enabled") << std::endl;
      }
   }
   fSources[fActiveSourceTab]->FindPeaks(sigma, threshold, peakRatio, force, fast);
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "Found peaks in source tab " << fActiveSourceTab << ", updating data" << std::endl; }
   UpdateData();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "Updating FWHM" << std::endl; }
   UpdateFwhm();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "Source tab " << fActiveSourceTab << " done!" << std::endl; }
}

void TChannelTab::FindAllPeaks(const double& sigma, const double& threshold, const double& peakRatio, const bool& force, const bool& fast)
{
   for(auto* source : fSources) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << DYELLOW << "Finding peaks in source tab:" << std::endl;
         source->Print();
      }
      source->FindPeaks(sigma, threshold, peakRatio, force, fast);
      fProgressBar->Increment(1);
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "Found peaks in all source tabs, updating data" << std::endl; }
   UpdateData();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "Updating FWHM" << std::endl; }
   UpdateFwhm();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "Source tab " << fActiveSourceTab << " done!" << std::endl; }
}

void TChannelTab::FindCalibratedPeaks()
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << DYELLOW << "Finding calibrated peaks in source tab " << fActiveSourceTab << ". Old: " << fSourceTab->GetCurrent() << " = " << fSources[fSourceTab->GetCurrent()] << ", current tab element " << fSourceTab->GetCurrentTab() << " is enabled = " << fSourceTab->GetCurrentTab()->IsEnabled() << std::endl;
      fSourceTab->Print();
      for(int tab = 0; tab < fSourceTab->GetNumberOfTabs(); ++tab) {
         std::cout << "Tab " << tab << " = " << fSourceTab->GetTabTab(tab) << " = " << fSourceTab->GetTabTab(tab)->GetText()->GetString() << (fSourceTab->GetTabTab(tab)->IsActive() ? " is active" : " is inactive") << (fSourceTab->GetTabTab(tab)->IsEnabled() ? " is enabled" : " is not enabled") << std::endl;
      }
   }
   fSources[fActiveSourceTab]->FindCalibratedPeaks(fData->FitFunction());
   UpdateData();
   UpdateFwhm();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << DYELLOW << "Done finding calibrated peaks in source tab " << fActiveSourceTab << ". Old: " << fSourceTab->GetCurrent() << " = " << fSources[fSourceTab->GetCurrent()] << ", current tab element " << fSourceTab->GetCurrentTab() << " is enabled = " << fSourceTab->GetCurrentTab()->IsEnabled() << std::endl;
   }
}

void TChannelTab::FindAllCalibratedPeaks()
{
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << DYELLOW << "Finding calibrated peaks in all source tabs" << std::endl;
   }
   for(auto* source : fSources) {
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
         std::cout << DYELLOW << "Finding calibrated peaks in source tab " << source->SourceName() << std::endl;
      }
      source->FindCalibratedPeaks(fData->FitFunction());
   }
   UpdateData();
   UpdateFwhm();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
      std::cout << DYELLOW << "Done finding calibrated peaks in all source tabs" << std::endl;
   }
}

void TChannelTab::Draw()
{
   fFwhmCanvas->GetCanvas()->cd();
   fFwhm->DrawCalibration("*");
   fLegend->Draw();
   for(auto* source : fSources) {
      source->Draw();
   }
}

void TChannelTab::Write(TFile* output)
{
   /// Write graphs to output.
   output->cd();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "writing " << fName << std::endl; }
   fData->Write(Form("calGraph_%s", fName.c_str()), TObject::kOverwrite);
   fFwhm->Write(Form("fwhm_%s", fName.c_str()), TObject::kOverwrite);
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) { std::cout << DYELLOW << "wrote data " << fName << std::endl; }
}

void TChannelTab::ZoomX()
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
      std::cout << "ZoomX - Failed to find histogram for pad " << gPad->GetName() << std::endl;
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) { gPad->GetListOfPrimitives()->Print(); }
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
      std::cout << "ZoomX - Failed to find histogram for partner pad " << newName << std::endl;
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) { pad2->GetListOfPrimitives()->Print(); }
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

void TChannelTab::ZoomY()
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
      std::cout << "ZoomY - Failed to find histogram for pad " << gPad->GetName() << std::endl;
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) { gPad->GetListOfPrimitives()->Print(); }
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
      std::cout << "ZoomY - Failed to find histogram for partner pad " << newName << std::endl;
      if(TSourceCalibration::VerboseLevel() > EVerbosity::kLoops) { pad2->GetListOfPrimitives()->Print(); }
      return;
   }

   hist2->SetMinimum(hist1->GetMinimum());
   hist2->SetMaximum(hist1->GetMaximum());

   pad2->Modified();
   pad2->Update();
}

void TChannelTab::PrintCanvases() const
{
   std::cout << "TChannelTab " << Name() << " calibration canvas:" << std::endl;
   fCalibrationCanvas->GetCanvas()->Print();
   for(auto* obj : *(fCalibrationCanvas->GetCanvas()->GetListOfPrimitives())) {
      obj->Print();
   }
   std::cout << "TChannelTab fwhm canvas:" << std::endl;
   fFwhmCanvas->GetCanvas()->Print();
   for(auto* obj : *(fFwhmCanvas->GetCanvas()->GetListOfPrimitives())) {
      obj->Print();
   }
   for(auto* sourceTab : fSources) {
      sourceTab->PrintCanvases();
   }
}

void TChannelTab::PrintLayout() const
{
   std::cout << "TChannelTab frame:" << std::endl;
   fChannelFrame->Print();
   std::cout << "TChannelTab canvas:" << std::endl;
   fCalibrationCanvas->Print();
   std::cout << "TChannelTab status bar:" << std::endl;
   //fChannelStatusBar->Print();
   for(auto* sourceTab : fSources) {
      sourceTab->PrintLayout();
   }
}

//////////////////////////////////////// TSourceCalibration ////////////////////////////////////////
std::string TSourceCalibration::fLogFile;
EVerbosity  TSourceCalibration::fVerboseLevel       = EVerbosity::kQuiet;
int         TSourceCalibration::fPanelWidth         = 600;
int         TSourceCalibration::fPanelHeight        = 400;
int         TSourceCalibration::fStatusbarHeight    = 50;
int         TSourceCalibration::fSourceboxWidth     = 100;
int         TSourceCalibration::fParameterHeight    = 200;
int         TSourceCalibration::fDigitWidth         = 5;
int         TSourceCalibration::fMaxIterations      = 50;
int         TSourceCalibration::fFitRange           = 4;
bool        TSourceCalibration::fAcceptBadFits      = false;
bool        TSourceCalibration::fFast               = false;
bool        TSourceCalibration::fUseCalibratedPeaks = false;
double      TSourceCalibration::fMinIntensity       = 5.;
size_t      TSourceCalibration::fNumberOfThreads    = 4;

TSourceCalibration::TSourceCalibration(double sigma, double threshold, int degree, double peakRatio, int count...)
   : TGMainFrame(nullptr, 2 * fPanelWidth, fPanelHeight + 2 * fStatusbarHeight), fDefaultSigma(sigma), fDefaultThreshold(threshold), fDefaultDegree(degree), fDefaultPeakRatio(peakRatio)
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
   if(fVerboseLevel > EVerbosity::kQuiet) {
      std::cout << DGREEN << __PRETTY_FUNCTION__ << ": verbose level " << fVerboseLevel << std::endl   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
                << "using " << count << "/" << fMatrices.size() << " matrices:" << std::endl;
      for(auto* mat : fMatrices) {
         std::cout << mat << std::flush << " = " << mat->GetName() << std::endl;
      }
   }

   fOldErrorLevel        = gErrorIgnoreLevel;
   gErrorIgnoreLevel     = kError;
   gPrintViaErrorHandler = true;   // redirects all printf's to root's normal message system

   if(!fLogFile.empty()) {
      fRedirect = new TRedirect(fLogFile.c_str());
      std::cout << "Starting redirect to " << fLogFile << std::endl;
   }

   // check matrices (# of filled bins and bin labels) and resize some vectors for later use
   // use the first matrix to get a reference for everything
   fNofBins = 0;
   std::map<int, int> channelToIndex;   // used to be a member, but only used here
   for(int bin = 1; bin <= fMatrices[0]->GetNbinsX(); ++bin) {
      if(FilledBin(fMatrices[0], bin)) {
         fActualChannelId.push_back(fNofBins);   // at this point fNofBins is the index at which this projection will end up
         fActiveBins.push_back(bin);
         channelToIndex[bin] = fNofBins;   // this map simply stores which bin ends up at which index
         fChannelLabel.push_back(fMatrices[0]->GetXaxis()->GetBinLabel(bin));
         if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << bin << ". bin: fNofBins " << fNofBins << ", channelToIndex[" << bin << "] " << channelToIndex[bin] << ", fActualChannelId.back() " << fActualChannelId.back() << std::endl; }
         ++fNofBins;
      } else {
         if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << "skipping bin " << bin << std::endl; }
      }
   }
   // now loop over all other matrices and check vs. the first one
   for(size_t i = 1; i < fMatrices.size(); ++i) {
      int tmpBins = 0;
      for(int bin = 1; bin <= fMatrices[i]->GetNbinsX(); ++bin) {
         if(FilledBin(fMatrices[i], bin)) {   // good bin in the current matrix
            // current index is tmpBins, so we check if the bin agrees with what we have
            if(channelToIndex.find(bin) == channelToIndex.end() || tmpBins != channelToIndex[bin]) {   // found a full bin, but the corresponding bin in the first matrix isn't full or a different bin
               std::ostringstream error;
               error << "Mismatch in " << i << ". matrix (" << fMatrices[i]->GetName() << "), bin " << bin << " is " << tmpBins << ". filled bin, but should be " << (channelToIndex.find(bin) == channelToIndex.end() ? "not filled" : Form("%d", channelToIndex[bin])) << std::endl;
               throw std::invalid_argument(error.str());
            }
            if(strcmp(fMatrices[0]->GetXaxis()->GetBinLabel(bin), fMatrices[i]->GetXaxis()->GetBinLabel(bin)) != 0) {   // bin is full and matches the bin in the first matrix so we check the labels
               std::ostringstream error;
               error << i << ". matrix, " << bin << ". bin: label (" << fMatrices[i]->GetXaxis()->GetBinLabel(bin) << ") doesn't match bin label of the first matrix (" << fMatrices[0]->GetXaxis()->GetBinLabel(bin) << ")" << std::endl;
               throw std::invalid_argument(error.str());
            }
            ++tmpBins;
         } else {
            if(channelToIndex.find(bin) != channelToIndex.end()) {   //found an empty bin that was full in the first matrix
               std::ostringstream error;
               error << "Mismatch in " << i << ". matrix (" << fMatrices[i]->GetName() << "), bin " << bin << " is empty, but should be " << channelToIndex[bin] << ". filled bin" << std::endl;
               throw std::invalid_argument(error.str());
            }
         }
      }
   }

   if(fVerboseLevel > EVerbosity::kQuiet) { std::cout << fMatrices.size() << " matrices with " << fMatrices[0]->GetNbinsX() << " x-bins, fNofBins " << fNofBins << ", fActualChannelId.size() " << fActualChannelId.size() << std::endl; }

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

   delete fRedirect;
}

void TSourceCalibration::BuildFirstInterface()
{
   /// Build initial interface with histogram <-> source assignment

   auto* layoutManager = new TGTableLayout(this, fMatrices.size() + 1, 2, true, 5);
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << "created table layout manager with 2 columns, " << fMatrices.size() + 1 << " rows" << std::endl; }
   SetLayoutManager(layoutManager);

   // The matrices and source selection boxes
   size_t i = 0;
   for(i = 0; i < fMatrices.size(); ++i) {
      fMatrixNames.push_back(new TGLabel(this, fMatrices[i]->GetName()));
      if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Text height " << fMatrixNames.back()->GetFont()->TextHeight() << std::endl; }
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
            if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << i << "/" << index << ": Comparing matrix name " << fMatrices[i]->GetName() << " to file name " << file.path().stem().c_str() << std::endl; }
            if(std::strstr(fMatrices[i]->GetName(), file.path().stem().c_str()) != nullptr) {
               fSourceBox.back()->Select(index);
               SetSource(kSourceBox + fSourceBox.size() - 1, index);
               if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << i << ": selected source " << index << std::endl; }
            } else if(fVerboseLevel > EVerbosity::kSubroutines) {
               std::cout << "matrix name " << fMatrices[i]->GetName() << " not matching " << file.path().stem().c_str() << std::endl;
            }
            ++index;
         }
      }
      if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << i << ": created list with " << index << " sources" << std::endl; }
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

      fSourceBox.back()->SetMinHeight(fPanelHeight / 2);

      fSourceBox.back()->Resize(fSourceboxWidth, fLineHeight);
      if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Attaching " << i << ". label to 0, 1, " << i << ", " << i + 1 << ", and box to 1, 2, " << i << ", " << i + 1 << std::endl; }
      AddFrame(fMatrixNames.back(), new TGTableLayoutHints(0, 1, i, i + 1, kLHintsRight | kLHintsCenterY));
      AddFrame(fSourceBox.back(), new TGTableLayoutHints(1, 2, i, i + 1, kLHintsLeft | kLHintsCenterY));
   }

   // The buttons
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << "Attaching start button to 0, 2, " << i << ", " << i + 1 << std::endl; }
   fStartButton = new TGTextButton(this, "Accept && Continue", kStartButton);
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << "start button " << fStartButton << std::endl; }
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
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Deleted start button " << fStartButton << std::endl; }
}

void TSourceCalibration::SetSource(Int_t windowId, Int_t entryId)
{
   int index = windowId - kSourceBox;
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << ": windowId " << windowId << ", entryId " << entryId << " => " << index << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
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
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << ": fEmitter " << fEmitter << ", fStartButton " << fStartButton << ", fAcceptAllButton " << fAcceptAllButton << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fEmitter == fStartButton) {
      SecondWindow();
   } else if(fEmitter == fAcceptAllButton) {
      for(auto& channelTab : fChannelTab) {
         if(channelTab == nullptr) { continue; }
         channelTab->UpdateChannel();
         channelTab->Write(fOutput);
      }
      WriteCalibration();
      std::cout << "Closing window" << std::endl;
      CloseWindow();
      std::cout << "all done" << std::endl;
      exit(0);
   }
}

void TSourceCalibration::Start()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << ": fEmitter " << fEmitter << ", fStartButton " << fStartButton << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fEmitter == nullptr) {                                                                                                                                         // we only want to do this once at the beginning (after fEmitter was initialized to nullptr)
      fEmitter = fStartButton;
      TTimer::SingleShot(fWaitMs, "TSourceCalibration", this, "HandleTimer()");
   }
}

void TSourceCalibration::SecondWindow()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
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
   fProgressBar = new TGHProgressBar(this, TGProgressBar::kFancy, fPanelWidth);
   fProgressBar->SetRange(0., static_cast<Float_t>(fMatrices.size() * fActiveBins.size()));
   fProgressBar->Percent(true);
   fProgressBar->ShowPosition(true, true, "Graphics: %.0f");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Set range of progress bar to 0. - " << fProgressBar->GetMax() << " = " << fMatrices.size() * fActiveBins.size() << " = " << fMatrices.size() << "*" << fActiveBins.size() << std::endl; }
   AddFrame(fProgressBar, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(TGDimension(fPanelWidth, fLineHeight));

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
   Resize(TGDimension(2 * fPanelWidth, fPanelHeight + 2 * fStatusbarHeight));

   // Map main frame
   MapWindow();
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << DGREEN << __PRETTY_FUNCTION__ << " done" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

void TSourceCalibration::BuildSecondInterface()
{
   SetLayoutManager(new TGVerticalLayout(this));
   fTab = new TGTab(this, 2 * fPanelWidth, fPanelHeight + 2 * fStatusbarHeight);

   fChannelTab.resize(fMatrices[0]->GetNbinsX());
   for(auto& bin : fActiveBins) {
      // create vector with projections of this channel for each source
      std::vector<GH1D*> projections(fMatrices.size());
      size_t             index = 0;
      for(auto* matrix : fMatrices) {
         projections[index] = new GH1D(matrix->ProjectionY(Form("%s_%s", fSource[index]->GetName(), matrix->GetXaxis()->GetBinLabel(bin)), bin, bin));
         ++index;
      }
      auto* frame = fTab->AddTab(fMatrices[0]->GetXaxis()->GetBinLabel(bin));
      // create a new tab in a new thread
      if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << "Creating channel for bin " << bin << std::endl; }
      fChannelTab[bin - 1] = new TChannelTab(this, fSource, projections, fMatrices[0]->GetXaxis()->GetBinLabel(bin), frame, fDefaultSigma, fDefaultThreshold, fDefaultDegree, fSourceEnergy, fProgressBar);
      //// right now the lambda uses copies of all variables, not sure if that is neccesary, or if we only need that for projections?
      //fFutures.emplace(std::make_pair(bin - 1, std::async(std::launch::async, [=] {
      //				std::cout << "creating channel tab with these arguments: " << this << ", fSource vector of size " << fSource.size() << ", projection vector of size " << projections.size() << ", " << fMatrices[0]->GetXaxis()->GetBinLabel(bin) << ", " << frame << ", " << fDefaultSigma << ", " << fDefaultThreshold << ", " << fDefaultDegree << ", " << fSourceEnergy.size() << " source energy vectors, " << fProgressBar << "."<< std::endl;
      //				auto* newTab = new TChannelTab(this, fSource, projections, fMatrices[0]->GetXaxis()->GetBinLabel(bin), frame, fDefaultSigma, fDefaultThreshold, fDefaultDegree, fSourceEnergy, fProgressBar);
      //				std::cout << "created new tab " << newTab << std::endl;
      //				return newTab;
      //				})));
      //// if we have too many threads running, get the result of the first one (this means waiting on that thread to finish) and assign the results
      //if(fFutures.size() >= fNumberOfThreads) {
      //	if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << fFutures.size() << " threads running, waiting for thread #" << fFutures.front().first << " to finish before starting next, max. # threads " << fNumberOfThreads << std::endl; }
      //	auto tmp = fFutures.front().second.get();
      //	fChannelTab[fFutures.front().first] = tmp;
      //	fFutures.pop();
      //}
   }
   // wait for all remaining threads to finish
   //while(!fFutures.empty()) {
   //	if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << fFutures.size() << " threads left, waiting for thread #" << fFutures.front().first << " to finish before starting next, max. # threads " << fNumberOfThreads << std::endl; }
   //	auto tmp = fFutures.front().second.get();
   //	fChannelTab[fFutures.front().first] = tmp;
   //	fFutures.pop();
   //}

   //if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << fMatrices.size() << " matrices with " << fMatrices[0]->GetNbinsX() << " x-bins, fNofBins " << fNofBins << ", fActualChannelId.size() " << fActualChannelId.size() << ", fChannelTab[0]->SourceTab()->GetNumberOfTabs() " << fChannelTab[0]->SourceTab()->GetNumberOfTabs() << std::endl; }

   //for(int i = 0; i < fChannelTab[0]->SourceTab()->GetNumberOfTabs(); ++i) {
   //if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << i << ": " << fChannelTab[0]->SourceTab()->GetTabTab(i)->GetText()->GetString() << std::endl; }
   //}

   AddFrame(fTab, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

   // bottom frame with navigation button group, text entries, etc.
   fBottomFrame = new TGHorizontalFrame(this, 2 * fPanelWidth, TSourceCalibration::StatusbarHeight());

   fLeftFrame       = new TGVerticalFrame(fBottomFrame, fPanelWidth, fParameterHeight);
   fNavigationGroup = new TGHButtonGroup(fLeftFrame, "");
   fPreviousButton  = new TGTextButton(fNavigationGroup, "&Previous");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << DGREEN << "prev button " << fPreviousButton << std::endl; }
   fPreviousButton->SetEnabled(false);
   fDiscardButton = new TGTextButton(fNavigationGroup, "&Discard");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "discard button " << fDiscardButton << std::endl; }
   fAcceptButton = new TGTextButton(fNavigationGroup, "&Accept");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "accept button " << fAcceptButton << std::endl; }
   fAcceptAllButton = new TGTextButton(fNavigationGroup, "Accept All && Finish");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "accept all button " << fAcceptAllButton << std::endl; }
   fWriteButton = new TGTextButton(fNavigationGroup, "&Write");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "write button " << fWriteButton << std::endl; }
   fNextButton = new TGTextButton(fNavigationGroup, "&Next");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "next button " << fNextButton << std::endl; }

   fLeftFrame->AddFrame(fNavigationGroup, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

   fFittingGroup        = new TGHButtonGroup(fLeftFrame, "");
   fFindPeaksButton     = new TGTextButton(fFittingGroup, "Find Peaks");
   fFindPeaksFastButton = new TGTextButton(fFittingGroup, "&Find Peaks Fast");
   fFindPeaksCalButton  = new TGTextButton(fFittingGroup, "&Use Calibration");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "find peaks button " << fFindPeaksButton << std::endl; }
   fFindPeaksCalAllButton = new TGTextButton(fFittingGroup, "Use Calibration (All)");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "find all peaks button " << fFindPeaksCalAllButton << std::endl; }
   fCalibrateButton = new TGTextButton(fFittingGroup, "&Calibrate");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "cal button " << fCalibrateButton << std::endl; }
   fRemoveButton = new TGTextButton(fFittingGroup, "&Remove");
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "remove button " << fRemoveButton << std::endl; }

   fLeftFrame->AddFrame(fFittingGroup, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));

   fBottomFrame->AddFrame(fLeftFrame, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

   fRightFrame     = new TGVerticalFrame(fBottomFrame, fPanelWidth, fParameterHeight);
   fParameterFrame = new TGGroupFrame(fRightFrame, "Parameters", kHorizontalFrame);
   fParameterFrame->SetLayoutManager(new TGMatrixLayout(fParameterFrame, 0, 4, 2));
   fSigmaLabel          = new TGLabel(fParameterFrame, "Sigma");
   fSigmaEntry          = new TGNumberEntry(fParameterFrame, fDefaultSigma, fDigitWidth, kSigmaEntry, TGNumberFormat::EStyle::kNESRealTwo, TGNumberFormat::EAttribute::kNEAPositive);
   fThresholdLabel      = new TGLabel(fParameterFrame, "Threshold");
   fThresholdEntry      = new TGNumberEntry(fParameterFrame, fDefaultThreshold, fDigitWidth, kThresholdEntry, TGNumberFormat::EStyle::kNESRealThree, TGNumberFormat::EAttribute::kNEAPositive, TGNumberFormat::ELimit::kNELLimitMinMax, 0., 1.);
   fDegreeLabel         = new TGLabel(fParameterFrame, "Degree of polynomial");
   fDegreeEntry         = new TGNumberEntry(fParameterFrame, fDefaultDegree, 2, kDegreeEntry, TGNumberFormat::EStyle::kNESInteger, TGNumberFormat::EAttribute::kNEAPositive);
   fMaxResidualLabel    = new TGLabel(fParameterFrame, "Max. residual (in sigma)");
   fMaxResidualEntry    = new TGNumberEntry(fParameterFrame, fDefaultMaxResidual, fDigitWidth, kMaxResidualEntry, TGNumberFormat::EStyle::kNESRealTwo, TGNumberFormat::EAttribute::kNEAPositive);
   fPeakRatioLabel      = new TGLabel(fParameterFrame, "Ratio found/source peaks");
   fPeakRatioEntry      = new TGNumberEntry(fParameterFrame, fDefaultPeakRatio, fDigitWidth, kPeakRatioEntry, TGNumberFormat::EStyle::kNESRealTwo, TGNumberFormat::EAttribute::kNEAPositive);
   fWriteNonlinearities = new TGCheckButton(fParameterFrame, "Write nonlinearities", kWriteNonlinearities);
   fWriteNonlinearities->SetState(kButtonUp);

   fParameterFrame->AddFrame(fSigmaLabel);
   fParameterFrame->AddFrame(fSigmaEntry);
   fParameterFrame->AddFrame(fThresholdLabel);
   fParameterFrame->AddFrame(fThresholdEntry);
   fParameterFrame->AddFrame(fDegreeLabel);
   fParameterFrame->AddFrame(fDegreeEntry);
   fParameterFrame->AddFrame(fMaxResidualLabel);
   fParameterFrame->AddFrame(fMaxResidualEntry);
   fParameterFrame->AddFrame(fPeakRatioLabel);
   fParameterFrame->AddFrame(fPeakRatioEntry);
   fParameterFrame->AddFrame(fWriteNonlinearities);

   fRightFrame->AddFrame(fParameterFrame, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 2, 2, 2, 2));

   fBottomFrame->AddFrame(fRightFrame, new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 2, 2));

   AddFrame(fBottomFrame, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));

   SelectedTab(0);

   fProgressBar->Reset();
   fProgressBar->ShowPosition(true, true, "Peaks: %.0f");
   for(auto* channelTab : fChannelTab) {
      if(channelTab != nullptr) {
         if(TSourceCalibration::VerboseLevel() > EVerbosity::kSubroutines) {
            for(int i = 0; i < 20; ++i) { std::cout << std::endl; }
            for(int i = 0; i < 5; ++i) {
               std::cout << "================================================================================" << std::endl;
            }
            for(int i = 0; i < 20; ++i) { std::cout << std::endl; }
         }
         channelTab->Initialize(Sigma(), Threshold(), PeakRatio(), true, fFast);
         channelTab->FindAllCalibratedPeaks();
         channelTab->Calibrate();
         channelTab->Draw();
         channelTab->PrintCanvases();
      }
   }

   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << DGREEN << "Second interface done" << std::endl; }
}

void TSourceCalibration::MakeSecondConnections()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fNavigationGroup->Connect("Clicked(Int_t)", "TSourceCalibration", this, "Navigate(Int_t)");
   fFittingGroup->Connect("Clicked(Int_t)", "TSourceCalibration", this, "Fitting(Int_t)");
   fTab->Connect("Selected(Int_t)", "TSourceCalibration", this, "SelectedTab(Int_t)");
   // we don't need to connect the sigma, threshold, and degree number entries, those are automatically read when we start the calibration
   for(auto* channelTab : fChannelTab) {
      if(channelTab != nullptr) {
         channelTab->MakeConnections();
      }
   }
}

void TSourceCalibration::DisconnectSecond()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fNavigationGroup->Disconnect("Clicked(Int_t)", this, "Navigate(Int_t)");
   fFittingGroup->Disconnect("Clicked(Int_t)", this, "Fitting(Int_t)");
   fTab->Disconnect("Selected(Int_t)", this, "SelectedTab(Int_t)");
   for(auto* channelTab : fChannelTab) {
      if(channelTab != nullptr) {
         channelTab->Disconnect();
      }
   }
}

void TSourceCalibration::Navigate(Int_t id)
{
   // we get the current source tab id and use it to get the channel tab from the right source tab
   // since the current id only refers to the position within the open tabs we need to keep track of the actual id it relates to
   // for this we created a vector with all ids at the beginning and now remove the position correspoding to the current id when we remove the tab
   int currentChannelId = fTab->GetCurrent();
   int actualChannelId  = fActualChannelId[currentChannelId];
   int nofTabs          = fTab->GetNumberOfTabs();
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << ": id " << id << ", channel tab id " << currentChannelId << ", actual channel tab id " << actualChannelId << ", # of tabs " << nofTabs << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Before: active source tab of tab " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->Name() << ", #" << fTab->GetCurrent() << " = bin " << fActiveBins[fTab->GetCurrent()] << ": " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->ActiveSourceTab() << std::endl; }
   switch(id) {
   case ENavigate::kPrevious:   // previous
      fTab->SetTab(currentChannelId - 1);
      SelectedTab(currentChannelId - 1);
      break;
   case ENavigate::kDiscard:   // discard
      // select the next (or if we are on the last tab, the previous) tab
      if(currentChannelId < nofTabs - 1) {
         fTab->SetTab(currentChannelId + 1);
      } else {
         fTab->SetTab(currentChannelId - 1);
      }
      // remove the original active tab
      fTab->RemoveTab(currentChannelId);
      break;
   case ENavigate::kAccept:   // accept
      AcceptChannel(currentChannelId);
      break;
   case ENavigate::kAcceptAll:   // accept all (no argument = -1 = all)
      AcceptChannel();
      return;
   case ENavigate::kWrite:   // write
      WriteCalibration();
      break;
   case ENavigate::kNext:   // next
      fTab->SetTab(currentChannelId + 1);
      SelectedTab(currentChannelId + 1);
      break;
   default:
      break;
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DGREEN << "After: active source tab of tab " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->Name() << ", #" << fTab->GetCurrent() << " = bin " << fActiveBins[fTab->GetCurrent()] << ": " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->ActiveSourceTab() << std::endl; }
}

void TSourceCalibration::Fitting(Int_t id)
{
   // we get the current source tab id and use it to get the channel tab from the right source tab
   // since the current id only refers to the position within the open tabs we need to keep track of the actual id it relates to
   // for this we created a vector with all ids at the beginning and now remove the position correspoding to the current id when we remove the tab
   int currentChannelId = fTab->GetCurrent();
   int actualChannelId  = fActualChannelId[currentChannelId];
   int nofTabs          = fTab->GetNumberOfTabs();
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << ": id " << id << ", channel tab id " << currentChannelId << ", actual channel tab id " << actualChannelId << ", # of tabs " << nofTabs << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "Before: active source tab of tab " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->Name() << ", #" << fTab->GetCurrent() << " = bin " << fActiveBins[fTab->GetCurrent()] << ": " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->ActiveSourceTab() << std::endl; }
   switch(id) {
   case EFitting::kFindPeaks:   // find peaks
      FindPeaks();
      break;
   case EFitting::kFindPeaksFast:   // find peaks fast
      FindPeaksFast();
      break;
   case EFitting::kFindPeaksCal:   // find calibrated peaks
      FindCalibratedPeaks();
      break;
   case EFitting::kFindPeaksCalAll:   // find all calibrated peaks
      FindAllCalibratedPeaks();
      break;
   case EFitting::kCalibrate:   // calibrate
      Calibrate();
      break;
   case EFitting::kRemove:   // remove outliers
      Remove();
      break;
   default:
      break;
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DGREEN << "After: active source tab of tab " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->Name() << ", #" << fTab->GetCurrent() << " = bin " << fActiveBins[fTab->GetCurrent()] << ": " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->ActiveSourceTab() << std::endl; }
}

void TSourceCalibration::SelectedTab(Int_t id)
{
   /// Simple function that enables and disables the previous and next buttons depending on which tab was selected.
   /// Also sets gPad to the projection of the source selected in this tab.
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << ": id " << id << ", nofTabs " << fTab->GetNumberOfTabs() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(id < 0) { return; }
   if(id == 0) {
      fPreviousButton->SetEnabled(false);
   } else {
      fPreviousButton->SetEnabled(true);
   }

   if(id >= fTab->GetNumberOfTabs() - 1) {
      fNextButton->SetEnabled(false);
   } else {
      fNextButton->SetEnabled(true);
   }

   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DGREEN << "active source tab of tab " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->Name() << ", #" << fTab->GetCurrent() << " = bin " << fActiveBins[fTab->GetCurrent()] << ": " << fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->ActiveSourceTab() << RESET_COLOR << std::endl; }

   if(fChannelTab[fActiveBins[id] - 1] == nullptr) {
      std::cout << "Failed to get fChannelTab[" << fActiveBins[id] - 1 << " = fActiveBins[" << id << "]-1]" << RESET_COLOR << std::endl;
      return;
   }
   if(fChannelTab[fActiveBins[id] - 1]->SelectedSourceTab() == nullptr) {
      std::cout << "Failed to get fChannelTab[" << fActiveBins[id] - 1 << " = fActiveBins[" << id << "]-1]->SelectedSourceTab()" << RESET_COLOR << std::endl;
      return;
   }
   if(fChannelTab[fActiveBins[id] - 1]->SelectedSourceTab()->ProjectionCanvas() == nullptr) {
      std::cout << "Failed to get fChannelTab[" << fActiveBins[id] - 1 << " = fActiveBins[" << id << "]-1]->SelectedSourceTab()->ProjectionCanvas()" << RESET_COLOR << std::endl;
      return;
   }
   if(fChannelTab[fActiveBins[id] - 1]->SelectedSourceTab()->ProjectionCanvas()->GetCanvas() == nullptr) {
      std::cout << "Failed to get fChannelTab[" << fActiveBins[id] - 1 << " = fActiveBins[" << id << "]-1]->SelectedSourceTab()->ProjectionCanvas()->GetCanvas()" << RESET_COLOR << std::endl;
      return;
   }
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << DGREEN << "Changing gpad from  \"" << gPad->GetName() << "\" to \""; }
   gPad = fChannelTab[fActiveBins[id] - 1]->SelectedSourceTab()->ProjectionCanvas()->GetCanvas();
   fChannelTab[fActiveBins[id] - 1]->SelectedSourceTab()->ProjectionCanvas()->GetCanvas()->cd();
   if(TSourceCalibration::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << gPad->GetName() << "\"" << RESET_COLOR << std::endl; }
}

void TSourceCalibration::AcceptChannel(const int& channelId)
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << ": channelId " << channelId << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   // select the next (or if we are on the last tab, the previous) tab
   int nofTabs    = fTab->GetNumberOfTabs();
   int minChannel = 0;
   int maxChannel = nofTabs - 1;
   if(channelId >= 0) {
      minChannel = channelId;
      maxChannel = channelId;
   }

   // we need to loop backward, because removing the first channel would make the second one the new first and so on
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": accepting channels " << maxChannel << " to " << minChannel << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   for(int currentChannelId = maxChannel; currentChannelId >= minChannel; --currentChannelId) {
      int actualChannelId = fActualChannelId[currentChannelId];
      if(fVerboseLevel > EVerbosity::kLoops) { std::cout << __PRETTY_FUNCTION__ << ": currentChannelId " << currentChannelId << ", actualChannelId " << actualChannelId << ", fChannelTab.size() " << fChannelTab.size() << ", fActualChannelId.size() " << fActualChannelId.size() << ", nofTabs " << nofTabs << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

      if(minChannel == maxChannel) {   // we don't need to select the tab if we close all
         if(currentChannelId < nofTabs) {
            fTab->SetTab(currentChannelId + 1);
            if(fVerboseLevel > EVerbosity::kLoops) { std::cout << DGREEN << __PRETTY_FUNCTION__ << ": set tab to " << currentChannelId + 1 << " = " << fTab->GetCurrent() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
         } else {
            fTab->SetTab(currentChannelId - 1);
            if(fVerboseLevel > EVerbosity::kLoops) { std::cout << DGREEN << __PRETTY_FUNCTION__ << ": set tab to " << currentChannelId - 1 << " = " << fTab->GetCurrent() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
         }
      }

      // remove the original active tab
      fTab->RemoveTab(currentChannelId);   // this sets the selected tab to zero if the deleted tab is the currently selected one, which is why we do it now (after we've selected a new tab)
      fActualChannelId.erase(fActualChannelId.begin() + currentChannelId);
      fActiveBins.erase(fActiveBins.begin() + currentChannelId);
   }
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << DGREEN << __PRETTY_FUNCTION__ << ": # of channel tabs " << fActualChannelId.size() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   // check if this changes which buttons are enabled or not (not using the variables from the beginning of this block because they might have changed by now!)
   SelectedTab(fTab->GetCurrent());
   // if this was also the last source vector we initiate the last screen
   if(fActualChannelId.empty() || fActiveBins.empty()) {
      if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "last channel tab done - writing files now" << std::endl; }
      fEmitter = fAcceptAllButton;
      TTimer::SingleShot(fWaitMs, "TSourceCalibration", this, "HandleTimer()");
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << std::flush; }
}

void TSourceCalibration::FindPeaks()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fChannelTab[fActiveBins[fTab->GetCurrent()] - 1] != nullptr) {
      fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->FindPeaks(Sigma(), Threshold(), PeakRatio(), true, false);   // force = true, fast = false
      Calibrate();
   } else {
      std::cout << __PRETTY_FUNCTION__ << ": fChannelTab[" << fActiveBins[fTab->GetCurrent()] << " = fActiveBins[" << fTab->GetCurrent() << "]] is a nullptr!" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << std::flush; }
}

void TSourceCalibration::FindPeaksFast()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fChannelTab[fActiveBins[fTab->GetCurrent()] - 1] != nullptr) {
      fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->FindPeaks(Sigma(), Threshold(), PeakRatio(), true, true);   // force = true, fast = true
      Calibrate();
   } else {
      std::cout << __PRETTY_FUNCTION__ << ": fChannelTab[" << fActiveBins[fTab->GetCurrent()] << " = fActiveBins[" << fTab->GetCurrent() << "]] is a nullptr!" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << std::flush; }
}

void TSourceCalibration::FindCalibratedPeaks()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fChannelTab[fActiveBins[fTab->GetCurrent()] - 1] != nullptr) {
      fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->FindCalibratedPeaks();
      Calibrate();
   } else {
      std::cout << __PRETTY_FUNCTION__ << ": fChannelTab[" << fActiveBins[fTab->GetCurrent()] << " = fActiveBins[" << fTab->GetCurrent() << "]] is a nullptr!" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << std::flush; }
}

void TSourceCalibration::FindAllCalibratedPeaks()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   for(auto& bin : fActiveBins) {
      if(fChannelTab[bin] != nullptr) {
         fChannelTab[bin]->FindCalibratedPeaks();
         fChannelTab[bin]->Calibrate(Degree(), true);
      } else {
         std::cout << __PRETTY_FUNCTION__ << ": fChannelTab[" << bin << "] is a nullptr!" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      }
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << std::flush; }
}

void TSourceCalibration::Calibrate()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fChannelTab[fActiveBins[fTab->GetCurrent()] - 1] != nullptr) {
      fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->Calibrate(Degree(), true);
   } else {
      std::cout << __PRETTY_FUNCTION__ << ": fChannelTab[" << fActiveBins[fTab->GetCurrent()] << " = fActiveBins[" << fTab->GetCurrent() << "]] is a nullptr!" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << std::flush; }
}

void TSourceCalibration::Remove()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fChannelTab[fActiveBins[fTab->GetCurrent()] - 1] != nullptr) {
      fChannelTab[fActiveBins[fTab->GetCurrent()] - 1]->Remove(MaxResidual());
   } else {
      std::cout << __PRETTY_FUNCTION__ << ": fChannelTab[" << fActiveBins[fTab->GetCurrent()] << " = fActiveBins[" << fTab->GetCurrent() << "]] is a nullptr!" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << RESET_COLOR << std::flush; }
}

void TSourceCalibration::WriteCalibration()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   std::ostringstream fileName;
   for(auto* source : fSource) {
      fileName << source->GetName();
   }
   fileName << ".cal";
   TChannel::WriteCalFile(fileName.str());
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << ": wrote " << fileName.str() << " with " << TChannel::GetNumberOfChannels() << " channels" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fVerboseLevel > EVerbosity::kSubroutines) { TChannel::WriteCalFile(); }
}

void TSourceCalibration::PrintLayout() const
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   Print();
   if(fBottomFrame != nullptr) { fBottomFrame->Print(); }
   if(fLeftFrame != nullptr) { fLeftFrame->Print(); }
   if(fRightFrame != nullptr) { fRightFrame->Print(); }
   if(fTab != nullptr) { fTab->Print(); }
   std::cout << "TSourceCalibration all channel tabs:" << std::endl;
   for(auto* channelTab : fChannelTab) {
      if(channelTab != nullptr) { channelTab->PrintLayout(); }
   }
}

void TSourceCalibration::PrintCanvases() const
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << DGREEN << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

   for(auto* channelTab : fChannelTab) {
      if(channelTab != nullptr) { channelTab->PrintCanvases(); }
   }
}
