
#include <TCalibrator.h>

#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>

#include <TH1.h>
#include <TSpectrum.h>
#include <TLinearFitter.h>
#include <TGraphErrors.h>

#include <TChannel.h>
#include <TNucleus.h>
#include <TTransition.h>

#include <GRootFunctions.h>
#include <GRootCommands.h>
#include <GCanvas.h>
#include <GPeak.h>
#include <Globals.h>

#include "combinations.h"

/// \cond CLASSIMP
ClassImp(TCalibrator)
   /// \endcond

   TCalibrator::TCalibrator()
{
   linfit = nullptr;
   efffit = nullptr;
   Clear();
}

TCalibrator::~TCalibrator()
{
   if(linfit != nullptr) {
      delete linfit;
   }
   if(efffit != nullptr) {
      delete efffit;
   }
}

void TCalibrator::Copy(TObject&) const
{
}

void TCalibrator::Print(Option_t*) const
{
   int counter = 0;
   std::cout << "\t  senergy          scent          scalc          sarea       snuc        sintensity" << std::endl;
   for(auto it : fPeaks) {
      double caleng = it.centroid * GetParameter(1) + GetParameter(0);
      double pdiff  = std::abs(caleng - it.energy) / it.energy;
      std::cout << counter++ << ":\t" << std::setw(7) << it.energy << std::setw(16) << it.centroid << std::setw(8) << caleng
                << "   [%%" << std::setw(3) << pdiff * 100. << "]" << std::setw(16) << it.area << std::setw(8) << it.nucleus << std::setw(16) << it.intensity << std::endl;
   }
   std::cout << "-------------------------------" << std::endl;
}

std::string TCalibrator::PrintEfficency(const char* filename)
{
   std::string toprint;
   std::string file    = filename;
   int         counter = 1;
   toprint.append("line\teng\tcounts\tt1/2\tactivity\n");
   toprint.append("--------------------------------------\n");
   for(auto it : fPeaks) {
      toprint.append(
         Form("%i\t%.02f\t%.02f\t%i\t%.02f\n", counter++, it.energy, it.area, 100, (it.intensity / 100) * 1e5));
   }
   toprint.append("--------------------------------------\n");

   if(file.length() != 0u) {
      std::ofstream ofile;
      ofile.open(file.c_str());
      ofile << toprint;
      ofile.close();
   }
   std::cout << toprint << std::endl;
   return toprint;
}

TGraphErrors& TCalibrator::MakeEffGraph(double seconds, double bq, Option_t* opt)
{
   // this currently assumes it only has 152Eu data.
   // one day it may be smarter, but today is not that day.
   TString option(opt);
   TString fitopt;
   if(option.Contains("Q")) {
      fitopt.Append("Q");
      option.ReplaceAll("Q", "");
   }
   std::vector<double> energy;
   std::vector<double> error_e;
   std::vector<double> observed;
   std::vector<double> error_o;
   for(auto it : fPeaks) {
      // TNucleus n(it.nucleus.c_str());
      energy.push_back(it.energy);
      error_e.push_back(0.0);
      observed.push_back((it.area / seconds) / ((it.intensity / 100) * bq));
      error_o.push_back(observed.back() * (sqrt(it.area) / it.area));
   }

   eff_graph.Clear();
   eff_graph = TGraphErrors(fPeaks.size(), energy.data(), observed.data(), error_e.data(), error_o.data());

   if(efffit != nullptr) {
      efffit->Delete();
   }
   static int counter = 0;
   efffit             = new TF1(Form("eff_fit_%i", counter++), GRootFunctions::GammaEff, 0, 1500, 4);
   eff_graph.Fit(efffit, fitopt.Data());

   if(option.Contains("draw", TString::kIgnoreCase)) {
      TVirtualPad* current = gPad;
      new GCanvas;
      eff_graph.Draw("AP");
      if(current != nullptr) {
         current->cd();
      }
   }
   for(unsigned int i = 0; i < energy.size(); i++) {
      std::cout << "[" << energy.at(i) << "] Observed  = " << observed.at(i) << "  | Calculated = " << efffit->Eval(energy.at(i)) << "  |  per diff = "
                << (std::abs(observed.at(i) - efffit->Eval(energy.at(i))) / observed.at(i)) * 100. << std::endl;
   }
   return eff_graph;
}

