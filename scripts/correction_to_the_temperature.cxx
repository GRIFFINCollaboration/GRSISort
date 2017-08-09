//Written by: B. Olaizola 2017-06-21 

//Experiment: S1607

//Root version: 6.08/02

//GRSIsort version: 2.4.00

//This script takes the result from temperature_correction.cxx and applies it to analysistrees. Notice that you have to apply both scripts to the same data file for it to work.

#include <iostream>
#include <fstream>
using namespace std;
#include <iomanip>
#include <string>
#include <vector>

#include "TGriffin.h"
#include "TSceptar.h"
#include "TLaBr.h"
#include "TZeroDegree.h"
#include "TTAC.h"
#include "TString.h"

#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TTree.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"


//Hardcoded cal file, this may change in the future.
const char* calfile = "/data1/bruno/griffin/S1607/calibration_HPGe_S1607.cal";



int main(int argc, char** argv) {
  if(argc == 1) {
    std::cout<<"Usage: "<<argv[0]<<" <analysis tree file(s)>"<<std::endl;
    return 1;
  }
  
  std::cout<<"List of files being corrected: "<<std::endl;
  std::cout<<" "<<std::endl;
  for(int i = 1; i < argc; ++i) {
    std::cout<<argv[i]<<std::endl;
  }
  std::cout<<" "<<std::endl;
  
  //Variable declaration
  double_t time;//Timestamp()
  double_t correction;//Temperature correction for the TACs
  double_t temperature_correction_factor,temperature_correction_offset;
  
  //TTree AnalysisTree("AnalysisTree");
  TFile * datafile;//File with the data to be corrected (the AnalysisTree)
  TFile * correctionfile;//File with the temperature correction
  
  TGraphErrors *calibration_graph[8];
  TGraphErrors *offset_graph[8];

  for(int file_num = 1; file_num < argc; ++file_num) {

    //Automatically generates the file name with the temperature correction
    std::string temperature_name = argv[file_num];
    temperature_name.replace(temperature_name.begin(),temperature_name.end()-14,"temperature_correction_");
    std::cout<<"Temperature correction from: '"<<temperature_name<<"'"<<std::endl;
    const char *name_temperature = temperature_name.c_str();
    correctionfile = new TFile(name_temperature);
    correctionfile->cd();

    for(int i=0;i<8; ++i){
      correctionfile->GetObject(Form("calibration_TAC%d",i),calibration_graph[i]);
      correctionfile->GetObject(Form("offset_TAC%d",i),offset_graph[i]);
    }

    datafile = new TFile(argv[file_num]);
    datafile->cd();//Make sure we are in the datafile
    TTree * AnalysisTree = (TTree*)datafile->Get("AnalysisTree");

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
    outname.replace(outname.begin(),outname.end()-14,"Corrected_");
    std::cout<<"Writing to '"<<outname<<"'"<<std::endl;
    
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
    TH2D* TAC_vs_time_corrected[8];
    for(int i = 0; i < 8; ++i){
      TAC_vs_time_corrected[i] = new TH2D(Form("TAC_vs_time_corrected_%d",i), Form("Temperature corrected TAC_vs_time_%d; time (s); TAC (ps)",i),time_bins,time_min,time_max,TAC_bins,0.,50000.); list.Add(TAC_vs_time_corrected[i]); 
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
      
      //Not corrected histogram
      if(multi_tac>=1 && multi_labr==0){
	for(int i =0; i<multi_tac; ++i){
	  tac_num=tac->GetTACHit(i)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
	  TAC_vs_time[tac_num]->Fill(tac->GetTACHit(i)->GetTime()/1E9,tac->GetTACHit(i)->GetEnergy());   
	}
      }
     

      //Temperature corrected histogram
      if(multi_tac>=1 && multi_labr==0){
	for(int i =0; i<multi_tac; ++i){
	  tac_num=tac->GetTACHit(i)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
	  time=tac->GetTACHit(i)->GetTime()/1E9;
	  //correctionfile->cd();
	  temperature_correction_factor = calibration_graph[tac_num]->Eval(time);
	  temperature_correction_offset = offset_graph[tac_num]->Eval(time);
	  //datafile->cd();
	  TAC_vs_time_corrected[tac_num]->Fill(tac->GetTACHit(i)->GetTime()/1E9,(tac->GetTACHit(i)->GetCharge() + gRandom->Uniform())*temperature_correction_factor+temperature_correction_offset);   
	}
      }
    
    //Just a counter to show the progress
    if(n%10000 == 0) {
      progress = ((double_t) n)/((double_t) nEntries);
      std::cout<<std::setw(4)<<100*progress<<"% done\r"<<std::flush;
    }



    }//This closes the loop over the AnalysisTree entries


    outfile.cd();
    std::cout<<"100% done"<<std::endl;
    
    list.Sort();
    std::cout<<"List sorted"<<std::endl;
    list.Write();
    std::cout<<"List written"<<std::endl;

    
  }//This closes the file loop
}//This closes the main function loop
