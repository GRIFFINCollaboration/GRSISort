//g++ MakeMatrices.C -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries -lGRSIFormat -lAnalysisTreeBuilder -lGriffin -lSceptar -lDescant -lPaces -lGRSIDetector -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml `root-config --cflags --libs` -lTreePlayer -o MakeMatrices

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
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TCanvas.h"
#include "TStopwatch.h"
#include "TMath.h"

#ifndef __CINT__ 
#include "TGriffin.h"
#include "TSceptar.h"
#include "TPaces.h"
#include "TDescant.h"
#endif

#ifdef __CINT__ 
void MakeMatrices() {
   if(!AnalysisTree) {
      printf("No analysis tree found!\n");
      return;
   }

   //coinc window = 0-20, bg window 40-60, 6000 bins from 0. to 6000. (default is 4000)
   TList *list = MakeMatrices(AnalysisTree, 0., 20., 80., 6000, 0., 6000.);

   std::string fileName = gFile->GetName();
   if(fileName.find_last_of("/") != std::string::npos) {
      fileName.insert(fileName.find_last_of("/")+1,"matrices_");
   } else {
      fileName.insert(0,"matrices_");
   }
   TFile *outfile = new TFile(file.c_str(),"create");
   list->Write();
}
#endif

std::vector<std::pair<double,int> > AngleCombinations(double binWidth = 2., double distance = 110.) {
   std::vector<std::pair<double,int> > result;

   std::vector<double> angle;
   for(int firstDet = 1; firstDet <= 16; ++firstDet) {
      for(int firstCry = 0; firstCry < 4; ++firstCry) {
         for(int secondDet = 1; secondDet <= 16; ++secondDet) {
            for(int secondCry = 0; secondCry < 4; ++secondCry) {
               if(firstDet == secondDet && firstCry == secondCry) {
                  continue;
               }
               angle.push_back(TGriffin::GetPosition(firstDet, firstCry, distance).Angle(TGriffin::GetPosition(secondDet, secondCry, distance))*180./TMath::Pi());
               if(angle.back() > 90.) {
                  angle.back() = 180. - angle.back();
               }
               //std::cout<<firstDet<<", "<<firstCry<<" - "<<secondDet<<", "<<secondCry<<": "<<angle.back()<<std::endl;
            }
         }
      }
   }

   //std::cout<<"sorting "<<angle.size()<<" angles"<<std::endl;

   //sort angles
   std::sort(angle.begin(),angle.end());

   //std::cout<<"done sorting "<<angle.size()<<" angles"<<std::endl;

   //add all angles within binWidth of each other into the same result
   size_t r;
   for(size_t a = 0; a < angle.size(); ++a) {
      //std::cout<<angle[a]<<" \t"<<result.size()<<std::endl;
      for(r = 0; r < result.size(); ++r) {
         if(TMath::Abs(angle[a] - result[r].first/result[r].second) < binWidth) {
            //std::cout<<"found another angle close to "<<result[r].first/result[r].second<<": "<<angle[a]<<std::endl;
            result[r].first += angle[a];
            (result[r].second)++;
            break;
         }
      }
      if(result.size() == 0 || r == result.size()) {
         //std::cout<<"found new angle "<<angle[a]<<" ("<<r<<"/"<<result.size()<<")"<<std::endl;
         result.push_back(std::make_pair(angle[a],1));
      }
   }

   //the angle is right now the sum of all angles, so we need to average it
   for(auto res = result.begin(); res != result.end(); res++) {
      (*res).first /= (*res).second;
   }
   
   //std::cout<<"found "<<result.size()<<" angles"<<std::endl;

   return result;
}
   
