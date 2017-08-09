//Written by: B. Olaizola 2017-06-13 

//Experiment: S1067

//Root version: 6.08/02

//GRSIsort version: 2.4.00

//This script will plot the dependeance of the TAC channels with the temperature. It makes a temperature-dependant calibration that is then fed to another script that then corrects these oscillations. The input is one or more (chain) analysistrees.


#include <iostream>
#include <fstream>
using namespace std;
#include <iomanip>
#include <string>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "THnSparse.h"
#include "TGraph.h"
#include "TGraphErrors.h"

#include "TGriffin.h"
#include "TSceptar.h"
#include "TLaBr.h"
#include "TZeroDegree.h"
#include "TTAC.h"
#include "TString.h"

#include <TCanvas.h> //For plots

//Hardcoded cal file, this may change in the future.
const char* calfile = "/data1/bruno/griffin/S1607/calibration_HPGe_S1607.cal";

//Gaussian function
Double_t gauss(Double_t *t, Double_t *par)
{
  Double_t fillval ;
  fillval = par[0] * (TMath::Exp(-1/2*pow((t[0]-par[1])/par[2],2)));
    return fillval;
}

//Linear function
Double_t linear(Double_t *t, Double_t *par)
{
  Double_t fillval ;
  fillval = par[0] + t[0]*par[1];
    return fillval;
}

