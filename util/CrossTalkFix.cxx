#include <iostream>

#include "TFile.h"
#include "TH2.h"
#include "TF1.h"
#include "TCutG.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TProfile.h"

#include "Globals.h"
#include "TGRSIUtilities.h"
#include "TChannel.h"
#include "TUserSettings.h"

double CrossTalkFit(double* x, double* par)   // NOLINT(readability-non-const-parameter)
{
   // This function is the linear fit function, but uses the CT coefficients as parameters instead of slope and
   // intercept
   double k0     = par[0] / (1. - par[0]);
   double k1     = par[1] / (1. - par[1]);
   double energy = par[2];

   double slope     = -(1. + k0) / (1. + k1);
   double intercept = energy * (1. + k0 / (1. + k1));

   return x[0] * slope + intercept;
}

double* CrossTalkFix(TFile* inputFile, int det, double energy, int minimumCounts, const std::string& channelPrefix, const std::string& channelPostfix)
{
   // The outfile is implicit since it was the last file that was open.
   static double largestCorrection = 0.0;

   // Clear all of the CT calibrations for this detector.
   for(int i = 0; i < 4; ++i) {
      auto*     channelName = Form("%s%02d%s%s", channelPrefix.c_str(), det, GetColorFromNumber(i), channelPostfix.c_str());
      TChannel* chan        = TChannel::FindChannelByName(channelName);
      if(chan == nullptr) {
         std::cout << DRED << "Couldn't find a channel for " << channelName << RESET_COLOR << std::endl;
         continue;
      }
      chan->DestroyCTCal();
   }

   static int largestDet      = -1;
   static int largestCrystal1 = -1;
   static int largestCrystal2 = -1;

   std::string namebase = Form("det_%d", det);

   // This range seems to be working fairly well since no shift should be larger than say 6 or 7 keV
   double lowCut  = energy - 15;
   double highCut = energy + 15;

   // create diagonal cut
   std::array<double, 5> xpts = {lowCut, 0, 0, highCut, lowCut};
   std::array<double, 5> ypts = {0, lowCut, highCut, 0, 0};
   TCutG                 cut("cut", 5, xpts.data(), ypts.data());

   auto* d  = new double[16];   // matrix of coefficients
   auto* eD = new double[16];   // matrix of errors

   for(int crystal1 = 0; crystal1 < 4; crystal1++) {
      for(int crystal2 = crystal1 + 1; crystal2 < 4; crystal2++) {
         // Load all of the addback matrices in and put them into a vector of TH2*
         std::string name = Form("%s_%d_%d", namebase.c_str(), crystal1, crystal2);
         TH2*        mat  = dynamic_cast<TH2*>(inputFile->Get(name.c_str()));
         if(mat == nullptr) {
            std::cout << "can not find:  " << name << std::endl;
            return nullptr;
         }
         std::cout << mat->GetName() << std::endl;
         int xbins = mat->GetNbinsX();
         int ybins = mat->GetNbinsY();

         TH2* cmat = dynamic_cast<TH2*>(mat->Clone(Form("%s_clone", mat->GetName())));
         cmat->Reset();

         // I make a graph out of the "addback line" because I don't like the way TProfile handles the empty bins
         auto* fitGraph = new TGraphErrors;
         fitGraph->SetNameTitle(Form("%s_graph", mat->GetName()), "Graph");

         // This loop turns the addback plot and TCut into the TGraphErrors
         int rejectedBins = 0;
         for(int i = 1; i <= xbins; i++) {
            bool insideYet = false;
            for(int j = 1; j <= ybins; j++) {
               double xc = mat->GetXaxis()->GetBinCenter(i);
               double yc = mat->GetYaxis()->GetBinCenter(j);
               if(cut.IsInside(xc, yc) != 0) {
                  if(!insideYet) {
                     insideYet = true;
                  }
                  cmat->Fill(xc, yc, mat->GetBinContent(i, j));
               }
            }
            cmat->GetXaxis()->SetRange(i, i);
            // This makes sure that there are at least 4 counts in the "y bin". I'd prefer this to be higher,
            // but that requires more 60Co statistics. The reason I do this is because RMS and SD of the mean really only
            // works for us if we have enough counts that the mean is actually a good representation of the true value.
            // This is something TProfile does not do for us, and seems to skew the result a bit.
            if(cmat->Integral() > minimumCounts) {
               fitGraph->SetPoint(fitGraph->GetN(), cmat->GetYaxis()->GetBinCenter(i), cmat->GetMean(2));
               fitGraph->SetPointError(fitGraph->GetN() - 1, cmat->GetXaxis()->GetBinWidth(i), cmat->GetMeanError(2));
            } else {
               ++rejectedBins;
            }
         }
         // unzoom the x-axis
         cmat->GetXaxis()->SetRange(1, cmat->GetXaxis()->GetNbins());
         cmat->Write();

         if(rejectedBins > 0) {
            std::cout << name << ": rejected " << rejectedBins << " bins out of " << xbins << " bins, because the number of counts in the projection of this bin is less than " << minimumCounts << std::endl;
         }

         if(fitGraph->GetN() <= 0) {
            std::cout << name << ": graph doesn't have any data points (" << fitGraph->GetN() << "), going to skip fitting it" << std::endl;
            continue;
         }

         // This fits the TGraph
         auto* fpx = new TF1(Form("pxfit_%i_%i_%i", det, crystal1, crystal2), CrossTalkFit, 6, 1167, 3);
         fpx->SetParameter(0, 0.0001);
         fpx->SetParameter(1, 0.0001);
         fpx->SetParameter(2, energy);
         fpx->FixParameter(2, energy);
         fitGraph->Fit(fpx);
         fitGraph->Write();
         delete fitGraph;

         // create and write profile
         TProfile* px = cmat->ProfileX();
         px->Write();
         // Make a residuals plot
         TH1* residualPlot = new TH1D(Form("%s_resid", fpx->GetName()), "residuals", 2000, 0, 2000);
         for(int i = 0; i < residualPlot->GetNbinsX(); ++i) {
            if(px->GetBinContent(i) != 0) {
               residualPlot->SetBinContent(i, px->GetBinContent(i) - fpx->Eval(residualPlot->GetBinCenter(i)));
            }
            residualPlot->SetBinError(i, px->GetBinError(i));
         }
         residualPlot->Write();
         delete residualPlot;
         delete cmat;

         // for some reason from here on out crystal1 and crystal2 are used in reverse?
         // was this maybe because before we used xind and yind instead which were from the tokenized name using entry N-1 (for xind) and N-2 (for yind)?
         // temporary swapped the two - except for the largest stuff?
         std::cout << "=====================" << std::endl;
         std::cout << mat->GetName() << std::endl;
         std::cout << "d" << crystal1 << crystal2 << " at zero   " << (fpx->Eval(energy)) / energy << std::endl;
         std::cout << "=====================" << std::endl;

         // Fill the parameter matrix with the parameters from the fit.
         d[crystal2 * 4 + crystal1]  = fpx->GetParameter(0);
         d[crystal1 * 4 + crystal2]  = fpx->GetParameter(1);
         eD[crystal2 * 4 + crystal1] = fpx->GetParError(0);
         eD[crystal1 * 4 + crystal2] = fpx->GetParError(1);

         // Keep track of the largest correction and output that to screen,
         // This helps identify problem channels, or mistakes
         if(fpx->GetParameter(0) > largestCorrection) {
            largestCorrection = fpx->GetParameter(0);
            largestDet        = det;
            largestCrystal2   = crystal2;
            largestCrystal1   = crystal1;
         }
         if(fpx->GetParameter(1) > largestCorrection) {
            largestCorrection = fpx->GetParameter(1);
            largestDet        = det;
            largestCrystal2   = crystal2;
            largestCrystal1   = crystal1;
         }
      }   //for(int crystal2 = crystal1 + 1; crystal2 < 4; crystal2++)
   }   // for(int crystal1 = 0; crystal1 < 4; crystal1++)

   std::cout << " -------------------- " << std::endl;
   std::cout << " -------------------- " << std::endl;
   std::cout << std::endl;

   // Set the diagonal elements to 0 since you can't cross-talk yourself
   // store current precision and set to fixed 10 precision
   std::streamsize prec = std::cout.precision();
   std::cout.precision(10);
   std::cout.setf(std::ios::fixed, std::ios::floatfield);
   for(int i = 0; i < 4; i++) {
      for(int j = 0; j < 4; j++) {
         if(i == j) {
            d[i * 4 + j]  = 0.0000;
            eD[i * 4 + j] = 0.0000;
         }
         // output a matrix to screen
         std::cout << d[j * 4 + i] << "\t";

         // Time to find the proper channels and build the corrections xind/i is row number
         auto*     channelName = Form("%s%02d%s%s", channelPrefix.c_str(), det, GetColorFromNumber(j), channelPostfix.c_str());
         TChannel* chan        = TChannel::FindChannelByName(channelName);
         if(chan == nullptr) {
            std::cout << DRED << "Couldn't find a channel for " << channelName << RESET_COLOR << std::endl;
            continue;
         }
         // Writes the coefficient to the found channel above
         chan->AddCTCoefficient(d[i * 4 + j]);
      }
      std::cout << std::endl;
   }
   // set precision back to old value and remove fixed flag
   std::cout.precision(prec);
   std::cout.unsetf(std::ios::floatfield);

   std::cout << std::endl;
   std::cout << "Largest correction = " << largestCorrection << " Shift = " << largestCorrection * energy << std::endl;
   std::cout << "Largest combo, det = " << largestDet << " Crystals = " << largestCrystal1 << ", " << largestCrystal2 << std::endl;
   std::cout << " -------------------- " << std::endl;
   std::cout << " -------------------- " << std::endl;
   return d;
}

