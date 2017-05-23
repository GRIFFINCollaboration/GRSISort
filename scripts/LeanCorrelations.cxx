
//g++ exAnalysis.C -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries -lAnalysisTreeBuilder -lGriffin -lSceptar -lDescant -lPaces -lGRSIDetector -lTGRSIFit -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lGRSIFormat -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml `grsi-config --cflags --libs` `root-config --cflags --libs`  -lTreePlayer -lGROOT -lX11 -lXpm -lSpectrum
#include <iostream>
#include <iomanip>
#include <utility>
#include <vector>
#include <cstdio>
#include <sys/stat.h>
#include <map>

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
#include "TGRSISortInfo.h"
#include "Globals.h"
#include "THnSparse.h"

#ifndef __CINT__ 
#include "TGriffin.h"
#include "TSceptar.h"
#endif

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
   TList *list = exAnalysis(AnalysisTree, TPPG, TGRSIRunInfo, 0.);

   TFile *outfile = new TFile("output.root","recreate");
   list->Write();
}
#endif

std::vector<std::pair<double,int> > AngleCombinations(double distance = 110., bool folding = false, bool addback = false) {
   std::vector<std::pair<double,int> > result;
   std::vector<std::pair<double,int> > grouped_result;
   std::vector<double> angle;
   for(int firstDet = 1; firstDet <= 16; ++firstDet) {
      for(int firstCry = 0; firstCry < 4; ++firstCry) {
         for(int secondDet = 1; secondDet <= 16; ++secondDet) {
            for(int secondCry = 0; secondCry < 4; ++secondCry) {
               if(firstDet == secondDet && firstCry == secondCry) {
                  continue;
               }
               if(!addback){
                  angle.push_back(TGriffin::GetPosition(firstDet, firstCry, distance).Angle(TGriffin::GetPosition(secondDet, secondCry, distance))*180./TMath::Pi());
               }
               if(addback){
                  if(((TGriffin::GetPosition(firstDet, firstCry, distance).Angle(TGriffin::GetPosition(secondDet, secondCry, distance))*180./TMath::Pi() > 18.786) && (TGriffin::GetPosition(firstDet, firstCry, distance).Angle(TGriffin::GetPosition(secondDet, secondCry, distance))*180./TMath::Pi() < 18.788)) || ((TGriffin::GetPosition(firstDet, firstCry, distance).Angle(TGriffin::GetPosition(secondDet, secondCry, distance))*180./TMath::Pi() > 26.6800) && (TGriffin::GetPosition(firstDet, firstCry, distance).Angle(TGriffin::GetPosition(secondDet, secondCry, distance))*180./TMath::Pi() < 26.6915))){
                     angle.push_back(18.7868);
                  } else {
                     angle.push_back(TGriffin::GetPosition(firstDet, firstCry, distance).Angle(TGriffin::GetPosition(secondDet, secondCry, distance))*180./TMath::Pi());
                  }
               }
               if(folding && angle.back() > 90.) {
                  angle.back() = 180. - angle.back();
               }
            }
         }
      }
   }

   std::sort(angle.begin(),angle.end());
   size_t r;
   for(size_t a = 0; a < angle.size(); ++a) {
      for(r = 0; r < result.size(); ++r) {
         if(angle[a] >= result[r].first-0.001 && angle[a] <= result[r].first+0.001) {
            (result[r].second)++;
            break;
         }
      }
      if(result.size() == 0 || r == result.size()) {
         result.push_back(std::make_pair(angle[a],1));
      }
   }

   if(folding) {//if we fold we also want to group
      std::vector<std::pair<double,int> > groupedResult;
      for(size_t i = 0; i < result.size(); ++i) {
         switch(i) {
            case 0:
            case 1:
               groupedResult.push_back(result[i]);
               break;
            case 2:
            case 4:
            case 6:
               if(i+1 >= result.size()) {
                  std::cerr<<"Error!"<<std::endl;
               }
               groupedResult.push_back(std::make_pair((result[i].first+result[i+1].first)/2.,result[i].second+result[i+1].second));
               ++i;
               break;
            default:
               groupedResult.push_back(std::make_pair((result[i].first+result[i+1].first+result[i+2].first)/3.,result[i].second+result[i+1].second+result[i+2].second));
               i+=2;
               break;
         }
      }
      return groupedResult;
   }

   return result;
}





