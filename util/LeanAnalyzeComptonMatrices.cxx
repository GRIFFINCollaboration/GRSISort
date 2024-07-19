/*
This script accompanies LeanComptonMatrices.c which is a derivative of the ComptonMatrices.c file.
It is called lean because I have taken out many of the portions that were simply for comparing different
methods.  As I have essentially settled on one method, these additional steps are just bloating.
Part of this fact means that I no longer do any weighting other that event mixing, and I also no longer
include the metzger Nparallel/Nperpendicular method of Compton polarimetry.  This script only includes
the portions that I believe would be useful for actual implementation of this code.  To use this you
should:
Set RUNNUMBER appropriately, as I do not have this information automatically pulled form input files.
Set the correct parameters within the PolarizationCalculation() function, either use an appropriate template one or create the missing one for your cascade.
Set things in Parameter Setup to what you want.
For appropriate theory plot set AGATA_Q to a known quality factor (preferrably one you recently measured)
Whatever scattering event was used to obtain AGATA_Q should be set as E_Q_Measure.  Q will then be scaled
appropriately to predict a scatter for an event of energy E_Q_Use, which must also be set.

Compile:
g++ LeanAnalyzeComptonMatrices.c -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries `grsi-config --cflags --all-libs` `root-config --cflags --libs` -lTreePlayer -lMathMore -lSpectrum -o MakeComptonPlots
Run:
Run with selected filename and no max entries:
./MakeComptonPlots /pathtomatrixfile/CompFile.root

This script is designed to create just the histograms required for producing the Compton polarimetry
plots.  These histograms can the be combined for multiple subruns using the hadd function.  The summed
file can be analysed using the AnalyzeComptonMatrices.c.

Because input files may have odd names, no dynamic output file names are given.  The output file
will be put in the directory specified when the outfile object is declared.  This names the output
based on the runnumber set in the definition section just below includes.

______________________________________________________________________________________________
*/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <Globals.h>
#include <string>

#include "TTree.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "TFile.h"
#include "TList.h"
#include "TFragment.h"
#include "TChannel.h"
#include "TApplication.h"
#include "TROOT.h"
#include "TChain.h"
#include "TMath.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TVector.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TStopwatch.h"
#include "TSpectrum.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"

#include "TRunInfo.h"
#include "TGRSISortInfo.h"
#include "TGRSIFunctions.h"

#define RUNNUMBER 10577

#define AGATA_Q 0.2509     // The quality factor for creating a predicted Asymmetry plot of 0.5*Q*P*Cos(2*Xi), where                 \
                           // P is determined by PolarizationCalculation()  <- Parameters must be set internally for each transition \
                           // Dan measured 0.24492 with 12 clovers in Summer 2016.                                                   \
                           // Adam measured 0.2509 with 16 clovers in Dec 2017.
#define E_Q_Measure 1332   // The energy of the scattered gamma used to calculate AGATA_Q
#define E_Q_Use 1332       // The energy of the scattered gamma currently being examined.  Used for scaling Q

// Functions
TList* ComptonPol(TFile* f, TStopwatch* w);

double Kcoefficents(int mu, int L1, int L2);
double PolarizationCalculation();
TList* AGATATheory(TList* list, double Q);
double ScaleQ(double E1, double E2);

#ifdef HAS_MATHMORE
// Main
int main(int argc, char** argv)
{
   if(argc != 2) {
      printf("try again (usage: %s <matrix file>).\n", argv[0]);
      return 0;
   }
   // We use a stopwatch so that we can watch progress
   TStopwatch w;
   w.Start();

   auto* file = new TFile(argv[1]);

   if(file == nullptr || !file->IsOpen()) {
      printf("Failed to open file '%s'!\n", argv[1]);
      return 1;
   }

   printf("Analyzing file:" DBLUE " %s" RESET_COLOR "\n", file->GetName());

   auto* outfile = new TFile(Form("./CompPlots_%05d.root", RUNNUMBER), "recreate");

   std::cout << argv[0] << ": starting Analysis after " << w.RealTime() << " seconds" << std::endl;
   w.Continue();
   auto* outlist = new TList();
   outlist       = ComptonPol(file, &w);
   outlist       = AGATATheory(outlist, AGATA_Q * ScaleQ(E_Q_Measure, E_Q_Use));
   if(outlist == nullptr) {
      std::cout << "ComptonPol returned TList* nullptr!" << std::endl;
      return 1;
   }
   outlist->Write();
   outfile->Close();
   std::cout << argv[0] << " done after " << w.RealTime() << " seconds" << std::endl
             << std::endl;

   std::cout << "ScaleQ(" << E_Q_Measure << "," << E_Q_Use << ") = " << ScaleQ(E_Q_Measure, E_Q_Use) << "\nE_Q_Measure = " << AGATA_Q << "\nE_Q_Use = " << (AGATA_Q * ScaleQ(E_Q_Measure, E_Q_Use)) << std::endl;

   return 0;
}

