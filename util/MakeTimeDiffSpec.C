

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

     TH1F *gg_diff_mod[4];
     TH1F *gg_diff_mod[0] = new TH1F("gg_diff_0","gg_diff_0",64000,-4000,60000); list->Add(gg_diff_mod[0]);
     TH1F *gg_diff_mod[1] = new TH1F("gg_diff_1","gg_diff_1",64000,-4000,60000); list->Add(gg_diff_mod[1]);
     TH1F *gg_diff_mod[2] = new TH1F("gg_diff_2","gg_diff_2",64000,-4000,60000); list->Add(gg_diff_mod[2]);
     TH1F *gg_diff_mod[3] = new TH1F("gg_diff_3","gg_diff_3",64000,-4000,60000); list->Add(gg_diff_mod[3]);


     TH2F *gg_diff_mat_same16 = new TH2F("gg_diff_mat_same16","gg_diff_mat_same16",10,0,10,600,0,600); list->Add(gg_diff_mat_same16);
     TH2F *gg_diff_mat_diff16 = new TH2F("gg_diff_mat_diff16","gg_diff_mat_diff16",10,0,10,600,0,600); list->Add(gg_diff_mat_diff16);


     TH1F *gb_diff = new TH1F("gb_diff","gb_diff",60000,-4000,60000); list->Add(gb_diff);
     TH1F *bg_diff = new TH1F("bg_diff","bg_diff",64000,-4000,60000); list->Add(bg_diff);
     TH1F *bb_diff = new TH1F("bb_diff","bb_diff",64000,-4000,60000); list->Add(bb_diff);

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
               
               if( ((currentFrag->ChannelNumber-1)/16) == ((oldFrag_gamma->ChannelNumber-1)/16) ) {
                  gg_diff_mod[((currentFrag->ChannelNumber-1)/16)]->Fill(currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
                  gg_diff_mat_same16->Fill(((currentFrag->ChannelNumber-1)/16), currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());
               }
               else
                  gg_diff_mat_diff16->Fill(((currentFrag->ChannelNumber-1)/16), currentFrag->GetTimeStamp() - oldFrag_gamma->GetTimeStamp());

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



