#include <map>
#include <vector>

#include "TH1.h"
#include "TF1.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TMultiGraph.h"
#include "TFile.h"
#include "TH2D.h"
#include "TGraphErrors.h"
#include "TPeak.h"
#include "Math/Minimizer.h"

#include "ArgParser.h"
#include "TGRSITransition.h"

double transition[21][3]{
   {121.7830, 13620., 160.},
   {244.6920, 3590., 60.},
   {295.9390, 211., 5.},
   {344.2760, 12750., 90.},
   {367.7890, 405., 8.},
   {411.1150, 1070., 10.},
   {443.9760, 1480., 20.},
   {488.6610, 195., 2.},
   {564.0210, 236., 5.},
   {586.2940, 220., 5.},
   {678.5780, 221., 4.},
   {688.6780, 400., 8.},
   {778.9030, 6190., 80.},
   {867.3880, 1990., 40.},
   {964.1310, 6920., 90.},
   {1005.2790, 310., 7.},
   {1212.9500, 670., 8.},
   {1299.1240, 780., 10.},
   {1408.0110, 10000., 30.},
   {1173.228, 99.85, 0.03},
   {1332.492, 99.9826, 0.0006}};

double tranrange[21][2] = {
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10},
   {10, 10}};

void GetCal(TH1* hist, TGraphErrors* ge, bool abs_flag)
{
   int start = 0;
   int end   = 0;

   if(abs_flag) {
      start = 18;
      end   = 19;
   } else {
      start = 0;
      end   = 19;
   }
   for(int i = start; i < end; i++) {
      double    energy     = transition[i][0];
      double    intensity  = transition[i][1];
      double    dintensity = transition[i][2];
      double    rangelow   = tranrange[i][0];
      double    rangehigh  = tranrange[i][1];
      TSpectrum spec;
      hist->GetXaxis()->SetRangeUser(energy - rangelow, energy + rangehigh);
      spec.Search(hist);

      Double_t peak_pos = spec.GetPositionX()[0];
      hist->GetXaxis()->UnZoom();
      std::cout << "PEAK POS " << peak_pos << std::endl;
      auto* peak = new TPeak(peak_pos, peak_pos - rangelow, peak_pos + rangehigh);
      //   peak->Clear();
      peak->InitParams(hist);
      peak->Fit(hist, "+");
      double effic  = 0.0;
      double deffic = 0.0;

      effic  = peak->GetArea() / intensity;
      deffic = effic * TMath::Sqrt(TMath::Power((peak->GetAreaErr() / peak->GetArea()), 2) + TMath::Power((dintensity / intensity), 2));

      ge->SetPoint(i, energy, effic);
      ge->SetPointError(i, 0.0, deffic);

      delete peak;
   }
}

int main(int argc, char** argv)
{
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "MIGRAD");
   TVirtualFitter::SetPrecision(1.0e-5);
   TVirtualFitter::SetMaxIterations(10000);
   if(argc != 3) {
      printf("try again (usage: %s <hist file>. (0/1 = eu/co)\n", argv[0]);
      return 1;
   }

   bool abs_flag = (atoi(argv[2]) != 0);

   auto* file = new TFile(argv[1]);
   if(file == nullptr) {
      printf("Failed to open file '%s'!\n", argv[1]);
      return 1;
   }
   if(!file->IsOpen()) {
      printf("Failed to open file '%s'!\n", argv[1]);
      return 1;
   }

   auto* eng_mat = new TH2D;
   auto* eng_sum = new TH1D;
   file->GetObject("hp_energy", eng_mat);
   file->GetObject("EnergySum", eng_sum);

   std::string type;
   if(abs_flag) {
      type = "abs";
	} else {
      type = "rel";
	}

   const char* suff = type.c_str();

   auto* outfile = new TFile(Form("calibration%s.root", suff), "RECREATE");

   auto* current_hist = new TH1D;
   auto* ge           = new TGraphErrors;
   for(int i = 1; i <= 64; i++) {
      printf("NOW FITTING CHANNEL: %d \n", i);
      ge->Clear();
      current_hist = eng_mat->ProjectionY(Form("chan%d_py", i), i + 1, i + 1);
      GetCal(current_hist, ge, abs_flag);
      if(abs_flag) {
         ge->SetName(Form("chan%d_abs", i));
		} else {
         ge->SetName(Form("chan%d_rel", i));
		}

      ge->GetXaxis()->SetTitle("Energy (keV)");
      ge->GetYaxis()->SetTitle("Effic");
      current_hist->GetXaxis()->SetRangeUser(100, 1500);
      current_hist->Write();
      ge->Write();
   }
   // Now do sum
   ge->Clear();
   GetCal(eng_sum, ge, abs_flag);
   if(abs_flag) {
      ge->SetName("sum_abs");
	} else {
      ge->SetName("sum_rel");
	}
   ge->GetXaxis()->SetTitle("Energy (keV)");
   ge->GetYaxis()->SetTitle("Effic");
   eng_sum->Write();
   ge->Write();

   file->Close();
   outfile->Close();
   return 0;
}
