#include "TFile.h"
#include "TH2.h"
#include "TH1.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TLegend.h"
#include "Fit/Fitter.h"
#include "TMatrixD.h"
#include "TPaveText.h"
#include "TMath.h"
#include "Math/SpecFuncMathMore.h"

#include "ArgParser.h"
#include "TUserSettings.h"

double Polarization(TUserSettings* settings);

/// program to read in 2D matrices from ComptonPolarimetryHelper and analyze them
int main(int argc, char** argv)
{
   // --------------------------------------------------------------------------------
   // Reading and verifying inputs and settings.
   // --------------------------------------------------------------------------------

   // input parameters
   bool                help             = false;
   double              qValue           = -1.;   // quality factor for predicted asymmetry plot of 0.5*Q*P*Cos(2*xi)
   double              measuredQEnergy  = -1.;   // energy of the scattered gamma ray used to calculated the q value
   double              usedQEnergy      = -1.;   // energy of the scattered gamma ray currently being analyzed, used to scale the q value
   bool                useDetectorAngle = false;
   double              minTheta         = 0.;
   double              maxTheta         = 180.;
   std::vector<double> xiBinEdges;
   std::string         inputFile;
   std::string         settingsFile;
   std::string         outputFile = "ComptonPolarimetry.root";

   // set up the argument parser
   ArgParser parser;
   parser.option("h help ?", &help, true).description("Show this help message.");
   parser.option("q-value", &qValue, true)
      .description("Quality factor for predicted asymmetry plot of 0.5*Q*P*Cos(2*xi).");
   parser.option("measured-q-energy", &measuredQEnergy, true)
      .description("Energy of the scattered gamma ray used to calculated the q value.");
   parser.option("used-q-energy", &usedQEnergy, true)
      .description("Energy of the scattered gamma ray currently being analyzed, used to scale the q value.");
   parser.option("input", &inputFile, true).description("Name of input file (required).");
   parser.option("settings", &settingsFile, true).description("Name of settings file.");
   parser.option("output", &outputFile, true)
      .description("Name of output file, default is \"ComptonPolarimetry.root\".");

   parser.parse(argc, argv, true);

   if(help) {
      std::cout << parser << std::endl;
      return 1;
   }

   // open the input root file and read any settings stored there, then add the ones potentially read in from command
   // line
   if(inputFile.empty()) {
      std::cerr << "Need an input file!" << std::endl;
      std::cout << parser << std::endl;
      return 1;
   }

   TFile input(inputFile.c_str());

   if(!input.IsOpen()) {
      std::cerr << "Failed to open input file " << inputFile << std::endl;
      return 1;
   }

   auto* settings = static_cast<TUserSettings*>(input.Get("UserSettings"));

   // if we have a path for a settings file provided on command line, we either add it to the ones read
   // from the root file or (if there weren't any) create a new instance from it
   if(!settingsFile.empty()) {
      if(settings == nullptr) {
         settings = new TUserSettings(settingsFile);
      } else {
         settings->ReadSettings(settingsFile);
      }
   }

   // check that we got settings either from the root file or a path provided on command line
   if(settings == nullptr || settings->empty()) {
      std::cerr << "Failed to get user settings from input file." << std::endl;
      return 1;
   }

   // set all variables from settings if they haven't been set from command line
   if(qValue == -1.) {
      qValue = settings->GetDouble("QValue");
   } else {
      settings->SetDouble("QValue", qValue);
   }
   if(measuredQEnergy == -1.) {
      measuredQEnergy = settings->GetDouble("QValue.MeasuredEnergy");
   } else {
      settings->SetDouble("QValue.MeasuredEnergy", measuredQEnergy);
   }
   if(usedQEnergy == -1.) {
      usedQEnergy = settings->GetDouble("QValue.UsedEnergy");
   } else {
      settings->SetDouble("QValue.UsedEnergy", usedQEnergy);
   }

   if(xiBinEdges.empty()) {
      xiBinEdges = {0., 2., 3., 6., 14., 31., 39., 50., 61., 74., 87.,
                    93., 106., 119., 130., 141., 149., 166., 174., 177., 178., 180.};
   } else {
      settings->SetDoubleVector("XiBinEdges", xiBinEdges);
   }

   // check if all necessary settings have been provided
   if(qValue == -1.) {
      std::cerr << "Need a quality factor" << std::endl;
      std::cout << parser << std::endl;
      return 1;
   }

   if(measuredQEnergy == -1.) {
      std::cerr << "Need the energy at which the quality factor was measured" << std::endl;
      std::cout << parser << std::endl;
      return 1;
   }

   if(usedQEnergy == -1.) {
      std::cerr << "Need the energy which is currently analyzed" << std::endl;
      std::cout << parser << std::endl;
      return 1;
   }

   // --------------------------------------------------------------------------------
   // Create the angular distribution.
   // --------------------------------------------------------------------------------

   // get the histograms we need
   auto* prompt = static_cast<TH2*>(input.Get(useDetectorAngle ? "xiThetaDet" : "xiThetaCry"));
   if(prompt == nullptr) {
      std::cerr << "Failed to find histogram '" << (useDetectorAngle ? "xiThetaDet" : "xiThetaCry") << "'!"
                << std::endl;
      return 1;
   }
   auto* mixed = static_cast<TH2*>(input.Get(useDetectorAngle ? "xiThetaDetMixed" : "xiThetaCryMixed"));
   if(mixed == nullptr) {
      std::cerr << "Failed to find histogram '" << (useDetectorAngle ? "xiThetaDetMixed" : "xiThetaCryMixed") << "'!"
                << std::endl;
      return 1;
   }
   auto* count = static_cast<TH2*>(input.Get(useDetectorAngle ? "xiThetaDetCount" : "xiThetaCryCount"));
   if(count == nullptr) {
      std::cerr << "Failed to find histogram '" << (useDetectorAngle ? "xiThetaDetCount" : "xiThetaCryCount") << "'!"
                << std::endl;
      return 1;
   }

   // create projections onto xi (x-axis)
   auto* xiPrompt = prompt->ProjectionX("xiPrompt", prompt->GetYaxis()->FindBin(minTheta), prompt->GetYaxis()->FindBin(maxTheta));
   auto* xiMixed  = mixed->ProjectionX("xiMixed", mixed->GetYaxis()->FindBin(minTheta), mixed->GetYaxis()->FindBin(maxTheta));
   auto* xiCount  = count->ProjectionX("xiCount", count->GetYaxis()->FindBin(minTheta), count->GetYaxis()->FindBin(maxTheta));

   // open output file and create graphs
   TFile output(outputFile.c_str(), "recreate");

   auto* asymmetry = new TGraphErrors();
   asymmetry->SetName("asymmetry");
   asymmetry->SetTitle("Asymmetry A^{CE}");
   asymmetry->GetXaxis()->SetTitle("Azimuthal Compton Scattering Angle #xi (#circ)");
   asymmetry->GetXaxis()->CenterTitle();
   asymmetry->GetYaxis()->SetTitle("Asymmetry A^{CE}");
   asymmetry->GetYaxis()->CenterTitle();
   auto* asymmetryGrouped = new TGraphErrors();
   asymmetryGrouped->SetName("asymmetryGrouped");
   asymmetryGrouped->SetTitle("Asymmetry A^{CE}, grouped");
   asymmetryGrouped->GetXaxis()->SetTitle("Azimuthal Compton Scattering Angle #xi (#circ)");
   asymmetryGrouped->GetXaxis()->CenterTitle();
   asymmetryGrouped->GetYaxis()->SetTitle("Asymmetry A^{CE}");
   asymmetryGrouped->GetYaxis()->CenterTitle();
   auto* asymmetryFolded = new TGraphErrors();
   asymmetryFolded->SetName("asymmetryFolded");
   asymmetryFolded->SetTitle("Asymmetry A^{CE}, grouped and folded");
   asymmetryFolded->GetXaxis()->SetTitle("Azimuthal Compton Scattering Angle #xi (#circ)");
   asymmetryFolded->GetXaxis()->CenterTitle();
   asymmetryFolded->GetYaxis()->SetTitle("Asymmetry A^{CE}");
   asymmetryFolded->GetYaxis()->CenterTitle();

   // write the user settings to the output file
   settings->Write();

   // create the asymmetry plots
   auto promptIntegral = xiPrompt->Integral();
   auto mixedIntegral  = xiMixed->Integral();
   // un-grouped
   for(int bin = 1; bin < xiPrompt->GetXaxis()->GetNbins(); ++bin) {
      if(xiPrompt->GetBinContent(bin) != 0 && xiMixed->GetBinContent(bin) != 0) {
         // original code excluded bins 2, 42, 48, 89, 131, 136, and 177 because they "are isolated as single bins in
         // the plot" (which plot?)
         auto point = asymmetry->GetN();
         asymmetry->SetPoint(point, xiPrompt->GetBinCenter(bin),
                             1. - (xiPrompt->GetBinContent(bin) / promptIntegral) /
                                     (xiMixed->GetBinContent(bin) / mixedIntegral));
         asymmetry->SetPointError(
            point, xiPrompt->GetBinWidth(bin) / 2.,
            (1. - (xiPrompt->GetBinContent(bin) / promptIntegral) / (xiMixed->GetBinContent(bin) / mixedIntegral)) *
               TMath::Sqrt(1. / xiPrompt->GetBinContent(bin) + 1. / promptIntegral + 1. / xiMixed->GetBinContent(bin) +
                           1. / mixedIntegral));
      }
   }
   // grouped
   for(size_t i = 0; i < xiBinEdges.size() - 1; ++i) {
      // original code excluded bins 2, 42, 48, 89, 131, 136, and 177 because they "are isolated as single bins in the
      // plot" (which plot?)
      auto tmp      = xiPrompt->Integral(xiPrompt->FindBin(xiBinEdges[i]), xiPrompt->FindBin(xiBinEdges[i + 1]));
      auto tmpMixed = xiMixed->Integral(xiMixed->FindBin(xiBinEdges[i]), xiMixed->FindBin(xiBinEdges[i + 1]));
      if(tmp != 0 && tmpMixed != 0) {
         auto point = asymmetryGrouped->GetN();
         asymmetryGrouped->SetPoint(point, (xiBinEdges[i] + xiBinEdges[i + 1]) / 2.,
                                    1. - (tmp / promptIntegral) / (tmpMixed / mixedIntegral));
         // original code does not include the uncertainty in the integrals for grouped points?
         asymmetryGrouped->SetPointError(
            point, (xiBinEdges[i] - xiBinEdges[i + 1]) / 2.,
            (1. - (tmp / promptIntegral) / (tmpMixed / mixedIntegral)) *
               TMath::Sqrt(1. / tmp + 1. / promptIntegral + 1. / tmpMixed + 1. / mixedIntegral));
      }
   }
   // grouped and folded
   for(size_t i = 0; i < xiBinEdges.size() - 1; ++i) {
      if(xiBinEdges[i] > 90.) { break; }
      // original code excluded bins 2, 42, 48, 89, 131, 136, and 177 because they "are isolated as single bins in the
      // plot" (which plot?)
      auto tmp      = xiPrompt->Integral(xiPrompt->FindBin(xiBinEdges[i]), xiPrompt->FindBin(xiBinEdges[i + 1]));
      auto tmpMixed = xiMixed->Integral(xiMixed->FindBin(xiBinEdges[i]), xiMixed->FindBin(xiBinEdges[i + 1]));
      // if the i+1th edge is over 90 degree (and we already checked the ith is below 90 degree), adding the inverted
      // angle integral would double count
      if(xiBinEdges[i + 1] < 90.) {
         tmp +=
            xiPrompt->Integral(xiPrompt->FindBin(180. - xiBinEdges[i + 1]), xiPrompt->FindBin(180. - xiBinEdges[i]));
         tmpMixed +=
            xiMixed->Integral(xiMixed->FindBin(180. - xiBinEdges[i + 1]), xiMixed->FindBin(180. - xiBinEdges[i]));
      }
      if(tmp != 0 && tmpMixed != 0) {
         auto point = asymmetryFolded->GetN();
         asymmetryFolded->SetPoint(point, (xiBinEdges[i] + xiBinEdges[i + 1]) / 2.,
                                   1. - (tmp / promptIntegral) / (tmpMixed / mixedIntegral));
         // original code does not include the uncertainty in the integrals for grouped points?
         asymmetryFolded->SetPointError(
            point, (xiBinEdges[i] - xiBinEdges[i + 1]) / 2.,
            (1. - (tmp / promptIntegral) / (tmpMixed / mixedIntegral)) *
               TMath::Sqrt(1. / tmp + 1. / promptIntegral + 1. / tmpMixed + 1. / mixedIntegral));
      }
   }

   // fit the asymmetry graphs
   TF1 fitFunction("fitFunction", "[0]*TMath::Cos(2.*x*TMath::Pi()/180.)", 0., 180.);
   fitFunction.SetParLimits(0, -10., 10.);
   asymmetry->Fit(&fitFunction, "R");
   asymmetryGrouped->Fit(&fitFunction, "R");
   asymmetryFolded->Fit(&fitFunction, "R");

   // calculate predicted asymmetry
   auto  p                  = Polarization(settings);
   int   nPoints            = settings->GetInt("NumberOfPoints", 100);
   auto* predictedAsymmetry = new TGraph(nPoints + 1);
   predictedAsymmetry->SetName("PredictedAsymmetry");
   predictedAsymmetry->SetTitle(Form("Predicted Asymmetry A^{CE} for Q = %f, P = %f", qValue, p));
   predictedAsymmetry->GetXaxis()->SetTitle("Azimuthal Compton Scattering Angle #xi (#circ)");
   predictedAsymmetry->GetYaxis()->SetTitle("Asymmetry A^{CE}");
   for(int i = 0; i <= nPoints; ++i) {
      predictedAsymmetry->SetPoint(i, i * (180. / nPoints),
                                   0.5 * qValue * p * TMath::Cos(2. * i * (180. / nPoints) * TMath::DegToRad()));
   }

   output.cd();

   xiPrompt->Write();
   xiMixed->Write();
   xiCount->Write();

   asymmetry->Write();
   asymmetryGrouped->Write();
   asymmetryFolded->Write();

   predictedAsymmetry->Write();

   output.Close();
   input.Close();

   return 0;
}

