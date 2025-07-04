#include <iostream>
#include <string>
#include <vector>

#include "TFile.h"
#include "TH2.h"
#include "TGraphErrors.h"
#include "TF1.h"

#include "TPriorityValue.h"
#include "TUserSettings.h"
#include "TChannel.h"
#include "TRunInfo.h"
#include "TRedirect.h"
#include "TPeakFitter.h"
#include "TRWPeak.h"

void FindGainDrift(TH2* hist, std::vector<double> energies, double range, const std::string& prefix, const std::string& label, TFile* output);

int main(int argc, char** argv)
{
   bool printUsage = (argc == 1);

   double                   range = 10.;
   std::vector<double>      energies;
   std::vector<std::string> fileNames;
   std::string              prefix        = "GainDrift";
   std::string              histogramName = "EnergyVsChannel";

   for(int i = 1; i < argc; ++i) {
      if(strcmp(argv[i], "-if") == 0) {
         // if we have a next argument, check if it starts with '-'
         while(i + 1 < argc) {
            if(argv[i + 1][0] == '-') {
               break;
            }
            // if we get here we can add the next argument to the list of file names
            fileNames.emplace_back(argv[++i]);
         }
      } else if(strcmp(argv[i], "-sf") == 0) {
         if(i + 1 < argc) {
            TUserSettings settings(argv[++i]);
            prefix = settings.GetString("Prefix", "GainDrift");
            if(!energies.empty()) {
               std::cerr << "Warning, already got " << energies.size() << " energies:";
               for(auto energy : energies) { std::cerr << "   " << energy; }
               std::cerr << std::endl;
               std::cerr << "These will now be overwritten by what is read from the settings file " << argv[i] << std::endl;
            }
            energies = settings.GetDoubleVector("Energies");
         } else {
            std::cout << "Error, -sf flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-el") == 0) {
         if(!energies.empty()) {
            std::cerr << "Warning, already got " << energies.size() << " energies:";
            for(auto energy : energies) { std::cerr << "   " << energy; }
            std::cerr << std::endl;
            std::cerr << "What is read from command line will not overwrite these but be added to them!" << std::endl;
         }
         // if we have a next argument, check if it starts with '-'
         while(i + 1 < argc) {
            if(argv[i + 1][0] == '-') {
               break;
            }
            // if we get here we can add the next argument to the list of energies
            energies.push_back(std::atof(argv[++i]));
         }
      } else if(strcmp(argv[i], "-r") == 0) {
         if(i + 1 < argc) {
            range = std::atof(argv[++i]);
         } else {
            std::cout << "Error, -r flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-hn") == 0) {
         if(i + 1 < argc) {
            histogramName = argv[++i];
         } else {
            std::cout << "Error, -hn flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-pre") == 0) {
         if(i + 1 < argc) {
            prefix = argv[++i];
         } else {
            std::cout << "Error, -pre flag needs an argument!" << std::endl;
            printUsage = true;
         }
      }
   }

   // check that we got the necessary parameters set
   if(fileNames.empty() || energies.size() < 2) {
      printUsage = true;
   }

   if(printUsage) {
      std::cout << "Arguments for " << argv[0] << ":" << std::endl
                << "-if  <input files>                    (required)" << std::endl
                << "-sf  <settings file>                  (semi-optional, TUserSettings format identifying the peaks to be used)" << std::endl
                << "-el  <list of energies to be used>    (semi-optional, either this or a settings file need to be used)" << std::endl
                << "-r   <+-range of fit>                 (optional, default = 10.)" << std::endl
                << "-hn  <histogram name>                 (optional, default = \"EnergyVsChannel\")" << std::endl
                << "-pre <prefix>                         (optional, default = \"GainDrift\")" << std::endl;

      return 1;
   }

   // create output file to save all spectra, graphs, and fits
   auto* output = new TFile(Form("%s.root", prefix.c_str()), "recreate");

   // at this stage all parameters should be set correctly, so we can start looping through the files
   int goodFiles = 0;
   for(const auto& fileName : fileNames) {
      // try and get the histogram from the file
      auto* file = TFile::Open(fileName.c_str());
      if(!file->IsOpen()) {
         std::cerr << "Failed to open file \"" << fileName << "\", will ignore it and move on!" << std::endl;
         delete file;
         continue;
      }
      auto* hist = static_cast<TH2*>(file->Get(histogramName.c_str()));
      if(hist == nullptr) {
         std::cerr << "Failed to find \"" << histogramName << "\" in \"" << fileName << "\", will ignore it and move on!" << std::endl;
         file->Close();
         delete file;
         continue;
      }

      // read cal-file from this root file
      TChannel::ReadCalFromFile(file);

      // create and store "label" (aka runNumber_subRunNumber)
      TRunInfo::ReadInfoFromFile(file);
      auto label = TRunInfo::CreateLabel(true);
      // find the gain drift for all channels
      FindGainDrift(hist, energies, range, prefix, label, output);

      // write the gain drift to new cal-file
      TChannel::WriteCalFile(Form("%s%s.cal", prefix.c_str(), label.c_str()));

      ++goodFiles;

      std::cout << std::setw(6) << goodFiles << " / " << std::setw(6) << fileNames.size() << std::endl;
   }

   std::cout << goodFiles << " files out of " << fileNames.size() << " were processed successfully" << std::endl;

   output->Close();

   return 0;
}