void TCalibrator::Clear(Option_t* opt)
{
   fit_graph.Clear(opt);
   eff_graph.Clear(opt);
   // all_fits.clear();

   for(double& i : eff_par) {
      i = 0.;
   }

   total_points = 0;
}

void TCalibrator::Draw(Option_t* opt)
{
   // if((graph_of_everything.GetN()<1) &&
   //    (all_fits.size()>0))
   // MakeCalibrationGraph();
   // Fit();
   TString option(opt);
   if(option.Contains("new", TString::kIgnoreCase)) {
      new GCanvas;
   }
   fit_graph.Draw("AP");
}

void TCalibrator::Fit(int order)
{

   // if((graph_of_everything.GetN()<1) &&
   //    (all_fits.size()>0))
   MakeCalibrationGraph();
   if(fit_graph.GetN() < 1) {
      return;
   }
   if(order == 1) {
      linfit = new TF1("linfit", GRootFunctions::LinFit, 0, 1, 2);
      linfit->SetParameter(0, 0.0);
      linfit->SetParameter(1, 1.0);
      linfit->SetParName(0, "intercept");
      linfit->SetParName(1, "slope");
   } else if(order == 2) {
      linfit = new TF1("linfit", GRootFunctions::QuadFit, 0, 1, 3);
      linfit->SetParameter(0, 0.0);
      linfit->SetParameter(1, 1.0);
      linfit->SetParameter(2, 0.0);
      linfit->SetParName(0, "A");
      linfit->SetParName(1, "B");
      linfit->SetParName(2, "C");
   }
   fit_graph.Fit(linfit);
   Print();
}

double TCalibrator::GetParameter(int i) const
{
   if(linfit != nullptr) {
      return linfit->GetParameter(i);
   }
   return sqrt(-1);
}

double TCalibrator::GetEffParameter(int i) const
{
   if(efffit != nullptr) {
      return efffit->GetParameter(i);
   }
   return sqrt(-1);
}

TGraph& TCalibrator::MakeCalibrationGraph(double)
{
   std::vector<double> xvalues;
   std::vector<double> yvalues;
   // std::vector<double> xerrors;
   // std::vector<double> yerrors;

   for(auto it : fPeaks) {
      xvalues.push_back(it.centroid);
      yvalues.push_back(it.energy);
   }
   fit_graph.Clear();
   fit_graph = TGraph(xvalues.size(), xvalues.data(), yvalues.data());

   return fit_graph;
}

std::vector<double> TCalibrator::Calibrate(double)
{
   std::vector<double> vec;
   return vec;
}

int TCalibrator::AddData(TH1* data, const std::string& source, double sigma, double threshold, double error)
{
   if((data == nullptr) || (source.length() == 0u)) {
      std::cout << "data not added. data = " << data << " \t source = " << source << std::endl;
      return 0;
   }
   TNucleus n(source.c_str());
   return AddData(data, &n, sigma, threshold, error);
}

int TCalibrator::AddData(TH1* data, TNucleus* source, double sigma, double threshold, double)
{
   if((data == nullptr) || (source == nullptr)) {
      std::cout << "data not added. data = " << data << " \t source = " << source << std::endl;
      return 0;
   }
   int actual_x_max    = std::floor(data->GetXaxis()->GetXmax());
   int actual_x_min    = std::floor(data->GetXaxis()->GetXmax());
   int displayed_x_max = std::floor(data->GetXaxis()->GetBinUpEdge(data->GetXaxis()->GetLast()));
   int displayed_x_min = std::floor(data->GetXaxis()->GetBinLowEdge(data->GetXaxis()->GetFirst()));

   std::string name;
   if((actual_x_max == displayed_x_max) && (actual_x_min == displayed_x_min)) {
      name = source->GetName();
   } else {
      name = Form("%s_%i_%i", source->GetName(), displayed_x_min, displayed_x_max);
   }

   TIter                    iter(source->GetTransitionList());
   std::vector<double>      source_energy;
   std::map<double, double> src_eng_int;
   while(TTransition* transition = static_cast<TTransition*>(iter.Next())) {
      source_energy.push_back(transition->GetEnergy());
      src_eng_int[transition->GetEnergy()] = transition->GetIntensity();
   }
   std::sort(source_energy.begin(), source_energy.end());

   TSpectrum spectrum;
   spectrum.Search(data, sigma, "", threshold);
   std::vector<double>      data_channels;
   std::map<double, double> peak_area;
   for(int x = 0; x < spectrum.GetNPeaks(); x++) {
      double range = 8 * data->GetXaxis()->GetBinWidth(1);
      GPeak* fit =
         PhotoPeakFit(data, spectrum.GetPositionX()[x] - range, spectrum.GetPositionX()[x] + range, "no-print");
      // data_channels
      data_channels.push_back(fit->GetCentroid());
      data->GetListOfFunctions()->Remove(fit);
      peak_area[fit->GetCentroid()] = fit->GetSum();
   }

   std::map<double, double> datatosource = Match(data_channels, source_energy);

   for(auto it : datatosource) {
      AddPeak(it.first, it.second, source->GetName(), peak_area.at(it.first), src_eng_int[it.second]);
   }

   // Print();
   int counter = 0;
   for(auto it : datatosource) {
      if(!std::isnan(it.second)) {
         counter++;
      }
   }
   return counter;   // CheckMap(datatosource);
}

