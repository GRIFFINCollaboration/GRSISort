// g++ AnalyzeFragmentTree.C -I$GRSISYS/include -L$GRSISYS/libraries -lGRSIFormat `root-config --cflags --libs` -lTreePlayer -o AnalyzeFragmentTree

// Takes in a FragmentTree, makes CES time diff spectra and diagnostics spectra
//
//
#include <utility>
#include <vector>
#include <cstdio>
#include <iostream>
#include <iomanip>

#include "TTree.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "TFile.h"
#include "TList.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TStopwatch.h"

#include "TChannel.h"

#include "TFragment.h"

TList* AnalyzeFragmentTree(TTree* tree, int64_t entries = 0, TStopwatch* w = nullptr)
{

   if(w == nullptr) {
      w = new TStopwatch;
      w->Start();
   }

   const size_t MEM_SIZE = static_cast<size_t>(1024) * static_cast<size_t>(1024) * static_cast<size_t>(1024) * static_cast<size_t>(8);   // 8 GB

   auto* list = new TList;

   TFragment*                           currentFrag = nullptr;
   std::vector<std::pair<int64_t, int>> oldBetaTimeStamp;
   std::vector<std::pair<int64_t, int>> oldGammaTimeStamp;

   bool last_beta_filled  = false;
   bool last_gamma_filled = false;

   TChannel::ReadCalFromTree(tree);

   TBranch* branch = tree->GetBranch("TFragment");
   branch->SetAddress(&currentFrag);
   tree->LoadBaskets(MEM_SIZE);

   printf("Tree Index not found, building index on %s/%s...",
          "TimeStampHigh", "TimeStampLow");
   fflush(stdout);
   tree->BuildIndex("TimeStampHigh", "TimeStampLow");
   printf(" done!\n");

   auto*     index       = static_cast<TTreeIndex*>(tree->GetTreeIndex());
   Long64_t* indexvalues = index->GetIndex();
   int       fEntries    = index->GetN();

   int minTime = -5000;
   int maxTime = 5000;

   TH1F* gamma = new TH1F("gamma", "#gamma;energy [keV];counts/keV", 16000, 0., 16000.);
   list->Add(gamma);
   TH1F* betaGamma = new TH1F("betaGamma", "#beta#gamma;energy [keV];counts/keV", 16000, 0., 16000.);
   list->Add(betaGamma);

   TH2F* griffinTimeDiff = new TH2F("griffinTimeDiff", "time difference between GRIFFIN detectors and other detectors;#Delta t [10 ns];detector type",
                                    maxTime - minTime, minTime, maxTime, 10, 0, 10);
   TH2F* sceptarTimeDiff = new TH2F("sceptarTimeDiff", "time difference between SCEPTAR detectors and other detectors;#Delta t [10 ns];detector type",
                                    maxTime - minTime, minTime, maxTime, 10, 0, 10);
   TH2F* pacesTimeDiff   = new TH2F("pacesTimeDiff", "time difference between PACES detectors and other detectors;#Delta t [10 ns];detector type",
                                    maxTime - minTime, minTime, maxTime, 10, 0, 10);
   TH2F* descantTimeDiff = new TH2F("descantTimeDiff", "time difference between DESCANT detectors and other detectors;#Delta t [10 ns];detector type",
                                    maxTime - minTime, minTime, maxTime, 10, 0, 10);

   TH1F* gg_diff = new TH1F("gg_diff", "gg_diff", maxTime - minTime, minTime, maxTime);
   list->Add(gg_diff);
   TH1F* gb_diff = new TH1F("gb_diff", "gb_diff", maxTime - minTime, minTime, maxTime);
   list->Add(gb_diff);
   TH1F* bg_diff = new TH1F("bg_diff", "bg_diff", maxTime - minTime, minTime, maxTime);
   list->Add(bg_diff);
   TH1F* bb_diff = new TH1F("bb_diff", "bb_diff", maxTime - minTime, minTime, maxTime);
   list->Add(bb_diff);

   TH1F* bg_coinc_gE = new TH1F("bg_coinc_gE", "bg_coinc_gE", 16000, 0, 16000);
   list->Add(bg_coinc_gE);

   int FragsIn = 0;

   tree->GetEntry(indexvalues[0]);
   FragsIn++;
   int64_t entry = 0;
   for(entry = 1; entry < fEntries; entry++) {
      if(tree->GetEntry(indexvalues[entry]) == -1) {   // move current frag to the next (x'th) entry in the tree
         printf("FIRE!!!"
                "\n");
         continue;
      }

      TFragment myFrag = *currentFrag;   // Set myfrag to be the x'th fragment before incrementing it.

      if(myFrag.GetDetectorType() == 1) {
         gamma->Fill(myFrag.GetEnergy());
      }

      int64_t time      = currentFrag->GetTimeStamp();   // Get the timestamp of the x'th fragment
      int64_t timelow   = time + minTime;
      int64_t timehigh  = time + maxTime;
      int     time_low  = static_cast<int>(timelow & 0x0fffffff);
      int     time_high = static_cast<int>(timelow >> 28);

      // find the entry where the low part of the gate fits.
      int64_t start = index->FindValues(time_high, time_low);

      time_low  = static_cast<int>(timehigh & 0x0fffffff);
      time_high = static_cast<int>(timehigh >> 28);

      // Find the entry where the high part of the gate fits
      int64_t stop = index->FindValues(time_high, time_low);

      // printf("\nlooping over y = %ld - %ld\n",start,stop);

      // printf("Multiplicity = %d\n",stop-start)
      for(int64_t y = start; y < stop; y++) {
         // If the index of the compared fragment equals the index of the first fragment, do nothing
         if(y == entry) {
            continue;
         }
         if(tree->GetEntry(indexvalues[y]) == -1) {   // move currentfrag to the next fragment
            printf("FIRE!!!"
                   "\n");
            continue;
         }

         if(myFrag.GetDetectorType() == 1) {
            if(currentFrag->GetDetectorType() == 1) {
               gg_diff->Fill(myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
            } else if(currentFrag->GetDetectorType() == 2) {
               gb_diff->Fill(myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
               bg_coinc_gE->Fill(myFrag.GetEnergy());
               betaGamma->Fill(myFrag.GetEnergy());
            } else {
               // printf("Unknown detector type\n");
            }
            griffinTimeDiff->Fill(currentFrag->GetDetectorType(), myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
         } else if(myFrag.GetDetectorType() == 2) {
            if(currentFrag->GetDetectorType() == 1) {
               bg_diff->Fill(myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
               bg_coinc_gE->Fill(currentFrag->GetEnergy());
               betaGamma->Fill(myFrag.GetEnergy());
            } else if(currentFrag->GetDetectorType() == 2) {
               bb_diff->Fill(myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
            } else {
               // printf("Unknown detector type\n");
            }
            sceptarTimeDiff->Fill(currentFrag->GetDetectorType(), myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
         } else if(myFrag.GetDetectorType() == 5) {
            pacesTimeDiff->Fill(currentFrag->GetDetectorType(), myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
         } else if(myFrag.GetDetectorType() == 6) {
            descantTimeDiff->Fill(currentFrag->GetDetectorType(), myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
         }
      }

      if(entry % 25000 == 0) {
         std::cout << "\t" << entry << " / " << entries << " = " << (100. * entry) / entries << "%. " << w->RealTime() << " seconds"
                   << "\r" << std::flush;
         w->Continue();
      }
   }
   std::cout << "\t" << entry << " / " << entries << " = " << (100. * entry) / entries << "%. " << w->RealTime() << " seconds" << std::endl
             << std::endl;
   w->Continue();

   return list;
}

#ifndef __CINT__

int main(int argc, char** argv)
{

   if(argc != 4 && argc != 3 && argc != 2) {
      printf("try again (usage: %s <fragment tree file> <optional: output file> <max entries>).\n", argv[0]);
      return 0;
   }

   TStopwatch w;
   w.Start();

   std::string fileName;
   if(argc == 2) {
      fileName = argv[1];
      if(fileName.find_last_of('/') != std::string::npos) {
         fileName.insert(fileName.find_last_of('/') + 1, "matrices_");
      } else {
         fileName.insert(0, "diagn_");
      }
   } else {
      fileName = argv[2];
   }

   auto* file = new TFile(argv[1]);
   if(file == nullptr) {
      printf("Failed to open file '%s'!\n", argv[1]);
      return 1;
   }
   if(!file->IsOpen()) {
      printf("Failed to open file '%s'!\n", argv[1]);
      return 1;
   }

   auto* tree = static_cast<TTree*>(file->Get("FragmentTree"));

   if(tree == nullptr) {
      printf("Failed to find fragment tree in file '%s'!\n", argv[1]);
      return 1;
   }

   std::cout << argv[0] << ": starting AnalyzeFragmentTree after " << w.RealTime() << " seconds" << std::endl;
   w.Continue();

   int64_t entries = tree->GetEntries();
   if(argc == 4 && atoi(argv[3]) < entries) {
      entries = atoi(argv[3]);
      std::cout << "Limiting processing of fragment tree to " << entries << " entries!" << std::endl;
   }
   TList* list = AnalyzeFragmentTree(tree, entries, &w);

   auto* outfile = new TFile(fileName.c_str(), "recreate");
   list->Write();
   outfile->Close();

   std::cout << argv[0] << " done after " << w.RealTime() << " seconds" << std::endl;

   return 0;
}

#endif