bool GoodFit(TRWPeak& peak, const double& low, const double& high)
{
   // for a good fit the centroid should be in the fitting range
   if(peak.Centroid() < low || high < peak.Centroid()) { return false; }
   // for a good fit the FWHM should be less than half the fitting range (half is kind of arbitrary)
   if(peak.FWHM() > (high - low) / 2.) { return false; }
   // for a good fit the area should be positive (can't really put a non-arbitrary positive limit on this)
   if(peak.Area() < 0.) { return false; }

   // found no reason the fit isn't good
   return true;
}

void FindGainDrift(TH2* hist, std::vector<double> energies, double range, const std::string& prefix, const std::string& label, TFile* output)
{
   auto* yAxis  = hist->GetYaxis();
   TF1*  linear = new TF1("linear", "[0] + [1]*x", yAxis->GetBinLowEdge(yAxis->GetFirst()), yAxis->GetBinLowEdge(yAxis->GetLast()));

   // loop over all x-bins
   for(int bin = 1; bin <= hist->GetXaxis()->GetNbins(); ++bin) {
      auto* proj = hist->ProjectionY(Form("proj%s_%d", label.c_str(), bin), bin, bin);
      if(proj == nullptr || proj->GetEntries() < 1000) {
         continue;
      }

      // fit all energies and add them to a graph
      auto* redirect = new TRedirect("/dev/null", true);
      auto* graph    = new TGraphErrors(energies.size());
      graph->SetName(Form("graph%s_%d", label.c_str(), bin));
      for(size_t i = 0; i < energies.size(); ++i) {
         TPeakFitter pf(energies[i] - range, energies[i] + range);
         TRWPeak     peak(energies[i]);
         pf.AddPeak(&peak);
         pf.Fit(proj, "qn");

         proj->GetListOfFunctions()->Add(pf.GetFitFunction()->Clone(Form("fit%.1f", energies[i])));
         // check if the fit was good
         if(GoodFit(peak, energies[i] - range, energies[i] + range)) {
            graph->SetPoint(i, energies[i], peak.Centroid());
            graph->SetPointError(i, 0., peak.CentroidErr());
         } else {
            static_cast<TF1*>(proj->GetListOfFunctions()->Last())->SetLineColor(kGray);
            std::cout << "Not using bad fit with centroid " << peak.Centroid() << " +- " << peak.CentroidErr() << ", area " << peak.Area() << " +- " << peak.AreaErr() << ", fwhm " << peak.FWHM() << " in range " << energies[i] - range << " - " << energies[i] + range << std::endl;
         }
      }

      // reset the parameters of the linear function and fit the graph
      linear->SetParameters(0., 1.);
      graph->Fit(linear, "q");
      delete redirect;

      redirect = new TRedirect(Form("%s.log", prefix.c_str()), true);

      std::cout << "bin " << bin << ":" << std::endl;
      graph->Print();
      std::cout << "linear fit: " << linear->GetParameter(0) << " + " << linear->GetParameter(1) << " * x" << std::endl;
      std::cout << "========================================" << std::endl;

      // get the channel belonging to this bin
      unsigned int      address = 0;
      std::stringstream str(hist->GetXaxis()->GetBinLabel(bin));
      str >> std::hex >> address;
      if(TChannel::GetChannel(address) == nullptr) {
         std::cout << "Failed to find channel for address " << address << " (from bin label \"" << hist->GetXaxis()->GetBinLabel(bin) << "\"), can't write gain drift factors for this channel!" << std::endl;
         continue;
      }

      // update the energy drift coefficents
      std::vector<Float_t> coeff = {static_cast<Float_t>(linear->GetParameter(0)), static_cast<Float_t>(linear->GetParameter(1))};
      TChannel::GetChannel(address)->SetENGDriftCoefficents(TPriorityValue<std::vector<Float_t>>(coeff, EPriority::kForce));

      // write spectrum and graph to output file
      output->cd();
      proj->Write();
      graph->Write();

      // done with this bin, clean up
      delete redirect;
      delete graph;
      delete proj;

      std::cout << std::setw(3) << bin << " / " << std::setw(3) << hist->GetXaxis()->GetNbins() << "\r" << std::flush;
   }
}