TList *MakeMatrices(TTree* tree, int coincLow = 0, int coincHigh = 10, int bg = 100, int nofBins = 4000, double low = 0., double high = 4000., long maxEntries = 0, TStopwatch* w = NULL) {
   if(w == NULL) {
      w = new TStopwatch;
      w->Start();
   }
   TList* list = new TList;

   const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

   //time spectra
   TH1F* timeDiff = new TH1F("timeDiff","#gamma-#gamma time difference;time[10 ns]",200,0.,200.); list->Add(timeDiff);
   TH1F* timeEGate = new TH1F("timeEGate","#gamma-#gamma time difference energy gate on 1808 and 1130;time[10 ns]",200,0.,200.);  list->Add(timeEGate);
   TH1F* timeModuleOne = new TH1F("timeModuleOne","#gamma-#gamma time difference for 1. grif-16;time[10 ns]",200,0.,200.); list->Add(timeModuleOne);

   TH1F* cfdDiff = new TH1F("cfdDiff","cfd difference",2000,-1000.,1000.); list->Add(cfdDiff);
   TH1F* cfdDiffEGate = new TH1F("cfdDiffEGate","cfd difference energy gate on 1808 and 1130",2000,-1000.,1000.); list->Add(cfdDiffEGate);
   TH1F* cfdDiffModuleOne = new TH1F("cfdDiffModuleOne","cfd difference for 1. grif-16",2000,-1000.,1000.); list->Add(cfdDiffModuleOne);

   TH1F* timeB = new TH1F("timeB","time difference with coincident beta;time[10 ns]",200,0.,200.); list->Add(timeB);
   TH1F* timeEGateB = new TH1F("timeEGateB","time difference energy gate on 1808 and 1130, and coincident beta;time[10 ns]",200,0.,200.);  list->Add(timeEGateB);
   TH1F* timeModuleOneB = new TH1F("timeModuleOneB","time difference for 1. grif-16 with coincident beta;time[10 ns]",200,0.,200.); list->Add(timeModuleOneB);

   TH1F* cfdDiffB = new TH1F("cfdDiffB","cfd difference with coincident beta",2000,-1000.,1000.); list->Add(cfdDiffB);
   TH1F* cfdDiffEGateB = new TH1F("cfdDiffEGateB","cfd difference energy gate on 1808 and 1130, and coincident beta",2000,-1000.,1000.); list->Add(cfdDiffEGateB);
   TH1F* cfdDiffModuleOneB = new TH1F("cfdDiffModuleOneB","cfd difference for 1. grif-16 with coincident beta",2000,-1000.,1000.); list->Add(cfdDiffModuleOneB);

   TH1F* addbackTimeDiff = new TH1F("addbackTimeDiff","time difference;time[10 ns]",200,0.,200.); list->Add(addbackTimeDiff);
   TH1F* addbackCfdDiff = new TH1F("addbackCfdDiff","cfd difference",2000,-1000.,1000.); list->Add(addbackCfdDiff);

   TH1F* addbackTimeB = new TH1F("addbackTimeB","time difference with coincident beta;time[10 ns]",200,0.,200.); list->Add(addbackTimeB);
   TH1F* addbackCfdDiffB = new TH1F("addbackCfdDiffB","cfd difference with coincident beta",2000,-1000.,1000.); list->Add(addbackCfdDiffB);

   TH1F* griffinSceptarTime = new TH1F("griffinSceptarTime","griffin-sceptar time difference;time[10 ns]",400,-200.,200.); list->Add(griffinSceptarTime);
   TH1F* pacesSceptarTime = new TH1F("pacesSceptarTime","paces-sceptar time difference;time[10 ns]",400,-200.,200.); list->Add(pacesSceptarTime);
   TH1F* descantSceptarTime = new TH1F("descantSceptarTime","descant-sceptar time difference;time[10 ns]",400,-200.,200.); list->Add(descantSceptarTime);

   TH1F* griffinSceptarCfd = new TH1F("griffinSceptarCfd","griffin-sceptar cfd difference;time[10 ns]",400,-200.,200.); list->Add(griffinSceptarCfd);
   TH1F* pacesSceptarCfd = new TH1F("pacesSceptarCfd","paces-sceptar cfd difference;time[10 ns]",400,-200.,200.); list->Add(pacesSceptarCfd);
   TH1F* descantSceptarCfd = new TH1F("descantSceptarCfd","descant-sceptar cfd difference;time[10 ns]",400,-200.,200.); list->Add(descantSceptarCfd);

   //energy vs time spectra
   TH2F* griffinSceptarEnergyVsTime = new TH2F("griffinSceptarEnergyVsTime","griffin energy vs. griffin-sceptar time difference;time[10 ns];energy[keV]",400,-200.,200.,nofBins, low, high); list->Add(griffinSceptarEnergyVsTime);

   TH2F* griffinSceptarEnergyVsCfd = new TH2F("griffinSceptarEnergyVsCfd","griffin energy vs. griffin-sceptar cfd time difference;time[10 ns];energy[keV]",2000,-1000.,1000.,nofBins, low, high); list->Add(griffinSceptarEnergyVsCfd);

   //gamma singles
   TH1F* gammaSingles = new TH1F("gammaSingles","#gamma singles;energy[keV]",nofBins, low, high); list->Add(gammaSingles);
   TH1F* gammaSinglesB = new TH1F("gammaSinglesB","#beta #gamma;energy[keV]",nofBins, low, high); list->Add(gammaSinglesB);
   TH1F* gammaSinglesSup = new TH1F("gammaSinglesSup","#brem sup gamma singles; energy[keV]",nofBins,low,high);list->Add(gammaSinglesSup);

   TH1F* bremB = new TH1F("bremB","#brem in coinc with sceptar; energy[keV]",nofBins,low,high); list->Add(bremB);list->Add(bremB);

   TH2F* singlesVsDetNum = new TH2F("singlesVsDetNum","#gamma energy vs. detector number;detector number;energy[keV]", 16, 0.5, 16.5, nofBins, low, high); list->Add(singlesVsDetNum);

   TH1F* addbackSingles = new TH1F("addbackSingles","#gamma addback singles;energy[keV]",nofBins, low, high); list->Add(addbackSingles);
   TH1F* addbackSinglesB = new TH1F("addbackSinglesB","#beta #gamma addback;energy[keV]",nofBins, low, high); list->Add(addbackSinglesB);

   TH1F* addbackCloverSingles = new TH1F("addbackCloverSingles","#gamma clover addback singles;energy[keV]",nofBins, low, high); list->Add(addbackCloverSingles);
   TH1F* addbackCloverSinglesB = new TH1F("addbackCloverSinglesB","#beta #gamma clover addback;energy[keV]",nofBins, low, high); list->Add(addbackCloverSinglesB);

   TH2F* addbackVsDetNum = new TH2F("addbackVsDetNum","addback energy vs. detector number;detector number;energy[keV]", 16, 0.5, 16.5, nofBins, low, high); list->Add(addbackVsDetNum);
   TH2F* addbackCloverVsDetNum = new TH2F("addbackCloverVsDetNum","clover addback energy vs. detector number;detector number;energy[keV]", 16, 0.5, 16.5, nofBins, low, high); list->Add(addbackCloverVsDetNum);

   //gamma-gamma matrices
   TH2F* matrix = new TH2F("matrix","#gamma-#gamma matrix",nofBins, low, high,nofBins, low, high); list->Add(matrix);
   TH2F* matrix_coinc = new TH2F("matrix_coinc",Form("#gamma-#gamma matrix, coincident within %d - %d [10 ns]", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(matrix_coinc);
   TH2F* matrix_bg = new TH2F("matrix_bg",Form("#gamma-#gamma matrix, background within %d - %d [ 10 ns]", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(matrix_bg);

   TH2F* matrixB = new TH2F("matrixB","#gamma-#gamma matrix with coincident beta",nofBins, low, high,nofBins, low, high); list->Add(matrixB);
   TH2F* matrix_coincB = new TH2F("matrix_coincB",Form("#gamma-#gamma matrix, coincident within %d - %d [10 ns], and coincident beta", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(matrix_coincB);
   TH2F* matrix_bgB = new TH2F("matrix_bgB",Form("#gamma-#gamma matrix, background within %d - %d [ 10 ns], and coincident beta", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(matrix_bgB);

   TH2F* addbackMatrix = new TH2F("addbackMatrix","#gamma-#gamma matrix, addback",nofBins, low, high,nofBins, low, high); list->Add(addbackMatrix);
   TH2F* addbackMatrix_coinc = new TH2F("addbackMatrix_coinc",Form("#gamma-#gamma matrix, addback, coincident within %d - %d [10 ns]", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackMatrix_coinc);
   TH2F* addbackMatrix_bg = new TH2F("addbackMatrix_bg",Form("#gamma-#gamma matrix, addback, background within %d - %d [ 10 ns]", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackMatrix_bg);

   TH2F* addbackMatrixB = new TH2F("addbackMatrixB","#gamma-#gamma matrix, addback, and coincident #beta",nofBins, low, high,nofBins, low, high); list->Add(addbackMatrixB);
   TH2F* addbackMatrix_coincB = new TH2F("addbackMatrix_coincB",Form("#gamma-#gamma matrix, addback, coincident within %d - %d [10 ns], and coincident #beta", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackMatrix_coincB);
   TH2F* addbackMatrix_bgB = new TH2F("addbackMatrix_bgB",Form("#gamma-#gamma matrix, addback, background within %d - %d [ 10 ns], and coincident #beta", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackMatrix_bgB);

   TH2F* addbackCloverMatrix = new TH2F("addbackCloverMatrix","#gamma-#gamma matrix, clover addback",nofBins, low, high,nofBins, low, high); list->Add(addbackCloverMatrix);
   TH2F* addbackCloverMatrix_coinc = new TH2F("addbackCloverMatrix_coinc",Form("#gamma-#gamma matrix, clover addback, coincident within %d - %d [10 ns]", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackCloverMatrix_coinc);
   TH2F* addbackCloverMatrix_bg = new TH2F("addbackCloverMatrix_bg",Form("#gamma-#gamma matrix, clover addback, background within %d - %d [ 10 ns]", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackCloverMatrix_bg);

   TH2F* addbackCloverMatrixB = new TH2F("addbackCloverMatrixB","#gamma-#gamma matrix, clover addback, and coincident #beta",nofBins, low, high,nofBins, low, high); list->Add(addbackCloverMatrixB);
   TH2F* addbackCloverMatrix_coincB = new TH2F("addbackCloverMatrix_coincB",Form("#gamma-#gamma matrix, clover addback, coincident within %d - %d [10 ns], and coincident #beta", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackCloverMatrix_coincB);
   TH2F* addbackCloverMatrix_bgB = new TH2F("addbackCloverMatrix_bgB",Form("#gamma-#gamma matrix, clover addback, background within %d - %d [ 10 ns], and coincident #beta", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackCloverMatrix_bgB);

   TH2F* addbackMatrixClose = new TH2F("addbackMatrixClose","#gamma-#gamma matrix, addback, angle between detectors < 90^{o}",nofBins, low, high,nofBins, low, high); list->Add(addbackMatrixClose);
   TH2F* addbackMatrixClose_coinc = new TH2F("addbackMatrixClose_coinc",Form("#gamma-#gamma matrix, addback, angle between detectors < 90^{o}, coincident within %d - %d [10 ns]", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackMatrixClose_coinc);
   TH2F* addbackMatrixClose_bg = new TH2F("addbackMatrixClose_bg",Form("#gamma-#gamma matrix, addback, angle between detectors < 90^{o}, background within %d - %d [ 10 ns]", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackMatrixClose_bg);

   TH2F* addbackMatrixCloseB = new TH2F("addbackMatrixCloseB","#gamma-#gamma matrix, addback, angle between detectors < 90^{o}, and coincident #beta",nofBins, low, high,nofBins, low, high); list->Add(addbackMatrixCloseB);
   TH2F* addbackMatrixClose_coincB = new TH2F("addbackMatrixClose_coincB",Form("#gamma-#gamma matrix, addback, angle between detectors < 90^{o}, coincident within %d - %d [10 ns], and coincident #beta", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackMatrixClose_coincB);
   TH2F* addbackMatrixClose_bgB = new TH2F("addbackMatrixClose_bgB",Form("#gamma-#gamma matrix, addback, angle between detectors < 90^{o}, background within %d - %d [ 10 ns], and coincident #beta", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackMatrixClose_bgB);

   TH2F* addbackCloverMatrixClose = new TH2F("addbackCloverMatrixClose","#gamma-#gamma matrix, clover addback, angle between detectors < 90^{o}",nofBins, low, high,nofBins, low, high); list->Add(addbackCloverMatrixClose);
   TH2F* addbackCloverMatrixClose_coinc = new TH2F("addbackCloverMatrixClose_coinc",Form("#gamma-#gamma matrix, clover addback, angle between detectors < 90^{o}, coincident within %d - %d [10 ns]", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackCloverMatrixClose_coinc);
   TH2F* addbackCloverMatrixClose_bg = new TH2F("addbackCloverMatrixClose_bg",Form("#gamma-#gamma matrix, clover addback, angle between detectors < 90^{o}, background within %d - %d [ 10 ns]", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackCloverMatrixClose_bg);

   TH2F* addbackCloverMatrixCloseB = new TH2F("addbackCloverMatrixCloseB","#gamma-#gamma matrix, clover addback, angle between detectors < 90^{o}, and coincident #beta",nofBins, low, high,nofBins, low, high); list->Add(addbackCloverMatrixCloseB);
   TH2F* addbackCloverMatrixClose_coincB = new TH2F("addbackCloverMatrixClose_coincB",Form("#gamma-#gamma matrix, clover addback, angle between detectors < 90^{o}, coincident within %d - %d [10 ns], and coincident #beta", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackCloverMatrixClose_coincB);
   TH2F* addbackCloverMatrixClose_bgB = new TH2F("addbackCloverMatrixClose_bgB",Form("#gamma-#gamma matrix, clover addback, angle between detectors < 90^{o}, background within %d - %d [ 10 ns], and coincident #beta", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackCloverMatrixClose_bgB);

   //multiplicities
   TH1F* grifMult    = new TH1F("grifMult",        "Griffin multiplicity in built event with coincident beta",65,0.,65.);                                             list->Add(grifMult);
   TH1F* grifMultCut = new TH1F("grifMultCut",Form("Griffin multiplicity in built event with #Delta t = %d - %d, and coincident beta",coincLow,coincHigh),65,0.,65.); list->Add(grifMultCut);
   
   TH1F* scepMult    = new TH1F("scepMult",        "Sceptar multiplicity in built event with coincident beta",65,0.,65.);                                             list->Add(scepMult);
   TH1F* scepMultCut = new TH1F("scepMultCut",Form("Sceptar multiplicity in built event with #Delta t = %d - %d, and coincident beta",coincLow,coincHigh),65,0.,65.); list->Add(scepMultCut);
 
   TH1F* grifMultB    = new TH1F("grifMultB",        "Griffin multiplicity in built event with coincident beta",65,0.,65.);                                             list->Add(grifMultB);
   TH1F* grifMultCutB = new TH1F("grifMultCutB",Form("Griffin multiplicity in built event with #Delta t = %d - %d, and coincident beta",coincLow,coincHigh),65,0.,65.); list->Add(grifMultCutB);
   
   TH1F* scepMultB    = new TH1F("scepMultB",        "Sceptar multiplicity in built event with coincident beta",65,0.,65.);                                             list->Add(scepMultB);
   TH1F* scepMultCutB = new TH1F("scepMultCutB",Form("Sceptar multiplicity in built event with #Delta t = %d - %d, and coincident beta",coincLow,coincHigh),65,0.,65.); list->Add(scepMultCutB);
 
   //diagnostics
   TH2F* eVsAngle1460 = new TH2F("eVsAngle1460","#gamma addback energy vs. angle between detectors", 180, 0., 180., 1500, 0., 1500.); list->Add(eVsAngle1460);
   TH2F* eVsTimeNeighbours = new TH2F("eVsTimeNeighbours","energy vs. time for neighbouring crystals",400, -200., 200., nofBins, low, high); list->Add(eVsTimeNeighbours);

   //hit pattern
   TH2F* griffinHits = new TH2F("griffinHits","#gamma-#gamma hitpattern",70,0,70,70,0,70); list->Add(griffinHits);
   TH2F* griffinHitsB = new TH2F("griffinHitsB","#gamma-#gamma hitpattern, coincident #beta",70,0,70,70,0,70); list->Add(griffinHitsB);

   TH2F* addbackHits = new TH2F("addbackHits","#gamma-#gamma hitpattern, addback",70,0,70,70,0,70); list->Add(addbackHits);
   TH2F* addbackHitsB = new TH2F("addbackHitsB","#gamma-#gamma hitpattern, addback, coincident #beta",70,0,70,70,0,70); list->Add(addbackHitsB);

   TH2F* addbackCloverHits = new TH2F("addbackCloverHits","#gamma-#gamma hitpattern, clover addback",70,0,70,70,0,70); list->Add(addbackCloverHits);
   TH2F* addbackCloverHitsB = new TH2F("addbackCloverHitsB","#gamma-#gamma hitpattern, clover addback, coincident #beta",70,0,70,70,0,70); list->Add(addbackCloverHitsB);

   //TH2F* sceptarHits = new TH2F("sceptarHits","sceptar-sceptar hitpattern",25,0,25,25,0,25); list->Add(sceptarHits);
   //TH2F* pacesHits = new TH2F("pacesHits","paces-paces hitpattern",10,0,10,10,0,10); list->Add(pacesHits);
   //TH2F* descantHits = new TH2F("descantHits","descant-descant hitpattern",70,0,70,70,0,70); list->Add(descantHits);

   TH2F* griffinSceptarHits = new TH2F("griffinSceptarHits","GRIFFIN hits vs. SCEPTAR hits", 25,0,25,70,0,70); list->Add(griffinSceptarHits);
   TH2F* addbackSceptarHits = new TH2F("addbackSceptarHits","GRIFFIN hits vs. SCEPTAR hits", 25,0,25,70,0,70); list->Add(addbackSceptarHits);

   //descant waveform - not added to list, this only happens if we find any waveforms
   TH2F* rawWaveform = new TH2F("rawWaveform","raw DESCANT waveforms;time [ns];pulse height [ADC channels]",120,0.,120.,4096,0,4096);
   TH2F* intNormWaveform = new TH2F("intNormWaveform","DESCANT waveforms normalized by integral;time [ns];pulse height [ADC channels]",120,0.,120.,1200,-0.1,1.1);
   TH2F* peakNormWaveform = new TH2F("peakNormWaveform","DESCANT waveforms normalized by peak height;time [ns];pulse height [ADC channels]",120,0.,120.,1200,-0.1,1.1);

   TH2F* rawWaveformCfd = new TH2F("rawWaveformCfd","raw DESCANT waveforms, shifted by cfd time;time [ns];pulse height [ADC channels]",120,-20.,100.,4096,0,4096);
   TH2F* intNormWaveformCfd = new TH2F("intNormWaveformCfd","DESCANT waveforms normalized by integral, shifted by cfd time;time [ns];pulse height [ADC channels]",120,-20.,100.,1200,-0.1,1.1);
   TH2F* peakNormWaveformCfd = new TH2F("peakNormWaveformCfd","DESCANT waveforms normalized by peak height, shifted by cfd time;time [ns];pulse height [ADC channels]",120,-20.,100.,1200,-0.1,1.1);

   TH2F* rawWaveformPsd = new TH2F("rawWaveformPsd","raw DESCANT waveforms, shifted by psd time;time [ns];pulse height [ADC channels]",120,-40.,80.,4096,0,4096);
   TH2F* intNormWaveformPsd = new TH2F("intNormWaveformPsd","DESCANT waveforms normalized by integral, shifted by psd time;time [ns];pulse height [ADC channels]",120,-40.,80.,1200,-0.1,1.1);
   TH2F* peakNormWaveformPsd = new TH2F("peakNormWaveformPsd","DESCANT waveforms normalized by peak height, shifted by psd time;time [ns];pulse height [ADC channels]",120,-40.,80.,1200,-0.1,1.1);

   TH2F* timevsenergy = new TH2F("timevsenergy","time vs #gamma-ray energy",4000,0,4000,6000,0,6000); list->Add(timevsenergy); 
   
   //angular correlation cube
   double angleWidth = 1.;
   TH3F* angCorr = new TH3F("angCorr","angular correlation cube;energy [keV];energy [keV];angle [^{o}]", nofBins/5, low, high, nofBins/5, low, high, 90*angleWidth, 0., 90.); list->Add(angCorr);
   TH3F* angCorrAddback = new TH3F("angCorrAddback","angular correlation cube;energy [keV];energy [keV];angle [^{o}]", nofBins/5, low, high, nofBins/5, low, high, 90*angleWidth, 0., 90.); list->Add(angCorrAddback);

   TH3F* angCorr_coinc = new TH3F("angCorr_coinc",Form("angular correlation cube, within %d - %d [10 ns];energy [keV];energy [keV];angle [^{o}]", coincLow, coincHigh), nofBins/5, low, high, nofBins/5, low, high, 90*angleWidth, 0., 90.); list->Add(angCorr_coinc);
   TH3F* angCorrAddback_coinc = new TH3F("angCorrAddback_coinc",Form("angular correlation cube, within %d - %d [10 ns];energy [keV];energy [keV];angle [^{o}]", coincLow, coincHigh), nofBins/5, low, high, nofBins/5, low, high, 90*angleWidth, 0., 90.); list->Add(angCorrAddback_coinc);

   TH3F* angCorr_bg = new TH3F("angCorr_bg",Form("angular correlation cube, background within %d - %d [ 10 ns];energy [keV];energy [keV];angle [^{o}]", bg+coincLow, bg+coincHigh), nofBins/5, low, high, nofBins/5, low, high, 90*angleWidth, 0., 90.); list->Add(angCorr_bg);
   TH3F* angCorrAddback_bg = new TH3F("angCorrAddback_bg",Form("angular correlation cube, background within %d - %d [ 10 ns];energy [keV];energy [keV];angle [^{o}]", bg+coincLow, bg+coincHigh), nofBins/5, low, high, nofBins/5, low, high, 90*angleWidth, 0., 90.); list->Add(angCorrAddback_bg);

   std::vector<std::pair<double,int> > angleCombinations = AngleCombinations(angleWidth, 110.);
   std::cout<<"got "<<angleCombinations.size()<<" angles"<<std::endl;
   for(auto ang = angleCombinations.begin(); ang != angleCombinations.end(); ang++) {
      std::cout<<(*ang).first<<" degree: "<<(*ang).second<<" combinations"<<std::endl;
   }

   //set up branches
   TGriffin* grif = 0;
   TSceptar* scep = 0;
   TPaces*   pace = 0;
   TDescant* desc = 0;
   tree->SetBranchAddress("TGriffin", &grif);

   bool gotSceptar;
   if(tree->FindBranch("TSceptar") == 0) {
      gotSceptar = false;
   } else {
      tree->SetBranchAddress("TSceptar", &scep);
      gotSceptar = true;
   }

   bool gotPaces;
   if(tree->FindBranch("TPaces") == 0) {
      gotPaces = false;
   } else {
      tree->SetBranchAddress("TPaces", &pace);
      gotPaces = true;
   }

   bool gotDescant;
   if(tree->FindBranch("TDescant") == 0) {
      gotDescant = false;
   } else {
      tree->SetBranchAddress("TDescant", &desc);
      gotDescant = true;
   }


   //tree->LoadBaskets(MEM_SIZE);   

   long entries = tree->GetEntries();

   int one;
   int two;

   std::cout<<std::fixed<<std::setprecision(1);
   int entry;
   size_t angIndex;
   if(maxEntries == 0 || maxEntries > tree->GetEntries()) {
      maxEntries = tree->GetEntries();
   }
   for(entry = 0; entry < maxEntries; ++entry) {
      tree->GetEntry(entry);
      //std::cout<<entry<<": got "<<grif->GetMultiplicity()<<" griffin detectors"<<std::flush;
      //if(gotSceptar) {
      //   std::cout<<", "<<scep->GetMultiplicity()<<" sceptar detectors"<<std::flush;
      //}
      //if(gotPaces) {
      //   std::cout<<", "<<pace->GetMultiplicity()<<" paces detectors"<<std::flush;
      //}
      //if(gotDescant) {
      //   std::cout<<", "<<desc->GetMultiplicity()<<" descant detectors"<<std::flush;
      //}
      //std::cout<<std::endl;
      //griffin multiplicites
      grifMult->Fill(grif->GetMultiplicity());
      int coincGammaMult = 0;
      for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
         for(two = one+1; two < (int) grif->GetMultiplicity(); ++two) {
            if(coincLow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < coincHigh) {
               break;
            }
         }
         if(two < grif->GetMultiplicity()) {
            ++coincGammaMult;
         }
      }
      grifMultCut->Fill(coincGammaMult);
      //sceptar multiplicities
      if(gotSceptar) {
         scepMult->Fill(scep->GetMultiplicity());
         int coincBetaMult = 0;
         for(one = 0; one < (int) scep->GetMultiplicity(); ++one) {
            for(two = one+1; two < (int) scep->GetMultiplicity(); ++two) {
               if(coincLow <= TMath::Abs(scep->GetSceptarHit(two)->GetTime()-scep->GetSceptarHit(one)->GetTime()) && TMath::Abs(scep->GetSceptarHit(two)->GetTime()-scep->GetSceptarHit(one)->GetTime()) < coincHigh) {
                  break;
               }
            }
            if(two < scep->GetMultiplicity()) {
               ++coincBetaMult;
            }
         }
         scepMultCut->Fill(coincBetaMult);
      }
      //sceptar coincident multiplicities
      if(gotSceptar && scep->GetMultiplicity() >= 1) {
         grifMultB->Fill(grif->GetMultiplicity());
         int coincGammaMult = 0;
         for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
            for(two = one+1; two < (int) grif->GetMultiplicity(); ++two) {
               if(coincLow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < coincHigh) {
                  break;
               }
            }
            if(two < grif->GetMultiplicity()) {
               ++coincGammaMult;
            }
         }
         grifMultCutB->Fill(coincGammaMult);
         scepMultB->Fill(scep->GetMultiplicity());
         int coincBetaMult = 0;
         for(one = 0; one < (int) scep->GetMultiplicity(); ++one) {
            for(two = one+1; two < (int) scep->GetMultiplicity(); ++two) {
               if(coincLow <= TMath::Abs(scep->GetSceptarHit(two)->GetTime()-scep->GetSceptarHit(one)->GetTime()) && TMath::Abs(scep->GetSceptarHit(two)->GetTime()-scep->GetSceptarHit(one)->GetTime()) < coincHigh) {
                  break;
               }
            }
            if(two < scep->GetMultiplicity()) {
               ++coincBetaMult;
            }
         }
         scepMultCutB->Fill(scep->GetMultiplicity());
      }

      //loop over all gamma's in two loops
      for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
         gammaSingles->Fill(grif->GetGriffinHit(one)->GetEnergyLow());
         timevsenergy->Fill(grif->GetGriffinHit(one)->GetEnergyLow(),grif->GetGriffinHit(one)->GetTime()/1.0e8);
         singlesVsDetNum->Fill(grif->GetGriffinHit(one)->GetDetectorNumber(),grif->GetGriffinHit(one)->GetEnergyLow());
         //gamma-gamma spectra
         for(two = 0; two < (int) grif->GetMultiplicity(); ++two) {
            if(two == one) {
               continue;
            }
            if((grif->GetGriffinHit(one)->GetPosition()-grif->GetGriffinHit(two)->GetPosition()).Mag()<105) {
               eVsTimeNeighbours->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime(),grif->GetGriffinHit(one)->GetEnergyLow());
            }
            griffinHits->Fill(grif->GetGriffinHit(one)->GetArrayNumber(),grif->GetGriffinHit(two)->GetArrayNumber());
            double ang = grif->GetGriffinHit(one)->GetPosition().Angle(grif->GetGriffinHit(two)->GetPosition())*180./TMath::Pi();
            if(ang > 90.) {
               ang = 180. - ang;
            }
            for(angIndex = 0; angIndex < angleCombinations.size(); ++angIndex) {
               if(TMath::Abs(ang - angleCombinations[angIndex].first) < angleWidth) {
                  break;
               }
            }
            if(angIndex < angleCombinations.size()) {
               //std::cout<<ang<<" = "<<angleCombinations[angIndex].first<<" => "<<angleCombinations[angIndex].second<<std::endl;
               angCorr->Fill(grif->GetGriffinHit(one)->GetEnergyLow(),grif->GetGriffinHit(two)->GetEnergyLow(),angleCombinations[angIndex].first,1./angleCombinations[angIndex].second);
            } else {
               std::cout<<"Error, didn't find any matching angle for "<<ang<<std::endl;
            }
            if(two > one) {
               timeDiff->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime());
               cfdDiff->Fill(grif->GetGriffinHit(two)->GetCfd()-grif->GetGriffinHit(one)->GetCfd());
               if((1125<grif->GetGriffinHit(one)->GetEnergyLow() && grif->GetGriffinHit(one)->GetEnergyLow()<1135 && 1805<grif->GetGriffinHit(two)->GetEnergyLow() && grif->GetGriffinHit(two)->GetEnergyLow()<1815) ||
                  (1125<grif->GetGriffinHit(two)->GetEnergyLow() && grif->GetGriffinHit(two)->GetEnergyLow()<1135 && 1805<grif->GetGriffinHit(one)->GetEnergyLow() && grif->GetGriffinHit(one)->GetEnergyLow()<1815)) {
                  timeEGate->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime());
                  cfdDiffEGate->Fill(grif->GetGriffinHit(two)->GetCfd()-grif->GetGriffinHit(one)->GetCfd());
               }
               if(grif->GetGriffinHit(one)->GetAddress()<16 && grif->GetGriffinHit(two)->GetAddress()<16) {
                  timeModuleOne->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime());
                  cfdDiffModuleOne->Fill(grif->GetGriffinHit(two)->GetCfd()-grif->GetGriffinHit(one)->GetCfd());
               }
            }
            if(!gotSceptar && (grif->GetGriffinHit(one)->GetAddress()%16 == 15 || grif->GetGriffinHit(two)->GetAddress()%16 == 15)) {
               continue;
            }
            matrix->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
            if(coincLow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < coincHigh) {
               matrix_coinc->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
               if(angIndex < angleCombinations.size()) {
                  angCorr_coinc->Fill(grif->GetGriffinHit(one)->GetEnergyLow(),grif->GetGriffinHit(two)->GetEnergyLow(),angleCombinations[angIndex].first,1./angleCombinations[angIndex].second);
               }
            } else if((bg+coincLow) <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < (bg+coincHigh)) {
               matrix_bg->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
               if(angIndex < angleCombinations.size()) {
                  angCorr_bg->Fill(grif->GetGriffinHit(one)->GetEnergyLow(),grif->GetGriffinHit(two)->GetEnergyLow(),angleCombinations[angIndex].first,1./angleCombinations[angIndex].second);
               }
            }
         }
      }

      //all beta coincident spectra
      if(gotSceptar && scep->GetMultiplicity() >= 1) {
         for(int b = 0; b < scep->GetMultiplicity(); ++b) {
            for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
               //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetGriffinHit(one)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetGriffinHit(one)->GetTime()) > coincHigh) {
                  //continue;
               //}
               griffinSceptarTime->Fill(grif->GetGriffinHit(one)->GetTime()-scep->GetSceptarHit(b)->GetTime());
               griffinSceptarCfd->Fill(grif->GetGriffinHit(one)->GetTime()-scep->GetSceptarHit(b)->GetTime());
               griffinSceptarEnergyVsTime->Fill(grif->GetGriffinHit(one)->GetTime()-scep->GetSceptarHit(b)->GetTime(),grif->GetGriffinHit(one)->GetEnergyLow());
               griffinSceptarEnergyVsCfd->Fill(grif->GetGriffinHit(one)->GetTime()-scep->GetSceptarHit(b)->GetTime(),grif->GetGriffinHit(one)->GetEnergyLow());
               //beta-gamma
               gammaSinglesB->Fill(grif->GetGriffinHit(one)->GetEnergyLow());
               
               griffinSceptarHits->Fill(scep->GetSceptarHit(b)->GetDetectorNumber(), grif->GetGriffinHit(one)->GetArrayNumber());

               for(two = 0; two < (int) grif->GetMultiplicity(); ++two) {
                  if(two == one) {
                     continue;
                  }
                  //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetGriffinHit(two)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetGriffinHit(two)->GetTime()) > coincHigh) {
                     //continue;
                  //}
                  griffinHitsB->Fill(grif->GetGriffinHit(one)->GetArrayNumber(),grif->GetGriffinHit(two)->GetArrayNumber());
                  if(two > one) {
                     timeB->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime());
                     cfdDiffB->Fill(grif->GetGriffinHit(two)->GetCfd()-grif->GetGriffinHit(one)->GetCfd());
                     if((1125<grif->GetGriffinHit(one)->GetEnergyLow() && grif->GetGriffinHit(one)->GetEnergyLow()<1135 && 1805<grif->GetGriffinHit(two)->GetEnergyLow() && grif->GetGriffinHit(two)->GetEnergyLow()<1815) ||
                        (1125<grif->GetGriffinHit(two)->GetEnergyLow() && grif->GetGriffinHit(two)->GetEnergyLow()<1135 && 1805<grif->GetGriffinHit(one)->GetEnergyLow() && grif->GetGriffinHit(one)->GetEnergyLow()<1815)) {
                        timeEGateB->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime());
                        cfdDiffEGateB->Fill(grif->GetGriffinHit(two)->GetCfd()-grif->GetGriffinHit(one)->GetCfd());
                     }
                     if(grif->GetGriffinHit(one)->GetAddress()<16 && grif->GetGriffinHit(two)->GetAddress()<16) {
                        timeModuleOneB->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime());
                        cfdDiffModuleOneB->Fill(grif->GetGriffinHit(two)->GetCfd()-grif->GetGriffinHit(one)->GetCfd());
                     }
                  }
                  if(!gotSceptar && (grif->GetGriffinHit(one)->GetAddress()%16 == 15 || grif->GetGriffinHit(two)->GetAddress()%16 == 15)) {
                     continue;
                  }
                  matrixB->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
                  if(coincLow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < coincHigh) {
                     matrix_coincB->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
                  } else if((bg+coincLow) <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < (bg+coincHigh)) {
                     matrix_bgB->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
                  }
               }
            }
            for(one = 0; gotPaces && one < (int) pace->GetMultiplicity(); ++one) {
               pacesSceptarTime->Fill(pace->GetPacesHit(one)->GetTime()-scep->GetSceptarHit(b)->GetTime());
            }
            for(one = 0; gotDescant && one < (int) desc->GetMultiplicity(); ++one) {
               descantSceptarTime->Fill(desc->GetDescantHit(one)->GetTime()-scep->GetSceptarHit(b)->GetTime());
            }
         }
      }

      //descant spectra
      if(gotDescant && desc->GetMultiplicity() >= 1) {
         for(int d = 0; d < desc->GetMultiplicity(); ++d) {
            //std::cout<<std::endl<<entry<<": trying to get descant waveform "<<d<<std::endl;
            if(desc->GetDescantHit(d)->GetWaveform().size() == 0) {
               continue;
            }
            //std::cout<<"size of descant waveform "<<desc->GetDescantHit(d)->GetWaveform().size()<<std::endl;
            std::vector<Short_t> waveform = desc->GetDescantHit(d)->GetWaveform();
            double offset = 0.;
            double max = 0.;
            for(size_t i = 0; i < waveform.size(); ++i) {
               if(i < 10)
                  offset += waveform[i];
               if(max < waveform[i]) {
                  max = waveform[i];
               }
            }
            offset /= 10.;
            max -= offset;
            for(size_t i = 0; i < waveform.size(); ++i) {
               rawWaveform->Fill(i,waveform[i]);
               intNormWaveform->Fill(i,(waveform[i]-offset)/desc->GetDescantHit(d)->GetCharge());
               peakNormWaveform->Fill(i,(waveform[i]-offset)/max);
               rawWaveformCfd->Fill(i - desc->GetDescantHit(d)->GetCfd()/256.,waveform[i]);
               intNormWaveformCfd->Fill(i - desc->GetDescantHit(d)->GetCfd()/256.,(waveform[i]-offset)/desc->GetDescantHit(d)->GetCharge());
               peakNormWaveformCfd->Fill(i - desc->GetDescantHit(d)->GetCfd()/256.,(waveform[i]-offset)/max);
               rawWaveformPsd->Fill(i - desc->GetDescantHit(d)->GetPsd()/256.,waveform[i]);
               intNormWaveformPsd->Fill(i - desc->GetDescantHit(d)->GetPsd()/256.,(waveform[i]-offset)/desc->GetDescantHit(d)->GetCharge());
               peakNormWaveformPsd->Fill(i - desc->GetDescantHit(d)->GetPsd()/256.,(waveform[i]-offset)/max);
            }
            //std::cout<<"done with descant waveform "<<d<<" of size "<<waveform.size()<<std::endl;
         }
      }

      //addback spectra
      //addback timing spectra
      //gamma-gamma spectra
      for(one = 0; one < (int) grif->GetAddBackMultiplicity(); ++one) {
         addbackSingles->Fill(grif->GetAddBackHit(one)->GetEnergyLow());
         addbackVsDetNum->Fill(grif->GetAddBackHit(one)->GetDetectorNumber(),grif->GetAddBackHit(one)->GetEnergyLow());
         for(two = 0; two < (int) grif->GetAddBackMultiplicity(); ++two) {
            if(two == one) {
               continue;
            }
            if(1458. < grif->GetAddBackHit(one)->GetEnergyLow()+grif->GetAddBackHit(two)->GetEnergyLow() && grif->GetAddBackHit(one)->GetEnergyLow()+grif->GetAddBackHit(two)->GetEnergyLow() < 1465.) {
               eVsAngle1460->Fill(grif->GetAddBackHit(one)->GetPosition().Angle(grif->GetAddBackHit(two)->GetPosition())*180./TMath::Pi(), grif->GetAddBackHit(one)->GetEnergyLow());
            }
            addbackHits->Fill(grif->GetAddBackHit(one)->GetArrayNumber(),grif->GetAddBackHit(two)->GetArrayNumber());
            double ang = grif->GetAddBackHit(one)->GetPosition().Angle(grif->GetAddBackHit(two)->GetPosition())*180./TMath::Pi();
            if(ang > 90.) {
               ang = 180. - ang;
            }
            for(angIndex = 0; angIndex < angleCombinations.size(); ++angIndex) {
               if(TMath::Abs(ang - angleCombinations[angIndex].first) < angleWidth) {
                  break;
               }
            }
            if(angIndex < angleCombinations.size()) {
               angCorrAddback->Fill(grif->GetGriffinHit(one)->GetEnergyLow(),grif->GetGriffinHit(two)->GetEnergyLow(),angleCombinations[angIndex].first,1./angleCombinations[angIndex].second);
            } else {
               std::cout<<"Error, didn't find any matching angle for "<<ang<<std::endl;
            }
            if(two > one) {
               addbackTimeDiff->Fill(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime());
               addbackCfdDiff->Fill(grif->GetAddBackHit(two)->GetCfd()-grif->GetAddBackHit(one)->GetCfd());
            }
            if(!gotSceptar && (grif->GetAddBackHit(one)->GetAddress()%16 == 15 || grif->GetAddBackHit(two)->GetAddress()%16 == 15)) {
               continue;
            }
            addbackMatrix->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
            if(coincLow <= TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) < coincHigh) {
               addbackMatrix_coinc->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
               if(angIndex < angleCombinations.size()) {
                  angCorrAddback_coinc->Fill(grif->GetGriffinHit(one)->GetEnergyLow(),grif->GetGriffinHit(two)->GetEnergyLow(),angleCombinations[angIndex].first,1./angleCombinations[angIndex].second);
               }
            } else if((bg+coincLow) <= TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) < (bg+coincHigh)) {
               addbackMatrix_bg->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
               if(angIndex < angleCombinations.size()) {
                  angCorrAddback_bg->Fill(grif->GetGriffinHit(one)->GetEnergyLow(),grif->GetGriffinHit(two)->GetEnergyLow(),angleCombinations[angIndex].first,1./angleCombinations[angIndex].second);
               }
            }
            if(grif->GetAddBackHit(one)->GetPosition().Angle(grif->GetAddBackHit(two)->GetPosition()) < TMath::Pi()/2.) {
               addbackMatrixClose->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
               if(coincLow <= TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) < coincHigh) {
                  addbackMatrixClose_coinc->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
               } else if((bg+coincLow) <= TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) < (bg+coincHigh)) {
                  addbackMatrixClose_bg->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
               }
            }
         }
      }
      //addback coincident with beta
      if(gotSceptar && scep->GetMultiplicity() >= 1) {
         for(int b = 0; b < scep->GetMultiplicity(); ++b) {
            for(one = 0; one < (int) grif->GetAddBackMultiplicity(); ++one) {
               //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackHit(one)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackHit(one)->GetTime()) > coincHigh) {
                  //continue;
               //}
               addbackSinglesB->Fill(grif->GetAddBackHit(one)->GetEnergyLow());
               for(two = 0; two < (int) grif->GetAddBackMultiplicity(); ++two) {
                  if(two == one) {
                     continue;
                  }
                  //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackHit(two)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackHit(two)->GetTime()) > coincHigh) {
                     //continue;
                  //}
                  addbackHitsB->Fill(grif->GetAddBackHit(one)->GetArrayNumber(),grif->GetAddBackHit(two)->GetArrayNumber());
                  addbackSceptarHits->Fill(scep->GetSceptarHit(b)->GetDetectorNumber(), grif->GetAddBackHit(one)->GetArrayNumber());
                  if(two > one) {
                     addbackTimeB->Fill(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime());
                     addbackCfdDiffB->Fill(grif->GetAddBackHit(two)->GetCfd()-grif->GetAddBackHit(one)->GetCfd());
                  }
                  if(!gotSceptar && (grif->GetAddBackHit(one)->GetAddress()%16 == 15 || grif->GetAddBackHit(two)->GetAddress()%16 == 15)) {
                     continue;
                  }
                  addbackMatrixB->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
                  if(coincLow <= TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) < coincHigh) {
                     addbackMatrix_coincB->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
                  } else if((bg+coincLow) <= TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) < (bg+coincHigh)) {
                     addbackMatrix_bgB->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
                  }
                  if(grif->GetAddBackHit(one)->GetPosition().Angle(grif->GetAddBackHit(two)->GetPosition()) < TMath::Pi()/2.) {
                     addbackMatrixCloseB->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
                     if(coincLow <= TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) < coincHigh) {
                        addbackMatrixClose_coincB->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
                     } else if((bg+coincLow) <= TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) < (bg+coincHigh)) {
                        addbackMatrixClose_bgB->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
                     }
                  }
               }
            }
         }
      }

      //clover addback spectra
      //clover addback timing spectra
      //gamma-gamma spectra
      for(one = 0; one < (int) grif->GetAddBackCloverMultiplicity(); ++one) {
         addbackCloverSingles->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow());
         addbackCloverVsDetNum->Fill(grif->GetAddBackCloverHit(one)->GetDetectorNumber(),grif->GetAddBackCloverHit(one)->GetEnergyLow());
         for(two = 0; two < (int) grif->GetAddBackCloverMultiplicity(); ++two) {
            if(two == one) {
               continue;
            }
            addbackCloverHits->Fill(grif->GetAddBackCloverHit(one)->GetArrayNumber(),grif->GetAddBackCloverHit(two)->GetArrayNumber());
            addbackCloverMatrix->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
            if(coincLow <= TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) < coincHigh) {
               addbackCloverMatrix_coinc->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
            } else if((bg+coincLow) <= TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) < (bg+coincHigh)) {
               addbackCloverMatrix_bg->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
            }
            if(grif->GetAddBackCloverHit(one)->GetPosition().Angle(grif->GetAddBackCloverHit(two)->GetPosition()) < TMath::Pi()/2.) {
               addbackCloverMatrixClose->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
               if(coincLow <= TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) < coincHigh) {
                  addbackCloverMatrixClose_coinc->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
               } else if((bg+coincLow) <= TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) < (bg+coincHigh)) {
                  addbackCloverMatrixClose_bg->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
               }
            }
         }
      }
      //addback coincident with beta
      if(gotSceptar && scep->GetMultiplicity() >= 1) {
         for(int b = 0; b < scep->GetMultiplicity(); ++b) {
            for(one = 0; one < (int) grif->GetAddBackCloverMultiplicity(); ++one) {
               //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) > coincHigh) {
                  //continue;
               //}
               addbackCloverSinglesB->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow());
               for(two = 0; two < (int) grif->GetAddBackCloverMultiplicity(); ++two) {
                  if(two == one) {
                     continue;
                  }
                  //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackCloverHit(two)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackCloverHit(two)->GetTime()) > coincHigh) {
                     //continue;
                  //}
                  addbackCloverHitsB->Fill(grif->GetAddBackCloverHit(one)->GetArrayNumber(),grif->GetAddBackCloverHit(two)->GetArrayNumber());
                  addbackCloverMatrixB->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
                  if(coincLow <= TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) < coincHigh) {
                     addbackCloverMatrix_coincB->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
                  } else if((bg+coincLow) <= TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) < (bg+coincHigh)) {
                     addbackCloverMatrix_bgB->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
                  }
                  if(grif->GetAddBackCloverHit(one)->GetPosition().Angle(grif->GetAddBackCloverHit(two)->GetPosition()) < TMath::Pi()/2.) {
                     addbackCloverMatrixCloseB->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
                     if(coincLow <= TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) < coincHigh) {
                        addbackCloverMatrixClose_coincB->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
                     } else if((bg+coincLow) <= TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackCloverHit(two)->GetTime()-grif->GetAddBackCloverHit(one)->GetTime()) < (bg+coincHigh)) {
                        addbackCloverMatrixClose_bgB->Fill(grif->GetAddBackCloverHit(one)->GetEnergyLow(), grif->GetAddBackCloverHit(two)->GetEnergyLow());
                     }
                  }
               }
            }
         }
      }


      if(entry%25000 == 0) {
         std::cout << "\t" << entry << " / " << entries << " = "<< (float)entry/entries*100.0 << "%. " << w->RealTime() << " seconds" << "\r" << std::flush;
         w->Continue();
      }
   }
   std::cout << "\t" << entry << " / " << entries << " = "<< (float)entry/entries*100.0 << "%. " << w->RealTime() << " seconds" << std::endl;
   w->Continue();

   //create all background corrected matrices
   TH2F* matrix_bgcorr = (TH2F*) matrix_coinc->Clone("matrix_bgcorr"); list->Add(matrix_bgcorr);
   matrix_bgcorr->SetTitle(Form("#gamma-#gamma matrix, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   matrix_bgcorr->Add(matrix_bg,-1.);

   TH2F* matrix_bgcorrB = (TH2F*) matrix_coincB->Clone("matrix_bgcorrB"); list->Add(matrix_bgcorrB);
   matrix_bgcorrB->SetTitle(Form("#gamma-#gamma matrix with coincident beta, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   matrix_bgcorrB->Add(matrix_bgB,-1.);

   TH2F* addbackMatrix_bgcorr = (TH2F*) addbackMatrix_coinc->Clone("addbackMatrix_bgcorr"); list->Add(addbackMatrix_bgcorr);
   addbackMatrix_bgcorr->SetTitle(Form("#gamma-#gamma matrix, addback, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackMatrix_bgcorr->Add(addbackMatrix_bg,-1.);

   TH2F* addbackMatrix_bgcorrB = (TH2F*) addbackMatrix_coincB->Clone("addbackMatrix_bgcorrB"); list->Add(addbackMatrix_bgcorrB);
   addbackMatrix_bgcorrB->SetTitle(Form("#gamma-#gamma matrix with coincident beta, addback, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackMatrix_bgcorrB->Add(addbackMatrix_bgB,-1.);

   TH2F* addbackCloverMatrix_bgcorr = (TH2F*) addbackCloverMatrix_coinc->Clone("addbackCloverMatrix_bgcorr"); list->Add(addbackCloverMatrix_bgcorr);
   addbackCloverMatrix_bgcorr->SetTitle(Form("#gamma-#gamma matrix, clover addback, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackCloverMatrix_bgcorr->Add(addbackCloverMatrix_bg,-1.);

   TH2F* addbackCloverMatrix_bgcorrB = (TH2F*) addbackCloverMatrix_coincB->Clone("addbackCloverMatrix_bgcorrB"); list->Add(addbackCloverMatrix_bgcorrB);
   addbackCloverMatrix_bgcorrB->SetTitle(Form("#gamma-#gamma matrix with coincident beta, clover addback, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackCloverMatrix_bgcorrB->Add(addbackCloverMatrix_bgB,-1.);

   TH2F* addbackMatrixClose_bgcorr = (TH2F*) addbackMatrixClose_coinc->Clone("addbackMatrixClose_bgcorr"); list->Add(addbackMatrixClose_bgcorr);
   addbackMatrixClose_bgcorr->SetTitle(Form("#gamma-#gamma matrix, addback, angle between detectors < 90^{o}, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackMatrixClose_bgcorr->Add(addbackMatrixClose_bg,-1.);

   TH2F* addbackMatrixClose_bgcorrB = (TH2F*) addbackMatrixClose_coincB->Clone("addbackMatrixClose_bgcorrB"); list->Add(addbackMatrixClose_bgcorrB);
   addbackMatrixClose_bgcorrB->SetTitle(Form("#gamma-#gamma matrix with coincident beta, addback, angle between detectors < 90^{o}, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackMatrixClose_bgcorrB->Add(addbackMatrixClose_bgB,-1.);

   TH2F* addbackCloverMatrixClose_bgcorr = (TH2F*) addbackCloverMatrixClose_coinc->Clone("addbackCloverMatrixClose_bgcorr"); list->Add(addbackCloverMatrixClose_bgcorr);
   addbackCloverMatrixClose_bgcorr->SetTitle(Form("#gamma-#gamma matrix, clover addback, angle between detectors < 90^{o}, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackCloverMatrixClose_bgcorr->Add(addbackCloverMatrixClose_bg,-1.);

   TH2F* addbackCloverMatrixClose_bgcorrB = (TH2F*) addbackCloverMatrixClose_coincB->Clone("addbackCloverMatrixClose_bgcorrB"); list->Add(addbackCloverMatrixClose_bgcorrB);
   addbackCloverMatrixClose_bgcorrB->SetTitle(Form("#gamma-#gamma matrix with coincident beta, clover addback, angle between detectors < 90^{o}, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackCloverMatrixClose_bgcorrB->Add(addbackCloverMatrixClose_bgB,-1.);

   TH2F* angCorr_bgcorr = (TH2F*) angCorr_coinc->Clone("angCorr_bgcorr"); list->Add(angCorr_bgcorr);
   angCorr_bgcorr->SetTitle(Form("angular correlation cube, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   angCorr_bgcorr->Add(angCorr_bg,-1.);

   TH2F* angCorrAddback_bgcorr = (TH2F*) angCorrAddback_coinc->Clone("angCorrAddback_bgcorr"); list->Add(angCorrAddback_bgcorr);
   angCorrAddback_bgcorr->SetTitle(Form("angular correlation cube, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   angCorrAddback_bgcorr->Add(angCorrAddback_bg,-1.);

   //if we have waveforms, add the spectra to the list
   if(rawWaveform->GetEntries() > 0) {
      list->Add(rawWaveform);
      list->Add(intNormWaveform);
      list->Add(peakNormWaveform);
      list->Add(rawWaveformCfd);
      list->Add(intNormWaveformCfd);
      list->Add(peakNormWaveformCfd);
      list->Add(rawWaveformPsd);
      list->Add(intNormWaveformPsd);
      list->Add(peakNormWaveformPsd);
   }

#ifdef __CINT__ 
   TCanvas* c = new TCanvas;
   c->Divide(2,2);
   c->cd(1);
   matrix->Draw("colz");
   c->cd(2);
   matrix_bgcorr->Draw("colz");
   c->cd(3);
   matrix_coinc->Draw("colz");
   c->cd(4);
   matrix_bg->Draw("colz");
   TCanvas* c2 = new TCanvas;
   c2->Divide(2);
   c2->cd(1);
   timeDiff->Draw();
   timeEGate->SetLineColor(2);
   timeEGate->Draw("same");
   timeModuleOne->SetLineColor(4);
   timeModuleOne->Draw("same");
   c2->cd(2);
   grifMult->Draw();
   grifMultCut->SetLineColor(2);
   grifMultCut->Draw("same");
#endif

   list->Sort();
   std::cout << "creating histograms done after " << w->RealTime() << " seconds" << std::endl;
   w->Continue();
   return list;
}


#ifndef __CINT__ 

int main(int argc, char **argv) {
   if(argc != 4 && argc != 3 && argc != 2) {
      printf("try again (usage: %s <analysis tree file> <optional: output file> <max entries>).\n",argv[0]);
      return 0;
   }

   TStopwatch w;
   w.Start();

   std::string fileName;
   if(argc == 2) {
      fileName = argv[1];
      if(fileName.find_last_of("/") != std::string::npos) {
         fileName.insert(fileName.find_last_of("/")+1,"matrices_");
      } else {
         fileName.insert(0,"matrices_");
      }
   } else {
      fileName = argv[2];
   }

   struct stat fileInfo; 
   if(stat(fileName.c_str(),&fileInfo) == 0) {
      printf("File '%s' already exists, please remove it before re-running %s!\n",fileName.c_str(),argv[0]);
      return 1;
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

   TTree* tree = (TTree*) file->Get("AnalysisTree");

   if(tree == NULL) {
      printf("Failed to find analysis tree in file '%s'!\n",argv[1]);
      return 1;
   }
   
   std::cout << argv[0] << ": starting MakeMatrices after " << w.RealTime() << " seconds" << std::endl;
   w.Continue();

   //coinc window = 0-20, bg window 40-60, 6000 bins from 0. to 6000. (default is 4000)
   TList *list;
   if(argc < 4) {
      list = MakeMatrices(tree, 0., 20., 80., 6000, 0., 6000., 0, &w);
   } else {
      int entries = atoi(argv[3]);
      std::cout<<"Limiting processing of analysis tree to "<<entries<<" entries!"<<std::endl;
      list = MakeMatrices(tree, 0., 20., 80., 6000, 0., 6000., entries, &w);
   }

   TFile *outfile = new TFile(fileName.c_str(),"create");
   list->Write();
   outfile->Close();

   std::cout << argv[0] << " done after " << w.RealTime() << " seconds" << std::endl;

   return 0;
}

#endif
