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
#include <map>
#include <vector>
#include "TROOT.h"
#include "TMultiGraph.h"
#include "TFile.h"
#include "TH2D.h"
#include "TGraphErrors.h"
#include "TPeak.h"
#include "Math/Minimizer.h"
//#include "../include/TNucleus.h"
#include "../include/TGRSITransition.h"

const int ntrans = 15;

//double transition[8][2] = {
double transition[ntrans][2] = {
   {121.783 , 0.002},
   {244.692 , 0.002},
   {295.939 , 0.008},
   {344.276 , 0.004},
   {367.789 , 0.005},
   {411.115 , 0.005},
   {443.976 , 0.005},
   {688.678 , 0.006},
   {778.903 , 0.006},
   {867.388 , 0.008},
   {964.131 , 0.009},
   {1005.279, 0.017},
   {1212.95 , 0.012},
   {1299.124, 0.012},
   {1408.011, 0.140}};
/*
   {131.6 , 0.2},
   {212.8 , 0.1},
   {229.1 , 0.1},
//   {326.1 , 0.1},
   {336.241,0.025},
//   {372.2 , 0.1},
//   {388.9 , 0.3},
   {472.7 , 0.1},
   {649.1 , 0.1},
   {698.1 , 0.1}};
*/
double tranrange[ntrans][2] = {
   {15,15},
   {15,15},
   {10,10},
   {15,15},
   {10,10},
   {15,15},
   {15,15},
   {5,15},
   {15,15},
   {15,15},
   {15,15},
   {10,10},
   {15,15},
   {15,15},
   {15,15}};
   
   
   /*   {10,10},
   {10,10},
   {10,5},
//   5,
   {7,16},
//   {8,7},
//   4,
   {20,9},
   {10,10},
   {20,15}};
*/

void GetCal(TH1* hist, TGraphErrors* ge, TGraphErrors *fwhm){
   for(int i =0;i<ntrans;i++){
      double energy = transition[i][0];
      double denergy= transition[i][1];
      double rangelow = tranrange[i][0];
      double rangehigh = tranrange[i][1];
      TSpectrum s;
      hist->GetXaxis()->SetRangeUser(energy-rangelow,energy+rangehigh);
      s.Search(hist);

      Double_t peak_pos = s.GetPositionX()[0];
      hist->GetXaxis()->UnZoom();
      std::cout << "PEAK POS " << peak_pos <<  std::endl;
      TPeak *peak = new TPeak(peak_pos,peak_pos-rangelow,peak_pos+rangehigh);
 //     peak->Clear();
   //   peak->InitParams(hist);
      peak->Fit(hist,"+");
      ge->SetPoint(i,energy,energy - peak->GetCentroid());
      ge->SetPointError(i,denergy, TMath::Sqrt( TMath::Power(denergy,2) + TMath::Power(peak->GetCentroidErr(),2)));
      fwhm->SetPoint(i,energy,peak->GetFWHM());
      fwhm->SetPointError(i,denergy, peak->GetFWHMErr());
      
      delete peak;
   }
}



int main(int argc, char **argv) {
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","MIGRAD");
   TVirtualFitter::SetPrecision(1.0e-5);
   TVirtualFitter::SetMaxIterations(10000);
   if(argc != 2) {
      printf("try again (usage: %s <hist file>.\n",argv[0]);
      return 0;
   }

   TFile* file = new TFile(argv[1]);
   if(file == NULL) {
      printf("Failed to open file '%s'!\n",argv[1]);
      return 1;
   }
   if(!file->IsOpen()) {
      printf("Failed to open file '%s'!\n",argv[1]);
      return 1;
   }

   TH2D* eng_mat = new TH2D;
   TH1D* eng_sum = new TH1D;
   file->GetObject("hp_energy",eng_mat);
   file->GetObject("EnergySum",eng_sum);

   TFile *outfile = new TFile("calibration.root","RECREATE");

   TH1D* current_hist = new TH1D;
   TGraphErrors* ge = new TGraphErrors;
   TGraphErrors* fwhm = new TGraphErrors;
   for(int i =1;i<=64; i++){
      printf("NOW FITTING CHANNEL: %d \n",i);
      fwhm->Clear();
      ge->Clear();
      current_hist = eng_mat->ProjectionY(Form("chan%d_py",i),i+1,i+1);
      if(current_hist->Integral() < 1000) continue;
      GetCal(current_hist, ge, fwhm);
      ge->SetName(Form("chan%d_g",i));
      ge->GetXaxis()->SetTitle("Actual Energy (keV)");
      ge->GetYaxis()->SetTitle("Actual Energy - Centroid");
      fwhm->SetName(Form("chan%d_fwhm",i));
      fwhm->GetXaxis()->SetTitle("Actual Energy (keV)");
      fwhm->GetYaxis()->SetTitle("FWHM (keV)");
      current_hist->GetXaxis()->SetRangeUser(100,750);
      current_hist->Write();
      ge->Write();
      fwhm->Write();
   }
      //Now do sum
      fwhm->Clear();
      ge->Clear();
      GetCal(eng_sum,ge,fwhm);
      ge->SetName("sum_g");
      ge->GetXaxis()->SetTitle("Actual Energy (keV)");
      ge->GetYaxis()->SetTitle("Actual Energy - Centroid");
      fwhm->SetName("sum_fwhm");
      fwhm->GetXaxis()->SetTitle("Actual Energy (keV)");
      fwhm->GetYaxis()->SetTitle("FWHM (keV)");
      eng_sum->Write();
      ge->Write();
      fwhm->Write();

   

   file->Close();
   outfile->Close();
   return 1;
}