// ****************************************************************** //
// ********************* FUNCTION DEFINITIONS *********************** //
// ****************************************************************** //

TList* ComptonPol(TFile* f, TStopwatch* w)
{
   if(w == nullptr) {
      w = new TStopwatch;
      w->Start();
   }

   ////////////////////////////////////////////////////////////////////////////////////
   //---------------------------- Parameter Setup -----------------------------------//
   ////////////////////////////////////////////////////////////////////////////////////

   bool   UseDetCoincidenceAngle      = true;   // If true gates are performed on DetDet angle, if false on CryCry angle
   double RestrictCoincidenceAngleMin = 85.0;   // All cuts with bin centers b/w min and max included in //was 85 for det
   double RestrictCoincidenceAngleMax = 95.0;   // the resulting histograms (these are inclusive)  //was 95 for det
   // Adam measures the following Q factors for 1332keV in Dec 2017, with the angle selection above.
   // 85-95 = 0.249
   // 50-130 = 0.1618
   // 40-140 = 0.1423
   // Conclusion is that including more detectors at angles away from 90 reduces the quality factor.

   ////////////////////////////////////////////////////////////////////////////////////
   //--------------------- Loading/Making histograms --------------------------------//
   ////////////////////////////////////////////////////////////////////////////////////

   auto* list = new TList;   // Output list

   auto* XiHist2D_DetDet = static_cast<TH2D*>(f->Get("XiHist2D_DetDetCoincidenceTheta"));
   list->Add(XiHist2D_DetDet);
   auto* XiHist2D_CryCry = static_cast<TH2D*>(f->Get("XiHist2D_CryCryCoincidenceTheta"));
   list->Add(XiHist2D_CryCry);

   auto* XiHist2DGeo_DetDet = static_cast<TH2D*>(f->Get("XiHist2D_DetDetCoincidenceTheta_Geo"));
   list->Add(XiHist2DGeo_DetDet);
   auto* XiHist2DGeo_CryCry = static_cast<TH2D*>(f->Get("XiHist2D_CryCryCoincidenceTheta_Geo"));
   list->Add(XiHist2DGeo_CryCry);

   auto* XiHist2DNonCo_DetDet = static_cast<TH2D*>(f->Get("XiHist2D_DetDetCoincidenceTheta_NonCo"));
   list->Add(XiHist2DNonCo_DetDet);
   auto* XiHist2DNonCo_CryCry = static_cast<TH2D*>(f->Get("XiHist2D_CryCryCoincidenceTheta_NonCo"));
   list->Add(XiHist2DNonCo_CryCry);

   int ThetaBins = XiHist2D_DetDet->GetNbinsY();   // Binsize = 180deg / bins
   int XiBins    = XiHist2D_DetDet->GetNbinsX();

   char* XiHistTitle      = Form("Measured #xi Angles for Real Triplets ( %.1f <= #theta <= %.1f ) -> UseDetCoincidenceAngle = %d;Experimental Angle #xi (#circ);Counts", RestrictCoincidenceAngleMin, RestrictCoincidenceAngleMax, static_cast<int>(UseDetCoincidenceAngle));
   char* XiHistGeoTitle   = Form("Possible #xi Angles in GRIFFIN Array ( %.1f <= #theta <= %.1f ) -> UseDetCoincidenceAngle = %d;Experimental Angle #xi (#circ);Counts", RestrictCoincidenceAngleMin, RestrictCoincidenceAngleMax, static_cast<int>(UseDetCoincidenceAngle));
   char* XiHistNonCoTitle = Form("Measured #xi Angles for Non-Coincident #gamma_{1} and #gamma_{2} ( %.1f <= #theta <= %.1f ) -> UseDetCoincidenceAngle = %d;Experimental Angle #xi (#circ);Counts", RestrictCoincidenceAngleMin, RestrictCoincidenceAngleMax, static_cast<int>(UseDetCoincidenceAngle));

   auto* XiHist = new TH1D("XiHist", XiHistTitle, XiBins, XiHist2D_DetDet->GetXaxis()->GetBinLowEdge(1), XiHist2D_DetDet->GetXaxis()->GetBinLowEdge(XiBins + 1));
   list->Add(XiHist);
   auto* XiHistGeo = new TH1D("XiHist_Geo", XiHistGeoTitle, XiBins, XiHist2D_DetDet->GetXaxis()->GetBinLowEdge(1), XiHist2D_DetDet->GetXaxis()->GetBinLowEdge(XiBins + 1));
   list->Add(XiHistGeo);
   auto* XiHistNonCo = new TH1D("XiHist_NonCo", XiHistNonCoTitle, XiBins, XiHist2D_DetDet->GetXaxis()->GetBinLowEdge(1), XiHist2D_DetDet->GetXaxis()->GetBinLowEdge(XiBins + 1));
   list->Add(XiHistNonCo);

   auto* AsymmetryNonCo = new TGraphErrors();
   list->Add(AsymmetryNonCo);
   auto* AsymmetryBinnedNonCo = new TGraphErrors();
   list->Add(AsymmetryBinnedNonCo);
   auto* AsymmetryBinFoldNonCo = new TGraphErrors();
   list->Add(AsymmetryBinFoldNonCo);

   /////////////////////////////////////////////////////////////////////////////////////
   //-------------------------- Gated Projection Plots -------------------------------//
   /////////////////////////////////////////////////////////////////////////////////////

   for(int bin = 1; bin < ThetaBins; bin++) {
      if(XiHist2D_DetDet->GetYaxis()->GetBinCenter(bin) >= RestrictCoincidenceAngleMin && XiHist2D_DetDet->GetYaxis()->GetBinCenter(bin) <= RestrictCoincidenceAngleMax) {
         if(UseDetCoincidenceAngle) {
            XiHist->Add(XiHist2D_DetDet->ProjectionX("", bin, bin));
            XiHistGeo->Add(XiHist2DGeo_DetDet->ProjectionX("", bin, bin));
            XiHistNonCo->Add(XiHist2DNonCo_DetDet->ProjectionX("", bin, bin));
         } else {
            XiHist->Add(XiHist2D_DetDet->ProjectionX("", bin, bin));
            XiHistGeo->Add(XiHist2DGeo_DetDet->ProjectionX("", bin, bin));
            XiHistNonCo->Add(XiHist2DNonCo_DetDet->ProjectionX("", bin, bin));
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////////
   //---------------------------- Asymmetry Plots ------------------------------------//
   /////////////////////////////////////////////////////////////////////////////////////

   TF1* fun1 = new TF1("fun1", "[0]*cos(2*(x*TMath::Pi()/180.0))", 0.0, 180.0);
   fun1->SetParLimits(0, -10.0, 10.0);

   int                 nPoints = 0;
   std::vector<double> AsymmetryX, AsymmetryY, AsymmetryYerr;
   std::vector<double> XiBinEdges = {0.0, 2.0, 3.0, 6.0, 14.0, 31.0, 39.0, 50.0, 61., 74., 87., 93., 106., 119., 130., 141., 149., 166., 174., 177., 178., 180.0};   // Assymetric

   //--------NonCo Asymm Plot-------//

   AsymmetryX.clear();
   AsymmetryX.clear();
   AsymmetryY.clear();
   AsymmetryYerr.clear();
   for(int bin = 1; bin < XiHist->GetXaxis()->GetNbins(); bin++) {
      printf("%d %d\n", bin, static_cast<int>(XiHist->GetBinCenter(bin)));
      if(XiHist->GetBinContent(bin) == 0 || XiHistNonCo->GetBinContent(bin) == 0) { continue; }

      // Exclude specific bins which are isolated as single bins in the plot -they have low statistics because cannot group with anything else
      if(static_cast<int>(XiHist->GetBinCenter(bin)) == 2 || static_cast<int>(XiHist->GetBinCenter(bin)) == 43 || static_cast<int>(XiHist->GetBinCenter(bin)) == 48 || static_cast<int>(XiHist->GetBinCenter(bin)) == 89 || static_cast<int>(XiHist->GetBinCenter(bin)) == 90 || static_cast<int>(XiHist->GetBinCenter(bin)) == 131 || static_cast<int>(XiHist->GetBinCenter(bin)) == 136 || static_cast<int>(XiHist->GetBinCenter(bin)) == 177) { continue; }

      double Ace = 1 - (XiHist->GetBinContent(bin) / XiHist->Integral()) / (XiHistNonCo->GetBinContent(bin) / XiHistNonCo->Integral());
      AsymmetryNonCo->SetPoint(nPoints, XiHist->GetBinCenter(bin), Ace);
      AsymmetryX.push_back(XiHist->GetBinCenter(bin));
      AsymmetryY.push_back(Ace);
      double ex = XiHist->GetBinWidth(bin) / 2.0;
      double ey = Ace * TMath::Sqrt(1.0 / XiHist->GetBinContent(bin) + 1.0 / XiHist->Integral() + 1.0 / XiHistNonCo->GetBinContent(bin) + 1.0 / XiHistNonCo->Integral());
      AsymmetryYerr.push_back(ey);
      AsymmetryNonCo->SetPointError(nPoints, ex, ey);
      nPoints++;
   }

   //--------Binned NonCo Asymm Plot-------//
   nPoints = 0;
   for(size_t loop = 1; loop < XiBinEdges.size(); ++loop) {
      double XiBinCount    = 0.0;
      double UnPolBinCount = 0.0;
      for(int bin = 1; bin < XiHist->GetXaxis()->GetNbins(); bin++) {
         if(XiHist->GetBinContent(bin) == 0 || XiHistNonCo->GetBinContent(bin) == 0) { continue; }

         // Exclude specific bins which are isolated as single bins in the plot -they have low statistics because cannot group with anything else
         if(static_cast<int>(XiHist->GetBinCenter(bin)) == 2 || static_cast<int>(XiHist->GetBinCenter(bin)) == 43 || static_cast<int>(XiHist->GetBinCenter(bin)) == 48 || static_cast<int>(XiHist->GetBinCenter(bin)) == 89 || static_cast<int>(XiHist->GetBinCenter(bin)) == 90 || static_cast<int>(XiHist->GetBinCenter(bin)) == 131 || static_cast<int>(XiHist->GetBinCenter(bin)) == 136 || static_cast<int>(XiHist->GetBinCenter(bin)) == 177) { continue; }

         if(XiHist->GetBinCenter(bin) >= XiBinEdges[loop - 1] && XiHist->GetBinCenter(bin) < XiBinEdges[loop]) {
            XiBinCount += XiHist->GetBinContent(bin);
            UnPolBinCount += XiHistNonCo->GetBinContent(bin);
         }
      }
      if(XiBinCount == 0.0 || UnPolBinCount == 0.0) { continue; }
      double Ace = 1.0 - (XiBinCount / XiHist->Integral()) / (UnPolBinCount / XiHistNonCo->Integral());
      double ey  = TMath::Sqrt(1.0 / XiBinCount + 1.0 / UnPolBinCount);
      double ex  = (XiBinEdges[loop] - XiBinEdges[loop - 1]) / 2.0;
      AsymmetryBinnedNonCo->SetPoint(nPoints, (XiBinEdges[loop] + XiBinEdges[loop - 1]) / 2.0, Ace);
      AsymmetryBinnedNonCo->SetPointError(nPoints, ex, ey);
      nPoints++;
   }
   AsymmetryBinnedNonCo->Fit("fun1", "R");

   //--------Binned and Folded NonCo Asymm Plot-------//
   nPoints = 0;
   for(size_t loop = 1; loop < XiBinEdges.size(); ++loop) {
      double XiBinCount    = 0.0;
      double UnPolBinCount = 0.0;
      for(int bin = 1; bin < XiHist->GetXaxis()->GetNbins(); bin++) {
         // Exclude specific bins which are isolated as single bins in the plot -they have low statistics because cannot group with anything else
         if(static_cast<int>(XiHist->GetBinCenter(bin)) == 2 || static_cast<int>(XiHist->GetBinCenter(bin)) == 43 || static_cast<int>(XiHist->GetBinCenter(bin)) == 48 || static_cast<int>(XiHist->GetBinCenter(bin)) == 89 || static_cast<int>(XiHist->GetBinCenter(bin)) == 90 || static_cast<int>(XiHist->GetBinCenter(bin)) == 131 || static_cast<int>(XiHist->GetBinCenter(bin)) == 136 || static_cast<int>(XiHist->GetBinCenter(bin)) == 177) { continue; }

         double FoldedCenter = std::min(XiHist->GetBinCenter(bin), 180.0 - XiHist->GetBinCenter(bin));
         if(XiHist->GetBinContent(bin) == 0 || XiHistNonCo->GetBinContent(bin) == 0) { continue; }

         if(FoldedCenter >= XiBinEdges[loop - 1] && FoldedCenter < XiBinEdges[loop]) {
            XiBinCount += XiHist->GetBinContent(bin);
            UnPolBinCount += XiHistNonCo->GetBinContent(bin);
         }
      }
      if(XiBinCount == 0.0 || UnPolBinCount == 0.0) { continue; }
      double Ace = 1.0 - (XiBinCount / XiHist->Integral()) / (UnPolBinCount / XiHistNonCo->Integral());
      double ey  = TMath::Sqrt(1.0 / XiBinCount + 1.0 / UnPolBinCount);
      double ex  = (XiBinEdges[loop] - XiBinEdges[loop - 1]) / 2.0;
      AsymmetryBinFoldNonCo->SetPoint(nPoints, (XiBinEdges[loop] + XiBinEdges[loop - 1]) / 2.0, Ace);
      AsymmetryBinFoldNonCo->SetPointError(nPoints, ex, ey);
      nPoints++;
   }
   AsymmetryBinFoldNonCo->Fit("fun1", "R");

   ////////////////////////////////////////////////////////////////////////////////////
   //---------------------------- Display Setup -------------------------------------//
   ////////////////////////////////////////////////////////////////////////////////////

   AsymmetryNonCo->SetMarkerStyle(21);
   AsymmetryNonCo->SetMarkerSize(0.5);
   AsymmetryNonCo->SetMarkerColor(kRed);
   AsymmetryNonCo->SetLineColor(kBlack);
   AsymmetryNonCo->SetTitle("Asymmetry A^{CE} with Non-Coincident (in time) Counts for Unpolarized Events");
   AsymmetryNonCo->SetName("AsymmetryNonCo");
   AsymmetryNonCo->GetXaxis()->SetTitle("Azimuthal Compton Scattering Angle #xi (#circ)");
   AsymmetryNonCo->GetXaxis()->CenterTitle();
   AsymmetryNonCo->GetYaxis()->SetTitle("Asymmetry A^{CE}");
   AsymmetryNonCo->GetYaxis()->CenterTitle();
   AsymmetryNonCo->GetXaxis()->SetRangeUser(0.0, 180.0);

   AsymmetryBinnedNonCo->SetMarkerStyle(21);
   AsymmetryBinnedNonCo->SetMarkerSize(0.5);
   AsymmetryBinnedNonCo->SetMarkerColor(kRed);
   AsymmetryBinnedNonCo->SetLineColor(kBlack);
   AsymmetryBinnedNonCo->SetTitle("Asymmetry A^{CE} Binned with Non-Coincident (in time) Counts for Unpolarized Events");
   AsymmetryBinnedNonCo->SetName("AsymmetryBinnedNonCo");
   AsymmetryBinnedNonCo->GetXaxis()->SetTitle("Azimuthal Compton Scattering Angle #xi (#circ)");
   AsymmetryBinnedNonCo->GetXaxis()->CenterTitle();
   AsymmetryBinnedNonCo->GetYaxis()->SetTitle("Asymmetry A^{CE}");
   AsymmetryBinnedNonCo->GetYaxis()->CenterTitle();
   AsymmetryBinnedNonCo->GetXaxis()->SetRangeUser(0.0, 180.0);

   AsymmetryBinFoldNonCo->SetMarkerStyle(21);
   AsymmetryBinFoldNonCo->SetMarkerSize(0.5);
   AsymmetryBinFoldNonCo->SetMarkerColor(kRed);
   AsymmetryBinFoldNonCo->SetLineColor(kBlack);
   AsymmetryBinFoldNonCo->SetTitle("Asymmetry A^{CE} Binned and Folded with Non-Coincident (in time) Counts for Unpolarized Events");
   AsymmetryBinFoldNonCo->SetName("AsymmetryBinFoldNonCo");
   AsymmetryBinFoldNonCo->GetXaxis()->SetTitle("Azimuthal Compton Scattering Angle #xi (#circ)");
   AsymmetryBinFoldNonCo->GetXaxis()->CenterTitle();
   AsymmetryBinFoldNonCo->GetYaxis()->SetTitle("Asymmetry A^{CE}");
   AsymmetryBinFoldNonCo->GetYaxis()->CenterTitle();
   AsymmetryBinFoldNonCo->GetXaxis()->SetRangeUser(0.0, 180.0);
   std::cout << std::endl;
   return list;
}

TList* AGATATheory(TList* list, double Q)
{
   auto*  PredictedAsymmetry = new TGraph();
   int    MaxPoints          = 100;
   double P                  = PolarizationCalculation();
   for(int loop = 0; loop <= MaxPoints; loop++) {
      PredictedAsymmetry->SetPoint(loop, loop * (180.0 / MaxPoints), 0.5 * Q * P * TMath::Cos(2.0 * TMath::DegToRad() * (loop * (180.0 / MaxPoints))));
   }

   PredictedAsymmetry->SetMarkerStyle(21);
   PredictedAsymmetry->SetMarkerSize(0.6);
   PredictedAsymmetry->SetMarkerColor(kBlack);
   PredictedAsymmetry->SetLineColor(kBlack);
   PredictedAsymmetry->SetTitle(Form("Predicted Asymmetry A^{CE} for Q = %f, P = %f", Q, PolarizationCalculation()));
   PredictedAsymmetry->SetName("AGATATheory");
   PredictedAsymmetry->GetXaxis()->SetTitle("Azimuthal Compton Scattering Angle #xi (#circ)");
   PredictedAsymmetry->GetXaxis()->CenterTitle();
   PredictedAsymmetry->GetYaxis()->SetTitle("Asymmetry A^{CE}");
   PredictedAsymmetry->GetYaxis()->CenterTitle();
   PredictedAsymmetry->GetXaxis()->SetRangeUser(0.0, 180.0);

   list->Add(PredictedAsymmetry);
   return list;
}

double PolarizationCalculation()
{
   // This function can depend highly on the state.  As it requires the user to look up values in tables
   // the actual calculations are not easily automated.  Several values are required as input parameters.
   // Calculations will be made for A, Aprime, and B.  These require both F and K coeffients which can
   // be found in tables and are dependant on the states in question. The sample values for 60Co are found
   // at the bottom of the function

   /*
   //207Bi 1064->569   (M4/E5)-E2
   // Scattering of the 569 keV gamma ray
   double F_JiJx[12] = {-0.4130, 0.3419, 0.3700, 0.0291, -0.1367, -0.2374, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, }; //13/2->5/2 (only for even nu??)
   double F_JfJx[12] = { -0.5345, -0.3780, -0.8018, -0.6172, 1.0911, 0.1543, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; // 5/2->1/2 (only even)
   double PlusOrMinus = 1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.03; //The mixing ratio of the first transition
   double Mixing2 = 0.0; //The mixing ratio of the second transition
   int L1 = 4;
   int L2 = 2;
   double theta = 90.0; //Deg
   //--------------------------//
   */
   /*
   //207Bi 569->1064   E2-(M4/E5)
   // Scattering of the 1064 keV gamma ray
   double F_JiJx[12] = { -0.5345, -0.3780, -0.8018, -0.6172, 1.0911, 0.1543, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; // 5/2->1/2 (only even)
   double F_JfJx[12] = {-0.4130, 0.3419, 0.3700, 0.0291, -0.1367, -0.2374, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, }; //13/2->5/2 (only for even nu??)
   double PlusOrMinus = -1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.0; //The mixing ratio of the first transition
   double Mixing2 = 0.03; //The mixing ratio of the second transition
   int L1 = 2;
   int L2 = 4;
   double theta = 90.0; //Deg
   //--------------------------//
   */
   /*
   //60Co 1332->1173   E2-E2
   // Scattering of the 1173 keV gamma ray
   double F_JiJx[12] = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 2->0
   double F_JfJx[12] = { -.1707, 0.5051, 0.4482, -0.0085, 0.0627, -0.0297, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 4->2
   double PlusOrMinus = 1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.0; //The mixing ratio of the first transition
   double Mixing2 = 0.0; //The mixing ratio of the second transition
   int L1 = 2;
   int L2 = 2;
   double theta = 90.0; //Deg
   //--------------------------//
   */
   /*
   //60Co 1173->1332   E2-E2
   // Scattering of the 1332 keV gamma ray
   double F_JiJx[12] = { -.1707, 0.5051, 0.4482, -0.0085, 0.0627, -0.0297, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 4->2
   double F_JfJx[12] = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 2->0
   double PlusOrMinus = 1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.0; //The mixing ratio of the first transition
   double Mixing2 = 0.0; //The mixing ratio of the second transition
   int L1 = 2;
   int L2 = 2;
   double theta = 90.0; //Deg
   //--------------------------//
   */
   /*
   //62Ga 1388->954   2-2-0 (M1+E2)-E2
   // Scattering of the 1388 keV gamma ray
   double F_JiJx[12] = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 0->2
   double F_JfJx[12] = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 2->0
   double PlusOrMinus = 1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.0; //The mixing ratio of the first transition
   double Mixing2 = 0.0; //The mixing ratio of the second transition
   int L1 = 2;
   int L2 = 2;
   double theta = 90.0; //Deg
   //--------------------------//
   */

   // 62Ga 1388->954   0-2-0 E2-E2
   //  Scattering of the 1388 keV gamma ray
   double F_JiJx[12]  = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};   // 0->2
   double F_JfJx[12]  = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};   // 2->0
   double PlusOrMinus = 1.0;                                                                    // either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1     = 0.5;                                                                    // The mixing ratio of the first transition
   double Mixing2     = 0.0;                                                                    // The mixing ratio of the second transition
   int    L2          = 2;
   double theta       = 90.0;   // Deg
   //--------------------------//

   /*
   //152Eu 779->344    E1-E2
   double F_JiJx[12] = {0.1195, 0.6547, 0.3415, 0.0, 0.0, 0.0764, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; //3->2
   double F_JfJx[12] = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 2->0
   double PlusOrMinus = 1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.0; //The mixing ratio of the first transition
   double Mixing2 = 0.0; //The mixing ratio of the second transition
   int L1 = 1;
   int L2 = 2;
   double theta = 90.0; //Deg
   //--------------------------//
   */
   /*
   //152Eu 1299->344    (E1/M2)-E2
   double F_JiJx[12] = {-0.4183, -0.6124, 0.1281, 0.0, 0.0, -0.3054, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; //2->2
   double F_JfJx[12] = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 2->0
   double PlusOrMinus = 1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.0; //The mixing ratio of the first transition This one may not be zero but I couldn't find it.  It is E1/M2 so it should be small.
   double Mixing2 = 0.0; //The mixing ratio of the second transition
   int L1 = 1;
   int L2 = 2;
   double theta = 90.0; //Deg
   //--------------------------//
   */
   /*
   //152Eu 344->1299    E2-(E1/M2)
   double F_JiJx[12] = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 0->2
   double F_JfJx[12] = {-0.4183, -0.6124, 0.1281, 0.0, 0.0, -0.3054, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; //2->2
   double PlusOrMinus = 1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.0; //The mixing ratio of the first transition
   double Mixing2 = 0.0; //The mixing ratio of the second transition This one may not be zero but I couldn't find it.  It is E1/M2 so it should be small.
   int L1 = 2;
   int L2 = 1;
   double theta = 90.0; //Deg
   //--------------------------//
   */
   /*
   //152Eu 411->344    E2-E2
   double F_JiJx[12] = { -.1707, 0.5051, 0.4482, -0.0085, 0.0627, -0.0297, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 4->2
   double F_JfJx[12] = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 2->0
   double PlusOrMinus = 1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.0; //The mixing ratio of the first transition
   double Mixing2 = 0.0; //The mixing ratio of the second transition
   int L1 = 2;
   int L2 = 2;
   double theta = 90.0; //Deg
   //--------------------------//
   */
   /*
   //152Eu 1089->344    (M1/E2)-E2
   double F_JiJx[12] = {0.1195, 0.6547, 0.3415, 0.0, 0.0, 0.0764, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; //3->2
   double F_JfJx[12] = {-0.5976, 0.0, 0.0, -1.0690, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // 2->0
   double PlusOrMinus = 1.0; //either 1.0 or -1.0 depending on (+-)_L2.
   double Mixing1 = 0.0; //The mixing ratio of the first transition
   double Mixing2 = 0.0; //The mixing ratio of the second transition
   int L1 = 1;
   int L2 = 2;
   double theta = 90.0; //Deg
   //--------------------------//
   */

   //---- Calculations ----//

   double A[4];
   double Ap[4];
   double B[4];
   double Pnum        = 0.0;
   double Pdenom      = 1.0;
   int    MuValues[4] = {2, 4, 6, 8};   // When making this initally I was not 100% certain these indices
                                        // would always be just the even ones, as I found the paper unclear
                                        // on that point.  That is why they are placed here, rather tha just
                                        // multiplying the current index by two or whatever to give the appropriate number.

   for(int loop = 0; loop < 4; loop++) {
      A[loop]  = (1.0 / (1.0 + TMath::Power(Mixing1, 2.0))) * (F_JiJx[3 * loop + 0] - 2.0 * Mixing1 * F_JiJx[3 * loop + 1] + TMath::Power(Mixing1, 2) * F_JiJx[3 * loop + 2]);
      B[loop]  = (1.0 / (1.0 + TMath::Power(Mixing2, 2.0))) * (F_JfJx[3 * loop + 0] + 2.0 * Mixing2 * F_JfJx[3 * loop + 1] + TMath::Power(Mixing2, 2) * F_JfJx[3 * loop + 2]);
      Ap[loop] = (1.0 / (1.0 + TMath::Power(Mixing2, 2.0))) * (Kcoefficents(MuValues[loop], L2, L2) * F_JfJx[3 * loop + 0] - Kcoefficents(MuValues[loop], L2, L2 + 1) * 2.0 * Mixing2 * F_JfJx[3 * loop + 1] - Kcoefficents(MuValues[loop], L2 + 1, L2 + 1) * TMath::Power(Mixing2, 2) * F_JfJx[3 * loop + 2]);
      Pnum += A[loop] * Ap[loop] * ROOT::Math::assoc_legendre(MuValues[loop], 2, TMath::Cos(TMath::DegToRad() * theta));
      Pdenom += A[loop] * B[loop] * ROOT::Math::legendre(MuValues[loop], TMath::Cos(TMath::DegToRad() * theta));
   }

   double P = PlusOrMinus * (Pnum / Pdenom);
   return P;
}

double Kcoefficents(int mu, int L1, int L2)
{
   double k = 0.;
   double l1 = L1;
   double l2 = L2;
   L1 = std::max(l1, l2);
   L2 = std::min(l1, l2);

   if((L1 + L2) % 2 == 0) {
      k = (static_cast<double>(mu) * (mu + 1.0) * (L1 * (L1 + 1.0) + L2 * (L2 + 1.0)) - TMath::Power(L2 * (L2 + 1.0) - L1 * (L1 + 1.0), 2)) / (L1 * (L1 + 1.0) + L2 * (L2 + 1.0) - mu * (mu + 1.0));
   } else {
      k = L2 * (L2 + 1.0) - L1 * (L1 + 1.);
   }
   return k * (TMath::Factorial(mu - 2.) / TMath::Factorial(mu + 2));
}

double ScaleQ(double E1, double E2)
{
   // E1 is the energy of the scattering gamma used to determine the Q you wish to scale.  E2 is the
   // energy  of the gamma you are trying to examine now.
   int    nDiv = 1000;
   double m_eC2 = 510.9989;   // keV
   double Q1    = 0.0;
   double Q2    = 0.0;
   for(int loop = 0; loop < nDiv; loop++) {
      double theta = loop * (180.0 / nDiv);

      double E1p = E1 / (1.0 + E1 / m_eC2 * (1 - TMath::Cos(TMath::DegToRad() * theta)));
      double E2p = E2 / (1.0 + E2 / m_eC2 * (1 - TMath::Cos(TMath::DegToRad() * theta)));

      double sigma1 = TMath::Power(TMath::Sin(TMath::DegToRad() * theta), 2) / (E1p / E1 + E1 / E1p - TMath::Power(TMath::Sin(TMath::DegToRad() * theta), 2));
      double sigma2 = TMath::Power(TMath::Sin(TMath::DegToRad() * theta), 2) / (E2p / E2 + E2 / E2p - TMath::Power(TMath::Sin(TMath::DegToRad() * theta), 2));
      Q1 += (180.0 / nDiv) * (sigma1);
      Q2 += (180.0 / nDiv) * (sigma2);
   }
   Q1 /= 180.0;
   Q2 /= 180.0;

   return Q2 / Q1;
}

#else
int main(int, char** argv)
{
   std::cout << "No mathmore support in ROOT, " << argv[0] << " can't run!" << std::endl;
   return 1;
}
#endif