TList *exAnalysis(TTree* tree, TPPG*, TGRSIRunInfo*, long maxEntries = 0, TStopwatch* w = nullptr) {
   ///////////////////////////////////// SETUP ///////////////////////////////////////
   //Histogram paramaters
   Double_t low = 0;
   Double_t high = 4096;
   Int_t nofBins = 4096;

   //Coincidence Parameters
   Double_t ggTlow = 0.;   //Times are in 10's of ns
   Double_t ggThigh = 40.;
   Double_t gbTlow =  -20.;
   Double_t gbThigh = 20.;

   //Double_t ggBGlow = 100.;
   //Double_t ggBGhigh = 175.;
   Double_t gbBGlow = 50.;
   Double_t gbBGhigh = 170.;
   //Double_t ggBGScale = (ggThigh - ggTlow)/(ggBGhigh - ggBGlow);
   //Double_t gbBGScale = (gbThigh - gbTlow)/(gbBGhigh - gbBGlow);

   Double_t betaThres = 800.;

   //this is in ms
   //Double_t cycleLength = 15000;

   //Double_t bgStart  =  1.5e8;
   //Double_t bgEnd    =  3.5e8;
   //Double_t onStart  =  3.5e8;
   //Double_t onEnd    = 14.0e8;
   //Double_t offStart = 14.5e8;
   //Double_t offEnd   = 15.5e8;

   if(w == nullptr) {
      w = new TStopwatch;
      w->Start();
   }
   TList* list = new TList;

   //const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

   //This should be map to speed this up significantly.

   std::vector<std::pair<double,int> > angleCombinations = AngleCombinations(110., false, false);
   std::cout<<"got "<<angleCombinations.size()<<" angles"<<std::endl;
   for(auto ang = angleCombinations.begin(); ang != angleCombinations.end(); ang++) {
      std::cout<<(*ang).first<<" degree: "<<(*ang).second<<" combinations"<<std::endl;
   }


   std::vector<std::pair<double,int> > angleCombinationsab = AngleCombinations(110., false, true);
   std::cout<<"got "<<angleCombinationsab.size()<<" angles"<<std::endl;
   for(auto ang = angleCombinationsab.begin(); ang != angleCombinationsab.end(); ang++) {
      std::cout<<(*ang).first<<" degree: "<<(*ang).second<<" combinations"<<std::endl;
   }

   double* xBins = new double[1501];
   double* yBins = new double[1501];
   for(int i = 0; i <= 1500; ++i) {
      xBins[i] = (double) i;
      yBins[i] = (double) i;
   }
   double* zBins = new double[angleCombinations.size()+1];
   for(size_t i = 0; i<angleCombinations.size()+1; i++){
      zBins[i] = (double) i;
   }

   std::map<Double_t,Int_t> angleComboMapab;
   std::map<Double_t,Int_t> angleComboMap;

   Double_t min[2] = {low,low};
   Double_t max[2] = {high,high};
   Int_t bins[2]= {nofBins,nofBins};

   //   TH2F** angCorr_coinc_Binnedab = new TH2F*[angleCombinationsab.size()+1];
   THnSparseF** angCorr_coinc_Binnedab = new THnSparseF*[angleCombinationsab.size()+1];
   THnSparseF** angCorr_coinc_Binnedab_bg = new THnSparseF*[angleCombinationsab.size()+1];
   THnSparseF* ggbmatrix = new THnSparseF("ggbmatrix","#gamma-#gamma-#beta matrix",2,bins,min,max); list->Add(ggbmatrix);
   THnSparseF* ggbmatrix_bg = new THnSparseF("ggbmatrix_bg", "#gamma-#gamma-#beta background matrix", 2, bins, min, max); list->Add(ggbmatrix_bg);
   for(int i = 0; i < (int) angleCombinationsab.size()+1; ++i) {
      //  angCorr_coinc_Binnedab[i] = new TH2F(Form("angCorr_coinc_Binnedab_%d",i),Form("angular correlation at %.1f ^{o} addback on beam window;energy [keV];energy [keV]",angleCombinationsab[i].first), 1500, xBins, 1500, yBins); list->Add(angCorr_coinc_Binnedab[i]);
      //angCorr_coinc_Binnedab[i] = new TH2F(Form("angCorr_coinc_Binnedab_%d",i),Form("angular correlation at %.1f ^{o} addback on beam window;energy [keV];energy [keV]",angleCombinationsab[i].first), nofBins, low, high, nofBins,low,high); list->Add(angCorr_coinc_Binnedab[i]);
      angCorr_coinc_Binnedab[i] = new THnSparseF(Form("angCorr_coinc_Binnedab_%d",i),Form("angular correlation at %.1f ^{o} addback on beam window;energy [keV];energy [keV]",angleCombinationsab[i].first), 2,bins,min,max); list->Add(angCorr_coinc_Binnedab[i]);
      angCorr_coinc_Binnedab_bg[i] = new THnSparseF(Form("angCorr_coinc_Binnedab_bg_%d",i),Form("angular correlation at %.1f ^{o} addback on beam window, time randoms;energy [keV];energy [keV]",angleCombinationsab[i].first), 2,bins,min,max); list->Add(angCorr_coinc_Binnedab_bg[i]);
      angleComboMapab.insert(std::make_pair(angleCombinationsab.at(i).first,i));
   }
   //TH2F** angCorr_coinc_Binned = new TH2F*[angleCombinations.size()+1];
   THnSparseF** angCorr_coinc_Binned = new THnSparseF*[angleCombinations.size()+1];
   THnSparseF** angCorr_coinc_Binned_bg = new THnSparseF*[angleCombinations.size()+1];
   THnSparseF** angCorr_coinc_Binned_uncorr = new THnSparseF*[angleCombinations.size()+1];
   for(int i = 0; i < (int) angleCombinations.size()+1; ++i) {
      //  angCorr_coinc_Binned[i] = new TH2F(Form("angCorr_coinc_Binned_%d",i),Form("angular correlation at %.1f ^{o} on beam window;energy [keV];energy [keV]",angleCombinations[i].first), 1500, xBins, 1500, yBins); list->Add(angCorr_coinc_Binned[i]);
      // angCorr_coinc_Binned[i] = new TH2F(Form("angCorr_coinc_Binned_%d",i),Form("angular correlation at %.1f ^{o} on beam window;energy [keV];energy [keV]",angleCombinations[i].first), nofBins, low, high,nofBins,low,high); list->Add(angCorr_coinc_Binned[i]);
      angCorr_coinc_Binned[i] = new THnSparseF(Form("angCorr_coinc_Binned_%d",i),Form("angular correlation at %.1f ^{o} on beam window;energy [keV];energy [keV]",angleCombinations[i].first), 2,bins, min,max); list->Add(angCorr_coinc_Binned[i]);
      angCorr_coinc_Binned_bg[i] = new THnSparseF(Form("angCorr_coinc_Binned_bg_%d",i),Form("angular correlation at %.1f ^{o} on beam window, time randoms;energy [keV];energy [keV]",angleCombinations[i].first), 2,bins, min,max); list->Add(angCorr_coinc_Binned_bg[i]);
      angCorr_coinc_Binned_uncorr[i] = new THnSparseF(Form("angCorr_coinc_Binned_uncorr_%d",i),Form("angular correlation at %.1f ^{o} on beam window (uncorrelated);energy [keV];energy [keV]",angleCombinations[i].first), 2,bins, min,max); list->Add(angCorr_coinc_Binned_uncorr[i]);
      angleComboMap.insert(std::make_pair(angleCombinations[i].first,i));
   }

   //Double_t tlow[2] = {0,0};
   //Double_t thigh[2] = {200,200};
   TH2D* bggTimeDiff = new TH2D("bggTimeDiff", "Time Difference;t_{#gamma2}-t_{#beta};t_{#gamma1}-t_{#beta};",400,-200.,200.,400,-200.,200.); list->Add(bggTimeDiff);
   TH2D* bggTimeDiffLargeE = new TH2D("bggTimeDiffLargeE", "Time Difference, E_{#gamma} > 1 MeV;t_{#gamma2}-t_{#beta};t_{#gamma1}-t_{#beta};",400,-200.,200.,400,-200.,200.); list->Add(bggTimeDiffLargeE);

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

   //long entries = tree->GetEntries();

   //These are the indices of the two hits being compared
   int one;
   int two;
   //int three;
   //size_t angIndexab;
   //int index = 0;
   std::cout<<std::fixed<<std::setprecision(1); //This just make outputs not look terrible
   long entry;
   int bggCount = 0;
   //size_t angIndex;

   TGriffin oldgrifArray[3];
   TSceptar oldscepArray[3];

   if(maxEntries == 0 || maxEntries > tree->GetEntries()) {
      maxEntries = tree->GetEntries();
   }
   for(entry = 1; entry < maxEntries; ++entry) { //Only loop over the set number of entries
      //I'm starting at entry 1 because of the weird high stamp of 4.
      //bool iso_flag = false;
      //if(entry > 2)tree->GetEntry(entry-2);
      //if(grif->GetMultiplicity()>1)
      //   iso_flag = true;
      //TGriffin* grif2 = (TGriffin*)(grif->Clone()); 
      tree->GetEntry(entry);
      grif->ResetAddback();
      TGriffin* oldgrif = 0;
      TSceptar* oldscep = 0;
      if(bggCount > 2) {
         oldgrif = &oldgrifArray[(bggCount-2)%3];
         oldscep = &oldscepArray[(bggCount-2)%3];
      }
      //Now we make beta gamma coincident matrices
      if(gotSceptar && scep->GetMultiplicity() > 0) {
         //We do an outside loop on gammas so that we can break on the betas if we see a beta in coincidence (we don't want to bin twice just because we have two betas)
         bool foundBeta = false;
         for(int b = 0; b < scep->GetMultiplicity() && !foundBeta; ++b) {
            if(scep->GetHit(b)->GetEnergy() < betaThres) continue;
            for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
               if(grif->GetGriffinHit(one)->NPileUps() > 1) continue;
               for(two = 0; two < (int) grif->GetMultiplicity(); ++two) {
                  if(grif->GetGriffinHit(two)->NPileUps() > 1) continue;
                  if(two == one){ //If we are looking at the same gamma we don't want to call it a coincidence
                     continue;
                  }
                  bggTimeDiff->Fill(grif->GetGriffinHit(one)->GetTime()-scep->GetSceptarHit(b)->GetTime(),grif->GetGriffinHit(two)->GetTime() - scep->GetSceptarHit(b)->GetTime());
                  if(grif->GetGriffinHit(one)->GetEnergy() > 1000. && grif->GetGriffinHit(two)->GetEnergy() > 1000.) {
                     bggTimeDiffLargeE->Fill(grif->GetGriffinHit(one)->GetTime()-scep->GetSceptarHit(b)->GetTime(),grif->GetGriffinHit(two)->GetTime() - scep->GetSceptarHit(b)->GetTime());
                  }
                  foundBeta = true;
                  //Be careful about time ordering!!!! betas and gammas are not symmetric out of the DAQ
                  //Fill the time diffrence spectra
                  if(((gbTlow <= grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) && 
                     ((gbTlow <= grif->GetHit(two)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(two)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) &&
                     ((ggTlow <= TMath::Abs(grif->GetHit(one)->GetTime()-grif->GetHit(two)->GetTime())) && TMath::Abs((grif->GetHit(one)->GetTime()-grif->GetHit(two)->GetTime() <= ggThigh)))){
                     //Plots a gamma energy spectrum in coincidence with a beta
                     //Now we want to loop over gamma rays if they are in coincidence.
                     //If they are close enough in time, fill the gamma-gamma-beta matrix. This will be symmetric because we are doing a double loop over gammas
                     double angle = grif->GetGriffinHit(one)->GetPosition().Angle(grif->GetGriffinHit(two)->GetPosition())*180./TMath::Pi();
                     if(angle < 0.0001) continue;
                     auto angIndex = angleComboMap.lower_bound(angle-0.0005);
                     //angCorr_coinc_Binned[angIndex->second]->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy(), 1.);
                     Double_t fillval[2] = {grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy()};
                     angCorr_coinc_Binned[angIndex->second]->Fill(fillval);
                  } else if((((gbBGlow <= grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbBGhigh)) && 
                             ((gbTlow  <= grif->GetHit(two)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(two)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh))) ||
                            (((gbBGlow <= grif->GetHit(two)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(two)->GetTime()-scep->GetHit(b)->GetTime() <= gbBGhigh)) &&
                             ((gbTlow  <= grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)))) {
                     //Plots a gamma energy spectrum in coincidence with a beta
                     //Now we want to loop over gamma rays if they are in coincidence.
                     //If they are close enough in time, fill the gamma-gamma-beta matrix. This will be symmetric because we are doing a double loop over gammas
                     double angle = grif->GetGriffinHit(one)->GetPosition().Angle(grif->GetGriffinHit(two)->GetPosition())*180./TMath::Pi();
                     if(angle < 0.0001) continue;
                     auto angIndex = angleComboMap.lower_bound(angle-0.0005);
                     //angCorr_coinc_Binned[angIndex->second]->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy(), 1.);
                     Double_t fillval[2] = {grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy()};
                     angCorr_coinc_Binned_bg[angIndex->second]->Fill(fillval);
                  }
               }
               if(oldgrif){
                  bool oldfound =false;
                  for(int oldb=0; (oldb < (int) oldscep->GetMultiplicity()) && !oldfound; ++oldb){
                     //Be careful about time ordering!!!! betas and gammas are not symmetric out of the DAQ
                     for(int oldtwo =0; oldtwo < (int) oldgrif->GetMultiplicity(); ++oldtwo){   
                        if(oldgrif->GetGriffinHit(oldtwo)->NPileUps() > 1) continue;
                        if(oldtwo == one) continue;
                        if(((gbTlow <= grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) && 
                           ((gbTlow <= oldgrif->GetHit(oldtwo)->GetTime()-oldscep->GetHit(oldb)->GetTime()) && (oldgrif->GetHit(oldtwo)->GetTime()-oldscep->GetHit(oldb)->GetTime() <= gbThigh))) {
                           //Plots a gamma energy spectrum in coincidence with a beta
                           //Now we want to loop over gamma rays if they are in coincidence.
                           //If they are close enough in time, fill the gamma-gamma-beta matrix. This will be symmetric because we are doing a double loop over gammas
                           double angle = grif->GetGriffinHit(one)->GetPosition().Angle(oldgrif->GetGriffinHit(oldtwo)->GetPosition())*180./TMath::Pi();
                           if(angle < 0.0001) continue;
                           auto angIndex = angleComboMap.lower_bound(angle-0.0005);
                           //angCorr_coinc_Binned[angIndex->second]->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy(), 1.);
                           Double_t fillval[2] = {grif->GetGriffinHit(one)->GetEnergy(), oldgrif->GetGriffinHit(oldtwo)->GetEnergy()};
                           angCorr_coinc_Binned_uncorr[angIndex->second]->Fill(fillval);
                           Double_t fillvalswitched[2] = {oldgrif->GetGriffinHit(oldtwo)->GetEnergy(), grif->GetGriffinHit(one)->GetEnergy()};
                           angCorr_coinc_Binned_uncorr[angIndex->second]->Fill(fillvalswitched);
                           oldfound = true;
                        }
                  
                     }

                  }

               }
            }
         }
         for(one = 0; one < (int) grif->GetAddbackMultiplicity(); ++one) {
            for(int b = 0; b < scep->GetMultiplicity(); ++b) {
               if(scep->GetHit(b)->GetEnergy() < betaThres) continue;
               for(two = 0; two < (int) grif->GetAddbackMultiplicity(); ++two) {
                  if(two == one){ //If we are looking at the same gamma we don't want to call it a coincidence
                     continue;
                  }
                  if(((gbTlow <= grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) && 
                     ((gbTlow <= grif->GetAddbackHit(two)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetAddbackHit(two)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh))) {
                     double angab = grif->GetAddbackHit(one)->GetPosition().Angle(grif->GetAddbackHit(two)->GetPosition())*180./TMath::Pi();
                     auto angIndex = angleComboMap.lower_bound(angab-0.005);
                     //                             angCorr_coinc_Binnedab[angIndex->second]->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy(), 1.);
                     Double_t fillval[2] = {grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy()};
                     angCorr_coinc_Binnedab[angIndex->second]->Fill(fillval);
                     //                             angCorr_coinc_Binnedab[angIndexab]->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy(), 1.);
                  } else if(((gbBGlow <= grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbBGhigh)) && 
                            ((gbTlow  <= grif->GetAddbackHit(two)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetAddbackHit(two)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh))) {
                     double angab = grif->GetAddbackHit(one)->GetPosition().Angle(grif->GetAddbackHit(two)->GetPosition())*180./TMath::Pi();
                     auto angIndex = angleComboMap.lower_bound(angab-0.005);
                     //                             angCorr_coinc_Binnedab[angIndex->second]->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy(), 1.);
                     Double_t fillval[2] = {grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy()};
                     angCorr_coinc_Binnedab_bg[angIndex->second]->Fill(fillval);
                     //                             angCorr_coinc_Binnedab[angIndexab]->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy(), 1.);
                  }
               }
            }
         }

      }

      if(scep->GetMultiplicity() > 0 && grif->GetMultiplicity() > 1) {
         oldgrifArray[bggCount%3] = *grif;
         oldscepArray[bggCount%3] = *scep;
         ++bggCount;
      }

      if((entry%10000) == 1){
         printf("Completed %ld of %ld \r",entry,maxEntries);
      }
   }

   //for(int i = 0; i < angleCombinations.size()+1; ++i) {
   //   angCorr_coinc_Binned_bg[i]->Scale(-gbBGScale);
   //   angCorr_coinc_Binned_bg[i]->Add(angCorr_coinc_Binned[i]);
   //}

   //for(int i = 0; i < angleCombinationsab.size()+1; ++i) {
   //   angCorr_coinc_Binnedab_bg[i]->Scale(-gbBGScale);
   //   angCorr_coinc_Binnedab_bg[i]->Add(angCorr_coinc_Binnedab[i]);
   //}

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

   TFile* file = new TFile(argv[1]);

   if(file == nullptr) {
      printf("Failed to open file '%s'!\n",argv[1]);
      return 1;
   }
   if(!file->IsOpen()) {
      printf("Failed to open file '%s'!\n",argv[1]);
      return 1;
   }
   printf("Sorting file:" DBLUE " %s" RESET_COLOR"\n",file->GetName());

   //Get the TGRSIRunInfo from the analysis Tree.
   TGRSIRunInfo* runinfo  = (TGRSIRunInfo*)file->Get("TGRSIRunInfo");

   //Get PPG from File
   TPPG* myPPG = (TPPG*)file->Get("TPPG");

   TTree* tree = (TTree*) file->Get("AnalysisTree");
   TChannel::ReadCalFromTree(tree);
   if(tree == nullptr) {
      printf("Failed to find analysis tree in file '%s'!\n",argv[1]);
      return 1;
   }

   TList *list;//We return a list because we fill a bunch of TH1's and shove them into this list.
   TFile * outfile;
   if(argc<3)
   {
      if(!runinfo){
         printf("Could not find run info, please provide output file name\n");
         return 0;
      }
      int runnumber = runinfo->RunNumber();
      int subrunnumber = runinfo->SubRunNumber();
      outfile = new TFile(Form("angCorr%05d_%03d.root",runnumber,subrunnumber),"recreate");
   }
   else
   {
      outfile = new TFile(argv[2],"recreate");
   }

   std::cout << argv[0] << ": starting Analysis after " << w.RealTime() << " seconds" << std::endl;
   w.Continue();
   if(argc < 4) {
      list = exAnalysis(tree, myPPG, runinfo, 0, &w);
   } else {
      int entries = atoi(argv[3]);
      std::cout<<"Limiting processing of analysis tree to "<<entries<<" entries!"<<std::endl;
      list = exAnalysis(tree, myPPG, runinfo, entries, &w);
   }
   printf("Writing to File: " DYELLOW "%s" RESET_COLOR"\n",outfile->GetName());
   list->Write();
   //Write the run info into the tree as well if there is run info in the Analysis Tree
   TGRSISortList *sortinfolist = new TGRSISortList;
   if(runinfo){
      TGRSISortInfo *info = new TGRSISortInfo(runinfo);
      sortinfolist->AddSortInfo(info);
      sortinfolist->Write("TGRSISortList",TObject::kSingleKey);
   }
   outfile->Close();

   std::cout << argv[0] << " done after " << w.RealTime() << " seconds" << std::endl << std::endl;


   return 0;
}

#endif
