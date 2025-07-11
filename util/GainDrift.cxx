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

void FindGainDrift(TH2* hist, std::vector<double> energies, std::vector<double> energyUncertainties, double range, double minFWHM, int degree, const std::string& prefix, const std::string& label, TFile* output);

int main(int argc, char** argv)
{
   bool printUsage = (argc == 1);

	int							 degree = 1;
   double                   range = 10.;
   std::vector<double>      energies;
   std::vector<double>      energyUncertainties;
   std::vector<std::string> fileNames;
   std::string              prefix        = "GainDrift";
   std::string              histogramName = "EnergyVsChannel";
   double                   minFWHM       = 2.;

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
            degree = settings.GetInt("Degree", 1);
            range = settings.GetDouble("Range", 10.);
            minFWHM = settings.GetDouble("MinimumFWHM", 2.);
            prefix = settings.GetString("Prefix", "GainDrift");
            histogramName = settings.GetString("HistogramName", "EnergyVsChannel");
            if(!energies.empty()) {
               std::cerr << "Warning, already got " << energies.size() << " energies:";
               for(auto energy : energies) { std::cerr << "   " << energy; }
               std::cerr << std::endl;
               std::cerr << "These will now be overwritten by what is read from the settings file " << argv[i] << std::endl;
            }
            energies = settings.GetDoubleVector("Energies");
            if(!energyUncertainties.empty()) {
               std::cerr << "Warning, already got " << energyUncertainties.size() << " energy uncertainties:";
               for(auto energy : energyUncertainties) { std::cerr << "   " << energy; }
               std::cerr << std::endl;
               std::cerr << "These will now be overwritten by what is read from the settings file " << argv[i] << std::endl;
            }
            energyUncertainties = settings.GetDoubleVector("EnergyUncertainties");
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
      } else if(strcmp(argv[i], "-ul") == 0) {
			if(!energyUncertainties.empty()) {
				std::cerr << "Warning, already got " << energyUncertainties.size() << " energy uncertainties:";
				for(auto energy : energyUncertainties) { std::cerr << "   " << energy; }
				std::cerr << std::endl;
            std::cerr << "What is read from command line will not overwrite these but be added to them!" << std::endl;
			}
         // if we have a next argument, check if it starts with '-'
         while(i + 1 < argc) {
            if(argv[i + 1][0] == '-') {
               break;
            }
            // if we get here we can add the next argument to the list of energies
            energyUncertainties.push_back(std::atof(argv[++i]));
         }
      } else if(strcmp(argv[i], "-d") == 0) {
         if(i + 1 < argc) {
            degree = std::atoi(argv[++i]);
         } else {
            std::cout << "Error, -d flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-r") == 0) {
         if(i + 1 < argc) {
            range = std::atof(argv[++i]);
         } else {
            std::cout << "Error, -r flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-mw") == 0) {
         if(i + 1 < argc) {
            minFWHM = std::atof(argv[++i]);
         } else {
            std::cout << "Error, -mw flag needs an argument!" << std::endl;
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
      } else {
         std::cerr << "Error, unknown flag \"" << argv[i] << "\"!" << std::endl;
         printUsage = true;
      }  
   }

   // check that we got the necessary parameters set
   if(fileNames.empty() || energies.size() < 2) {
      printUsage = true;
   }

	// if we do not have any uncertainties, set them all to zero
	if(energyUncertainties.empty()) {
		energyUncertainties.resize(energies.size(), 0.);
	}

	// check that the number of uncertainties matches the number of energies
	if(energyUncertainties.size() != energies.size()) {
		std::cerr << "Error, mismatch between number of energies (" << energies.size() << ") and number of energy uncertainties (" << energyUncertainties.size() << ")!" << std::endl;
      printUsage = true;
   }

   if(printUsage) {
      std::cerr << "Arguments for " << argv[0] << ":" << std::endl
                << "-if  <input files>                                (required)" << std::endl
                << "-sf  <settings file>                              (semi-optional, TUserSettings format identifying the peaks to be used)" << std::endl
                << "-el  <list of energies to be used>                (semi-optional, either this or a settings file need to be used)" << std::endl
                << "-ul  <list of energy uncertainties to be used>    (optional, default = empty)" << std::endl
                << "-d   <degree of fit>                              (optional, default = 1, i.e. linear)" << std::endl
                << "-r   <+-range of fit>                             (optional, default = 10.)" << std::endl
                << "-mw  <minimum FWHM for good fit>                  (optional, default = 2.)" << std::endl
                << "-hn  <histogram name>                             (optional, default = \"EnergyVsChannel\")" << std::endl
                << "-pre <prefix>                                     (optional, default = \"GainDrift\")" << std::endl;

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
      FindGainDrift(hist, energies, energyUncertainties, range, minFWHM, degree, prefix, label, output);

      // write the gain drift to new cal-file
      TChannel::WriteCalFile(Form("%s%s.cal", prefix.c_str(), label.c_str()));

      ++goodFiles;

      std::cout << std::setw(6) << goodFiles << " / " << std::setw(6) << fileNames.size() << std::endl;
   }

   std::cout << goodFiles << " files out of " << fileNames.size() << " were processed successfully" << std::endl;

   output->Close();

   return 0;
}