double KCoefficent(int mu, int L1, int L2)
{
   auto l1 = std::max(L1, L2);
   auto l2 = std::min(L1, L2);

   double k = 0.;
   if((l1 + l2) % 2 == 0) {
      k = (mu * (mu + 1.) * (l1 * (l1 + 1.) + l2 * (l2 + 1.)) - TMath::Power(l2 * (l2 + 1.) - l1 * (l1 + 1.), 2.)) /
          (l1 * (l1 + 1.) + l2 * (l2 + 1.) - mu * (mu + 1.));
   } else {
      k = l2 * (l2 + 1.) - l1 * (l1 + 1.);
   }

   return k * (TMath::Factorial(mu - 2) / TMath::Factorial(mu + 2));
}

double Polarization(TUserSettings* settings)
{
   auto f_JiJx  = settings->GetDoubleVector("F.Ji.Jx");
   auto f_JfJx  = settings->GetDoubleVector("F.Jf.Jx");
   auto mixing1 = settings->GetDouble("Mixing.1");
   auto mixing2 = settings->GetDouble("Mixing.2");
   int  l2      = settings->GetInt("L2");
   auto theta   = settings->GetDouble("Theta");

   double nominator   = 0.;
   double denominator = 1.;

   for(int i = 0; i < 4; ++i) {
      auto a = (f_JiJx[3 * i] - 2. * mixing1 * f_JiJx[3 * i + 1] + TMath::Power(mixing1, 2.) * f_JiJx[3 * i + 2]) /
               (1. + TMath::Power(mixing1, 2.));
      auto b = (f_JiJx[3 * i] + 2. * mixing2 * f_JiJx[3 * i + 1] + TMath::Power(mixing2, 2.) * f_JiJx[3 * i + 2]) /
               (1. + TMath::Power(mixing2, 2.));
      auto ap = (KCoefficent(2 * (i + 1), l2, l2) * f_JiJx[3 * i] -
                 KCoefficent(2 * (i + 1), l2, l2 + 1) * 2. * mixing2 * f_JiJx[3 * i + 1] -
                 KCoefficent(2 * (i + 1), l2 + 1, l2 + 1) * TMath::Power(mixing2, 2.) * f_JiJx[3 * i + 2]) /
                (1. + TMath::Power(mixing2, 2.));
      nominator += a * ap * ROOT::Math::assoc_legendre(2 * (i + 1), 2, TMath::Cos(theta * TMath::DegToRad()));
      denominator += a * b * ROOT::Math::legendre(2 * (i + 1), TMath::Cos(theta * TMath::DegToRad()));
   }

   if(settings->GetBool("Negative")) { return -nominator / denominator; }
   return nominator / denominator;
}