int main(int argc, char** argv) {
  if(argc == 1) {
    std::cout<<"Usage: "<<argv[0]<<" <analysis tree file(s)>"<<std::endl;
    return 1;
  }

  std::cout<<"List of files: "<<std::endl;
  std::cout<<" "<<std::endl;
  for(int i = 1; i < argc; ++i) {
    std::cout<<argv[i]<<std::endl;
  }
  std::cout<<" "<<std::endl;

  
  //TTree AnalysisTree("AnalysisTree");
  TFile * f;
  
  for(int file_num = 1; file_num < argc; ++file_num) {
    f = new TFile(argv[file_num]);
    TTree * AnalysisTree = (TTree*)f->Get("AnalysisTree");
  
  //// Here is automatically loading a calibration file into each root file. If you do not wish you apply the same calibration file to each run, comment this section ///////
  printf("Reading calibration file: %s\n",calfile);
  TChannel::ReadCalFile(calfile);
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  TLaBr* labr = NULL;
  TZeroDegree* zds = NULL;
  TTAC* tac = NULL;
  TLaBrHit* labrhit = NULL;
  TZeroDegreeHit* zdshit = NULL;
  TTACHit* tachit = NULL;


  bool gotLaBr;
  if(AnalysisTree->FindBranch("TLaBr") == 0) {   //We check to see if we have a LaBr branch in the analysis tree
    gotLaBr = false;
  } else {
    AnalysisTree->SetBranchAddress("TLaBr", &labr);
    gotLaBr = true;
  }
  bool gotZeroDegree;
  if(AnalysisTree->FindBranch("TZeroDegree") == 0) {   //We check to see if we have a ZeroDegree branch in the analysis tree
    gotZeroDegree = false;
  } else {
    AnalysisTree->SetBranchAddress("TZeroDegree", &zds);
    gotZeroDegree = true;
  }
  bool gotTAC;
  if(AnalysisTree->FindBranch("TTAC") == 0) {   //We check to see if we have a TAC branch in the analysis tree
    gotTAC = false;
  } else {
    AnalysisTree->SetBranchAddress("TTAC", &tac);
    gotTAC = true;
  }
  
  Long64_t nEntries = AnalysisTree->GetEntries();


  //Automatically generates the outfile name
  std::string outname = argv[file_num];
  outname.replace(outname.begin(),outname.end()-14,"temperature_correction_");
  std::cout<<"writing to '"<<outname<<"'"<<std::endl;
  
  TFile outfile(outname.c_str(),"recreate");
  
  TList list;
  
  Double_t  progress;
  Int_t  multi_labr, multi_zds, multi_tac, tac_num;
  double_t sec_per_bin = 30.;//"calibration" of the number of seconds per bin (sec/chan). The tipical temperature cycle is ~300 secs, so adjust accordingly
  double_t file_length = 7000.;//time length of the file, in secs. Doesn't need to be precise
  double_t ps_per_bin = 1.;//TAC bining, in picoseconds per bin.
  int TAC_bins =static_cast<int>(50000/ps_per_bin);//number of bins for the TAC axis
  
//subsequent subruns won't start at 0, so I need to stablish the start and final time of each file
  double_t time_max,time_min;
  for(int counter_min = 0; counter_min<nEntries; ++counter_min){
    AnalysisTree->GetEntry(counter_min);
    if (tac->GetMultiplicity()>=1){
      time_min=tac->GetTACHit(0)->GetTime()/1E9;
      std::cout<<"File starts at time:"<<time_min<<" s"<<std::endl;
      std::cout<<"And counter: "<<counter_min<<std::endl;
      break;
    }else continue;
  }  
  for(int counter_max = nEntries-1; counter_max>0; counter_max--){
    AnalysisTree->GetEntry(counter_max);
    if (tac->GetMultiplicity()>=1){
      time_max=tac->GetTACHit(0)->GetTime()/1E9;
      std::cout<<"File ends at time:"<<time_max<<" s"<<std::endl;
      std::cout<<"And counter: "<<counter_max<<std::endl;
      break;
    }else continue;
  }  


  file_length = time_max - time_min;
  int time_bins =static_cast<int>(file_length/sec_per_bin);//number of bins for the time axis (not the TAC)
  

  //Histograms to be filled
  TH2D* TAC_vs_time[8];
  for(int i = 0; i < 8; ++i){
    TAC_vs_time[i] = new TH2D(Form("TAC_vs_time_%d",i), Form("TAC_vs_time_%d; time (s); TAC (ps)",i),time_bins,time_min,time_max,TAC_bins,0.,50000.); list.Add(TAC_vs_time[i]);//Notice that the TAC has 10 ps/chan
  }
  TH2D* TAC_vs_time_noconditions[8];
  for(int i = 0; i < 8; ++i){
    TAC_vs_time_noconditions[i] = new TH2D(Form("TAC_vs_time_noconditions_%d",i), Form("TAC_vs_time_%d; time (s); TAC (ps)",i),time_bins,time_min,time_max,TAC_bins,0.,50000.); list.Add(TAC_vs_time_noconditions[i]); 
  }

   
  for(Long64_t n = 0; n < nEntries; ++n) {  //This is the main loop, that will cycle through the entire AnalysisTree
    AnalysisTree->GetEntry(n);
    
    //Builds the number of detector hits in each event (multiplicity of the coincidence)
    multi_zds = 0;
    multi_labr = 0;
    multi_tac = 0;
    if(gotZeroDegree==true){
      multi_zds = zds->GetMultiplicity();
    }
    if(gotLaBr==true){
      multi_labr = labr->GetMultiplicity();
    }
    if(gotTAC==true){
      multi_tac = tac->GetMultiplicity();
    }
    
    //fills the histograms as soon as there is one TAC
    if(multi_tac>=1){
      for(int i =0; i<multi_tac; ++i){
	tac_num=tac->GetTACHit(i)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
	TAC_vs_time_noconditions[tac_num]->Fill(tac->GetTACHit(i)->GetTime()/1E9,tac->GetTACHit(i)->GetCharge());
      }
    }
    
    //more restrictive. Here we fill the histograms if we have a tac but have NO LaBr. This way we almost guarantee to only have the calibrator and almost no real LaBr-LaBr timing.
    if(multi_tac>=1 && multi_labr==0){
      for(int i =0; i<multi_tac; ++i){
	tac_num=tac->GetTACHit(i)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
	TAC_vs_time[tac_num]->Fill(tac->GetTACHit(i)->GetTime()/1E9,tac->GetTACHit(i)->GetCharge());   
      }
    }

    
    //Just a counter to show the progress
    if(n%10000 == 0) {
      progress = ((double_t) n)/((double_t) nEntries);
      std::cout<<std::setw(4)<<100*progress<<"% done\r"<<std::flush;
    }
    
  }//This closes the loop that goes through the entire AnalysisTree

  
  std::cout<<"100% done"<<std::endl;
  
  list.Sort();
  std::cout<<"List sorted"<<std::endl;
  list.Write();
  std::cout<<"List written"<<std::endl;
  

  /////////Now that we have created the temperature-dependant histograms, we slice them and fit them:
  outfile.cd();


  double peak_position[4];//vector that holds the centroid of each position
  TH1D * proj_TAC;//temporary histogram that holds the projection on the TAC for a time slice (of 10 secs)
  double_t time[time_bins-1];//time vector for the centroid vs time polot
  double_t peak_centroid[4][time_bins-1];//matrix with all the centroids
  TGraph *graph[8][4];//graph to check the evolution of the centroid
  TGraph *calibration_graph;
  TGraphErrors *calibration_result[8];
  TGraphErrors *offset_result[8];
  TF1 *fit_func = new TF1("gaus_fit",gauss,0,5000,3);
  TF1 *fit_linear = new TF1("linear_fit",linear,0,5000,2);
  float centroid_guess;//You are right, this is a centroid guess
  int lower_limit,upper_limit;
  double time_calibrator[4]={10000.,20000.,30000.,40000.};
  double calibration[time_bins-1],calibration_error[time_bins-1];
  double offset[time_bins-1],offset_error[time_bins-1];
  for(int n = 0; n < 8; ++n){//This is the TAC # counter
    for(int t = 0; t < time_bins; ++t){//This is the timestamp counter
      proj_TAC = TAC_vs_time[n]->ProjectionY("",t,t);
      for(int peak = 0; peak<4;++peak){
	centroid_guess = (2500.+3000.*(peak))/ps_per_bin;
	lower_limit =static_cast<int>(centroid_guess-1000./ps_per_bin);
	upper_limit =static_cast<int>(centroid_guess+1000./ps_per_bin);
	proj_TAC->GetXaxis()->SetRange(lower_limit,upper_limit);
	centroid_guess=proj_TAC->GetMean();//This is a more precise centroid guess
	lower_limit =static_cast<int>(centroid_guess-200./ps_per_bin);
	upper_limit =static_cast<int>(centroid_guess+200./ps_per_bin);
	proj_TAC->GetXaxis()->SetRange(lower_limit,upper_limit);
	fit_func->SetParameters(10,proj_TAC->GetMean(),proj_TAC->GetRMS());
	//peak_position[peak]=0.;
	//if(proj_TAC->Integral()>1.){
	  proj_TAC->Fit("gaus_fit","LLrQ");
	  peak_position[peak]=fit_func->GetParameter(1);//Uncomment this if you want to fit to a gaussian
	  //peak_position[peak]=proj_TAC->GetMean();//Uncomment this if you want to use the centroid of the arbitrary distribution (Doesn't seem to work very well)
	  //std::cout<<"TAC number: "<<n<<" Time: "<<t*sec_per_bin<<"(s) Peak #: "<<(peak+1)<<" Area: "<<proj_TAC->Integral()<<" Centroid :"<<peak_position[peak]<<std::endl;
	  //}
	peak_centroid[peak][t]=1.;
	if(peak_position[peak]<1.){
	  peak_centroid[peak][t]=1.;
	}else if(peak_position[peak]>50000.){
	  peak_centroid[peak][t]=50000.;
	}else if (peak_position[peak] != peak_position[peak]){//This is based that the compiler should not considered a NaN a number, so it isn't equalt to any float, including itself.
	  peak_centroid[peak][t]=1.;
	}else{
	  peak_centroid[peak][t]=peak_position[peak];
	}
      }
      time[t]=(t+gRandom->Uniform())*sec_per_bin + time_min;//+sec_per_bin/2;
      delete proj_TAC;
      calibration_graph =  new TGraph(4,peak_position,time_calibrator);
      if(n==6&&t==100){
	calibration_graph->Write("zzzzzzzzzzzzzz");
      }
      fit_linear->SetParameters(0.,3.0);
      calibration_graph->GetXaxis()->SetRange(0,50000.);
      //if(calibration_graph->Eval(500.)>10.1){
	calibration_graph->Fit("linear_fit","ROB R Q");
	//}
      calibration[t]=fit_linear->GetParameter(1);
      calibration_error[t]=fit_linear->GetParError(1);
      offset[t]=fit_linear->GetParameter(0);
      offset_error[t]=fit_linear->GetParError(0);
      //std::cout<<"Calibration is: "<< calibration[t]<<"("<<calibration_error[t]<<") ps/chan"<<std::endl;
    }
    for(int peak = 0; peak<4;++peak){
      //Nice plots to see the variation of the centroid
      graph[n][peak] = new TGraph(time_bins-1, time, peak_centroid[peak]);
      graph[n][peak]->Write(Form("graph_TAC%d_peak%d",n,peak+1));
    }
    calibration_result[n] = new TGraphErrors(time_bins-1, time, calibration, calibration_error);
    calibration_result[n]->Write(Form("calibration_TAC%d",n));
    offset_result[n] = new TGraphErrors(time_bins-1, time, offset, offset_error);
    offset_result[n]->Write(Form("offset_TAC%d",n));
  }


  outfile.Close();
  
  }//This closes the loop over all the files
  return 0;
}//This closes the main function
