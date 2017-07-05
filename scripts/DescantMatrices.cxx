// g++ DescantMatrices.cxx -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries -lAnalysisTreeBuilder -lGriffin -lSceptar
// -lDescant -lPaces -lGRSIDetector -lTGRSIFit -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lGRSILoop -lMidasFormat
// -lGRSIRootIO -lDataParser -lGRSIFormat -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml `grsi-config --cflags
// --libs` `root-config --cflags --libs`  -lTreePlayer -lGROOT -lX11 -lXpm -lSpectrum

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
#include "TGRSISortInfo.h"
#include "Globals.h"
#include "TVectorD.h"
#include "TPPG.h"
#include "THnSparse.h"

#ifndef __CINT__
#include "TGriffin.h"
#include "TSceptar.h"
#include "TDescant.h"
#include "TZeroDegree.h"
#endif

// This code is an example of how to write an analysis script to analyse an analysis tree
// The compiled script works like this
//
//  1. Starts in the main function by finding the analysis tree, setting up input and output files
//  2. Calls the DescantMatrices function
//  3. DescantMatrices creates 1D and 2D histograms and adds them to a list
//  4. Some loops over event "packets" decides which histograms should be filled and fills them.
//  5. The list of now filled histograms is returned to the main function
//  6. The list is written (meaning all of the histograms are written) to the output root file
//  7. Papers are published, theses are granted, high-fives are made
//
/////////////////////////////////////////////////////////////////////////////////////////

// This function gets run if running interpretively
// Not recommended for the analysis scripts
#ifdef __CINT__
void DescantMatrices()
{
   if(!AnalysisTree) {
      printf("No analysis tree found!\n");
      return;
   }
   // coinc window = 0-20, bg window 40-60, 6000 bins from 0. to 6000. (default is 4000)
   TList* list = DescantMatrices(AnalysisTree, TPPG, TGRSIRunInfo, 0.);

   TFile* outfile = new TFile("output.root", "recreate");
   list->Write();
}
#endif

