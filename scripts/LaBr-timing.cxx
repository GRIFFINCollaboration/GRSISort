//Written by: B. Olaizola 2017-04-25 Fork for the S1607 run

//Experiment: S1067

//Root version: 6.08/02

//GRSIsort version: 2.4.00

//g++ -o LaBr-timing LaBr-timing.cxx -std=c++0x `grsi-config --cflags --all-libs` `root-config --cflags --libs`  -lTreePlayer -lGROOT -lX11 -lXpm -lSpectrum -lMathMore

//This script will construct a tree with LaBr-LaBr-TAC-HPGe coincidences. It is important to notice that LaBrE1 will be the START, LaBrE2 will be the STOP and that HPGe is optional (if there is no GRIFFIN coincidence in this event, its energy is sent to channel -2000).

#include <iostream>
#include <fstream>
using namespace std;
#include <iomanip>
#include <string>
#include <vector>

#include "TFile.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include "THnSparse.h"

#include "TGriffin.h"
#include "TSceptar.h"
#include "TLaBr.h"
#include "TZeroDegree.h"
#include "TTAC.h"
#include "TString.h"

#include <TCanvas.h> //For plots

//const char* path =  "/data1/bruno/griffin/50Ca_Nov2016"; //Currently not being used
//const char* calfile = "/data1/bruno/griffin/timing_test/Co60-test.cal"; //If you don't want to automatically apply a calibration file, comment this
const char* calfile = "/data1/bruno/griffin/S1607/calibration_HPGe_S1607.cal";

