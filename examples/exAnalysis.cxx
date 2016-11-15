
//g++ exAnalysis.C -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries -lAnalysisTreeBuilder -lGriffin -lSceptar -lDescant -lPaces -lGRSIDetector -lTGRSIFit -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lGRSIFormat -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml `grsi-config --cflags --libs` `root-config --cflags --libs`  -lTreePlayer -lGROOT -lX11 -lXpm -lSpectrum
#include <iostream>
#include <iomanip>
#include <utility>
#include <vector>
#include <cstdio>
#include <sys/stat.h>

#include "TROOT.h"
#include "TTree.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "TChain.h"
#include "TFile.h"
#include "TList.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TCanvas.h"
#include "TStopwatch.h"
#include "TMath.h"
#include "TGRSIRunInfo.h"
#include "TUserSortInfo.h"
#include "Globals.h"

#ifndef __CINT__ 
#include "TGriffin.h"
#include "TSceptar.h"
#endif

#define CUBES 0
#define SCEP 0
#define ADDBACK 0
#define FRAGCOUNT 1

//This code is an example of how to write an analysis script to analyse an analysis tree
//The compiled script works like this
//
//  1. Starts in the main function by finding the analysis tree, setting up input and output files
//  2. Calls the exAnalysis function
//  3. exAnalysis creates 1D and 2D histograms and adds them to a list
//  4. Some loops over event "packets" decides which histograms should be filled and fills them.
//  5. The list of now filled histograms is returned to the main function
//  6. The list is written (meaning all of the histograms are written) to the output root file
//  7. Papers are published, theses are granted, high-fives are made
//
/////////////////////////////////////////////////////////////////////////////////////////

//This function gets run if running interpretively
//Not recommended for the analysis scripts
#ifdef __CINT__ 
void exAnalysis() {
   if(!AnalysisTree) {
      printf("No analysis tree found!\n");
      return;
   }
   //coinc window = 0-20, bg window 40-60, 6000 bins from 0. to 6000. (default is 4000)
   TList *list = exAnalysis(AnalysisTree, 0.);

   TFile *outfile = new TFile("output.root","recreate");
   list->Write();
}
#endif

TList *exAnalysis(TTree* tree, long maxEntries = 0, TStopwatch* w = NULL) {

   ///////////////////////////////////// SETUP ///////////////////////////////////////
   //Histogram paramaters
   Double_t low = 0;
   Double_t high = 4000;
   Double_t nofBins = 4000;

   //Coincidence Parameters
   Double_t ggTlow = 0.;   //Times are in 10's of ns
   Double_t ggThigh = 60.;
   Double_t gbTlow =  0.;
   Double_t gbThigh = 100.;

   if(w == NULL) {
      w = new TStopwatch;
      w->Start();
   }
   TList* list = new TList;

   const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

   //We create some spectra and then add it to the list
   TH1D* gammaSingles = new TH1D("gammaSingles","#gamma singles;energy[keV]",nofBins, low, high); list->Add(gammaSingles);
   TH1D* gammaSinglesB = new TH1D("gammaSinglesB","#beta #gamma;energy[keV]",nofBins, low, high); list->Add(gammaSinglesB);
   TH2D* ggmatrix = new TH2D("ggmatrix","#gamma-#gamma matrix",nofBins, low, high,nofBins, low, high); list->Add(ggmatrix);
   ///////////////////////////////////// PROCESSING /////////////////////////////////////

   //set up branches
   //Each branch can hold multiple hits
   //ie TGriffin grif holds 3 gamma rays on a triples event 
   TGriffin* grif = 0;
   TSceptar* scep = 0;
   tree->SetBranchAddress("TGriffin", &grif); //We assume we always have a Griffin

   bool gotSceptar;
   if(tree->FindBranch("TSceptar") == 0) {   //We check to see if we have a Scepter branch in the analysis tree
      gotSceptar = false;
   } else {
      tree->SetBranchAddress("TSceptar", &scep);
      gotSceptar = true;
   }

   //tree->LoadBaskets(MEM_SIZE);   

   long entries = tree->GetEntries();

   //These are the indices of the two hits being compared
   int one;
   int two;

   std::cout<<std::fixed<<std::setprecision(1); //This just make outputs not look terrible
   int entry;
   size_t angIndex;
   if(maxEntries == 0 || maxEntries > tree->GetEntries()) {
      maxEntries = tree->GetEntries();
   }
   for(entry = 0; entry < maxEntries; entry++) { //Only loop over the set number of entries
      tree->GetEntry(entry);
      //loop over the gammas in the event packet
      //grif is the variable which points to the current TGriffin
      for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
         //We want to put every gamma ray in this event into the singles
         gammaSingles->Fill(grif->GetGriffinHit(one)->GetEnergy()); 
         for(two = 0; two < (int) grif->GetMultiplicity(); ++two) {
            if(two == one){ //If we are looking at the same gamma we don't want to call it a coincidence
               continue;
            }
            if(ggTlow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < ggThigh) { 
               //If they are close enough in time, fill the gamma-gamma matrix. This will be symmetric because we are doing a double loop over gammas
               ggmatrix->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
            }
         }
      }

      //Now we make beta gamma coincident matrices
      if(gotSceptar && scep->GetMultiplicity() > 0) {
        //We do an outside loop on gammas so that we can break on the betas if we see a beta in coincidence (we don't want to bin twice just because we have two betas)
         for(int b = 0; b < scep->GetMultiplicity(); ++b) {
            for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
               //Be careful about time ordering!!!! betas and gammas are not symmetric out of the DAQ
               //Fill the time diffrence spectra
               if((gbTlow <= grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) {
                  gammaSinglesB->Fill(grif->GetGriffinHit(one)->GetEnergy());
               }
            }
         }
      }
      if((entry%10000) == 1){
         printf("Completed %d of %d \r",entry,maxEntries);
      }

   }

   list->Sort(); //Sorts the list alphabetically
   std::cout << "creating histograms done after " << w->RealTime() << " seconds" << std::endl;
   w->Continue();
   return list;

}