TList* DescantMatrices(TTree* tree, TPPG* ppg, TGRSIRunInfo* runInfo, long maxEntries = 0, TStopwatch* w = nullptr)
{
   if(runInfo == nullptr) {
      return nullptr;
   }

   ///////////////////////////////////// SETUP ///////////////////////////////////////

   // Draw Descant Waveform flag
   bool dWaveForm_flag = true;

   // Histogram paramaters
   Double_t low     = 0;
   Double_t high    = 10000;
   Double_t nofBins = 10000;

   // Coincidence Parameters
   Double_t ggTlow  = 0.; // Times are in 10's of ns
   Double_t ggThigh = 40.;
   Double_t gbTlow  = -10.; // was -15.
   Double_t gbThigh = 20.;  // was 10.

   Double_t gDnTlow  = 0.;
   Double_t gDnThigh = 100.;
   Double_t gDgTlow  = 0.;
   Double_t gDgThigh = 100.;
   Double_t gZTlow   = 0.;
   Double_t gZThigh  = 100.;

   Double_t ggBGlow   = 100.;
   Double_t ggBGhigh  = 175.;
   Double_t gbBGlow   = -160.;
   Double_t gbBGhigh  = 0.;
   Double_t ggBGScale = (ggThigh - ggTlow) / (ggBGhigh - ggBGlow);
   Double_t gbBGScale = (gbThigh - gbTlow) / (gbBGhigh - gbBGlow);

   Double_t betaThres = 800.;

   // this is in ms
   Double_t cycleLength = 15000;
   if(ppg != nullptr) {
      cycleLength = ppg->GetCycleLength() / 1e5;
   }

   Double_t bgStart  = 1.5e8;
   Double_t bgEnd    = 3.5e8;
   Double_t onStart  = 3.5e8;
   Double_t onEnd    = 14.0e8;
   Double_t offStart = 14.5e8;
   Double_t offEnd   = 15.5e8;

   if(w == nullptr) {
      w = new TStopwatch;
      w->Start();
   }
   auto* list = new TList;

   // const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

   // We create some spectra and then add it to the list
   // hit patterns
   auto* bIdVsgId = new TH2D("bIdVsgId", "Sceptar Id vs Griffin Id", 20, 1, 21, 64, 1, 65);
   list->Add(bIdVsgId);

   // gamma single spectra
   auto* gammaSingles = new TH1D("gammaSingles", "#gamma singles;energy[keV]", nofBins, low, high);
   list->Add(gammaSingles);
   auto* gammaSinglesB = new TH1D("gammaSinglesB", "#beta #gamma;energy[keV]", nofBins, low, high);
   list->Add(gammaSinglesB);
   auto* gammaSinglesBm =
      new TH1D("gammaSinglesBm", "#beta #gamma (multiple counting of #beta's);energy[keV]", nofBins, low, high);
   list->Add(gammaSinglesBm);
   auto* gammaSinglesBt = new TH1D("gammaSinglesBt", "#beta #gamma t-rand-corr; energy[keV]", nofBins, low, high);
   list->Add(gammaSinglesBt);
   auto* ggTimeDiff = new TH1D("ggTimeDiff", "#gamma-#gamma time difference", 300, 0, 300);
   list->Add(ggTimeDiff);
   auto* gbTimeDiff = new TH1D("gbTimeDiff", "#gamma-#beta time difference", 2000, -1000, 1000);
   list->Add(gbTimeDiff);
   auto* bbTimeDiff =
      new TH2D("bbTimeDiff", "#beta energy vs. #beta-#beta time difference", 2000, -1000, 1000, 1000, 0., 2e6);
   list->Add(bbTimeDiff);
   auto* gTimeDiff = new TH2D("gTimeDiff", "channel vs. time difference", 2000, 0, 2000, 65, 1., 65.);
   list->Add(gTimeDiff);
   auto* gtimestamp = new TH1F("gtimestamp", "#gamma time stamp", 10000, 0, 1000);
   list->Add(gtimestamp);
   auto* btimestamp = new TH1F("btimestamp", "#beta time stamp", 10000, 0, 1000);
   list->Add(btimestamp);
   auto* gbEnergyvsgTime =
      new TH2F("gbEnergyvsgTime", "#gamma #beta coincident: #gamma timestamp vs. #gamma energy; Time [s]; Energy [keV]",
               1000, 0, 1000, nofBins, low, high);
   list->Add(gbEnergyvsgTime);
   auto* gbEnergyvsbTime =
      new TH2F("gbEnergyvsbTime", "#gamma #beta coincident: #beta timestamp vs. #gamma energy; Time [s]; Energy [keV]",
               1000, 0, 1000, nofBins, low, high);
   list->Add(gbEnergyvsbTime);
   auto* ggmatrix = new TH2D("ggmatrix", "#gamma-#gamma matrix", nofBins, low, high, nofBins, low, high);
   list->Add(ggmatrix);
   auto* ggmatrixt = new TH2D("ggmatrixt", "#gamma-#gamma matrix t-corr", nofBins, low, high, nofBins, low, high);
   list->Add(ggmatrixt);
   auto* gammaSinglesB_hp = new TH2F("gammaSinglesB_hp", "#gamma-#beta vs. SC channel", nofBins, low, high, 20, 1, 21);
   list->Add(gammaSinglesB_hp);
   auto* ggbmatrix = new TH2F("ggbmatrix", "#gamma-#gamma-#beta matrix", nofBins, low, high, nofBins, low, high);
   list->Add(ggbmatrix);
   auto* ggbmatrixt =
      new TH2F("ggbmatrixt", "#gamma-#gamma-#beta matrix t-corr", nofBins, low, high, nofBins, low, high);
   list->Add(ggbmatrixt);
   auto* grifscep_hp = new TH2F("grifscep_hp", "Sceptar vs Griffin hit pattern", 64, 0, 64, 20, 0, 20);
   list->Add(grifscep_hp);
   auto* gbTimevsg = new TH2F("gbTimevsg", "#gamma energy vs. #gamma-#beta timing", 300, -150, 150, nofBins, low, high);
   list->Add(gbTimevsg);
   auto* ggbmatrixOn =
      new TH2D("ggbmatrixOn", "#gamma-#gamma-#beta matrix, beam on window", nofBins, low, high, nofBins, low, high);
   list->Add(ggbmatrixOn);
   auto* ggbmatrixBg =
      new TH2F("ggbmatrixBg", "#gamma-#gamma-#beta matrix, background window", nofBins, low, high, nofBins, low, high);
   list->Add(ggbmatrixBg);
   auto* ggbmatrixOff =
      new TH2F("ggbmatrixOff", "#gamma-#gamma-#beta matrix, beam off window", nofBins, low, high, nofBins, low, high);
   list->Add(ggbmatrixOff);
   auto* bSingles_hp = new TH2D("bSingles_hp", "#beta Energy vs Channel", 10000, 0, 10000, 20, 1, 21);
   list->Add(bSingles_hp);

   Double_t min[4]   = {0, 0, 0, 0};
   Double_t max[4]   = {cycleLength, 65, 1000, 2000.};
   Int_t    bins[4]  = {static_cast<Int_t>(cycleLength / 10), 65, 1000, 2000};
   Double_t pMin[4]  = {0, 0, 0, 0};
   Double_t pMax[4]  = {cycleLength, 65, 1000, 2.};
   Int_t    pBins[4] = {static_cast<Int_t>(cycleLength / 10), 65, 1000, 2};

   TH2F*       gammaSinglesCyc   = nullptr;
   TH2F*       gammaSinglesBCyc  = nullptr;
   TH2F*       gammaSinglesBmCyc = nullptr;
   THnSparseF* pileup            = nullptr;
   THnSparseF* gSinglesCyc_chan  = nullptr;
   THnSparseF* gbmatrixCyc_chan  = nullptr;
   TH1D*       gPUTotalCyc       = nullptr;
   TH1D*       gNPTotalCyc       = nullptr;
   TH1D*       gPUCyc            = nullptr;
   TH2F*       betaSinglesCyc    = nullptr;

   if(ppg != nullptr) {
      gPUTotalCyc =
         new TH1D("gPUTotalCyc", "Total Pileup hits as function of time in cycle", cycleLength / 10, 0, cycleLength);
      list->Add(gPUTotalCyc);
      gNPTotalCyc = new TH1D("gNPTotalCyc", "Total not Piled-up hits as function of time in cycle", cycleLength / 10, 0,
                             cycleLength);
      list->Add(gNPTotalCyc);
      gPUCyc = new TH1D("gPUCyc", "Pileup events as function of time in cycle", cycleLength / 10, 0, cycleLength);
      list->Add(gPUCyc);
      pileup = new THnSparseF("pileup", "Pileup events vs. channel vs. cycle vs. Time in cycle", 4, pBins, pMin, pMax);
      list->Add(pileup);
      gSinglesCyc_chan = new THnSparseF("gSinglesCyc_chan", "#gamma singles vs. channel vs. Time in cycle vs Energy", 4,
                                        bins, min, max);
      list->Add(gSinglesCyc_chan);
      gbmatrixCyc_chan = new THnSparseF("gbmatrixCyc_chan", "#gamma singles vs. channel vs. Time in cycle vs Energy", 4,
                                        bins, min, max);
      list->Add(gbmatrixCyc_chan);
      gammaSinglesCyc = new TH2F("gammaSinglesCyc", "Cycle time vs. #gamma energy", cycleLength / 10., 0., cycleLength,
                                 nofBins, low, high);
      list->Add(gammaSinglesCyc);
      gammaSinglesBCyc = new TH2F("gammaSinglesBCyc", "Cycle time vs. #beta coinc #gamma energy", cycleLength / 10., 0.,
                                  ppg->GetCycleLength() / 1e5, nofBins, low, high);
      list->Add(gammaSinglesBCyc);
      gammaSinglesBmCyc =
         new TH2F("gammaSinglesBmCyc", "Cycle time vs. #beta coinc #gamma energy (multiple counting of #beta's)",
                  cycleLength / 10., 0., cycleLength, nofBins, low, high);
      list->Add(gammaSinglesBmCyc);
      betaSinglesCyc = new TH2F("betaSinglesCyc", "Cycle number vs. cycle time for #beta's", cycleLength / 10., 0.,
                                cycleLength, 1000, 0, 1000);
      list->Add(betaSinglesCyc);
   }

   // addback spectra
   auto* gammaAddback = new TH1D("gammaAddback", "#gamma singles;energy[keV]", nofBins, low, high);
   list->Add(gammaAddback);
   auto* gammaAddbackB = new TH1D("gammaAddbackB", "#beta #gamma;energy[keV]", nofBins, low, high);
   list->Add(gammaAddbackB);
   auto* gammaAddbackBm =
      new TH1D("gammaAddbackBm", "#beta #gamma (multiple counting of #beta's);energy[keV]", nofBins, low, high);
   list->Add(gammaAddbackBm);
   auto* gammaAddbackBt = new TH1D("gammaAddbackBt", "#beta #gamma t-rand-corr; energy[keV]", nofBins, low, high);
   list->Add(gammaAddbackBt);
   auto* aaTimeDiff = new TH1D("aaTimeDiff", "#gamma-#gamma time difference", 300, 0, 300);
   list->Add(aaTimeDiff);
   auto* abTimeDiff = new TH1D("abTimeDiff", "#gamma-#beta time difference", 2000, -1000, 1000);
   list->Add(abTimeDiff);
   auto* abEnergyvsgTime =
      new TH2F("abEnergyvsgTime", "#gamma #beta coincident: #gamma timestamp vs. #gamma energy; Time [s]; Energy [keV]",
               1000, 0, 1000, nofBins, low, high);
   list->Add(abEnergyvsgTime);
   auto* abEnergyvsbTime =
      new TH2F("abEnergyvsbTime", "#gamma #beta coincident: #beta timestamp vs. #gamma energy; Time [s]; Energy [keV]",
               1000, 0, 1000, nofBins, low, high);
   list->Add(abEnergyvsbTime);
   auto* aamatrix = new TH2D("aamatrix", "#gamma-#gamma matrix", nofBins, low, high, nofBins, low, high);
   list->Add(aamatrix);
   auto* aamatrixt = new TH2D("aamatrixt", "#gamma-#gamma matrix t-corr", nofBins, low, high, nofBins, low, high);
   list->Add(aamatrixt);
   auto* gammaAddbackB_hp = new TH2F("gammaAddbackB_hp", "#gamma-#beta vs. SC channel", nofBins, low, high, 20, 1, 21);
   list->Add(gammaAddbackB_hp);
   auto* aabmatrix = new TH2F("aabmatrix", "#gamma-#gamma-#beta matrix", nofBins, low, high, nofBins, low, high);
   list->Add(aabmatrix);
   auto* aabmatrixt =
      new TH2F("aabmatrixt", "#gamma-#gamma-#beta matrix t-corr", nofBins, low, high, nofBins, low, high);
   list->Add(aabmatrixt);
   auto* abTimevsg = new TH2F("abTimevsg", "#gamma energy vs. #gamma-#beta timing", 300, -150, 150, nofBins, low, high);
   list->Add(abTimevsg);
   auto* abTimevsgf = new TH2F("abTimevsgf", "#gamma energy vs. #gamma-#beta timing (first #beta only)", 300, -150, 150,
                               nofBins, low, high);
   list->Add(abTimevsgf);
   auto* abTimevsgl = new TH2F("abTimevsgl", "#gamma energy vs. #gamma-#beta timing (last #beta only)", 300, -150, 150,
                               nofBins, low, high);
   list->Add(abTimevsgl);
   auto* aabmatrixOn =
      new TH2D("aabmatrixOn", "#gamma-#gamma-#beta matrix, beam on window", nofBins, low, high, nofBins, low, high);
   list->Add(aabmatrixOn);
   auto* aabmatrixBg =
      new TH2F("aabmatrixBg", "#gamma-#gamma-#beta matrix, background window", nofBins, low, high, nofBins, low, high);
   list->Add(aabmatrixBg);
   auto* aabmatrixOff =
      new TH2F("aabmatrixOff", "#gamma-#gamma-#beta matrix, beam off window", nofBins, low, high, nofBins, low, high);
   list->Add(aabmatrixOff);

   TH2F* gammaAddbackCyc;
   TH2F* gammaAddbackBCyc;
   TH2F* gammaAddbackBmCyc;
   gammaAddbackCyc = new TH2F("gammaAddbackCyc", "Cycle time vs. #gamma energy", cycleLength / 10., 0., cycleLength,
                              nofBins, low, high);
   list->Add(gammaAddbackCyc);
   gammaAddbackBCyc = new TH2F("gammaAddbackBCyc", "Cycle time vs. #beta coinc #gamma energy", cycleLength / 10., 0.,
                               cycleLength, nofBins, low, high);
   list->Add(gammaAddbackBCyc);
   gammaAddbackBmCyc =
      new TH2F("gammaAddbackBmCyc", "Cycle time vs. #beta coinc #gamma energy (multiple counting of #beta's)",
               cycleLength / 10., 0., cycleLength, nofBins, low, high);
   list->Add(gammaAddbackBmCyc);

   // Sceptar, Descant, ZDS stuff
   Int_t    dNbins[]   = {static_cast<Int_t>(cycleLength / 10.), 5000, 3000, 10000};
   Double_t dLowBin[]  = {0, 0, 0, -500};
   Double_t dHighBin[] = {cycleLength, 50000, 30000, 500};
   auto*    desEvSct = new THnSparseF("desEvSct", "Time in cycle, Descant Energy, PSD, Descant-Sceptar Time", 4, dNbins,
                                   dLowBin, dHighBin);
   list->Add(desEvSct);
   auto* desEvZDSt =
      new THnSparseF("desEvZDSt", "Time in cycle, Descant Energy, PSD, Descant-ZDS Time", 4, dNbins, dLowBin, dHighBin);
   list->Add(desEvZDSt);
   Int_t    d2Nbins[]   = {static_cast<Int_t>(cycleLength / 10.), 5000, 3000, 16};
   Double_t d2LowBin[]  = {0, 0, 0, 60};
   Double_t d2HighBin[] = {cycleLength, 50000, 30000, 76};
   auto*    desPsdECyc = new THnSparseF("desPsdECyc", "Time in cycle, Descant Energy, PSD, Detector Number", 4, d2Nbins,
                                     d2LowBin, d2HighBin);
   list->Add(desPsdECyc);
   Int_t    d3Nbins[]   = {5000, 10000};
   Double_t d3LowBin[]  = {0, -500};
   Double_t d3HighBin[] = {50000, 500};
   auto* desEvGrt = new THnSparseF("desEvGrt", "Descant Energy, Descant-Griffin Time", 2, d3Nbins, d3LowBin, d3HighBin);
   list->Add(desEvGrt);

   auto* grifE_desn = new TH1D("grifE_desn", "Descant neutron gated #gamma-ray energies", 8000, 0, 8000);
   list->Add(grifE_desn);
   auto* grifE_desg = new TH1D("grifE_desg", "Descant #gamma gated #gamma-ray energies", 8000, 0, 8000);
   list->Add(grifE_desg);
   auto* grifE_zds = new TH1D("grifE_zds", "ZDS gated #gamma-ray energies", 8000, 0, 8000);
   list->Add(grifE_zds);

   auto* grifZDSTimeDiff = new TH1D("grifZDSTimeDiff", "ZDS and GRIFFIN time differences", 5000, -2500, 2500);
   list->Add(grifZDSTimeDiff);
   auto* grifDnTimeDiff = new TH1D("grifDnTimeDiff", "Descant n and GRIFFIN time differences", 5000, -2500, 2500);
   list->Add(grifDnTimeDiff);
   auto* grifDgTimeDiff = new TH1D("grifDgTimeDiff", "Descant #gamma and GRIFFIN time differences", 5000, -2500, 2500);
   list->Add(grifDgTimeDiff);

   auto* desWaveForm = new TH2D("desWaveForm", "Normalized descant Waveforms", 1200, -30, 120, 1200, -0.1, 1.1);
   list->Add(desWaveForm);

   list->Sort(); // Sorts the list alphabetically
   if(ppg != nullptr) {
      list->Add(ppg);
   }
   list->Add(runInfo);

   if(ppg != nullptr) {
      TGRSIDetectorHit::SetPPGPtr(ppg);
   }

   ///////////////////////////////////// PROCESSING /////////////////////////////////////

   // set up branches
   // Each branch can hold multiple hits
   // ie TGriffin grif holds 3 gamma rays on a triples event
   TGriffin*    grif = nullptr;
   TSceptar*    scep = nullptr;
   TDescant*    desc = nullptr;
   TZeroDegree* zds  = nullptr;
   tree->SetBranchAddress("TGriffin", &grif); // We assume we always have a Griffin

   bool gotSceptar;
   bool gotZDS;
   bool gotDescant;
   if(tree->FindBranch("TSceptar") == nullptr) { // We check to see if we have a Scepter branch in the analysis tree
      gotSceptar = false;
   } else {
      tree->SetBranchAddress("TSceptar", &scep);
      gotSceptar = true;
   }

   if(tree->FindBranch("TDescant") == nullptr) {
      gotDescant = false;
   } else {
      tree->SetBranchAddress("TDescant", &desc);
      gotDescant = true;
   }

   if(tree->FindBranch("TZeroDegree") == nullptr) {
      gotZDS = false;
   } else {
      tree->SetBranchAddress("TZeroDegree", &zds);
      gotZDS = true;
   }

   // tree->LoadBaskets(MEM_SIZE);

   // long entries = tree->GetEntries();
   // long entries = 1e6;
   // These are the indices of the two hits being compared
   int one;
   int two;

   auto* t = new TVectorD(2);
   (*t)[0] = runInfo->RunStart();
   (*t)[1] = runInfo->RunStop();

   list->Add(t);

   // store the last timestamp of each channel
   std::vector<long> lastTimeStamp(65, 0);

   std::cout<<std::fixed<<std::setprecision(1); // This just make outputs not look terrible
   // size_t angIndex;
   if(maxEntries == 0 || maxEntries > tree->GetEntries()) {
      maxEntries = tree->GetEntries();
   }
   // maxEntries = 1e5;
   int entry;
   for(entry = 1; entry < maxEntries; ++entry) { // Only loop over the set number of entries
      // I'm starting at entry 1 because of the weird high stamp of 4.
      tree->GetEntry(entry);

      grif->ResetAddback();

      // Loop over Descants in the event packet
      if(gotDescant) {
         Double_t DFillArr[4]  = {0, 0, 0, 0};
         Double_t D2FillArr[2] = {0, 0};
         for(one = 0; one < static_cast<int>(desc->GetMultiplicity()); ++one) {
            if(ppg != nullptr) {
               DFillArr[0] = ppg->GetTimeInCycle((desc->GetDescantHit(one)->GetTime())) / 1e5;
            }
            DFillArr[1] = desc->GetDescantHit(one)->GetEnergy();
            DFillArr[2] = desc->GetDescantHit(one)->GetPsd();
            DFillArr[3] = desc->GetDescantHit(one)->GetChannel()->GetNumber();
            desPsdECyc->Fill(DFillArr);
            D2FillArr[0] = desc->GetDescantHit(one)->GetEnergy();
            if(gotSceptar) {
               for(two = 0; two < static_cast<int>(scep->GetMultiplicity()); ++two) {
                  DFillArr[3] = desc->GetDescantHit(one)->GetTime() - scep->GetHit(two)->GetTime();
                  desEvSct->Fill(DFillArr);
               }
            }
            if(gotZDS) {
               for(two = 0; two < static_cast<int>(zds->GetMultiplicity()); ++two) {
                  DFillArr[3] = desc->GetDescantHit(one)->GetTime() - zds->GetHit(two)->GetTime();
                  desEvZDSt->Fill(DFillArr);
               }
            }
            for(two = 0; two < (int)grif->GetMultiplicity(); ++two) {
               D2FillArr[1] = desc->GetDescantHit(one)->GetTime() - grif->GetHit(two)->GetTime();
               desEvGrt->Fill(D2FillArr);
            }
            if(dWaveForm_flag) {
               // Plot the normalized Waveform here.
               const std::vector<Short_t>* waveform = desc->GetDescantHit(one)->GetWaveform();
               Short_t                     maximum  = *std::max_element(waveform->begin(), waveform->end());
               for(size_t i = 0; i < waveform->size(); ++i) {
                  desWaveForm->Fill(i - desc->GetDescantHit(one)->GetCfd() / 256.,
                                    waveform->at(i) / static_cast<double>(maximum));
               }
            }
         }
      }

      // loop over the gammas in the event packet
      // grif is the variable which points to the current TGriffin
      for(one = 0; one < (int)grif->GetMultiplicity(); ++one) {
         // We want to put every gamma ray in this event into the singles
         gammaSingles->Fill(grif->GetGriffinHit(one)->GetEnergy());
         if(ppg != nullptr) {
            if(grif->GetGriffinHit(one)->PUHit() == 1) {
               gPUCyc->Fill(ppg->GetTimeInCycle((grif->GetHit(one)->GetTime()) / 1e5));
            }
            if(grif->GetGriffinHit(one)->NPileUps() < 2) {
               gNPTotalCyc->Fill(ppg->GetTimeInCycle((grif->GetHit(one)->GetTime()) / 1e5));
            }

            gPUTotalCyc->Fill(ppg->GetTimeInCycle((grif->GetHit(one)->GetTime()) / 1e5));
         }

         gtimestamp->Fill(grif->GetGriffinHit(one)->GetTime() / 100000000.);
         Long_t time = static_cast<Long_t>(grif->GetHit(one)->GetTime());
         if(ppg != nullptr) {
            time = time % ppg->GetCycleLength();
            // gammaSinglesCyc->Fill((time - ppg->GetLastStatusTime(time, TPPG::kTapeMove))/1e5,
            // grif->GetGriffinHit(one)->GetEnergy());
            gammaSinglesCyc->Fill(time / 1e5, grif->GetGriffinHit(one)->GetEnergy());
            Double_t fillval[4] = {((ppg->GetTimeInCycle(grif->GetHit(one)->GetTime())) / 1e5),
                                   static_cast<Double_t>(grif->GetGriffinHit(one)->GetArrayNumber()),
                                   static_cast<Double_t>((ppg->GetCycleNumber((grif->GetHit(one)->GetTime())))),
                                   grif->GetGriffinHit(one)->GetEnergy()};
            gSinglesCyc_chan->Fill(fillval);
            if(grif->GetGriffinHit(one)->GetArrayNumber() != 26) {
               if(grif->GetGriffinHit(one)->PUHit() == 1) {
                  if(grif->GetGriffinHit(one)->NPileUps() == 1) {
                     fillval[3] = 0;
                  } else {
                     fillval[3] = 1;
                  }
                  pileup->Fill(fillval);
               }
            }
         }
         if(grif->GetGriffinHit(one)->GetArrayNumber() < lastTimeStamp.size()) {
            if(lastTimeStamp.at(grif->GetGriffinHit(one)->GetArrayNumber()) > 0) {
               gTimeDiff->Fill(grif->GetHit(one)->GetTimeStamp() -
                                  lastTimeStamp.at(grif->GetGriffinHit(one)->GetArrayNumber()),
                               grif->GetGriffinHit(one)->GetArrayNumber());
            }
            lastTimeStamp.at(grif->GetGriffinHit(one)->GetArrayNumber()) = grif->GetHit(one)->GetTimeStamp();
         }
         // We now want to loop over any other gammas in this packet
         for(two = 0; two < (int)grif->GetMultiplicity(); ++two) {
            if(two == one) { // If we are looking at the same gamma we don't want to call it a coincidence
               continue;
            }
            // Check to see if the two gammas are close enough in time
            ggTimeDiff->Fill(TMath::Abs(grif->GetGriffinHit(two)->GetTime() - grif->GetGriffinHit(one)->GetTime()));
            if(ggTlow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime() - grif->GetGriffinHit(one)->GetTime()) &&
               TMath::Abs(grif->GetGriffinHit(two)->GetTime() - grif->GetGriffinHit(one)->GetTime()) < ggThigh) {
               // If they are close enough in time, fill the gamma-gamma matrix. This will be symmetric because we are
               // doing a double loop over gammas
               ggmatrix->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
            }
            if(ggBGlow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime() - grif->GetGriffinHit(one)->GetTime()) &&
               TMath::Abs(grif->GetGriffinHit(two)->GetTime() - grif->GetGriffinHit(one)->GetTime()) < ggBGhigh) {
               // If they are not close enough in time, fill the time-random gamma-gamma matrix. This will be symmetric
               // because we are doing a double loop over gammas
               ggmatrixt->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
            }
         }
      }

      // Now we make beta gamma coincident matrices
      if(gotSceptar && scep->GetMultiplicity() > 0) {
         bool plotted_flag = false;
         // We do an outside loop on gammas so that we can break on the betas if we see a beta in coincidence (we don't
         // want to bin twice just because we have two betas)
         for(int b = 0; b < scep->GetMultiplicity(); ++b) {
            bSingles_hp->Fill(scep->GetHit(b)->GetEnergy(), scep->GetHit(b)->GetDetector());
            if(scep->GetHit(b)->GetEnergy() < betaThres) {
               continue;
            }
            btimestamp->Fill(scep->GetHit(b)->GetTime() / 1e8);
            if((ppg != nullptr) && !plotted_flag) { // Fill on first hit only.
               betaSinglesCyc->Fill(((ppg->GetTimeInCycle(scep->GetHit(b)->GetTime())) / 1e5),
                                    ppg->GetCycleNumber(static_cast<ULong64_t>(scep->GetHit(b)->GetTime())));
               //  betaSinglesCyc->Fill((((ULong64_t)(scep->GetHit(b)->GetTime()))%(ppg->GetCycleLength()))/1e5,(scep->GetHit(b)->GetTime())/(ppg->GetCycleLength()));
               plotted_flag = true;
            }
            for(int b2 = 0; b2 < scep->GetMultiplicity(); ++b2) {
               if(b == b2) {
                  continue;
               }
               bbTimeDiff->Fill(scep->GetHit(b)->GetTime() - scep->GetHit(b2)->GetTime(), scep->GetHit(b)->GetEnergy());
            }
         }
         for(one = 0; one < (int)grif->GetMultiplicity(); ++one) {
            bool bfound = false;
            for(int b = 0; b < scep->GetMultiplicity(); ++b) {
               if(scep->GetHit(b)->GetEnergy() < betaThres) {
                  continue;
               }
               // Be careful about time ordering!!!! betas and gammas are not symmetric out of the DAQ
               // Fill the time diffrence spectra
               gbTimeDiff->Fill(grif->GetHit(one)->GetTime() - scep->GetHit(b)->GetTime());
               gbTimevsg->Fill(grif->GetHit(one)->GetTime() - scep->GetHit(b)->GetTime(),
                               grif->GetHit(one)->GetEnergy());
               if((gbTlow <= grif->GetHit(one)->GetTime() - scep->GetHit(b)->GetTime()) &&
                  (grif->GetHit(one)->GetTime() - scep->GetHit(b)->GetTime() <= gbThigh)) {
                  gbEnergyvsbTime->Fill(scep->GetHit(b)->GetTime(), grif->GetHit(one)->GetEnergy());
               }
               if((gbTlow <= grif->GetHit(one)->GetTime() - scep->GetHit(b)->GetTime()) &&
                  (grif->GetHit(one)->GetTime() - scep->GetHit(b)->GetTime() <= gbThigh)) {
                  ULong64_t time = static_cast<ULong64_t>(grif->GetHit(one)->GetTime());
                  if(ppg != nullptr) {
                     time = time % ppg->GetCycleLength();
                     gammaSinglesBmCyc->Fill(time / 1e5, grif->GetGriffinHit(one)->GetEnergy());
                  }
                  // Plots a gamma energy spectrum in coincidence with a beta
                  gbEnergyvsgTime->Fill(grif->GetGriffinHit(one)->GetTime() / 1e8,
                                        grif->GetGriffinHit(one)->GetEnergy());
                  gammaSinglesBm->Fill(grif->GetGriffinHit(one)->GetEnergy());
                  bIdVsgId->Fill(scep->GetSceptarHit(b)->GetDetector(), grif->GetGriffinHit(one)->GetArrayNumber());
                  if(!bfound) {
                     gammaSinglesB->Fill(grif->GetGriffinHit(one)->GetEnergy());
                     if(ppg != nullptr) {
                        Double_t fillval[4] = {((ppg->GetTimeInCycle(grif->GetHit(one)->GetTime())) / 1e5),
                                               static_cast<Double_t>(grif->GetGriffinHit(one)->GetArrayNumber()),
                                               static_cast<Double_t>(ppg->GetCycleNumber(grif->GetHit(one)->GetTime())),
                                               grif->GetHit(one)->GetEnergy()};
                        gammaSinglesBCyc->Fill(
                           ppg->GetTimeInCycle(static_cast<ULong64_t>(grif->GetHit(one)->GetTime())) / 1e5,
                           grif->GetGriffinHit(one)->GetEnergy());
                        gbmatrixCyc_chan->Fill(fillval);
                     }
                  }
                  gammaSinglesB_hp->Fill(grif->GetGriffinHit(one)->GetEnergy(), scep->GetSceptarHit(b)->GetDetector());
                  grifscep_hp->Fill(grif->GetGriffinHit(one)->GetArrayNumber(), scep->GetSceptarHit(b)->GetDetector());
                  // gammaSinglesBCyc->Fill((time - ppg->GetLastStatusTime(time, TPPG::kTapeMove))/1e5,
                  // grif->GetGriffinHit(one)->GetEnergy());
                  // Now we want to loop over gamma rays if they are in coincidence.
                  if(grif->GetMultiplicity() > 1) {
                     for(two = 0; two < (int)grif->GetMultiplicity(); ++two) {
                        if(two == one) { // If we are looking at the same gamma we don't want to call it a coincidence
                           continue;
                        }

                        if(ggTlow <=
                              TMath::Abs(grif->GetGriffinHit(two)->GetTime() - grif->GetGriffinHit(one)->GetTime()) &&
                           TMath::Abs(grif->GetGriffinHit(two)->GetTime() - grif->GetGriffinHit(one)->GetTime()) <
                              ggThigh) {
                           // If they are close enough in time, fill the gamma-gamma-beta matrix. This will be symmetric
                           // because we are doing a double loop over gammas
                           ggbmatrix->Fill(grif->GetGriffinHit(one)->GetEnergy(),
                                           grif->GetGriffinHit(two)->GetEnergy());
                           if((ppg != nullptr) &&
                              ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) > bgStart &&
                              ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) < bgEnd) {
                              ggbmatrixBg->Fill(grif->GetGriffinHit(one)->GetEnergy(),
                                                grif->GetGriffinHit(two)->GetEnergy());
                           } else if((ppg != nullptr) &&
                                     ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) > onStart &&
                                     ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) < onEnd) {
                              ggbmatrixOn->Fill(grif->GetGriffinHit(one)->GetEnergy(),
                                                grif->GetGriffinHit(two)->GetEnergy());
                           } else if((ppg != nullptr) &&
                                     ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) > offStart &&
                                     ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) < offEnd) {
                              ggbmatrixOff->Fill(grif->GetGriffinHit(one)->GetEnergy(),
                                                 grif->GetGriffinHit(two)->GetEnergy());
                           }
                        }
                        if(ggBGlow <=
                              TMath::Abs(grif->GetGriffinHit(two)->GetTime() - grif->GetGriffinHit(one)->GetTime()) &&
                           TMath::Abs(grif->GetGriffinHit(two)->GetTime() - grif->GetGriffinHit(one)->GetTime()) <
                              ggBGhigh) {
                           // If they are not close enough in time, fill the gamma-gamma-beta time-random matrix. This
                           // will be symmetric because we are doing a double loop over gammas
                           ggbmatrixt->Fill(grif->GetGriffinHit(one)->GetEnergy(),
                                            grif->GetGriffinHit(two)->GetEnergy());
                        }
                     }
                  }
                  bfound = true;
               }
               if((gbBGlow <= grif->GetHit(one)->GetTime() - scep->GetHit(b)->GetTime()) &&
                  (grif->GetHit(one)->GetTime() - scep->GetHit(b)->GetTime() <= gbBGhigh)) {
                  gammaSinglesBt->Fill(grif->GetHit(one)->GetEnergy());
               }
            }
            bool zfound = false;
            for(int b = 0; b < zds->GetMultiplicity(); ++b) {
               // Be careful about time ordering!!!! betas and gammas are not symmetric out of the DAQ
               // Fill the time diffrence spectra
               grifZDSTimeDiff->Fill(grif->GetHit(one)->GetTime() - zds->GetHit(b)->GetTime());
               if((gZTlow <= grif->GetHit(one)->GetTime() - zds->GetHit(b)->GetTime()) &&
                  (grif->GetHit(one)->GetTime() - zds->GetHit(b)->GetTime() <= gZThigh)) {
                  if(!zfound) {
                     grifE_zds->Fill(grif->GetGriffinHit(one)->GetEnergy());
                     zfound = true;
                  }
               }
            }
            bool Dnfound = false;
            bool Dgfound = false;
            for(int b = 0; b < desc->GetMultiplicity(); ++b) {
               // Be careful about time ordering!!!! betas and gammas are not symmetric out of the DAQ
               // Fill the time diffrence spectra
               // IF STATEMENT ABOUT NEUTRONS
               grifDnTimeDiff->Fill(grif->GetHit(one)->GetTime() - desc->GetHit(b)->GetTime());
               if((gDnTlow <= grif->GetHit(one)->GetTime() - desc->GetHit(b)->GetTime()) &&
                  (grif->GetHit(one)->GetTime() - desc->GetHit(b)->GetTime() <= gDnThigh)) {
                  if(!Dnfound) {
                     grifE_desn->Fill(grif->GetGriffinHit(one)->GetEnergy());
                     Dnfound = true;
                  }
               }
               // IF STATEMENT ABOUT GAMMAS
               grifDgTimeDiff->Fill(grif->GetHit(one)->GetTime() - desc->GetHit(b)->GetTime());
               if((gDgTlow <= grif->GetHit(one)->GetTime() - desc->GetHit(b)->GetTime()) &&
                  (grif->GetHit(one)->GetTime() - desc->GetHit(b)->GetTime() <= gDgThigh)) {
                  if(!Dgfound) {
                     grifE_desg->Fill(grif->GetGriffinHit(one)->GetEnergy());
                     Dgfound = true;
                  }
               }
            }
         }

         // loop over the addbacks in the event packet
         // grif is the variable which points to the current TGriffin
         for(one = 0; one < (int)grif->GetAddbackMultiplicity(); ++one) {
            // We want to put every gamma ray in this event into the singles
            gammaAddback->Fill(grif->GetAddbackHit(one)->GetEnergy());
            Long_t time = static_cast<Long_t>(grif->GetAddbackHit(one)->GetTime());
            if(ppg != nullptr) {
               time = time % ppg->GetCycleLength();
               // gammaSinglesCyc->Fill((time - ppg->GetLastStatusTime(time, TPPG::kTapeMove))/1e5,
               // grif->GetGriffinHit(one)->GetEnergy());
               gammaAddbackCyc->Fill(time / 1e5, grif->GetAddbackHit(one)->GetEnergy());
               // We now want to loop over any other gammas in this packet
            }
            for(two = 0; two < (int)grif->GetAddbackMultiplicity(); ++two) {
               if(two == one) { // If we are looking at the same gamma we don't want to call it a coincidence
                  continue;
               }
               // Check to see if the two gammas are close enough in time
               // VINZENZ I THINK THIS IS BREAKING THIS FOR SOME REASON.
               aaTimeDiff->Fill(TMath::Abs(grif->GetAddbackHit(two)->GetTime() - grif->GetAddbackHit(one)->GetTime()));
               if(ggTlow <= TMath::Abs(grif->GetAddbackHit(two)->GetTime() - grif->GetAddbackHit(one)->GetTime()) &&
                  TMath::Abs(grif->GetAddbackHit(two)->GetTime() - grif->GetAddbackHit(one)->GetTime()) < ggThigh) {
                  // If they are close enough in time, fill the gamma-gamma matrix. This will be symmetric because we
                  // are doing a double loop over gammas
                  aamatrix->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy());
               }
               if(ggBGlow <= TMath::Abs(grif->GetAddbackHit(two)->GetTime() - grif->GetAddbackHit(one)->GetTime()) &&
                  TMath::Abs(grif->GetAddbackHit(two)->GetTime() - grif->GetAddbackHit(one)->GetTime()) < ggBGhigh) {
                  // If they are not close enough in time, fill the time-random gamma-gamma matrix. This will be
                  // symmetric because we are doing a double loop over gammas
                  aamatrixt->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy());
               }
            }
         }

         // Now we make beta gamma coincident matrices
         if(gotSceptar && scep->GetMultiplicity() > 0) {
            // We do an outside loop on gammas so that we can break on the betas if we see a beta in coincidence (we
            // don't want to bin twice just because we have two betas)
            for(one = 0; one < (int)grif->GetAddbackMultiplicity(); ++one) {
               bool found = false;
               for(int b = 0; b < scep->GetMultiplicity(); ++b) {
                  if(scep->GetHit(b)->GetEnergy() < betaThres) {
                     continue;
                  }
                  // Be careful about time ordering!!!! betas and gammas are not symmetric out of the DAQ
                  // Fill the time diffrence spectra
                  abTimeDiff->Fill(grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime());
                  abTimevsg->Fill(grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime(),
                                  grif->GetAddbackHit(one)->GetEnergy());
                  if(b == 0) {
                     abTimevsgf->Fill(grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime(),
                                      grif->GetAddbackHit(one)->GetEnergy());
                  }
                  if(b == scep->GetMultiplicity() - 1) {
                     abTimevsgl->Fill(grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime(),
                                      grif->GetAddbackHit(one)->GetEnergy());
                  }
                  if((gbTlow <= grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime()) &&
                     (grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime() <= gbThigh)) {
                     abEnergyvsbTime->Fill(scep->GetHit(b)->GetTime(), grif->GetAddbackHit(one)->GetEnergy());
                  }
                  if((gbTlow <= grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime()) &&
                     (grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime() <= gbThigh)) {
                     ULong64_t time = static_cast<ULong64_t>(grif->GetAddbackHit(one)->GetTime());
                     if(ppg != nullptr) {
                        time = time % ppg->GetCycleLength();
                        gammaAddbackBmCyc->Fill(time / 1e5, grif->GetAddbackHit(one)->GetEnergy());
                        // Plots a gamma energy spectrum in coincidence with a beta
                     }
                     abEnergyvsgTime->Fill(grif->GetAddbackHit(one)->GetTime() / 1e8,
                                           grif->GetAddbackHit(one)->GetEnergy());
                     gammaAddbackBm->Fill(grif->GetAddbackHit(one)->GetEnergy());
                     if(!found) {
                        gammaAddbackB->Fill(grif->GetAddbackHit(one)->GetEnergy());
                        if(ppg != nullptr) {
                           gammaAddbackBCyc->Fill(
                              ppg->GetTimeInCycle(static_cast<ULong64_t>(grif->GetAddbackHit(one)->GetTime())) / 1e5,
                              grif->GetAddbackHit(one)->GetEnergy());
                        }
                     }
                     gammaAddbackB_hp->Fill(grif->GetAddbackHit(one)->GetEnergy(),
                                            scep->GetSceptarHit(b)->GetDetector());
                     // Now we want to loop over gamma rays if they are in coincidence.
                     if(grif->GetAddbackMultiplicity() > 1) {
                        for(two = 0; two < (int)grif->GetAddbackMultiplicity(); ++two) {
                           if(two == one) { // If we are looking at the same gamma we don't want to call it a
                                            // coincidence
                              continue;
                           }

                           if(ggTlow <= TMath::Abs(grif->GetAddbackHit(two)->GetTime() -
                                                   grif->GetAddbackHit(one)->GetTime()) &&
                              TMath::Abs(grif->GetAddbackHit(two)->GetTime() - grif->GetAddbackHit(one)->GetTime()) <
                                 ggThigh) {
                              // If they are close enough in time, fill the gamma-gamma-beta matrix. This will be
                              // symmetric because we are doing a double loop over gammas
                              aabmatrix->Fill(grif->GetAddbackHit(one)->GetEnergy(),
                                              grif->GetAddbackHit(two)->GetEnergy());
                              if((ppg != nullptr) &&
                                 ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) > bgStart &&
                                 ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) < bgEnd) {
                                 aabmatrixBg->Fill(grif->GetAddbackHit(one)->GetEnergy(),
                                                   grif->GetAddbackHit(two)->GetEnergy());
                              } else if((ppg != nullptr) &&
                                        ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) > onStart &&
                                        ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) < onEnd) {
                                 aabmatrixOn->Fill(grif->GetAddbackHit(one)->GetEnergy(),
                                                   grif->GetAddbackHit(two)->GetEnergy());
                              } else if((ppg != nullptr) &&
                                        ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) > offStart &&
                                        ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) < offEnd) {
                                 aabmatrixOff->Fill(grif->GetAddbackHit(one)->GetEnergy(),
                                                    grif->GetAddbackHit(two)->GetEnergy());
                              }
                           }
                           if(ggBGlow <= TMath::Abs(grif->GetAddbackHit(two)->GetTime() -
                                                    grif->GetAddbackHit(one)->GetTime()) &&
                              TMath::Abs(grif->GetAddbackHit(two)->GetTime() - grif->GetAddbackHit(one)->GetTime()) <
                                 ggBGhigh) {
                              // If they are not close enough in time, fill the gamma-gamma-beta time-random matrix.
                              // This will be symmetric because we are doing a double loop over gammas
                              aabmatrixt->Fill(grif->GetAddbackHit(one)->GetEnergy(),
                                               grif->GetAddbackHit(two)->GetEnergy());
                           }
                        }
                     }
                     found = true;
                  }
                  if((gbBGlow <= grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime()) &&
                     (grif->GetAddbackHit(one)->GetTime() - scep->GetHit(b)->GetTime() <= gbBGhigh)) {
                     gammaAddbackBt->Fill(grif->GetAddbackHit(one)->GetEnergy());
                  }
               }
            }
         }
      }
      if((entry % 10000) == 0) {
         printf("Completed %d of %ld \r", entry, maxEntries);
      }
   }

   // various background subtractions
   ggmatrixt->Scale(-ggBGScale);
   ggmatrixt->Add(ggmatrix);

   ggbmatrixt->Scale(-ggBGScale);
   ggbmatrixt->Add(ggbmatrix);

   gammaSinglesBt->Scale(-gbBGScale);
   gammaSinglesBt->Add(gammaSinglesB);

   aamatrixt->Scale(-ggBGScale);
   aamatrixt->Add(aamatrix);

   aabmatrixt->Scale(-ggBGScale);
   aabmatrixt->Add(aabmatrix);

   gammaAddbackBt->Scale(-gbBGScale);
   gammaAddbackBt->Add(gammaAddbackB);

   ////create a gSinglesCyc_chan matrix without bad cycles
   // THnSparseF* gSinglesCyc_chan_corr = (THnSparseF*) gSinglesCyc_chan->Clone("gSinglesCyc_chan_corr");
   // THnSparseF* pileup_corr = (THnSparseF*) pileup->Clone("pileup_corr");
   ////set energy cut (this also cuts the pulser)
   // gSinglesCyc_chan->GetAxis(3)->SetRangeUser(1800.,1813.);
   ////try and determine the good cycles (without any cut)
   // TH1D* cyc = gSinglesCyc_chan->Projection(2,"A");
   ////for later subruns remove the first bin (there might be crappy data in there)
   // if(runInfo->SubRunNumber() != 0) {
   //   cyc->SetBinContent(1,0);
   //}
   // int nofCycles = 0;
   // int sum = cyc->GetBinContent(1);
   // for(int i = 2; i < cyc->GetNbinsX(); ++i) {
   //   if(cyc->GetBinContent(i) > 0) {
   //      ++nofCycles;
   //      sum += cyc->GetBinContent(i);
   //   }
   //}
   // float average = ((float) sum)/nofCycles;
   // Int_t* coord = new Int_t[4];
   // int removed = 0;
   // std::vector<int> badCycles;
   // bool last = true;
   // int lastGoodCycle = 0;
   // for(int i = cyc->GetNbinsX(); i >= 2; --i) {
   //   if(last && cyc->GetBinContent(i) > 0) {
   //      std::cout<<"Removing last cycle "<<i<<": average = "<<average<<", cycle =
   //      "<<cyc->GetBinContent(i)<<std::endl;
   //      removed += cyc->GetBinContent(i);
   //      --nofCycles;
   //      cyc->SetBinContent(i,0);
   //      last = false;
   //      continue;
   //   }
   //   if(cyc->GetBinContent(i) > 0 && lastGoodCycle == 0) {
   //      lastGoodCycle = i;
   //      break;
   //   }
   //}
   // bool first = true;
   // int firstGoodCycle = 0;
   // if(runInfo->SubRunNumber() == 0) {
   //   first = false;
   //   firstGoodCycle = 1;
   //}
   // for(int i = 1; i <= cyc->GetNbinsX(); ++i) {
   //   if(i == 1 && runInfo->SubRunNumber() != 0) {
   //      continue;
   //   }
   //   if(first && cyc->GetBinContent(i) > 0) {
   //      std::cout<<"Removing first cycle "<<i<<": average = "<<average<<", cycle =
   //      "<<cyc->GetBinContent(i)<<std::endl;
   //      removed += cyc->GetBinContent(i);
   //      --nofCycles;
   //      cyc->SetBinContent(i,0);
   //      first = false;
   //      continue;
   //   }
   //   if(cyc->GetBinContent(i) > 1.5*average) {
   //      std::cout<<badCycles.size()<<". bad cycle "<<i<<": average = "<<average<<", cycle =
   //      "<<cyc->GetBinContent(i)<<std::endl;
   //      removed += cyc->GetBinContent(i);
   //      --nofCycles;
   //      cyc->SetBinContent(i,0);
   //      badCycles.push_back(i);
   //      //check if this was our last good cycle
   //      if(i == lastGoodCycle) {
   //         //if this was the last good cycle, we remove this cycle from the list of bad cycles
   //         badCycles.resize(badCycles.size()-1);
   //         for(int j = 1; j < i; ++j) {
   //            //check if this was the first good cycle
   //            if(i-j == firstGoodCycle || badCycles.size() == 0) {
   //               lastGoodCycle = i-j;
   //               break;
   //            }
   //            //make certain that this wasn't also a bad cycle (we can skip that last bad cycle because that was this
   //            one)
   //            for(size_t k = 0; k < badCycles.size() - 1; ++k) {
   //               if(badCycles[k] != i-j) {
   //                  lastGoodCycle = i-j;
   //                  break;
   //               }
   //            }
   //         }
   //      }
   //      continue;
   //   }
   //   if(cyc->GetBinContent(i) > 0 && firstGoodCycle == 0) {
   //      firstGoodCycle = i;
   //   }
   //}
   // std::cout<<"removed "<<removed<<" counts (from "<<sum<<" counts), changes average from "<<average;
   // average = ((float)(sum-removed))/nofCycles;
   // std::cout<<" to "<<average<<std::endl;
   // for(int i = 1; i <= cyc->GetNbinsX(); ++i) {
   //   if(cyc->GetBinContent(i) > 0) {
   //      if(cyc->GetBinContent(i) < 0.5*average || cyc->GetBinContent(i) > 1.5*average) {
   //         badCycles.push_back(i);
   //         std::cout<<badCycles.size()<<". bad cycle "<<i<<": average = "<<average<<", cycle =
   //         "<<cyc->GetBinContent(i)<<std::endl;
   //      }
   //   }
   //}
   // bool* goodCycle = new bool[cyc->GetNbinsX()];
   // for(int i = 0; i < cyc->GetNbinsX(); ++i) {
   //   if(i >= firstGoodCycle && i <= lastGoodCycle) {
   //      goodCycle[i] = true;
   //   } else {
   //      goodCycle[i] = false;
   //   }
   //}
   // std::cout<<"good cycles: "<<firstGoodCycle<<" - "<<lastGoodCycle<<" minus cycles ";
   //
   // for(size_t i = 0; i < badCycles.size(); ++i) {
   //   std::cout<<badCycles[i];
   //   if(i < badCycles.size()-1) {
   //      std::cout<<", ";
   //   }
   //   goodCycle[badCycles[i]] = false;
   //}
   //
   // std::cout<<std::endl;
   // for(Long64_t bin = 0; bin < gSinglesCyc_chan_corr->GetNbins(); ++bin) {
   //   if(gSinglesCyc_chan_corr->GetBinContent(bin, coord) > 0 && !goodCycle[coord[2]]) {
   //      gSinglesCyc_chan_corr->SetBinContent(bin,0);
   //   }
   //}
   // list->Add(gSinglesCyc_chan_corr);
   // for(Long64_t bin = 0; bin < pileup_corr->GetNbins(); ++bin) {
   //   if(pileup_corr->GetBinContent(bin, coord) > 0 && !goodCycle[coord[2]]) {
   //      pileup_corr->SetBinContent(bin,0);
   //   }
   //}
   // list->Add(pileup_corr);
   std::cout<<"creating histograms done after "<<w->RealTime()<<" seconds"<<std::endl;
   w->Continue();
   return list;
}

