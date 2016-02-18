//Written by PC Bender


#include <TFile.h>
#include <TTree.h>
#include <TVirtualIndex.h>
#include <TTreeIndex.h>
#include <TH2.h>

#include "TFragment.h"
#include "TChannel.h"
#include <TGRSIRunInfo.h>

#include <vector>

TFile        *fCurrentFragFile = 0;
TTree        *fCurrentFragTree = 0;
TFragment    *fCurrentFragPtr  = 0;
TGRSIRunInfo *fCurrentRunInfo = 0;

const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

TH2F *cfdhist  = new TH2F("cfd_eng","cfd_eng",120,-20,100,4000,0,4000);
TH2F *timehist_walk = new TH2F("time_eng_walk","time_eng_walk",120,-20,100,4000,0,4000);
TH2F *timehist = new TH2F("time_eng","time_eng",120,-20,100,4000,0,4000);

TH2F *timehist_walk2 = new TH2F("eng_time_walk","eng_time_walk",4000,0,4000,120,-20,100);
TH2F *timehist2      = new TH2F("eng_teng","eng_time",4000,0,4000,120,-20,100);

void ProcessEvent(std::vector<TFragment*> *event) {
  if(event->size()<2)
     return;
  
  for(size_t x=0;x<event->size();x++) {
    for(size_t y=x+1;y<event->size();y++) {
      if( ((event->at(x)->GetEnergy()>1330.0) &&  (event->at(x)->GetEnergy()<1335.0)) || 
          ((event->at(y)->GetEnergy()>1330.0) &&  (event->at(y)->GetEnergy()<1335.0))  )  {
			int timediff      = abs(event->at(x)->GetTimeStampLow()-event->at(y)->GetTimeStampLow());
			float timediff_walk = fabs((float)(event->at(x)->GetTimeStamp()-event->at(y)->GetTimeStamp()));
			int cfddiff  = abs(event->at(x)->GetCfd(0)-event->at(y)->GetCfd(0));
			timehist->Fill(timediff,event->at(x)->GetEnergy());
			timehist->Fill(timediff,event->at(y)->GetEnergy());
			cfdhist->Fill(cfddiff,event->at(x)->GetEnergy());
			timehist_walk->Fill(timediff_walk,event->at(x)->GetEnergy());
			timehist_walk->Fill(timediff_walk,event->at(y)->GetEnergy());

			timehist2->Fill(event->at(x)->GetEnergy(),timediff);
			timehist2->Fill(event->at(y)->GetEnergy(),timediff);
			timehist_walk2->Fill(event->at(x)->GetEnergy(),timediff_walk);
			timehist_walk2->Fill(event->at(y)->GetEnergy(),timediff_walk);
      }
    }
  }
}

void WriteHist() {
  TFile file("junk.root","recreate");
  timehist->Write();
  timehist_walk->Write();
  timehist2->Write();
  timehist_walk2->Write();
  cfdhist->Write();
}

void InitChannels() {
   //Initializes the channels from a cal file on the command line when 
   //grsisort is started. If no cal file is input on the command line
   //grsisort attempts to read the calibration from the fragment tree
   //if it exists.
   
   if(!fCurrentFragTree)
      return;

   //Delete channels from memory incase there is something in there still
   TChannel::DeleteAllChannels(); 
   //Try to read the calibration data from the fragment tree
	//TChannel::ReadCalFromTree(fCurrentFragTree);
   TChannel::ReadCalFile("46Ca_walk.cal");

   printf("AnalysisTreeBuilder:  read in %i TChannels.\n", TChannel::GetNumberOfChannels());
}  

void SetupFragmentTree() {
   //Set up the fragment Tree to be sorted on time stamps or trigger Id's. This also reads the the run info out of the fragment tree.
   fCurrentFragFile = fCurrentFragTree->GetCurrentFile();
   fCurrentRunInfo  = (TGRSIRunInfo*)fCurrentFragFile->Get("TGRSIRunInfo");
   //if(fCurrentRunInfo) {
   //   TGRSIRunInfo::ReadInfoFromFile(fCurrentRunInfo);
      fCurrentRunInfo->Print();
   //}

   //Intialize the TChannel Information
   InitChannels();
   TChannel::ReadCalFromTree(fCurrentFragTree);

   //Set the sorting to be done on the timestamps if the fragment contains Griffin fragments
   if(fCurrentRunInfo->Griffin()) {
      fCurrentRunInfo->SetMajorIndex("TimeStampHigh");
      fCurrentRunInfo->SetMinorIndex("TimeStampLow");
   }

   //Check to see if the fragment tree already has an index set. If not build the index based on timestamps if it is a Griffin 
   //fragment. If it is not Griffin build based on the trigger Id.
   if(!fCurrentFragTree->GetTreeIndex()) {
      if(fCurrentRunInfo->MajorIndex().length()>0) {
         printf(DBLUE "Tree Index not found, building index on %s/%s..." RESET_COLOR,
                        fCurrentRunInfo->MajorIndex().c_str(),fCurrentRunInfo->MinorIndex().c_str());  fflush(stdout); 
         if(fCurrentRunInfo->MinorIndex().length()>0) {
            fCurrentFragTree->BuildIndex(fCurrentRunInfo->MajorIndex().c_str(),fCurrentRunInfo->MinorIndex().c_str());
         } else {
            fCurrentFragTree->BuildIndex(fCurrentRunInfo->MajorIndex().c_str());
         }
      } else {
         printf(DBLUE "Tree Index not found, building index on TriggerId/FragmentId..." RESET_COLOR);  fflush(stdout);   
         fCurrentFragTree->BuildIndex("TriggerId","FragmentId");
      }
      printf(DBLUE " done!" RESET_COLOR "\n");
   }

   //Set the branch to point at the Fragment Tree.
   TBranch *branch = fCurrentFragTree->GetBranch("TFragment");
   //Make the fCurrentFragPtr point at the Fragment Tree.
   branch->SetAddress(&fCurrentFragPtr);
   //Load the fragment tree into the MEM_SIZE (described in the header and above) into memory
   fCurrentFragTree->LoadBaskets(MEM_SIZE);   
   printf(DRED "\t MEM_SIZE = %zd" RESET_COLOR  "\n", MEM_SIZE);
   return;
}