//This function gets run if running in compiled mode
#ifndef __CINT__ 
int main(int argc, char **argv) {
   if(argc != 4 && argc != 3 && argc != 2) {
      printf("try again (usage: %s <analysis tree file> <optional: output file> <max entries>).\n",argv[0]);
      return 0;
   }

   //We use a stopwatch so that we can watch progress
   TStopwatch w;
   w.Start();

   //Load the file containing the analysis tree.
   TFile* file = new TFile(argv[1]);

   if(file == NULL) {
      printf("Failed to open file '%s'!\n",argv[1]);
      return 1;
   }
   if(!file->IsOpen()) {
      printf("Failed to open file '%s'!\n",argv[1]);
      return 1;
   }
   printf("Sorting file:" DBLUE " %s" RESET_COLOR"\n",file->GetName());

   //Load the run info from the analysis file
   TGRSIRunInfo* runinfo  = (TGRSIRunInfo*)file->Get("TGRSIRunInfo");

   TTree* tree = (TTree*) file->Get("AnalysisTree");
   //Read in the calibration info. This code will crash without this.
   TChannel::ReadCalFromTree(tree);
   if(tree == NULL) {
      printf("Failed to find analysis tree in file '%s'!\n",argv[1]);
      return 1;
   }
   //Get the TGRSIRunInfo from the analysis Tree.
   
   TList *list;//We return a list because we fill a bunch of TH1's and shove them into this list.
   TFile * outfile;
   //This below part sets the name of the output file if one was not given upon starting the code.
   if(argc<3)
   {
      if(!runinfo){
         printf("Could not find run info, please provide output file name\n");
         return 0;
      }
      int runnumber = runinfo->RunNumber();
      int subrunnumber = runinfo->SubRunNumber();
      outfile = new TFile(Form("matrix%05d_%03d.root",runnumber,subrunnumber),"recreate");
   }
   else
   {
      outfile = new TFile(argv[2],"recreate");
   }

   std::cout << argv[0] << ": starting Analysis after " << w.RealTime() << " seconds" << std::endl;
   w.Continue();
   if(argc < 4) {
      list = exAnalysis(tree,0, &w);
   } else {
      int entries = atoi(argv[3]);
      std::cout<<"Limiting processing of analysis tree to "<<entries<<" entries!"<<std::endl;
      list = exAnalysis(tree, entries, &w);
   }
   printf("Writing to File: " DYELLOW "%s" RESET_COLOR"\n",outfile->GetName());
   list->Write();
   //Write the run info into the tree as well if there is run info in the Analysis Tree
   TGRSISortList *sortinfolist = new TGRSISortList;
   if(runinfo){
      TUserSortInfo *info = new TUserSortInfo(runinfo);
      sortinfolist->AddSortInfo(info);
      sortinfolist->Write("TGRSISortList",TObject::kSingleKey);
   }
   outfile->Close();

   std::cout << argv[0] << " done after " << w.RealTime() << " seconds" << std::endl << std::endl;


   return 0;
}

#endif