#ifndef __CINT__
int main(int argc, char** argv)
{
   // Do basic file checks
   if(argc != 3 && argc != 4) {
      std::cout << "Usage: " << argv[0] << " <matrix file> <cal file> <user settings (optional)>" << std::endl;
      std::cout << "User settings available (with default values where applicable), note whether integers, doubles, or strings are used:" << std::endl
                << "CrossTalkEnergy: 1332." << std::endl
                << "MinimumCounts: 10" << std::endl
                << "FirstDetector: 1" << std::endl
                << "LastDetector: 16" << std::endl
                << "ExcludedDetectors: (int vector, i.e. comma separated list of integers between FirstDetector and LastDetector)" << std::endl
                << "ChannelPrefix: GRG" << std::endl
                << "ChannelPostfix: N00A" << std::endl;
      return 1;
   }

   // We need a cal file to find the channels to write the corrections to
   if(TChannel::ReadCalFile(argv[2]) < 0) {
      std::cout << "Aborting" << std::endl;
      exit(1);
   }

   auto* inputFile = new TFile(argv[1]);
   if(inputFile == nullptr || !inputFile->IsOpen()) {
      std::cout << "Failed to open file '" << argv[1] << "'!" << std::endl;
      return 1;
   }

   // Create the output file
   std::string outputFileName = argv[1];
   auto        lastSlash      = outputFileName.find_last_of('/');
   if(lastSlash != std::string::npos) {
      outputFileName = std::string("ct_") + outputFileName.substr(lastSlash + 1);
   } else {
      outputFileName = std::string("ct_") + std::string(argv[1]);
   }

   auto* outputFile = new TFile(outputFileName.c_str(), "recreate");

   // read user settings
   auto* userSettings = new TUserSettings();
   if(argc == 4) {
      userSettings->ReadSettings(argv[3]);
   }

   std::vector<int> excludedDetectors;
   try {
      excludedDetectors = userSettings->GetIntVector("ExcludedDetectors", true);
   } catch(std::out_of_range& e) {
   }

   double      energy         = userSettings->GetDouble("CrossTalkEnergy", 1332.);
   int         minimumCounts  = userSettings->GetInt("MinimumCounts", 10);
   std::string channelPrefix  = userSettings->GetString("ChannelPrefix", "GRG");
   std::string channelPostfix = userSettings->GetString("ChannelPostfix", "N00A");

   for(int d = userSettings->GetInt("FirstDetector", 1); d <= userSettings->GetInt("LastDetector", 16); d++) {
      if(!excludedDetectors.empty() && std::find(excludedDetectors.begin(), excludedDetectors.end(), d) != excludedDetectors.end()) {
         std::cout << "Skipping excluded detector " << d << std::endl;
         continue;
      }
      std::cout << "Starting CrossTalkFix for detector " << d << ", using energy " << userSettings->GetDouble("CrossTalkEnergy", 1332.) << ", and minimum counts " << userSettings->GetInt("MinimumCounts", 10) << std::endl;
      CrossTalkFix(inputFile, d, energy, minimumCounts, channelPrefix, channelPostfix);
   }

   // This function writes a corrections cal_file which can be loaded in with your normal cal file.
   TChannel::WriteCTCorrections("ct_correction.cal");

   outputFile->Write();
   outputFile->Close();
}
#endif
