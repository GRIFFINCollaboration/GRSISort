//g++ MakeMatrices.C -I$GRSISYS/include -L$GRSISYS/libraries -lGRSIFormat -lAnalysisTreeBuilder -lGriffin -lSceptar -lDescant -lPaces -lGRSIDetector -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml `root-config --cflags --libs` -lTreePlayer -o MakeMatrices

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


TList *MakeMatrices(TChain* tree, int coincLow = 0, int coincHigh = 10, int bg = 100, int nofBins = 4000, double low = 0., double high = 4000.) {
   TList* list = new TList;

   const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

   TH1F* timeDiff = new TH1F("timeDiff","time difference;time[10 ns]",200,0.,200.); list->Add(timeDiff);
   TH1F* timeEGate = new TH1F("timeEGate","time difference energy gate on 1808 and 1130;time[10 ns]",200,0.,200.);  list->Add(timeEGate);
   TH1F* timeModuleOne = new TH1F("timeModuleOne","time difference for 1. grif-16;time[10 ns]",200,0.,200.); list->Add(timeModuleOne);
   TH2F* matrix = new TH2F("matrix","#gamma-#gamma matrix",nofBins, low, high,nofBins, low, high); list->Add(matrix);
   TH2F* matrix_coinc = new TH2F("matrix_coinc",Form("#gamma-#gamma matrix, coincident within %d - %d [10 ns]", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(matrix_coinc);
   TH2F* matrix_bg = new TH2F("matrix_bg",Form("#gamma-#gamma matrix, background within %d - %d [ 10 ns]", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(matrix_bg);

   TH1F* cfdDiff = new TH1F("cfdDiff","cfd difference",2000,-1000.,1000.); list->Add(cfdDiff);
   TH1F* cfdDiffEGate = new TH1F("cfdDiffEGate","cfd difference energy gate on 1808 and 1130",2000,-1000.,1000.); list->Add(cfdDiffEGate);
   TH1F* cfdDiffModuleOne = new TH1F("cfdDiffModuleOne","cfd difference for 1. grif-16",2000,-1000.,1000.); list->Add(cfdDiffModuleOne);

   TH1F* grifMult    = new TH1F("grifMult",        "Griffin multiplicity in built event with coincident beta",65,0.,65.);                                             list->Add(grifMult);
   TH1F* grifMultCut = new TH1F("grifMultCut",Form("Griffin multiplicity in built event with #Delta t = %d - %d, and coincident beta",coincLow,coincHigh),65,0.,65.); list->Add(grifMultCut);
   
   TH1F* scepMult    = new TH1F("scepMult",        "Sceptar multiplicity in built event with coincident beta",65,0.,65.);                                             list->Add(scepMult);
   TH1F* scepMultCut = new TH1F("scepMultCut",Form("Sceptar multiplicity in built event with #Delta t = %d - %d, and coincident beta",coincLow,coincHigh),65,0.,65.); list->Add(scepMultCut);
 
   TH1F* timeB = new TH1F("timeB","time difference with coincident beta;time[10 ns]",200,0.,200.); list->Add(timeB);
   TH1F* timeEGateB = new TH1F("timeEGateB","time difference energy gate on 1808 and 1130, and coincident beta;time[10 ns]",200,0.,200.);  list->Add(timeEGateB);
   TH1F* timeModuleOneB = new TH1F("timeModuleOneB","time difference for 1. grif-16 with coincident beta;time[10 ns]",200,0.,200.); list->Add(timeModuleOneB);
   TH2F* matrixB = new TH2F("matrixB","#gamma-#gamma matrix with coincident beta",nofBins, low, high,nofBins, low, high); list->Add(matrixB);
   TH2F* matrix_coincB = new TH2F("matrix_coincB",Form("#gamma-#gamma matrix, coincident within %d - %d [10 ns], and coincident beta", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(matrix_coincB);
   TH2F* matrix_bgB = new TH2F("matrix_bgB",Form("#gamma-#gamma matrix, background within %d - %d [ 10 ns], and coincident beta", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(matrix_bgB);

   TH1F* cfdDiffB = new TH1F("cfdDiffB","cfd difference with coincident beta",2000,-1000.,1000.); list->Add(cfdDiffB);
   TH1F* cfdDiffEGateB = new TH1F("cfdDiffEGateB","cfd difference energy gate on 1808 and 1130, and coincident beta",2000,-1000.,1000.); list->Add(cfdDiffEGateB);
   TH1F* cfdDiffModuleOneB = new TH1F("cfdDiffModuleOneB","cfd difference for 1. grif-16 with coincident beta",2000,-1000.,1000.); list->Add(cfdDiffModuleOneB);

   TH1F* grifMultB    = new TH1F("grifMultB",        "Griffin multiplicity in built event with coincident beta",65,0.,65.);                                             list->Add(grifMultB);
   TH1F* grifMultCutB = new TH1F("grifMultCutB",Form("Griffin multiplicity in built event with #Delta t = %d - %d, and coincident beta",coincLow,coincHigh),65,0.,65.); list->Add(grifMultCutB);
   
   TH1F* scepMultB    = new TH1F("scepMultB",        "Sceptar multiplicity in built event with coincident beta",65,0.,65.);                                             list->Add(scepMultB);
   TH1F* scepMultCutB = new TH1F("scepMultCutB",Form("Sceptar multiplicity in built event with #Delta t = %d - %d, and coincident beta",coincLow,coincHigh),65,0.,65.); list->Add(scepMultCutB);
 
   TH1F* addbackTimeDiff = new TH1F("addbackTimeDiff","time difference;time[10 ns]",200,0.,200.); list->Add(addbackTimeDiff);
   TH1F* addbackCfdDiff = new TH1F("addbackCfdDiff","cfd difference",2000,-1000.,1000.); list->Add(addbackCfdDiff);
   TH1F* addbackTimeB = new TH1F("addbackTimeB","time difference with coincident beta;time[10 ns]",200,0.,200.); list->Add(addbackTimeB);
   TH1F* addbackCfdDiffB = new TH1F("addbackCfdDiffB","cfd difference with coincident beta",2000,-1000.,1000.); list->Add(addbackCfdDiffB);

   TH2F* addbackMatrix = new TH2F("addbackMatrix","#gamma-#gamma matrix, addback",nofBins, low, high,nofBins, low, high); list->Add(addbackMatrix);
   TH2F* addbackMatrix_coinc = new TH2F("addbackMatrix_coinc",Form("#gamma-#gamma matrix, addback, coincident within %d - %d [10 ns]", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackMatrix_coinc);
   TH2F* addbackMatrix_bg = new TH2F("addbackMatrix_bg",Form("#gamma-#gamma matrix, addback, background within %d - %d [ 10 ns]", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackMatrix_bg);

   TH2F* addbackMatrixB = new TH2F("addbackMatrixB","#gamma-#gamma matrix, addback, and coincident #beta",nofBins, low, high,nofBins, low, high); list->Add(addbackMatrixB);
   TH2F* addbackMatrix_coincB = new TH2F("addbackMatrix_coincB",Form("#gamma-#gamma matrix, addback, coincident within %d - %d [10 ns], and coincident #beta", coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(addbackMatrix_coincB);
   TH2F* addbackMatrix_bgB = new TH2F("addbackMatrix_bgB",Form("#gamma-#gamma matrix, addback, background within %d - %d [ 10 ns], and coincident #beta", bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(addbackMatrix_bgB);



   TGriffin* grif = 0;
   TSceptar* scep = 0;
   tree->SetBranchAddress("TGriffin", &grif);

   bool gotSceptar;
   if(tree->FindBranch("TSceptar") == 0) {
      gotSceptar = false;
   } else {
      tree->SetBranchAddress("TSceptar", &scep);
      gotSceptar = true;
   }


   //tree->LoadBaskets(MEM_SIZE);   

   int skipped = 0;
   int notSkipped = 0;
   int coincident = 0;
   int background = 0;
   long entries = tree->GetEntries();

   int one;
   int two;

   for(int entry = 0; entry < tree->GetEntries(); ++entry) {
      tree->GetEntry(entry);
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

      if(grif->GetMultiplicity() < 2) {
         ++skipped;
         continue;
      }
      ++notSkipped;
      //std::cout<<"got multiplicity "<<grif->GetMultiplicity()<<", time 0 = "<<grif->GetGriffinHit(0)->GetTime()<<", time 1 = "<<grif->GetGriffinHit(1)->GetTime()<<std::endl;

      //loop over all gamma's in two loops
      for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
         for(two = 0; two < (int) grif->GetMultiplicity(); ++two) {
            if(two == one) {
               continue;
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
               ++coincident;
            } else if((bg+coincLow) <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < (bg+coincHigh)) {
               matrix_bg->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
               ++background;
            }
         }
      }
      if(gotSceptar && scep->GetMultiplicity() >= 1) {
         for(int b = 0; b < scep->GetMultiplicity(); ++b) {
            for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
               //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetGriffinHit(one)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetGriffinHit(one)->GetTime()) > coincHigh) {
                  //continue;
               //}
               for(two = 0; two < (int) grif->GetMultiplicity(); ++two) {
                  if(two == one) {
                     continue;
                  }
                  //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetGriffinHit(two)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetGriffinHit(two)->GetTime()) > coincHigh) {
                     //continue;
                  //}
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
         }
      }

      for(one = 0; one < (int) grif->GetAddBackMultiplicity(); ++one) {
         for(two = 0; two < (int) grif->GetAddBackMultiplicity(); ++two) {
            if(two == one) {
               continue;
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
            } else if((bg+coincLow) <= TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) && TMath::Abs(grif->GetAddBackHit(two)->GetTime()-grif->GetAddBackHit(one)->GetTime()) < (bg+coincHigh)) {
               addbackMatrix_bg->Fill(grif->GetAddBackHit(one)->GetEnergyLow(), grif->GetAddBackHit(two)->GetEnergyLow());
            }
         }
      }
      if(gotSceptar && scep->GetMultiplicity() >= 1) {
         for(int b = 0; b < scep->GetMultiplicity(); ++b) {
            for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
               //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackHit(one)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackHit(one)->GetTime()) > coincHigh) {
                  //continue;
               //}
               for(two = 0; two < (int) grif->GetMultiplicity(); ++two) {
                  if(two == one) {
                     continue;
                  }
                  //if(coincLow > TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackHit(two)->GetTime()) || TMath::Abs(scep->GetSceptarHit(b)->GetTime()-grif->GetAddBackHit(two)->GetTime()) > coincHigh) {
                     //continue;
                  //}
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
               }
            }
         }
      }



      if(entry%25000 == 0)
         std::cout << "\t" << entry << " / " << entries << " = "<< (float)entry/entries*100.0 << "%" << "\r" << std::flush;
   }

   std::cout<<"skipped "<<skipped<<" entries, didn't skip "<<notSkipped<<" entries: got "<<coincident<<" coincident and "<<background<<" background events"<<std::endl;

   TH2F* matrix_bgcorr = (TH2F*) matrix_coinc->Clone("matrix_bgcorr"); list->Add(matrix_bgcorr);
   matrix_bgcorr->SetTitle(Form("#gamma-#gamma matrix, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   matrix_bgcorr->Add(matrix_bg,-1.);

   TH2F* matrix_bgcorrB = (TH2F*) matrix_coincB->Clone("matrix_bgcorrB"); list->Add(matrix_bgcorrB);
   matrix_bgcorrB->SetTitle(Form("#gamma-#gamma matrix with coincident beta, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   matrix_bgcorrB->Add(matrix_bgB,-1.);

   TH2F* addbackMatrix_bgcorr = (TH2F*) addbackMatrix_coinc->Clone("addbackMatrix_bgcorr"); list->Add(addbackMatrix_bgcorr);
   addbackMatrix_bgcorr->SetTitle(Form("#gamma-#gamma matrix, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackMatrix_bgcorr->Add(addbackMatrix_bg,-1.);

   TH2F* addbackMatrix_bgcorrB = (TH2F*) addbackMatrix_coincB->Clone("addbackMatrix_bgcorrB"); list->Add(addbackMatrix_bgcorrB);
   addbackMatrix_bgcorrB->SetTitle(Form("#gamma-#gamma matrix with coincident beta, background corrected (%d - %d minus %d - %d)",coincLow, coincHigh, coincLow+bg, coincHigh+bg));
   addbackMatrix_bgcorrB->Add(addbackMatrix_bgB,-1.);

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

   //coinc window = 0-20, bg window 40-60
   TList *list = MakeMatrices(chain, 0., 20., 80.);

   TFile *outfile = new TFile(argv[argc-1],"create");
   list->Write();

   return 0;
}

#endif
