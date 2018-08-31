//Written by B. Olaizola
//Date: 2017-08-11

//ROOT:6.08/06
//GRSISort:3.1.3.2

//Usage is: tac_calibration /path/to/analysistree.root

//Takes an AnalysisTree, searches for anti-coincidences TAC-LaBr (so we are sure we only have the TAC-calibrator and not LaBr-LaBr timing) and performs a linear fit to the peaks it finds. The output is a *.cal file with the TAC calibrated. If a list of analysistrees is given, the script will calibrate them one by one independently, it will NOT add them together.

//NOTE1: The need for LaBr anticoincidences is the reason it has to work at the AnalysisTree level.
//NOTE2: The TAC and TAC-calibrator parameters must be edited for every experiment (but the default would be 50 ns TAC range and 10 ns calibrator period).
//NOTE3: There are several sections of the code commented. They produce different quality-control plots and files. Consider uncommenting them to double check your results.


#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TF1.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "TGraph.h"
#include "TGainMatch.h"

#include "TGriffin.h"
#include "TSceptar.h"
#include "TLaBr.h"
#include "TZeroDegree.h"
#include "TTAC.h"

#include <vector>
#include <string>

int main(int argc, char** argv) {
  if(argc == 1) {
    std::cout<<"Usage: "<<argv[0]<<" <analysis tree file(s)>"<<std::endl;
    return 1;
  }


  for(int file_num=1; file_num < argc; ++file_num){

    TFile * f = new TFile(argv[file_num]);

    std::cout << "Reading from file:   " << f << std::endl;
  

    ////////////////////////////EDIT THIS PART//////////////////////////////////////////////////////////EDIT THIS PART///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    double calibrator_period=10000.;//The period of the calibrator (manually set in the module). It is the time difference between the peaks, which will be 10 ns in most cases
    double tac_range = 50000.; //TAC range, 50 ns for most experiments
    double ps_per_chan = 10.; //binning of the TACs, in ps per channel, 10 is a reasonable number 
    int first_TAC_channel = 75; //This is the channel number of the first TAC, it is needed to write the calibration file. In general it will be 84
    int number_of_peaks=(int) (tac_range/calibrator_period); //maximum number of peaks that we expect to find in the TAC-calibrator spectra
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    std::string inname;
    inname = argv[file_num];
    std::string num;
    num.append(inname,inname.length()-14,9);
    std::string calfile="TAC_calibration_run"+num+".cal";

    TList list;


    TTree * tree = (TTree*)f->Get("AnalysisTree");
   
    TChannel::ReadCalFromTree(tree);


    TLaBr* labr = NULL;
    TZeroDegree* zds = NULL;
    TTAC* tac = NULL;


    if(tree->FindBranch("TLaBr") != 0) {    //We check to see if we have a LaBr branch in the analysis tree
      tree->SetBranchAddress("TLaBr", &labr);
    }
    if(tree->FindBranch("TZeroDegree") != 0) {   //We check to see if we have a ZeroDegree branch in the analysis tree
      tree->SetBranchAddress("TZeroDegree", &zds);
    }
    if(tree->FindBranch("TTAC") == 0) {   //We check to see if we have a TAC branch in the analysis tree
      std::cout << "Exiting the program because there are no TACs to calibrate   " << std::endl;
      exit(-1);
    } else {
      tree->SetBranchAddress("TTAC", &tac);
    }


    TH1F * raw_tac_calibrator[8]; for(int i=0; i<8; i++){
      raw_tac_calibrator[i] = new TH1F(Form("raw_TAC_calibrator%d",i),Form("raw_TAC_calibrator%d",i),(int) (tac_range/ps_per_chan),0,tac_range); list.Add(raw_tac_calibrator[i]);
    }

    for(int i=0; i<tree->GetEntries(); i++){ //Loops through the whole AnalysisTree
      //for(int i=0; i<1e6; i++){ //Loops through limited number of entries. A million usually is enough

			
      tree->GetEntry(i);

      if(tac->GetMultiplicity()>0 && labr->GetMultiplicity()==0){
	for(int j=0; j< tac->GetMultiplicity(); j++){
	  int histnum = tac->GetTACHit(j)->GetDetector()-1;
	  if(histnum >= 0 && histnum <= 7){   
	    double charge = tac->GetTACHit(j)->GetCharge();
	    raw_tac_calibrator[histnum]->Fill(charge);
	  }
	}
      }
    }

  
    TH1F * calibration_hist[8]; for(int i=0; i<8; i++){
      calibration_hist[i] = new TH1F(Form("calibration_hist%d",i),Form("calibration_hist%d",i),(tac_range/ps_per_chan),0,tac_range); list.Add(calibration_hist[i]);
    }

    for(int j=0; j<=7; j++){
      TSpectrum spec; //It can take an argument as the max number of peaks to be found, but I recomend leaving it blank
      spec.Search(raw_tac_calibrator[j],1,"",0.10); //The 4th argument will discard peaks below 10% of the highest peak. This is important to tune, in a very high rate the number of LaBr-LaBr coincidences that sneak in could be considerable.
      Int_t nfound = spec.Search(raw_tac_calibrator[j],1,"",0.10);
      if(nfound>number_of_peaks){
	std::cout<<" "<<std::endl;
	std::cout<<"WARNING!!! Tspectrum found "<<(nfound-number_of_peaks)<<" too many peaks in TAC #"<<j <<std::endl;//Warning for bad peak found, results should be inspected
	nfound=number_of_peaks; 
      }

      std::vector<double> vec;
      vec.resize(nfound);//we make sure we have a vector with the right number of peaks
      for(int i=0; i<nfound; i++) vec.at(i)=spec.GetPositionX()[i];
      std::sort(vec.begin(),vec.end());

      for(int k=0; k<nfound;k++){
	raw_tac_calibrator[j]->GetXaxis()->SetRange((vec.at(k)-100)/ps_per_chan,(vec.at(k)+100)/ps_per_chan);
	vec[k]=raw_tac_calibrator[j]->GetMean();
      }
     
      /*
	for(int i=0; i<nfound; i++){
	std::cout<<"TSpectrum point "<<i<<" has value : "<<spec.GetPositionX()[i] <<std::endl;//priting the peaks it has found to screen
	std::cout<<"Vector point "<<i<<" has value : "<<vec.at(i) <<std::endl;//priting the peaks it has found to screen
	}
      */

      TGraph g;
      for(int i=0;i<nfound;i++){
	if(vec.at(i)>10){
	  g.SetPoint(i,vec.at(i), (double)(i)*calibrator_period);
	  calibration_hist[j]->SetBinContent((int)vec.at(i)+1,(double)(i)*calibrator_period );
	}
      }
      TF1 * fitfunc = new TF1("fitfunc","[0] + [1]*x");//linear polinomy
      //TF1 * fitfunc = new TF1("fitfunc","[0] + [1]*x + [2]*x*x");//second order polinomy (it should not be necesary)
      g.Fit(fitfunc,"Q");
      calibration_hist[j]->Fit(fitfunc,"Q");

      TGainMatch * gm = new TGainMatch();
      gm->SetFitFunction(fitfunc);
      gm->SetChannel(j+first_TAC_channel);//TAC channel, this may need a better implamentation

      gm->GetFitFunction()->SetParameter(0,0);

      gm->WriteToChannel();

      vec.clear();//Clear the vector for the next iteration
    }
   
    f->cd();
    TChannel::WriteCalFile(calfile);
    std::cout<<""<<std::endl;
    std::cout << "--->>>    cal file written to:   " << calfile << std::endl;
    std::cout<<""<<std::endl;


    //Uncomment if you want to plot the calibrated TAC (right now it does a second loop on the AnalysisTree
    /*
    TH1F * raw_tac[8]; 
    for(int i=0; i<8; i++){
      raw_tac[i] = new TH1F(Form("raw_TAC%d",i),Form("raw_TAC%d",i),(int) (tac_range/ps_per_chan),0,tac_range); list.Add(raw_tac[i]);
    }

    TH1F * tac_calibrated[8];
    for(int i=0; i<8; i++){
      tac_calibrated[i] = new TH1F(Form("tac_calibrated%d",i),Form("tac_calibrated%d",i),(tac_range/ps_per_chan),0,tac_range);list.Add(tac_calibrated[i]);
    }

    TH1F * tac_sum = new TH1F("tac_sum","tac_sum",(tac_range/ps_per_chan),0,tac_range);list.Add(tac_sum);

    for(int i=0; i<tree->GetEntries(); i++){

      tree->GetEntry(i);
     
      if(tac->GetMultiplicity()>0&&labr->GetMultiplicity()==0){
	for(int j=0; j< tac->GetMultiplicity(); j++){
	  int histnum = tac->GetTACHit(j)->GetDetector()-1;
	  if(histnum >= 0 && histnum <= 7){
	    Float_t energy = tac->GetTACHit(j)->GetEnergy();
	    tac_calibrated[histnum]->Fill(energy);
	    tac_sum->Fill(energy);
	    double charge = tac->GetTACHit(j)->GetCharge();
	    raw_tac[histnum]->Fill(charge);
	  }
	}
      }
    }
    */

//This is only useful if you want to quickly study the width of the peaks
/*
    double fit_width = tac_range/200; // the size of the fit window around the centroid

    std::vector<double> fwhm;
    for(int i=0; i<8; i++){
      TSpectrum spec(number_of_peaks);
      spec.Search(tac_calibrated[i]);
      std::vector<double> peak_vec;
      for(int j=0; j<spec.GetNPeaks(); j++) { peak_vec.push_back(spec.GetPositionX()[j]); }
      std::sort(peak_vec.begin(),peak_vec.end());
        
      TF1 peakfit("peakfit","[0]*TMath::Exp(-TMath::Power((x-[1])/(TMath::Sqrt(2)*[2]),2)) + [3] + [4]*x",peak_vec.at(3)-fit_width/2.,peak_vec.at(3)+fit_width/2.);
      peakfit.SetParameters(spec.GetPositionY()[3],peak_vec.at(3),10,0,0);
      peakfit.SetParLimits(2,0,100);
      tac_calibrated[i]->Fit(&peakfit,"Q","",peak_vec.at(3)-fit_width/2.,peak_vec.at(3)+fit_width/2.);

      std::cout << "FWHM for tac_calibrated_" << i <<" = " << peakfit.GetParameter(2)*2*TMath::Sqrt(2*TMath::Log(2)) << std::endl;
    }
*/

//Only uncoment if you want an additional outfile with spectra to check the quality of your calibration
/*
  std::string outname = argv[file_num];
  outname.replace(outname.begin(),outname.end()-14,"tac_calibration");
  TFile outfile(outname.c_str(),"recreate");
  outfile.cd();
  list.Sort();
  std::cout<<""<<std::endl;
  std::cout<<"writing to '"<<outname<<"'"<<std::endl;
  list.Write();
  outfile.Close();
*/

  }
   return 0;

}
