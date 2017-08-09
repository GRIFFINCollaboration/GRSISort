//Written by B. Olaizola
//Date: 2017-07-27

//ROOT:6.08/06
//GRSISort:3.1.3.2

//Usage is: tac_calibration /path/to/filenumber.root

//Takes an AnalysisTree, searches for anti-coincidences TAC-LaBr (so we are sure we only have the TAC-calibrator and not LaBr-LaBr timing) and performs a linear fit to the peaks it finds
//NOTE: The TAC and TAC-calibrator parameters must be edited
//NOTE2: There are several sections of the code commented. They produce different quality-control plots and files. Consider uncommenting them to double check your results.


#include "TFile.h"
#include "TTree.h"
#include "TFragment.h"
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

#include <algorithm>
#include <functional>
#include <array>
#include <iostream>
#include <string>
#include <stdlib.h>     /* atoi */
using namespace std;


int main(int argc, char** argv) {
  if(argc == 1) {
    std::cout<<"Usage: "<<argv[0]<<" <analysis tree file(s)>"<<std::endl;
    return 1;
  }

   //TFile * f = new TFile(Form("fragment0%d_000.root",num));//this is just if you want to run it with the run number only, and in the same directory
   TFile * f = new TFile(argv[1]);

   std::cout << "Reading from file:   " << f << std::endl;
  

////////////////////////////EDIT THIS PART//////////////////////////////////////////////////////////EDIT THIS PART///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	double calibrator_period=160000.;//The period of the calibrator (manually set in the module). It is the time difference between the peaks, 80 ns in this case
	double tac_range = 2000000.; //TAC range, 1 microsec in this case
	double ps_per_chan = 200.; //binning of the TACs, in ps per channel 
	int first_TAC_channel = 84; //This is the channel number of the first TAC, it is needed to write the calibration file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



   std::string inname;
   inname = argv[1];
   std::string num;
   num.append(inname,inname.length()-14,9);
   std::string calfile="TAC_calibration_run"+num+".cal";

   TList list;


   TTree * tree = (TTree*)f->Get("AnalysisTree");
   
   TChannel::ReadCalFromTree(tree);


   TLaBr* labr = NULL;
   TZeroDegree* zds = NULL;
   TTAC* tac = NULL;
   //TLaBrHit* labrhit = NULL;
   //TZeroDegreeHit* zdshit = NULL;
   //TTACHit* tachit = NULL;


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


   TH1F * test[8]; for(int i=0; i<8; i++){
	 test[i] = new TH1F(Form("raw_TAC_calibrator%d",i),Form("raw_TAC_calibrator%d",i),(int) (tac_range/ps_per_chan),0,tac_range); list.Add(test[i]);
	}
   TH1F * raw_tac[8]; for(int i=0; i<8; i++){
	 raw_tac[i] = new TH1F(Form("raw_TAC%d",i),Form("raw_TAC%d",i),(int) (tac_range/ps_per_chan),0,tac_range); list.Add(raw_tac[i]);
	}

//   for(int i=0; i<tree->GetEntries(); i++){ //Loops through the whole AnalysisTree
   for(int i=0; i<1e6; i++){ //Loops through the whole AnalysisTree

			
      tree->GetEntry(i);
/*
		if(tac->GetMultiplicity()>0){
			for(int j=0; j< tac->GetMultiplicity(); j++){
				raw_tac[tac->GetTACHit(j)->GetDetector()-1]->Fill(tac->GetTACHit(j)->GetCharge());
			}
			if(labr->GetMultiplicity()==0 && zds->GetMultiplicity()==0){
				for(int j=0; j< tac->GetMultiplicity(); j++){
					test[tac->GetTACHit(j)->GetDetector()-1]->Fill(tac->GetTACHit(j)->GetCharge());
				}
			}
		}
}*/

			for(int j=0; j< tac->GetMultiplicity(); j++){
		      	int histnum = tac->GetTACHit(j)->GetDetector()-1;
 			     if(histnum >= 0 && histnum <= 7){   
    			     double charge = tac->GetTACHit(j)->GetCharge();
    			     raw_tac[histnum]->Fill(charge);
    			  }
			}
		if(tac->GetMultiplicity()>0 && labr->GetMultiplicity()==0 && zds->GetMultiplicity()==0){
		  //if(tac->GetMultiplicity()>0 && labr->GetMultiplicity()==0){
			for(int j=0; j< tac->GetMultiplicity(); j++){
		      	int histnum = tac->GetTACHit(j)->GetDetector()-1;
 			     if(histnum >= 0 && histnum <= 7){   
    			     double charge = tac->GetTACHit(j)->GetCharge();
    			     test[histnum]->Fill(charge);
    			  }
			}
		}
	}

  
   int number_of_peaks=(int) (tac_range/calibrator_period);
	//double time[number_of_peaks], centroid[number_of_peaks];
   TH1F * calibration_hist[8]; for(int i=0; i<8; i++){
	 calibration_hist[i] = new TH1F(Form("calibration_hist%d",i),Form("calibration_hist%d",i),(tac_range/ps_per_chan),0,tac_range); list.Add(calibration_hist[i]);
	}

   for(int j=0; j<=7; j++){
      TSpectrum spec(number_of_peaks); //The argument is the number of peaks to be found 
      //spec.Search(test[j]);
	  spec.Search(test[j],1,"",0.25);//The 4th argument will discard peaks below 25% of the highest peak. This is important to tune.

      std::vector<double> vec;
      for(int i=0; i<number_of_peaks; i++) vec.push_back(spec.GetPositionX()[i]);
      std::sort(vec.begin(),vec.end());

     TGraph g;
      for(int i=0;i<number_of_peaks;i++){
		//time[i]= (double)(i)*calibrator_period;
		//centroid[i]=(vec.at(i)+1)*ps_per_chan;
		g.SetPoint(i,vec.at(i), (double)(i)*calibrator_period);
		//calibration_hist[j]->SetBinContent((int)centroid[i],time[i]);
		calibration_hist[j]->SetBinContent((int)vec.at(i)+1,(double)(i)*calibrator_period );
	  }
      TF1 * fitfunc = new TF1("fitfunc","[0] + [1]*x");
      //TF1 * fitfunc = new TF1("fitfunc","[0] + [1]*x + [2]*x*x");
      g.Fit(fitfunc,"Q");
      calibration_hist[j]->Fit(fitfunc,"Q");

      TGainMatch * gm = new TGainMatch();
      gm->SetFitFunction(fitfunc);
      gm->SetChannel(j+first_TAC_channel);//TAC channel, this may need a better implamentation

      gm->GetFitFunction()->SetParameter(0,0);
      gm->GetFitFunction()->SetParameter(1,(gm->GetFitFunction()->GetParameter(1)*ps_per_chan));   

      gm->WriteToChannel();
   
      //if(gm) { delete gm; gm = NULL; }
      //if(fitfunc) { delete fitfunc; fitfunc = NULL; }

   }
   
	f->cd();
   TChannel::WriteCalFile(calfile);
   std::cout<<""<<std::endl;
   std::cout << "--->>>    cal file written to:   " << calfile << std::endl;

//Uncomment if you want to plot the calibrated TAC (right now it does a second loop on the AnalysisTree
/*
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


/*
   std::string outname = argv[1];
   outname.replace(outname.begin(),outname.end()-14,"tac_calibration");
   TFile outfile(outname.c_str(),"recreate");
   outfile.cd();
   list.Sort();
   std::cout<<""<<std::endl;
   std::cout<<"writing to '"<<outname<<"'"<<std::endl;
   list.Write();
   outfile.Close();
*/


   return 0;

}