// This function gets run if running in compiled mode
#ifndef __CINT__
int main(int argc, char** argv)
{
   if(argc != 4 && argc != 3 && argc != 2) {
      printf("try again (usage: %s <analysis tree file> <optional: output file> <max entries>).\n", argv[0]);
      return 0;
   }

   // We use a stopwatch so that we can watch progress
   TStopwatch w;
   w.Start();

   auto* file = new TFile(argv[1]);

   if(file == nullptr) {
      printf("Failed to open file '%s'!\n", argv[1]);
      return 1;
   }
   if(!file->IsOpen()) {
      printf("Failed to open file '%s'!\n", argv[1]);
      return 1;
   }
   printf("Sorting file:" DBLUE " %s" RESET_COLOR "\n", file->GetName());

   // Get PPG from File
   TPPG* myPPG = dynamic_cast<TPPG*>(file->Get("TPPG"));
   /*   if(myPPG == nullptr) {
        printf("Failed to find PPG information in file '%s'!\n",argv[1]);
        return 1;
        }
        */
   // Get run info from File
   TGRSIRunInfo* runInfo = dynamic_cast<TGRSIRunInfo*>(file->Get("TGRSIRunInfo"));
   if(runInfo == nullptr) {
      printf("Failed to find run information in file '%s'!\n", argv[1]);
      return 1;
   }

   TTree* tree = dynamic_cast<TTree*>(file->Get("AnalysisTree"));
   TChannel::ReadCalFromTree(tree);
   if(tree == nullptr) {
      printf("Failed to find analysis tree in file '%s'!\n", argv[1]);
      return 1;
   }
   // Get the TGRSIRunInfo from the analysis Tree.

   TList* list; // We return a list because we fill a bunch of TH1's and shove them into this list.
   TFile* outfile;
   if(argc < 3) {
      if(runInfo == nullptr) {
         printf("Could not find run info, please provide output file name\n");
         return 0;
      }
      int runnumber    = runInfo->RunNumber();
      int subrunnumber = runInfo->SubRunNumber();
      outfile          = new TFile(Form("cycle%05d_%03d.root", runnumber, subrunnumber), "recreate");
   } else {
      outfile = new TFile(argv[2], "recreate");
   }

   std::cout<<argv[0]<<": starting Analysis after "<<w.RealTime()<<" seconds"<<std::endl;
   w.Continue();
   if(argc < 4) {
      list = DescantMatrices(tree, myPPG, runInfo, 0, &w);
   } else {
      int entries = atoi(argv[3]);
      std::cout<<"Limiting processing of analysis tree to "<<entries<<" entries!"<<std::endl;
      list = DescantMatrices(tree, myPPG, runInfo, entries, &w);
   }
   if(list == nullptr) {
      std::cout<<"DescantMatrices returned TList* nullptr!\n"<<std::endl;
      return 1;
   }

   printf("Writing to File: " DYELLOW "%s" RESET_COLOR "\n", outfile->GetName());
   list->Write();
   // Write the run info into the tree as well if there is run info in the Analysis Tree
   auto* sortinfolist = new TGRSISortList;
   if(runInfo != nullptr) {
      auto* info = new TGRSISortInfo(runInfo);
      sortinfolist->AddSortInfo(info);
      sortinfolist->Write("TGRSISortList", TObject::kSingleKey);
   }

   outfile->Close();

   std::cout<<argv[0]<<" done after "<<w.RealTime()<<" seconds"<<std::endl<<std::endl;

   return 0;
}

#endif
