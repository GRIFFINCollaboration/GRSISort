//g++ MakeTimeDiffSpec.C -I$GRSISYS/include -L$GRSISYS/libraries -lGRSIFormat `root-config --cflags --libs` -lTreePlayer -o runfaster





//Takes in a FargmentTree, makes CES time diff spectra
//
//
#include <utility>
#include <vector>
#include <cstdio>

#include "TTree.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "TFile.h"
#include "TList.h"
#include "TH1F.h"
#include "TH2F.h"

#include "TChannel.h"

#include "TFragment.h"

TList *MakeTimeDiffSpec(TTree *tree) {

   const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

   TList *list = new TList;
   
   TFragment *currentFrag = 0;
   //TFragment *oldFrag_beta   = new TFragment; bool first_beta_set  = false;
   //TFragment *oldFrag_gamma  = new TFragment; bool first_gamma_set = false;
   std::vector<std::pair<long,int> > oldBetaTimeStamp;
   std::vector<std::pair<long,int> > oldGammaTimeStamp;

   bool last_beta_filled = false;
   bool last_gamma_filled = false;

   TChannel::ReadCalFromTree(tree);

   //tree->SetBranchAddress("TFragment",&currentFrag);
   TBranch *branch = tree->GetBranch("TFragment");
   branch->SetAddress(&currentFrag);
   tree->LoadBaskets(MEM_SIZE);

   printf("Tree Index not found, building index on %s/%s...",
          "TimeStampHigh","TimeStampLow");  fflush(stdout);
   tree->BuildIndex("TimeStampHigh","TimeStampLow");
   printf(" done!\n");


   TTreeIndex *index = (TTreeIndex*)tree->GetTreeIndex();
   Long64_t *indexvalues = index->GetIndex();
   int fEntries = index->GetN();


   TH1F *gg_diff = new TH1F("gg_diff","gg_diff",12000,-6000,6000); list->Add(gg_diff);

   //TH2F *gg_diff_mod[4];
   //gg_diff_mod[0] = new TH2F("gg_diff_0","gg_diff_0",4,0,4,600,0,600); list->Add(gg_diff_mod[0]);
   //gg_diff_mod[1] = new TH2F("gg_diff_1","gg_diff_1",4,0,4,600,0,600); list->Add(gg_diff_mod[1]);
   //gg_diff_mod[2] = new TH2F("gg_diff_2","gg_diff_2",4,0,4,600,0,600); list->Add(gg_diff_mod[2]);
   //gg_diff_mod[3] = new TH2F("gg_diff_3","gg_diff_3",4,0,4,600,0,600); list->Add(gg_diff_mod[3]);

   //TH2F *gg_diff_ch[16];
   //gg_diff_ch[0] = new TH2F("gg_diff_ch_0","gg_diff_0",16,0,16,600,0,600); list->Add(gg_diff_ch[0]);
   //gg_diff_ch[1] = new TH2F("gg_diff_ch_1","gg_diff_1",16,0,16,600,0,600); list->Add(gg_diff_ch[1]);
   //gg_diff_ch[2] = new TH2F("gg_diff_ch_2","gg_diff_2",16,0,16,600,0,600); list->Add(gg_diff_ch[2]);
   //gg_diff_ch[3] = new TH2F("gg_diff_ch_3","gg_diff_3",16,0,16,600,0,600); list->Add(gg_diff_ch[3]);
   //gg_diff_ch[4] = new TH2F("gg_diff_ch_4","gg_diff_4",16,0,16,600,0,600); list->Add(gg_diff_ch[4]);
   //gg_diff_ch[5] = new TH2F("gg_diff_ch_5","gg_diff_5",16,0,16,600,0,600); list->Add(gg_diff_ch[5]);
   //gg_diff_ch[6] = new TH2F("gg_diff_ch_6","gg_diff_6",16,0,16,600,0,600); list->Add(gg_diff_ch[6]);
   //gg_diff_ch[7] = new TH2F("gg_diff_ch_7","gg_diff_7",16,0,16,600,0,600); list->Add(gg_diff_ch[7]);
   //gg_diff_ch[8] = new TH2F("gg_diff_ch_8","gg_diff_8",16,0,16,600,0,600); list->Add(gg_diff_ch[8]);
   //gg_diff_ch[9] = new TH2F("gg_diff_ch_9","gg_diff_9",16,0,16,600,0,600); list->Add(gg_diff_ch[9]);
   //gg_diff_ch[10] = new TH2F("gg_diff_ch_10","gg_diff_10",16,0,16,600,0,600); list->Add(gg_diff_ch[10]);
   //gg_diff_ch[11] = new TH2F("gg_diff_ch_11","gg_diff_11",16,0,16,600,0,600); list->Add(gg_diff_ch[11]);
   //gg_diff_ch[12] = new TH2F("gg_diff_ch_12","gg_diff_12",16,0,16,600,0,600); list->Add(gg_diff_ch[12]);
   //gg_diff_ch[13] = new TH2F("gg_diff_ch_13","gg_diff_13",16,0,16,600,0,600); list->Add(gg_diff_ch[13]);
   //gg_diff_ch[14] = new TH2F("gg_diff_ch_14","gg_diff_14",16,0,16,600,0,600); list->Add(gg_diff_ch[14]);
   //gg_diff_ch[15] = new TH2F("gg_diff_ch_15","gg_diff_15",16,0,16,600,0,600); list->Add(gg_diff_ch[15]);

   //TH2F *gg_diff_mat_same16 = new TH2F("gg_diff_mat_same16","gg_diff_mat_same16",10,0,10,600,0,600); list->Add(gg_diff_mat_same16);
   //TH2F *gg_diff_mat_diff16 = new TH2F("gg_diff_mat_diff16","gg_diff_mat_diff16",10,0,10,600,0,600); list->Add(gg_diff_mat_diff16);
   //TH2F *gg_diff_mat_diffrev16 = new TH2F("gg_diff_mat_diffrev16","gg_diff_mat_diffrev16",10,0,10,600,0,600); list->Add(gg_diff_mat_diffrev16);

   //TH2F *gg_diff_mat_same16_ch = new TH2F("gg_diff_mat_same16_ch","gg_diff_mat_same16_ch",65,0,65,600,0,600); list->Add(gg_diff_mat_same16_ch);
   //TH2F *gg_diff_mat_diff16_ch = new TH2F("gg_diff_mat_diff16_ch","gg_diff_mat_diff16_ch",65,0,65,600,0,600); list->Add(gg_diff_mat_diff16_ch);
   //TH2F *gg_diff_mat_diffrev16_ch = new TH2F("gg_diff_mat_diffrev16_ch","gg_diff_mat_diffrev16_ch",65,0,65,600,0,600); list->Add(gg_diff_mat_diffrev16_ch);

   TH1F *gb_diff = new TH1F("gb_diff","gb_diff",12000,-6000,6000); list->Add(gb_diff);
   TH1F *bg_diff = new TH1F("bg_diff","bg_diff",12000,-6000,6000); list->Add(bg_diff);
   TH1F *bb_diff = new TH1F("bb_diff","bb_diff",12000,-6000,6000); list->Add(bb_diff);

   TH1F* bg_coinc_gE = new TH1F("bg_coinc_gE","bg_coinc_gE",16000,0,16000); list->Add(bg_coinc_gE);

   //TH2F *gg_diff_E1 = new TH2F("gg_diff_E1","gg time difference of card 3&1 vs energy of card 1",4000,0.,4000.,600,0.,600.); list->Add(gg_diff_E1);
   //TH2F *gg_diff_E3 = new TH2F("gg_diff_E3","gg time difference of card 3&1 vs energy of card 3",4000,0.,4000.,600,0.,600.); list->Add(gg_diff_E3);

   int FragsIn = 0;

   tree->GetEntry(indexvalues[0]);
   FragsIn++;

   for(long x=1;x<fEntries;x++) {
      if(tree->GetEntry(indexvalues[x]) == -1 ) { //move current frag to the next (x'th) entry in the tree
         printf( "FIRE!!!" "\n");
         continue;
      } 
   
      TFragment myFrag  = *currentFrag;         //Set myfrag to be the x'th fragment before incrementing it.
      long time = currentFrag->GetTimeStamp();  //Get the timestamp of the x'th fragment 
//      long timelow  = time + 120;
//      long timehigh  = time + 140; 
        long timelow = time + 0;
        long timehigh = time + 10000;   
      int time_low  = (int) (timelow & 0x0fffffff);
      int time_high = (int) (timelow >> 28); 


      //long start = indexvalues[index->FindValues(time_high,time_low)];////tree->GetEntryNumberWithBestIndex(time_high,time_low);
      
      //find the entry where the low part of the gate fits.
      long start = index->FindValues(time_high,time_low);////tree->GetEntryNumberWithBestIndex(time_high,time_low);

     
      time_low  = (int) (timehigh & 0x0fffffff);
      time_high = (int) (timehigh >> 28); 
    
      //long stop = indexvalues[index->FindValues(time_high,time_low)];////tree->GetEntryNumberWithBestIndex(time_high,time_low);
      
      //Find the entry where the high part of the gate fits
      long stop = index->FindValues(time_high,time_low);////tree->GetEntryNumberWithBestIndex(time_high,time_low);

      //printf("\nlooping over y = %ld - %ld\n",start,stop);
      //
      //printf("Multiplicity = %d\n",stop-start)
      for(long y=start;y<stop;y++) {
         //If the index of the comapred fragment equals the index of the first fragment, do nothing
         if(y == x) {
            continue;
         }
         if(tree->GetEntry(indexvalues[y]) == -1 ) { //move currentfrag to the next fragment
            printf( "FIRE!!!" "\n");
            continue;
         } 
    
         if(myFrag.DetectorType == 1) {
            if(currentFrag->DetectorType == 1) {
               gg_diff->Fill(myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
            } else if(currentFrag->DetectorType == 2) {
               gb_diff->Fill(myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
               bg_coinc_gE->Fill(myFrag.GetEnergy());
            } else {
               //printf("Unknown detector type\n");
            }
         } else if(myFrag.DetectorType == 2) {
            if(currentFrag->DetectorType == 1) {
               bg_diff->Fill(myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
               bg_coinc_gE->Fill(currentFrag->GetEnergy());
            } else if(currentFrag->DetectorType == 2) {
               bb_diff->Fill(myFrag.GetTimeStamp() - currentFrag->GetTimeStamp());
            } else {
               //printf("Unknown detector type\n");
            }
         }
      }

      if(x%1500==0)
         printf("\tOn fragment %i/%i               \r",x,fEntries);
  //    if(x>1000000)
  //       break;
   }
   printf("\n\n");
  
   return list;
}



#ifndef __CINT__ 

int main(int argc, char **argv) {
   
   if(argc!=2)  {
      printf("try again.\n");
      return 0;
   }
   TFile *f = new TFile(argv[1]);
   TTree *tree = (TTree*)f->Get("FragmentTree");

   TList *list = MakeTimeDiffSpec(tree);

   const char* name = f->GetName();

   TFile *outfile = new TFile(Form("mats_%s",name),"recreate");
   list->Write();

   return 0;

}

#endif






