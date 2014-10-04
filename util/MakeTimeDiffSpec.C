

//Takes in a FargmentTree, makes CES time diff spectra
//
//




TList *MakeTimeDiffSpec(TTree *tree) {

   const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

   TList *list = new TList;
   
   TFragment *currentFrag = 0;
   TFragment *oldFrag_beta   = new TFragment; bool first_beta_set  = false;
   TFragment *oldFrag_gamma  = new TFragment; bool first_gamma_set = false;

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


     TH1F *gg_diff = new TH1F("gg_diff","gg_diff",64000,-4000,60000); list->Add(gg_diff);

     TH2F *gg_diff_mod[4];
     TH2F *gg_diff_mod[0] = new TH2F("gg_diff_0","gg_diff_0",4,0,4,600,0,600); list->Add(gg_diff_mod[0]);
     TH2F *gg_diff_mod[1] = new TH2F("gg_diff_1","gg_diff_1",4,0,4,600,0,600); list->Add(gg_diff_mod[1]);
     TH2F *gg_diff_mod[2] = new TH2F("gg_diff_2","gg_diff_2",4,0,4,600,0,600); list->Add(gg_diff_mod[2]);
     TH2F *gg_diff_mod[3] = new TH2F("gg_diff_3","gg_diff_3",4,0,4,600,0,600); list->Add(gg_diff_mod[3]);

     TH2F *gg_diff_ch[16];
     TH2F *gg_diff_ch[0] = new TH2F("gg_diff_ch_0","gg_diff_0",16,0,16,600,0,600); list->Add(gg_diff_ch[0]);
     TH2F *gg_diff_ch[1] = new TH2F("gg_diff_ch_1","gg_diff_1",16,0,16,600,0,600); list->Add(gg_diff_ch[1]);
     TH2F *gg_diff_ch[2] = new TH2F("gg_diff_ch_2","gg_diff_2",16,0,16,600,0,600); list->Add(gg_diff_ch[2]);
     TH2F *gg_diff_ch[3] = new TH2F("gg_diff_ch_3","gg_diff_3",16,0,16,600,0,600); list->Add(gg_diff_ch[3]);
     TH2F *gg_diff_ch[4] = new TH2F("gg_diff_ch_4","gg_diff_4",16,0,16,600,0,600); list->Add(gg_diff_ch[4]);
     TH2F *gg_diff_ch[5] = new TH2F("gg_diff_ch_5","gg_diff_5",16,0,16,600,0,600); list->Add(gg_diff_ch[5]);
     TH2F *gg_diff_ch[6] = new TH2F("gg_diff_ch_6","gg_diff_6",16,0,16,600,0,600); list->Add(gg_diff_ch[6]);
     TH2F *gg_diff_ch[7] = new TH2F("gg_diff_ch_7","gg_diff_7",16,0,16,600,0,600); list->Add(gg_diff_ch[7]);
     TH2F *gg_diff_ch[8] = new TH2F("gg_diff_ch_8","gg_diff_8",16,0,16,600,0,600); list->Add(gg_diff_ch[8]);
     TH2F *gg_diff_ch[9] = new TH2F("gg_diff_ch_9","gg_diff_9",16,0,16,600,0,600); list->Add(gg_diff_ch[9]);
     TH2F *gg_diff_ch[10] = new TH2F("gg_diff_ch_10","gg_diff_10",16,0,16,600,0,600); list->Add(gg_diff_ch[10]);
     TH2F *gg_diff_ch[11] = new TH2F("gg_diff_ch_11","gg_diff_11",16,0,16,600,0,600); list->Add(gg_diff_ch[11]);
     TH2F *gg_diff_ch[12] = new TH2F("gg_diff_ch_12","gg_diff_12",16,0,16,600,0,600); list->Add(gg_diff_ch[12]);
     TH2F *gg_diff_ch[13] = new TH2F("gg_diff_ch_13","gg_diff_13",16,0,16,600,0,600); list->Add(gg_diff_ch[13]);
     TH2F *gg_diff_ch[14] = new TH2F("gg_diff_ch_14","gg_diff_14",16,0,16,600,0,600); list->Add(gg_diff_ch[14]);
     TH2F *gg_diff_ch[15] = new TH2F("gg_diff_ch_15","gg_diff_15",16,0,16,600,0,600); list->Add(gg_diff_ch[15]);

     TH2F *gg_diff_mat_same16 = new TH2F("gg_diff_mat_same16","gg_diff_mat_same16",10,0,10,600,0,600); list->Add(gg_diff_mat_same16);
     TH2F *gg_diff_mat_diff16 = new TH2F("gg_diff_mat_diff16","gg_diff_mat_diff16",10,0,10,600,0,600); list->Add(gg_diff_mat_diff16);
     TH2F *gg_diff_mat_diffrev16 = new TH2F("gg_diff_mat_diffrev16","gg_diff_mat_diffrev16",10,0,10,600,0,600); list->Add(gg_diff_mat_diffrev16);

     TH2F *gg_diff_mat_same16_ch = new TH2F("gg_diff_mat_same16_ch","gg_diff_mat_same16_ch",65,0,65,600,0,600); list->Add(gg_diff_mat_same16_ch);
     TH2F *gg_diff_mat_diff16_ch = new TH2F("gg_diff_mat_diff16_ch","gg_diff_mat_diff16_ch",65,0,65,600,0,600); list->Add(gg_diff_mat_diff16_ch);
     TH2F *gg_diff_mat_diffrev16_ch = new TH2F("gg_diff_mat_diffrev16_ch","gg_diff_mat_diffrev16_ch",65,0,65,600,0,600); list->Add(gg_diff_mat_diffrev16_ch);

     TH1F *gb_diff = new TH1F("gb_diff","gb_diff",60000,-4000,60000); list->Add(gb_diff);
     TH1F *bg_diff = new TH1F("bg_diff","bg_diff",64000,-4000,60000); list->Add(bg_diff);
     TH1F *bb_diff = new TH1F("bb_diff","bb_diff",64000,-4000,60000); list->Add(bb_diff);

     TH2F *gg_diff_E1 = new TH2F("gg_diff_E1","gg time difference of card 3&1 vs energy of card 1",4000,0.,4000.,600,0.,600.); list->Add(gg_diff_E1);
     TH2F *gg_diff_E3 = new TH2F("gg_diff_E3","gg time difference of card 3&1 vs energy of card 3",4000,0.,4000.,600,0.,600.); list->Add(gg_diff_E3);

     int FragsIn = 0;

     tree->GetEntry(indexvalues[0]);
     FragsIn++;

     for(int x=1;x<fEntries;x++) {
         if(currentFrag->DetectorType==1) {
            if(!first_gamma_set)
               printf("\n\n first gamma found on event %i\n\n",x);
            first_gamma_set = true;
            *oldFrag_gamma = *currentFrag;
         }   
         else if(currentFrag->DetectorType==2) {
            if(!first_beta_set)
               printf("\n\n first beta found on event %i\n\n",x);
            first_beta_set = true;
            *oldFrag_beta = *currentFrag;
         }
         if(tree->GetEntry(indexvalues[x]) == -1 ) {  //major,minor) == -1) {
            printf( "FIRE!!!" "\n");
            continue;
         } 
         FragsIn++;
         
         int cur_det = currentFrag->DetectorType;

         if(cur_det==1) {
            if(first_gamma_set) {
               gg_diff->Fill(currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
               
               gg_diff_mod[((currentFrag->ChannelNumber-1)/16)]->Fill(((oldFrag_gamma->ChannelNumber-1)/16), currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
               if( ((currentFrag->ChannelNumber-1)/16) == ((oldFrag_gamma->ChannelNumber-1)/16) ) {
                  gg_diff_mat_same16->Fill(((currentFrag->ChannelNumber-1)/16), currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
                  gg_diff_mat_same16_ch->Fill(currentFrag->ChannelNumber, currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
               }
               else{

                  gg_diff_mat_diff16->Fill(((currentFrag->ChannelNumber-1)/16), currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
                  gg_diff_mat_diffrev16->Fill(((oldFrag_gamma->ChannelNumber-1)/16), currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
                  
                  gg_diff_mat_diff16_ch->Fill(currentFrag->ChannelNumber, currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
                  gg_diff_mat_diffrev16_ch->Fill(oldFrag_gamma->ChannelNumber, currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
               }
               if( ((currentFrag->ChannelNumber-1)/16) == 3 && ((oldFrag_gamma->ChannelNumber-1)/16) == 1 ) {
                  gg_diff_ch[(currentFrag->ChannelNumber-1)%16]->Fill((oldFrag_gamma->ChannelNumber-1)%16, currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
                  gg_diff_E1->Fill(oldFrag_gamma->GetEnergy(), currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
                  gg_diff_E3->Fill(currentFrag->GetEnergy(), currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
               }
            }
            if(first_beta_set)
               gb_diff->Fill(currentFrag->GetTimeStamp() - oldFrag_beta->GetTimeStamp());
          } else if(cur_det ==2) {
            if(first_gamma_set) 
               bg_diff->Fill(currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
            if(first_beta_set)
               bb_diff->Fill(currentFrag->GetTimeStamp() - oldFrag_beta->GetTimeStamp());
         }
         if(x%1500==0)
            printf("\tOn fragment %i/%i               \r",x,fEntries);
      }
      printf("\n\n");
  
  return list;
}