int main(int argc, char **argv) {

   TFile file(argv[1]);
   fCurrentFragTree = (TTree*)file.Get("FragmentTree");
 

   SetupFragmentTree();


   TFragment *currentFrag = 0;

   //Find the TFragment cranch of the tree
   fCurrentFragTree->SetBranchAddress("TFragment",&currentFrag);
   //Get the tree index of the fragment tree
   TTreeIndex *index = (TTreeIndex*)fCurrentFragTree->GetTreeIndex();

   Int_t fFragmentsIn = 0;
   Int_t fEntries = index->GetN();
   Long64_t *indexvalues = index->GetIndex();
   //Set the major index to be sorted over as the high bits of the time stamp
   //int major_max = fCurrentFragTree->GetMaximum("TimeStampHigh");

   //Read in the first fragment from the fragment tree
   fCurrentFragTree->GetEntry(indexvalues[0]);
   long firstTimeStamp = currentFrag->GetTimeStamp();
   //int  firstDetectorType = currentFrag->DetectorType;

   //We set the buildevent flag to false by default. When the time gate closes we change this to true
   //to tell the code to build the event and send it to be written to the analysis tree.
   //bool buildevent = false;
   std::vector<TFragment*> *event = new std::vector<TFragment*>;//(1,*currentFrag);
   event->push_back(currentFrag);

   fFragmentsIn++; //Increment the number of fragments that have been read
   
   //std::vector<TFragment> event;
   //loop over all of the fragments in the tree 
   int x;
   for(x=1;x<fEntries;x++) {
      if(fCurrentFragTree->GetEntry(indexvalues[x]) == -1 ) {  //major,minor) == -1) {
         //GetEntry Reads all branches of entry and returns total number of bytes read.
         //This means that if this if statement passes, we have had an I/O error.
         printf(DRED "FIRE!!!" RESET_COLOR  "\n");
         continue;
      }
      if(indexvalues[x] == indexvalues[x-1]) {
         //If the last fragment index equals the current fragment index, you have a real fire
         //this is likely caused by using a root version where the method we are employing here
         //just so happens to not work. It is unlikely that there is a different reason for why
         //this is happening.
         printf(DRED "REAL FIRE!!! x = %i, indexvalues[x] = %lld, indexvalues[x-1] = %lld" RESET_COLOR  "\n", x, indexvalues[x], indexvalues[x-1]);
         continue;
      }
      fFragmentsIn++;//Now that we have read a new entry, we need to increment our counter

      //if we've already seen this channel we add the event to the queue
      //if(channelSeen.count(oldFrag->ChannelNumber) == 1) {
         //we might want to create an error statement here!!!
         //TEventQueue::Get()->Add(event);
         //event = new std::vector<TFragment>;
         //channelSeen.clear();


      //long timediff = currentFrag->GetTimeStamp() - firstTimeStamp;

      //We now set the "allowed" time windows for different detector streams to be called a coincidence
      //The way this is done right now is not correct and should be changed in the near future.
      //int currentDetectorType = currentFrag->DetectorType;


      if((currentFrag->GetTimeStamp() - firstTimeStamp) > 200) {  // 2 micro-sec.
        //printf( " x[%04i]  = %lu\n",x,timediff);
        ProcessEvent(event);
        event->clear();
        firstTimeStamp = currentFrag->GetTimeStamp();
      }

      event->push_back(currentFrag);
  
     if((x%15000)==0) {
        printf("   on %i / %i          \r",x,fEntries);
     }

   }
   printf("   on %i / %i          \n",x,fEntries);
   
   //Drop the TFragmentBranch from the Cache so we aren't still holding it in memory
   fCurrentFragTree->DropBranchFromCache(fCurrentFragTree->GetBranch("TFragment"),true);
   fCurrentFragTree->SetCacheSize(0);

   WriteHist();

   return 0;
}