int main(int argc, char** argv) {
   if(argc == 1) {
      std::cout<<"Usage: "<<argv[0]<<" <analysis tree file(s)>"<<std::endl;
      return 1;
   }

   TChain AnalysisTree("AnalysisTree");

   for(int i = 1; i < argc; ++i) {
      std::cout<<"'"<<argv[i]<<"': added "<<AnalysisTree.Add(argv[i])<<" files"<<std::endl;
   }

   //// Here is automatically loading a calibration file into each root file. If you do not wish you apply the same calibration file to each run, comment this section ///////
   //   TChannel::ReadCalFromCurrentFile();  //Not working
   //   TChannel::ReadCalFromFile("argv[1]"); //Not working
   	printf("Reading calibration file: %s\n",calfile);
   	TChannel::ReadCalFile(calfile);
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   TGriffin* grif = NULL;
   TSceptar* scep = NULL;
   TLaBr* labr = NULL;
   TZeroDegree* zds = NULL;
   TTAC* tac = NULL;
   TGriffinHit* grifhit = NULL;
   TSceptarHit* scphit = NULL;
   TLaBrHit* labrhit = NULL;
   TZeroDegreeHit* zdshit = NULL;
   TTACHit* tachit = NULL;


   bool gotGriffin;
   if(AnalysisTree.FindBranch("TGriffin") == 0) {   //We check to see if we have a GRIFFIN branch in the analysis tree
      gotGriffin = false;
   } else {
      AnalysisTree.SetBranchAddress("TGriffin", &grif);
      gotGriffin = true;
   }
   bool gotSceptar;
   if(AnalysisTree.FindBranch("TSceptar") == 0) {   //We check to see if we have a SCEPTAR branch in the analysis tree
      gotSceptar = false;
   } else {
      AnalysisTree.SetBranchAddress("TSceptar", &scep);
      gotSceptar = true;
   }
   bool gotLaBr;
   if(AnalysisTree.FindBranch("TLaBr") == 0) {   //We check to see if we have a LaBr branch in the analysis tree
      gotLaBr = false;
   } else {
      AnalysisTree.SetBranchAddress("TLaBr", &labr);
      gotLaBr = true;
   }
   bool gotZeroDegree;
   if(AnalysisTree.FindBranch("TZeroDegree") == 0) {   //We check to see if we have a ZeroDegree branch in the analysis tree
      gotZeroDegree = false;
   } else {
      AnalysisTree.SetBranchAddress("TZeroDegree", &zds);
      gotZeroDegree = true;
   }
   bool gotTAC;
   if(AnalysisTree.FindBranch("TTAC") == 0) {   //We check to see if we have a TAC branch in the analysis tree
      gotTAC = false;
   } else {
      AnalysisTree.SetBranchAddress("TTAC", &tac);
      gotTAC = true;
   }


   Long64_t nEntries = AnalysisTree.GetEntries();

   std::string outname = argv[1];
   outname.replace(outname.begin(),outname.end()-14,"timing");
   std::cout<<"writing to '"<<outname<<"'"<<std::endl;

   TFile outfile(outname.c_str(),"recreate");

   TList list;
	
   UInt_t labr1,labr2,tac1; //counters

   Int_t bol; //logic unit


////////////////////////////////////////////////////////////////////////EDIT THIS PART////////////////////////////////////////////////////////////////////////////////////////////////////////
   Int_t num_labr,num_tac; //Number of LaBr and TACs, so you only need to define it once:
   num_labr = 8;
   num_tac = 8;
   int E_decay_min, E_decay_max, E_feed_min, E_feed_max, E_HPGe_min, E_HPGe_max;
   E_decay_min = 1310;//416-30;
   E_decay_max = 1390;//416+30;
   E_feed_min = 1150;//625-30;
   E_feed_max = 1230;//625+30;
   E_HPGe_min = 724-3;//731-3;
   E_HPGe_max = 724+3;//731+3;
   int TAC_offset[num_labr][num_labr]; //TAC to center all of them at 25ns, the ordering is [START-LaBr][STOP-LaBr] (keep in mind that they go from 0 to 7)
   for(int i=0; i<num_labr; i++){
     for(int j=0; j<num_labr; j++){
       TAC_offset[i][j] = 0.;
     }
   }

   TAC_offset[0][1] = (25000.-29018.35);
   TAC_offset[0][2] = (25000.-30065.14);
   TAC_offset[0][3] = (25000.-27815.11);
   TAC_offset[0][4] = (25000.-28663.83);
   TAC_offset[0][5] = (25000.-28554.45);
   TAC_offset[0][6] = (25000.-29506.68);
   TAC_offset[0][7] = (25000.-25000);
   TAC_offset[1][2] = (25000.-32550.85);
   TAC_offset[1][3] = (25000.-29921.67);
   TAC_offset[1][4] = (25000.-30988.06);
   TAC_offset[1][5] = (25000.-31029.43);
   TAC_offset[1][6] = (25000.-32226.68);
   TAC_offset[1][7] = (25000.-25000);
   TAC_offset[2][3] = (25000.-26112.17);
   TAC_offset[2][4] = (25000.-26961.74);
   TAC_offset[2][5] = (25000.-26984.61);
   TAC_offset[2][6] = (25000.-28015.35);
   TAC_offset[2][7] = (25000.-25000);
   TAC_offset[3][4] = (25000.-33666.21);
   TAC_offset[3][5] = (25000.-33656.83);
   TAC_offset[3][6] = (25000.-34758.65);
   TAC_offset[3][7] = (25000.-25000);
   TAC_offset[4][5] = (25000.-26978.51);
   TAC_offset[4][6] = (25000.-28009.41);
   TAC_offset[4][7] = (25000.-25000);
   TAC_offset[5][6] = (25000.-29307);
   TAC_offset[5][7] = (25000.-25000);
   TAC_offset[6][7] = (25000.-25000);


////////////////////////////////////////////////////////////////////////EDIT THISTHE upper PART////////////////////////////////////////////////////////////////////////////////////////////////////////

	Double_t  progress;
	Int_t  multi_labr, multi_zds, multi_tac, multi_grif, multi_scep;

	//TAC spectra histograms (these will be with LaBr coincidences)
	TH1D* TACs[num_labr][num_labr];
	for(int i = 0; i < num_labr; ++i){
		for(int j = i+1; j < num_labr; ++j){
			TACs[i][j] = new TH1D(Form("TAC_%d_%d",i,j), Form("TAC_%d_%d; time (ps); counts/ps",i,j), 50000,0.,50000); list.Add(TACs[i][j]); 
		}
	}
	//TAC spectra histograms (these will be with LaBr gates)
	TH1D* TAC_gated[num_labr][num_labr];
	for(int i = 0; i < num_labr; ++i){
		for(int j = i+1; j < num_labr; ++j){
			TAC_gated[i][j] = new TH1D(Form("TAC_gated_%d_%d",i,j), Form("TAC-gated_%d_%d; time (ps); counts/ps",i,j), 50000,0.,50000); list.Add(TAC_gated[i][j]); 
		}
	}
	TH1D* TAC_gated_summed = new TH1D("TAC_gated_summed", "TAC-gated_summed; time (ps); counts/ps", 50000,0.,50000); list.Add(TAC_gated_summed) ;
	

	//Creates a tree that will contain the LaBr-LaBr-TAC-HPGe coincidences
	TTree *llgt_tree = new TTree("llgt_tree","LaBr v Labr v TAC v HPGe"); list.Add(llgt_tree);
	typedef struct {
		Double_t LaBrE1;
		Double_t LaBrE2;
		Double_t TACt;
	        Double_t GeE[10];
	} EVENT;
	static EVENT llgt_event;
	llgt_tree->Branch("llgt_event",&llgt_event,"LaBrE1/D:LaBrE2/D:TACt/D:GeE/D");

	
	for(Long64_t n = 0; n < nEntries; ++n) {  //This is the main loop, that will cycle through the entire AnalysisTree
	  AnalysisTree.GetEntry(n);


      //Builds the number of detector hits in each event (multiplicity of the coincidence)
	  multi_grif = 0;
	  multi_scep = 0;
	  multi_zds = 0;
	  multi_labr = 0;
	  multi_tac = 0;

      if(gotGriffin==true){
	multi_grif = grif->GetMultiplicity();
      }
      if(gotSceptar==true){
	multi_scep = scep->GetMultiplicity();
      }
      if(gotZeroDegree==true){
	multi_zds = zds->GetMultiplicity();
      }
      if(gotLaBr==true){
	multi_labr = labr->GetMultiplicity();
      }
      if(gotTAC==true){
	multi_tac = tac->GetMultiplicity();
      }


      if(multi_labr==2 && multi_tac==1){ //It only looks for LaBrx2+TAC coincidences
       	labr1=labr->GetLaBrHit(0)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
       	labr2=labr->GetLaBrHit(1)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
       	tac1=tac->GetTACHit(0)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
       	if(labr1<labr2){
       	  if(tac1==labr1){
	    llgt_event.LaBrE1 = labr->GetLaBrHit(0)->GetEnergy();
	    llgt_event.LaBrE2 = labr->GetLaBrHit(1)->GetEnergy();
	    llgt_event.TACt = tac->GetTACHit(0)->GetEnergy() + TAC_offset[labr1][labr2];
	    TACs[labr1][labr2]->Fill(tac->GetTACHit(0)->GetEnergy() + TAC_offset[labr1][labr2]);
	    if(llgt_event.LaBrE1>=E_feed_min && llgt_event.LaBrE1<=E_feed_max && llgt_event.LaBrE2>=E_decay_min && llgt_event.LaBrE2<=E_decay_max ){
	      //for(int k =0; k<grif->GetMultiplicity(); ++k){
	      //if(grif->GetGriffinHit(k)->GetEnergy() >= E_HPGe_min && grif->GetGriffinHit(k)->GetEnergy() <= E_HPGe_max){
		  TAC_gated[labr1][labr2]->Fill(tac->GetTACHit(0)->GetEnergy());
		  TAC_gated_summed->Fill(tac->GetTACHit(0)->GetEnergy() + TAC_offset[labr1][labr2]);		  
		  //}
		  //}  
	    }
	  }
	}else if(labr1>labr2){
	  if(tac1==labr2){
	    llgt_event.LaBrE1 = labr->GetLaBrHit(1)->GetEnergy();
	    llgt_event.LaBrE2 = labr->GetLaBrHit(0)->GetEnergy();
	    llgt_event.TACt = tac->GetTACHit(0)->GetEnergy() + TAC_offset[labr2][labr1];
	    TACs[labr2][labr1]->Fill(tac->GetTACHit(0)->GetEnergy() + TAC_offset[labr2][labr1]);
	    if(llgt_event.LaBrE1>=E_feed_min && llgt_event.LaBrE1<=E_feed_max && llgt_event.LaBrE2>=E_decay_min && llgt_event.LaBrE2<=E_decay_max ){ 
	      //for(int k =0; k<grif->GetMultiplicity(); ++k){
	      //if(grif->GetGriffinHit(k)->GetEnergy() >= E_HPGe_min && grif->GetGriffinHit(k)->GetEnergy() <= E_HPGe_max){
		  TAC_gated[labr2][labr1]->Fill(tac->GetTACHit(0)->GetEnergy());
		  TAC_gated_summed->Fill(tac->GetTACHit(0)->GetEnergy() + TAC_offset[labr2][labr1]);		  
		  //}
		  //}
	    }
	  }
	}
	///If there is no GRIFFIN hit in this event, we set an artificial count in channel -2000
	if(!grif->GetMultiplicity()){
	  for(int k =0; k<10; ++k){
	    llgt_event.GeE[k] = -2000.;
	  }
	}else{
	  for(int k =0; k<grif->GetMultiplicity(); ++k){
	    if(k<10){
	      llgt_event.GeE[k] = grif->GetGriffinHit(k)->GetEnergy();
	    }
	  }
	  for(int k =grif->GetMultiplicity(); k<10; ++k){
	    if(k<10){
	      llgt_event.GeE[k] = -2000.;
	    }
	  }
	}
	llgt_tree->Fill();
      }
      
	  
      if(n%10000 == 0) {
	progress = ((double_t) n)/((double_t) nEntries);
         std::cout<<std::setw(4)<<100*progress<<"% done\r"<<std::flush;
      }

   }//Analysistree loop
   std::cout<<"100% done"<<std::endl;

   list.Sort();
   list.Write();


   /////////////////////////Nice plot of all the TAC-LaBr-LaBr convinations//////////////////////
   TCanvas * c_a = new TCanvas();
   c_a->Divide(8,8);
   for(int i=0; i<8; i++){
     for(int j=i+1; j<8; j++){
       c_a->cd((j+1)+(i*8));
       TACs[i][j]->Draw();
     }
   }
   c_a->Write("TACs_canvas");

   TCanvas * c_b = new TCanvas();
   c_b->Divide(8,8);
   for(int i=0; i<8; i++){
     for(int j=i+1; j<8; j++){
       c_b->cd((j+1)+(i*8));
       TAC_gated[i][j]->Draw();
     }
   }
   c_b->Write("TAC_gated_canvas");

   std::cout<<"Canvas written"<<std::endl;
   ////////////////////////////////////////////////////////////////////////////////////////

   outfile.Close();

   return 0;
}


//////For debugging/////////
	// if(tac1==0&&labr2==0){
	//   std::cout<<"Si que puede valer: "<<std::setw(4)<<tac1<<std::endl;
	// }