void TCalibrator::ResetMap(std::map<double, double>& inmap)
{
   for(auto& it : inmap) {
      it.second = sqrt(-1);
   }
}

void TCalibrator::PrintMap(std::map<double, double>& inmap)
{
   std::cout << "\tfirst\tsecond" << std::endl;
   int counter = 0;
   for(auto& it : inmap) {
      std::cout << counter++ << "\t" << it.first << "\t" << it.second << std::endl;
   }
}

std::map<double, double> TCalibrator::Match(std::vector<double> peaks, std::vector<double> source)
{
   std::map<double, double> map;
   std::sort(peaks.begin(), peaks.end());
   std::sort(source.begin(), source.end());

   // Peaks are the fitted points.
   // source are the known values

   std::vector<bool> filled(source.size());
   std::fill(filled.begin(), filled.begin() + peaks.size(), true);

   TLinearFitter fitter(1, "1 ++ x");

   for(size_t num_data_points = peaks.size(); num_data_points > 0; num_data_points--) {
      double best_chi2 = DBL_MAX;
      for(auto peak_values : combinations(peaks, num_data_points)) {
         // Add a (0,0) point to the calibration.
         peak_values.push_back(0);
         for(auto source_values : combinations(source, num_data_points)) {
            source_values.push_back(0);

            if(peaks.size() > 3) {
               double max_err = 0.02;
               double pratio  = peak_values.front() / peak_values.at(peak_values.size() - 2);
               double sratio  = source_values.front() / source_values.at(source_values.size() - 2);
               // std::cout<<"ratio: "<<pratio<<" - "<<sratio<<" = "<<std::abs(pratio-sratio)<<std::endl;
               if(std::abs(pratio - sratio) > max_err) {
                  // std::cout<<"skipping"<<std::endl;
                  continue;
               }
            }

            fitter.ClearPoints();
            fitter.AssignData(source_values.size(), 1, peak_values.data(), source_values.data());
            fitter.Eval();

            if(fitter.GetChisquare() < best_chi2) {
               map.clear();
               for(size_t i = 0; i < num_data_points; i++) {
                  map[peak_values[i]] = source_values[i];
               }
               best_chi2 = fitter.GetChisquare();
            }
         }
      }

      // Remove one peak value from the best fit, make sure that we reproduce (0,0) intercept.
      if(map.size() > 2) {
         std::vector<double> peak_values;
         std::vector<double> source_values;
         for(auto& item : map) {
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
               map.clear();
               break;
            }

            std::swap(peak_values[skipped_point], peak_values.back());
            std::swap(source_values[skipped_point], source_values.back());
         }
      }

      if(!map.empty()) {
         return map;
      }
   }

   map.clear();
   return map;
}

bool TCalibrator::CheckMap(std::map<double, double> inmap)
{
   for(auto it : inmap) {
      if(std::isnan(it.second)) {
         return false;
      }
   }
   return true;
}

void TCalibrator::UpdateTChannel(TChannel*)
{
}

void TCalibrator::AddPeak(double cent, double eng, std::string nuc, double a, double inten)
{
   Peak p;
   p.centroid  = cent;
   p.energy    = eng;
   p.nucleus   = std::move(nuc);
   p.area      = a;
   p.intensity = inten;
   fPeaks.push_back(p);
}
