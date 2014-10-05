//g++ MakeCycleHistAna.C -I$GRSISYS/include -L$GRSISYS/libraries -lGRSIFormat -lAnalysisTreeBuilder -lGriffin -lSceptar -lGRSIDetector -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lMidasFormat -lXMLParser -lXMLIO -lProof `root-config --cflags --libs` -lTreePlayer -o MakeCycleHist

#include <iostream>
#include <utility>
#include <vector>
#include <cstdio>

#include "TTree.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "TChain.h"
#include "TFile.h"
#include "TList.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"

#include "TGriffin.h"
#include "TSceptar.h"


TList* MakeCycleHist(TChain *chain, 
                     int nofXBins = 30000, double lowXRange = 0., double highXRange = 30000., 
                     int nofYBins = 4000, double lowYRange = 0., double highYRange = 4000.) {
   //TChannel::ReadCalFromTree(AnalysisTree);

   TList* list = new TList;

   TH2F *gammaVsCycleTime = new TH2F("gammaVsCycleTime","E_{#gamma} vs. cycle time;cycle time [ms];E_{#gamma} [keV]",nofXBins,lowXRange,highXRange,nofYBins,lowYRange,highYRange);
   gammaVsCycleTime->SetStats(false);
   gammaVsCycleTime->GetXaxis()->CenterTitle();
   gammaVsCycleTime->GetYaxis()->CenterTitle();
   list->Add(gammaVsCycleTime);
   TH2F *betaVsCycleTime = new TH2F("betaVsCycleTime","E_{#beta} vs. cycle time;cycle time [ms];E_{#beta} [a.u.]",nofXBins,lowXRange,highXRange,nofYBins,lowYRange,highYRange);
   betaVsCycleTime->SetStats(false);
   betaVsCycleTime->GetXaxis()->CenterTitle();
   betaVsCycleTime->GetYaxis()->CenterTitle();
   list->Add(betaVsCycleTime);

   TH2F *gammaVsTime = new TH2F("gammaVsTime","E_{#gamma} vs. time;cycle time [ms];E_{#gamma} [keV]",nofXBins,lowXRange,highXRange,nofYBins,lowYRange,highYRange);
   gammaVsTime->SetStats(false);
   gammaVsTime->GetXaxis()->CenterTitle();
   gammaVsTime->GetYaxis()->CenterTitle();
   list->Add(gammaVsTime);
   TH2F *betaVsTime = new TH2F("betaVsTime","E_{#beta} vs. time;cycle time [ms];E_{#beta} [a.u.]",nofXBins,lowXRange,highXRange,nofYBins,lowYRange,highYRange);
   betaVsTime->SetStats(false);
   betaVsTime->GetXaxis()->CenterTitle();
   betaVsTime->GetYaxis()->CenterTitle();
   list->Add(betaVsTime);

   long timestamp;
   long time;
   long fillTime;
   long cycleTime;
   int maxCycleNumber = 0;
   int entries = chain->GetEntries();
   TGriffin *grif = 0;
   chain->SetBranchAddress("TGriffin",&grif);
   TSceptar *scep = 0;
   if(chain->FindBranch("TSceptar") != 0) {
      chain->SetBranchAddress("TSceptar",&scep);
   }

   for(int x=0;x<entries;x++) {
      chain->GetEntry(x);
      for(size_t g = 0; g < grif->GetMultiplicity(); ++g) {
         timestamp = grif->GetGriffinHit(g)->GetTime();
         timestamp = ((timestamp >> 1) & 0xffff0000000) | (timestamp & 0xfffffff);//combine lower 28bits of low timestamp with the high timestamp shifted by one bit, effectively dividing the high timestamp by two 
         gammaVsCycleTime->Fill(grif->GetCycleTimeInMilliSeconds(timestamp), grif->GetGriffinHit(g)->GetEnergyLow()); 
         gammaVsTime->Fill(timestamp/1e5, grif->GetGriffinHit(g)->GetEnergyLow()); 
      }
      if(scep != 0) {
         for(size_t b = 0; b < scep->GetMultiplicity(); ++b) {
            timestamp = scep->GetSceptarHit(b)->GetTime();
            timestamp = ((timestamp >> 1) & 0xffff0000000) | (timestamp & 0xfffffff);//combine lower 28bits of low timestamp with the high timestamp shifted by one bit, effectively dividing the high timestamp by two 
            gammaVsCycleTime->Fill(grif->GetCycleTimeInMilliSeconds(timestamp), scep->GetSceptarHit(b)->GetEnergy()); 
            gammaVsTime->Fill(timestamp/1e5, scep->GetSceptarHit(b)->GetEnergy()); 
         }
      }
      if(x%50000==0) 
         std::cout << "\t" << x << " / " << entries << "\r" << std::flush;
   }
   std::cout << "\t" << entries << " / " << entries << "\r" << std::endl;

   TCanvas *c = new TCanvas;
   c->Divide(2,2);
   c->cd(1);
   gammaVsCycleTime->Draw("colz");
   c->cd(2);
   betaVsCycleTime->Draw("colz");
   c->cd(3);
   gammaVsTime->Draw("colz");
   c->cd(4);
   betaVsTime->Draw("colz");

   return list;
}  


#ifndef __CINT__ 

int main(int argc, char **argv) {
   if(argc < 3)  {                     //This could reasonably <2. A tree can be a chain and we may not want to look at more than one sub run at a time
      printf("try again.\n");
      return 0;
   }
   TChain* chain = new TChain("AnalysisTree");
   
   for(int arg = 1; arg < argc-1; ++arg) {
      chain->Add(argv[arg]);
   }

   //time axis 0-6000 seconds in one second binning 
   TList *list = MakeCycleHist(chain, 6000, 0., 6000.*1000.);

   TFile *outfile = new TFile(argv[argc-1],"create");
   list->Write();

   return 0;
}

#endif