bool GoodFit(TRWPeak& peak, const double& low, const double& high, double& minFWHM)
{
   // for a good fit the centroid should be in the fitting range
   if(peak.Centroid() < low || high < peak.Centroid()) { return false; }
   // for a good fit the FWHM should be less than half the fitting range (half is kind of arbitrary), and larger than the minimum (2 is default value)
   if(peak.FWHM() < minFWHM || peak.FWHM() > (high - low) / 2.) { return false; }
   // for a good fit the area should be positive (can't really put a non-arbitrary positive limit on this)
   if(peak.Area() < 0.) { return false; }

   // found no reason the fit isn't good
   return true;
}

double Polynomial(double* x, double* par) {
	double result = par[1];
	for(int i = 0; i < par[0]; ++i) {
		result += par[i+2] * TMath::Power(x[0], i+1);
	}
	return result;
}

void FindGainDrift(TH2* hist, std::vector<double> energies, std::vector<double> energyUncertainties, double range, double minFWHM, int degree, const std::string& prefix, const std::string& label, TFile* output)
{
   auto* yAxis  = hist->GetYaxis();
   TF1*  polynomial = new TF1("polynomial", Polynomial, yAxis->GetBinLowEdge(yAxis->GetFirst()), yAxis->GetBinLowEdge(yAxis->GetLast()), degree+2);
	polynomial->FixParameter(0, degree);

   // loop over all x-bins
   for(int bin = 1; bin <= hist->GetXaxis()->GetNbins(); ++bin) {
      auto* proj = hist->ProjectionY(Form("proj%s_%d", label.c_str(), bin), bin, bin);
      if(proj == nullptr || proj->GetEntries() < 1000) {
         continue;
      }

      // fit all energies and add them to a graph
      auto* redirect = new TRedirect("/dev/null", true);
      auto* graph    = new TGraphErrors;
      graph->SetName(Form("graph%s_%d", label.c_str(), bin));
		graph->SetTitle(Form("Channel %s, bin %d;uncorr. energy;nominal energy", label.c_str(), bin));
		int graphIndex = 0;
      for(size_t i = 0; i < energies.size(); ++i) {
         TPeakFitter pf(energies[i] - range, energies[i] + range);
         TRWPeak     peak(energies[i]);
         pf.AddPeak(&peak);
         pf.Fit(proj, "qn");

         proj->GetListOfFunctions()->Add(pf.GetFitFunction()->Clone(Form("fit%.1f", energies[i])));
         // check if the fit was good
         if(GoodFit(peak, energies[i] - range, energies[i] + range, minFWHM)) {
            graph->SetPoint(graphIndex, peak.Centroid(), energies[i]);
            graph->SetPointError(graphIndex, peak.CentroidErr(), energyUncertainties[i]);
				++graphIndex;
         } else {
            static_cast<TF1*>(proj->GetListOfFunctions()->Last())->SetLineColor(kGray);
            std::cout << "Not using bad fit with centroid " << peak.Centroid() << " +- " << peak.CentroidErr() << ", area " << peak.Area() << " +- " << peak.AreaErr() << ", fwhm " << peak.FWHM() << " in range " << energies[i] - range << " - " << energies[i] + range << std::endl;
         }
      }

      // reset the parameters of the polynomial function and fit the graph
      polynomial->SetParameter(1, 0.);
      polynomial->SetParameter(2, 1.);
		for(int i = 3; i < degree + 2; ++i) {
			polynomial->SetParameter(i, 0.);
		}
      graph->Fit(polynomial, "q");
      delete redirect;

      redirect = new TRedirect(Form("%s.log", prefix.c_str()), true);

      std::cout << "bin " << bin << ":" << std::endl;
      graph->Print();
      std::cout << "polynomial fit " << degree << ". degree: " << polynomial->GetParameter(1) << " + " << polynomial->GetParameter(2) << " * x";
		for(int i = 3; i < degree + 2; ++i) {
			std::cout << " + " << polynomial->GetParameter(i) << " * x^" << i-1;
		}
		std::cout << std::endl;
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
      std::vector<Float_t> coeff(degree+1);
		for(int i = 0; i < degree+1; ++i) {
			coeff[i] = static_cast<Float_t>(polynomial->GetParameter(i+1));
		}
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
