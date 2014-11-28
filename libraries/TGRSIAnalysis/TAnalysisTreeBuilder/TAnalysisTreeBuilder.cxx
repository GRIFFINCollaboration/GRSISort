
#include "Globals.h"

#include <TVirtualIndex.h>
#include <TTreeIndex.h>
#include <TSystem.h>

#include "TAnalysisTreeBuilder.h"
#include "TGRSIOptions.h"

#include <TStopwatch.h>

#include <TGRSIOptions.h>

//#include "TSharcData.h"


std::mutex TEventQueue::m_event;
std::mutex TWriteQueue::m_write;

bool TEventQueue::elock = false;
std::queue<std::vector<TFragment>*> TEventQueue::fEventQueue;
TEventQueue *TEventQueue::fPtrToQue = 0;

TEventQueue::TEventQueue() { }

TEventQueue::~TEventQueue() { }

TEventQueue *TEventQueue::Get() {
   if(!fPtrToQue)
      fPtrToQue = new TEventQueue;
   return fPtrToQue;
}

void TEventQueue::Add(std::vector<TFragment> *event) {
   //while(elock) { }
   //SetLock();
   m_event.lock();
   fEventQueue.push(event);
   m_event.unlock();
   //UnsetLock();
   return;
}

std::vector<TFragment> *TEventQueue::PopEntry() {
   std::vector<TFragment> *temp;
   //while(elock) { }
   //SetLock();
   m_event.lock();
   temp = fEventQueue.front();
   fEventQueue.pop();
   m_event.unlock();
   //UnsetLock();
   return temp;
}

int TEventQueue::Size() {
   int temp;
   //while(elock) { }
   //SetLock();
   m_event.lock();
   temp = fEventQueue.size();
   //UnsetLock();
   m_event.unlock();
   return temp;
}

bool TWriteQueue::wlock = false;
std::queue<std::map<const char*, TGRSIDetector*>*> TWriteQueue::fWriteQueue;
TWriteQueue *TWriteQueue::fPtrToQue = 0;

TWriteQueue::TWriteQueue() { }

TWriteQueue::~TWriteQueue() { }

TWriteQueue *TWriteQueue::Get() {
   if(!fPtrToQue)
      fPtrToQue = new TWriteQueue;
   return fPtrToQue;
}

void TWriteQueue::Add(std::map<const char*, TGRSIDetector*> *event) {
   //while(wlock) { }
   //SetLock();
   m_write.lock();
   fWriteQueue.push(event);
   //UnsetLock();
   m_write.unlock();
   return;
}

std::map<const char*, TGRSIDetector*> *TWriteQueue::PopEntry() {
   std::map<const char*, TGRSIDetector*> *temp;
   //while(wlock) { }
   //SetLock();
   m_write.lock();
   temp = fWriteQueue.front();
   fWriteQueue.pop();
   //UnsetLock();
   m_write.unlock();
   return temp;
}

int TWriteQueue::Size() {
   int temp;
   //while(wlock) { }
   //SetLock();
   m_write.lock();
   temp = fWriteQueue.size();
   //UnsetLock();
   m_write.unlock();
   return temp;
}

///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///


ClassImp(TAnalysisTreeBuilder)


const size_t TAnalysisTreeBuilder::MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

TAnalysisTreeBuilder* TAnalysisTreeBuilder::fAnalysisTreeBuilder = 0;


long TAnalysisTreeBuilder::fEntries     = 0;
int  TAnalysisTreeBuilder::fFragmentsIn = 0;
int  TAnalysisTreeBuilder::fAnalysisIn  = 0;
int  TAnalysisTreeBuilder::fAnalysisOut = 0;


TAnalysisTreeBuilder* TAnalysisTreeBuilder::Get() {
   if(fAnalysisTreeBuilder == 0)
      fAnalysisTreeBuilder = new TAnalysisTreeBuilder;
   return fAnalysisTreeBuilder;
}

TAnalysisTreeBuilder::TAnalysisTreeBuilder() {
   fFragmentChain = 0;
   fCurrentFragTree = 0;
   fCurrentFragFile = 0;
   fCurrentAnalysisTree = 0;
   fCurrentAnalysisFile = 0;
   fCurrentRunInfo = 0;

   fCurrentFragPtr = 0;

   tigress = 0;//new TTigress;
   sharc = 0;//new TSharc;
   triFoil = 0;//new TTriFoil;
   //rf->Clear();
   csm = 0;//new TCSM;
   //spice->Clear(); s3->Clear();
   //tip->Clear();

   griffin = 0;//new TGriffin;
   sceptar = 0;//new TSceptar;
   paces   = 0;//new TPaces;
   descant = 0;//new TDescant;
   //dante->Clear();
   //zerodegree->Clear();

}


TAnalysisTreeBuilder::~TAnalysisTreeBuilder() { }

void TAnalysisTreeBuilder::StartMakeAnalysisTree(int argc, char** argv) {
   if(argc==1) {
      SetUpFragmentChain(TGRSIOptions::GetInputRoot());
   } else {
      return;
   }
   SortFragmentChain();   

}


void TAnalysisTreeBuilder::InitChannels() {
   
   if(!fCurrentFragTree)
      return;

   TChannel::DeleteAllChannels(); 
   /*TList *list = fCurrentFragTree->GetUserInfo();
   TIter iter(list);
   while(TObject *obj = iter.Next()) {
      if(!obj->InheritsFrom("TChannel"))
         continue;
      TChannel *chan = (TChannel*)obj;
      int number = chan->GetUserInfoNumber(); // I should need to do this.. 
      chan->SetNumber(number);
      TChannel::AddChannel(chan,"save");
      //chan->Print();
   }*/
	TChannel::ReadCalFromTree(fCurrentFragTree);

   if(!TGRSIOptions::GetInputCal().empty()) {
      for(int x = 0;x<TGRSIOptions::GetInputCal().size();x++) {
         TChannel::ReadCalFile(TGRSIOptions::GetInputCal().at(x).c_str());
      }
   }
   printf("AnalysisTreeBuilder:  read in %i TChannels.\n", TChannel::GetNumberOfChannels());
}  



void TAnalysisTreeBuilder::SetUpFragmentChain(std::vector<std::string> infiles) {
   TChain *chain = new TChain("FragmentTree");
   for(int x=0;x<infiles.size();x++) 
      chain->Add(infiles.at(x).c_str());
   SetUpFragmentChain(chain);
   
}

void TAnalysisTreeBuilder::SetUpFragmentChain(TChain *chain) {
   if(fFragmentChain)
      delete fFragmentChain;
   fFragmentChain = chain;
   fFragmentChain->CanDeleteRefs(true); 
}

void TAnalysisTreeBuilder::SortFragmentChain() {
   if(!fFragmentChain)
      return;
   
   int ntrees = fFragmentChain->GetNtrees();
   int nChainEntries = fFragmentChain->GetEntries();
   int treeNumber, lastTreeNumber = -1;

   printf("Found %i trees with %i total fragments.\n",ntrees,nChainEntries);

   for(int i=0;i<nChainEntries;i++) {
      fFragmentChain->LoadTree(i);
      treeNumber = fFragmentChain->GetTreeNumber();
      if(treeNumber != lastTreeNumber) {
         if(lastTreeNumber == -1) {
            printf(DYELLOW "Sorting tree[%d]: %s" RESET_COLOR "\n",treeNumber+1,fFragmentChain->GetTree()->GetCurrentFile()->GetName());
         } else {
            printf(DYELLOW "Changing to tree[%d/%d]: %s" RESET_COLOR "\n",treeNumber+1,ntrees,fFragmentChain->GetTree()->GetCurrentFile()->GetName());
            printf(DYELLOW "    Switched from tree[%d] at chain entry number %i" RESET_COLOR "\n",lastTreeNumber,i);
         }
         lastTreeNumber = treeNumber;
      }  else {
         continue;
      }
      fCurrentFragTree = fFragmentChain->GetTree();
      int nentries = fCurrentFragTree->GetEntries();
      SetupFragmentTree();
      SetupOutFile();
      SetupAnalysisTree();

      //printf("==== creating process and write thread ====\n");
      fSortFragmentDone = false;
      fPrintStatus = true;
      fProcessThread = new std::thread(&TAnalysisTreeBuilder::ProcessEvent,this);
      fWriteThread = new std::thread(&TAnalysisTreeBuilder::WriteAnalysisTree,this);
      fStatusThread = new std::thread(&TAnalysisTreeBuilder::Status,this);

      //if(fCurrentRunInfo->MajorIndex().length()>0 && fCurrentRunInfo->MajorIndex().compare("TimeStampHigh")==0) 
      fFragmentsIn = 0;
      fAnalysisIn  = 0;
      fAnalysisOut = 0;
      if(fCurrentRunInfo->Griffin())
         SortFragmentTreeByTimeStamp();
      else
         SortFragmentTree();
      fSortFragmentDone = true;

      fProcessThread->join();
      fWriteThread->join();
      fPrintStatus = false;
      //fStatusThread->join();
      
      CloseAnalysisFile();
      printf("\n");
      i+=(nentries);//-10);
      fCurrentFragFile->Close("r");
   }
   printf("Finished chain sort.\n");
   return;
}

void TAnalysisTreeBuilder::SortFragmentTree() {

   long major_min = (long) fCurrentFragTree->GetMinimum(fCurrentFragTree->GetTreeIndex()->GetMajorName());
   if(major_min<0)
      major_min = 0;
   long major_max = (long) fCurrentFragTree->GetMaximum(fCurrentFragTree->GetTreeIndex()->GetMajorName());

   for(int j=major_min;j<=major_max;j++) {
      std::vector<TFragment> *event = new std::vector<TFragment>;
      int fragno = 1;
      while(fCurrentFragTree->GetEntryWithIndex(j,fragno++) != -1) {
         fFragmentsIn++;
         event->push_back(*fCurrentFragPtr);
      }
      if(event->empty()) {
         delete event;
      } else {
         TEventQueue::Get()->Add(event);
      }
      //if((j%10000)==0 || j==major_max) {
         //printf("\tprocessing event " CYAN "%i " RESET_COLOR "/" DBLUE " %li" RESET_COLOR "         \r",j,major_max);
      //}
   }
   printf("\n");
   fCurrentFragTree->DropBranchFromCache(fCurrentFragTree->GetBranch("TFragment"),true);
   fCurrentFragTree->SetCacheSize(0);
   return;
}


void TAnalysisTreeBuilder::SortFragmentTreeByTimeStamp() {

   TFragment *currentFrag = 0;
   //TFragment *oldFrag = new TFragment;

   fCurrentFragTree->SetBranchAddress("TFragment",&currentFrag);

   TTreeIndex *index = (TTreeIndex*)fCurrentFragTree->GetTreeIndex();

   fEntries = index->GetN();
   Long64_t *indexvalues = index->GetIndex();
   int major_max = fCurrentFragTree->GetMaximum("TimeStampHigh");

   fCurrentFragTree->GetEntry(indexvalues[0]);
   //*oldFrag = *currentFrag;
   long firstTimeStamp = currentFrag->GetTimeStamp();
   std::vector<TFragment> *event = new std::vector<TFragment>;//(1,*currentFrag);
   event->push_back(*currentFrag);

   fFragmentsIn++;
   
   //std::set<int> channelSeen;
   for(int x=1;x<fEntries;x++) {
      if(fCurrentFragTree->GetEntry(indexvalues[x]) == -1 ) {  //major,minor) == -1) {
         printf(DRED "FIRE!!!" RESET_COLOR  "\n");
         continue;
      }
      if(indexvalues[x] == indexvalues[x-1]) {
         printf(DRED "REAL FIRE!!! x = %i, indexvalues[x] = %lld, indexvalues[x-1] = %lld" RESET_COLOR  "\n", x, indexvalues[x], indexvalues[x-1]);
         //printf("currentFrag->MidasId = %i   oldFrag->MidasId = %i\n",currentFrag->MidasId, oldFrag->MidasId);
         continue;
      }
      fFragmentsIn++;
      //if we've already seen this channel we add the event to the queue
      //if(channelSeen.count(oldFrag->ChannelNumber) == 1) {
         //we might want to create an error statement here!!!
         //TEventQueue::Get()->Add(event);
         //event = new std::vector<TFragment>;
         //channelSeen.clear();
      //}
      //event->push_back(*oldFrag);
      //channelSeen.insert(oldFrag->ChannelNumber);
      //printf("\ntime diff = %ld\n",abs(oldFrag->GetTimeStamp()-currentFrag->GetTimeStamp()));
      if(abs(currentFrag->GetTimeStamp() - firstTimeStamp) > 200) {  // 2 micro-sec.
         //printf("Adding %ld fragments to queue\n",event->size());
         //if(event->size() > 1) {
            //for(int i = 0; i < event->size(); ++i) {
               //event->at(i).Print();
            //}
         //}
         TEventQueue::Get()->Add(event);
         event = new std::vector<TFragment>;//(1,*currentFrag);
         event->push_back(*currentFrag);
         //channelSeen.clear();
         firstTimeStamp = currentFrag->GetTimeStamp();
      } else {
         event->push_back(*currentFrag);
      }
      //*oldFrag = *currentFrag;
      //if((x%10000)==0 ) {
         //printf("Reading %lld bytes in %d transactions\n",fCurrentFragFile->GetBytesRead(),  fCurrentFragFile->GetReadCalls());
         //printf("\tprocessing fragment " CYAN "%i " RESET_COLOR "/" DBLUE " %li" RESET_COLOR "         \r",x,fEntries);
      //}
      //Print();
   }
   //in case we have fragments left, we add them to the queue now
   if(event->size() > 0) {
      TEventQueue::Get()->Add(event);
   }
   
   fCurrentFragTree->DropBranchFromCache(fCurrentFragTree->GetBranch("TFragment"),true);
   fCurrentFragTree->SetCacheSize(0);
   return;
}







void TAnalysisTreeBuilder::SetupFragmentTree() {

   fCurrentFragFile = fCurrentFragTree->GetCurrentFile();
   fCurrentRunInfo  = (TGRSIRunInfo*)fCurrentFragFile->Get("TGRSIRunInfo");
   if(fCurrentRunInfo) {
      TGRSIRunInfo::SetInfoFromFile(fCurrentRunInfo);
      //TGRSIRunInfo::SetPaces(false);
      //TGRSIRunInfo::SetDescant(false);
      fCurrentRunInfo->Print();
   }

   InitChannels();

   if(fCurrentRunInfo->Griffin()) {
      fCurrentRunInfo->SetMajorIndex("TimeStampHigh");
      fCurrentRunInfo->SetMinorIndex("TimeStampLow");
   }


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

   TBranch *branch = fCurrentFragTree->GetBranch("TFragment");
   branch->SetAddress(&fCurrentFragPtr);
   fCurrentFragTree->LoadBaskets(MEM_SIZE);   
   printf(DRED "\t MEM_SIZE = %zd" RESET_COLOR  "\n", MEM_SIZE);
   return;
}

void TAnalysisTreeBuilder::SetupOutFile() {
   if(!fCurrentRunInfo)
      return;
   std::string outfilename = Form("analysis%05i_%03i.root",fCurrentRunInfo->RunNumber(),fCurrentRunInfo->SubRunNumber());
   TGRSIOptions::AddInputRootFile(outfilename);
   if(fCurrentAnalysisFile)
      delete fCurrentAnalysisFile;
   fCurrentAnalysisFile = new TFile(outfilename.c_str(),"recreate");
   fCurrentAnalysisFile->SetCompressionSettings(1);
   printf("created output file: %s\n",fCurrentAnalysisFile->GetName());
   return;
}

void TAnalysisTreeBuilder::SetupAnalysisTree() { 
   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;
   fCurrentAnalysisFile->cd();
   if(fCurrentAnalysisTree)
      delete fCurrentAnalysisTree;
   fCurrentAnalysisTree = new TTree("AnalysisTree","AnalysisTree");

   TGRSIRunInfo *info = fCurrentRunInfo;
   TTree *tree = fCurrentAnalysisTree;

   int basketSize = 128000; //128000;
   if(info->Tigress())   { tree->Bronch("TTigress","TTigress",&tigress); }//, basketSize); } 
   if(info->Sharc())     { tree->Bronch("TSharc","TSharc",&sharc); }//, basketSize); } 
   if(info->TriFoil())   { tree->Bronch("TTriFoil","TTriFoil",&triFoil); }//, basketSize); } 
   //if(info->Rf())        { tree->Bronch("TRf","TRf",&rf); }//, basketSize); } 
   if(info->CSM())       { tree->Bronch("TCSM","TCSM",&csm); }//, basketSize); } 
   //if(info->Spice())     { tree->Bronch("TSpice","TSpice",&spice); }//, basketSize); tree->SetBronch("TS3","TS3",&s3); }//, basketSize); } 
   //if(info->Tip())       { tree->Bronch("TTip","TTip",&tip); }//, basketSize); } 

   if(info->Griffin())   { TBranch *branch = tree->Bronch("TGriffin","TGriffin",&griffin, basketSize, 99);}// branch->SetAddress(0);} 
   if(info->Sceptar())   { TBranch *branch = tree->Bronch("TSceptar","TSceptar",&sceptar, basketSize, 99);}// branch->SetAddress(0);} 
   if(info->Paces())     { tree->Bronch("TPaces","TPaces",&paces, basketSize,99); }//, basketSize); } 
   //if(info->Dante())     { tree->Bronch("TDante","TDante",&dante); }//, basketSize); } 
   //if(info->ZeroDegree()){ tree->Bronch("TZeroDegree","TZeroDegree",&zerodegree); }//, basketSize); } 
   if(info->Descant())   { tree->Bronch("TDescant","TDescant",&descant); }//, basketSize);

   //tree->SetAutoFlush(-300000000);
   //tree->SetCacheSizeAux(true);

   //tree->SetAutoFlush(-500000000);
   //tree->SetCacheSize();
   //tree->AddBranchToCache("*",true);
   //tree->SetAutoSave(10000000);
   return;  
}

void TAnalysisTreeBuilder::ClearActiveAnalysisTreeBranches() {

   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;
   TGRSIRunInfo *info = fCurrentRunInfo;
   TTree *tree = fCurrentAnalysisTree;

   if(info->Tigress())   { tigress = 0; }//->Clear(); }
   if(info->Sharc())     { sharc = 0; }//->Clear(); }
   if(info->TriFoil())   { triFoil = 0; }//->Clear(); }
   //if(info->Rf())        { rf->Clear(); } 
   if(info->CSM())       { csm = 0; }//->Clear(); }
   //if(info->Spice())     { spice->Clear(); s3->Clear(); } 
   //if(info->Tip())       { tip->Clear(); } 
//printf("clearing griffin 0x08%x\n",griffin);
//griffin->Print();
   if(info->Griffin())   { griffin = 0; }//->Clear(); }
   if(info->Sceptar())   { sceptar = 0; }//->Clear(); }
   if(info->Paces())     { paces = 0; }//->Clear(); } 
   //if(info->Dante())     { dante->Clear(); } 
   //if(info->ZeroDegree()){ zerodegree->Clear(); } 
   if(info->Descant())   { descant = 0; }//->Clear();}
   //printf("ClearActiveAnalysisTreeBranches done\n");
}

void TAnalysisTreeBuilder::BuildActiveAnalysisTreeBranches(std::map<const char*, TGRSIDetector*> *detectors) {

   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;
   TGRSIRunInfo *info = fCurrentRunInfo;
   TTree *tree = fCurrentAnalysisTree;

   for(auto det = detectors->begin(); det != detectors->end(); det++) {
      det->second->BuildHits();
   }
}

void TAnalysisTreeBuilder::FillWriteQueue(std::map<const char*, TGRSIDetector*> *detectors) {
   fAnalysisIn++;
   TWriteQueue::Get()->Add(detectors);
}

void TAnalysisTreeBuilder::WriteAnalysisTree() {
   //TStopwatch w;
   int counter = 0;
   //w.Start();
   while(TWriteQueue::Size() > 0 || TEventQueue::Size() > 0 || !fSortFragmentDone) {
      if(TWriteQueue::Size() == 0) {
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
         //printf("WriteAnalysisTree: no events in write queue or so %i\n",counter++);
         continue;
      }
      std::map<const char*, TGRSIDetector*> *detectors = TWriteQueue::PopEntry();
      fAnalysisOut++;
      //if(fAnalysisOut==1000) {
         //printf("\n\n");
         //fCurrentAnalysisTree->OptimizeBaskets((Long64_t)1000000000,1.1);//,"d");
         //printf("\n\n");
      //}   
      FillAnalysisTree(detectors);
      //if(fSortFragmentDone && TEventQueue::Size() == 0 && TWriteQueue::Size()%10000==0) {
         //printf(DYELLOW HIDE_CURSOR " \t%12i " RESET_COLOR "/"
                //DBLUE " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR
                //"     write queue size / # of events / events written.\t\t%f seconds." SHOW_CURSOR "\r",
                //TWriteQueue::Size(),fAnalysisIn,fAnalysisOut,w.RealTime());
         //w.Continue(); 
         //Print();
      //}
   }
   //printf(DYELLOW HIDE_CURSOR " \t%12i " RESET_COLOR "/"
          //DBLUE " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR
          //"     write queue size / # of events / events written.\t\t%f seconds." SHOW_CURSOR "\n",
          //TWriteQueue::Size(),fAnalysisIn,fAnalysisOut,w.RealTime());
   //printf(RED "\t\t\t\t  WRITE QUE STOPPED." RESET_COLOR  "\n" );
}

void TAnalysisTreeBuilder::FillAnalysisTree(std::map<const char*, TGRSIDetector*> *detectors) {
   if(!fCurrentAnalysisTree || !detectors) {
      printf("returned from fill without filling (%p %p)!\n",fCurrentAnalysisTree, detectors);   
      return;
   }   
//printf("filling analysis tree with %lu detectors\n",detectors->size());
   
   // clear branches
   ClearActiveAnalysisTreeBranches();	

   for(auto det = detectors->begin(); det != detectors->end(); det++) {
      if(strcmp(det->first,"TI") == 0) {
      //if(strcmp(det->second->IsA()->GetName(),"TTigress") == 0) {
         tigress = (TTigress*) det->second;
      } else if(strcmp(det->first,"SH") == 0) {
      //} else if(strcmp(det->second->IsA()->GetName(),"TSharc") == 0) {
         sharc = (TSharc*) det->second;
      } else if(strcmp(det->first,"TR") == 0) {
      //} else if(strcmp(det->second->IsA()->GetName(),"TTriFoil") == 0) {
         triFoil = (TTriFoil*) det->second;
      //} else if(strcmp(det->second->IsA()->GetName(),"TRf") == 0) {
         //*rf = *((TRf*) det->second);
      } else if(strcmp(det->first,"CS") == 0) {
      //} else if(strcmp(det->second->IsA()->GetName(),"TCSM") == 0) {
         csm = (TCSM*) det->second;
      //} else if(strcmp(det->second->IsA()->GetName(),"TSpice") == 0) {
         //*spice = *((TSpice*) det->second);
      //} else if(strcmp(det->second->IsA()->GetName(),"TTip") == 0) {
         //*tip = *((TTip*) det->second);
      } else if(strcmp(det->first,"GR") == 0) {
      //} else if(strcmp(det->second->IsA()->GetName(),"TGriffin") == 0) {
         griffin = (TGriffin*) det->second;
         //printf(CYAN "Got GRIFFIN detector: %ld hits" RESET_COLOR "\n",griffin->GetMultiplicity());
      } else if(strcmp(det->first,"SE") == 0) {
      //} else if(strcmp(det->second->IsA()->GetName(),"TSceptar") == 0) {
         sceptar = (TSceptar*) det->second;
      } else if(strcmp(det->first,"DS") == 0) {
         descant = (TDescant*) det->second;
     // } else if(strcmp(det->second->IsA()->GetName(),"TPaces") == 0) {
      } else if(strcmp(det->first,"PA") == 0) {
         paces = (TPaces*) det->second;
      } 
   }
   //time1 += w1.RealTime();
   //w1.Reset();
   //w1.Start();
   fCurrentAnalysisTree->Fill();
   //time2 += w1.RealTime();
   //counter++;
   
   for(auto det = detectors->begin(); det != detectors->end(); det++) {
      delete det->second;
      det->second = 0;
   }
   delete detectors;
   detectors = 0;
   //if(counter>10000) {
   //   printf("\n\ntime1 = %.04f    |  time2 = %.04f\n\n",time1,time2);
   //   fflush(stdout);
   //   time1 = 0.0; time2 = 0.0; counter = 0;
   //}
   return;
}

void TAnalysisTreeBuilder::CloseAnalysisFile() {
   if(!fCurrentAnalysisFile)
      return;

   ///******************************////
   ///******************************////
   // this will be removed and put into a seperate thread later.

   printf("Writing file %s\n",fCurrentAnalysisFile->GetName());
   ///******************************////
   ///******************************////

   fCurrentAnalysisFile->cd();
   if(fCurrentAnalysisTree)
      fCurrentAnalysisTree->Write();
   fCurrentAnalysisFile->Close();

   fCurrentAnalysisTree = 0;
   fCurrentAnalysisFile = 0;
   return;
}


void TAnalysisTreeBuilder::ProcessEvent() {
//printf("\nSTARTING PROCESSEVENT\n");
   //TStopwatch w;
   int counter = 0;
   //w.Start();
   while(TEventQueue::Size() > 0 || !fSortFragmentDone) {
      if(TEventQueue::Size() == 0) { 
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
         //printf("ProcessEvent: no events in event queue %i\n",counter++);
         continue; 
      }
      std::vector<TFragment> *event = TEventQueue::PopEntry();
      //printf(DYELLOW "\n======================================== %ld" RESET_COLOR "\n",event->size());
      MNEMONIC mnemonic;
      std::map<const char*, TGRSIDetector*> *detectors = new std::map<const char*, TGRSIDetector*>;
      //std::vector<TGRSIDetector*> *detectors = new std::vector<TGRSIDetector*>;
      //TStopwatch w;
      //w.Start();
      for(int i=0;i<event->size();i++) {
         //printf("ChannelAddress =0x%08x\t",event->at(i).ChannelAddress);
      
         TChannel *channel = TChannel::GetChannel(event->at(i).ChannelAddress);
         //printf("name: %s \n",channel->GetChannelName());
      
         //event->at(i).Print();
         if(!channel)
            continue;
         ClearMNEMONIC(&mnemonic);
         ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
         
         //PrintMNEMONIC(&mnemonic);
         //channel->Print();
         //printf("ChannelName = %s\n",channel->GetChannelName());
      
         if(mnemonic.system.compare("TI")==0) {
            //detectors->push_back(new TTigress);
            if(detectors->find("TI") == detectors->end()) {
               (*detectors)["TI"] = new TTigress;
            }
            (*detectors)["TI"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("SH")==0) {
            //detectors->push_back(new TSharc);
            if(detectors->find("SH") == detectors->end()) {
               (*detectors)["SH"] = new TSharc;
            }
            (*detectors)["SH"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("TR")==0) {	
            //detectors->push_back(new TTriFoil);
            if(detectors->find("TR") == detectors->end()) {
               (*detectors)["TR"] = new TTriFoil;
            }
            (*detectors)["TR"]->FillData(&(event->at(i)),channel,&mnemonic);
          //else if(mnemonic.system.compare("RF")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("CS")==0) {	
            //detectors->push_back(new TCSM);
            if(detectors->find("RF") == detectors->end()) {
               (*detectors)["RF"] = new TCSM;
            }
            (*detectors)["RF"]->FillData(&(event->at(i)),channel,&mnemonic);
         //} else if(mnemonic.system.compare("SP")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("GR")==0) {	
            //printf("Found a GRIFFIN detector\n");
            //detectors->push_back(new TGriffin);
            if(detectors->find("GR") == detectors->end()) {
               (*detectors)["GR"] = new TGriffin;
            }
            (*detectors)["GR"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("SE")==0) {	
            //detectors->push_back(new TSceptar);
            if(detectors->find("SE") == detectors->end()) {
               (*detectors)["SE"] = new TSceptar;
            }
            (*detectors)["SE"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("PA")==0) {	
            //detectors->push_back(new TPaces);
            if(detectors->find("PA") == detectors->end()) {
               (*detectors)["PA"] = new TPaces;
            }
            (*detectors)["PA"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("DS")==0) {	
            if(detectors->find("DS") == detectors->end()) {
               (*detectors)["DS"] = new TDescant;
            }
            (*detectors)["DS"]->FillData(&(event->at(i)),channel,&mnemonic);
         //else if(mnemonic.system.compare("PA")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         //} else if(mnemonic.system.compare("DA")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         //} else if(mnemonic.system.compare("ZD")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         //} else if(mnemonic.system.compare("DE")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         }
      }

      if(!detectors->empty()) {
         //printf(DYELLOW "\t BUILDING " RESET_COLOR "\n");
         BuildActiveAnalysisTreeBranches(detectors);
         //printf(DYELLOW "\t WRITING " RESET_COLOR "\n");
         FillWriteQueue(detectors);
         //printf(DYELLOW "\t CLEARING " RESET_COLOR "\n");
         //ClearActiveAnalysisTreeBranches();	
         //printf(DYELLOW "\t DONE " RESET_COLOR "\n");
      }
      //printf(DRED "\n----------------------------------------" RESET_COLOR "\n");

      delete event;
   
      //if(fSortFragmentDone && TEventQueue::Size()%10000==0) {
         //printf(DYELLOW HIDE_CURSOR " \t%12i " RESET_COLOR "/"
                //DBLUE " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR
                //"     event queue size / # of events / events written.\t\t%f seconds." SHOW_CURSOR "\r",
                //TEventQueue::Size(),fAnalysisIn,fAnalysisOut,w.RealTime());
         //w.Continue(); 
      //}
   }
   //printf(DYELLOW HIDE_CURSOR " \t%12i " RESET_COLOR "/"
          //DBLUE " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR
          //"     event queue size / # of events / events written.\t\t%f seconds." SHOW_CURSOR "\n",
          //TEventQueue::Size(),fAnalysisIn,fAnalysisOut,w.RealT  ime());
  //printf(RED "\t\t\t\t  PROCESS QUE STOPPED." RESET_COLOR  "\n" );
}

void TAnalysisTreeBuilder::Print(Option_t *opt) {
   if(fCurrentFragFile && fCurrentAnalysisFile)
      printf(DMAGENTA " %s/%s" RESET_COLOR  "\n",fCurrentFragFile->GetName(),fCurrentAnalysisFile->GetName());
   printf(DMAGENTA " fSortFragmentDone         = %s" RESET_COLOR "\n",fSortFragmentDone ? "true":"false");
   printf(DYELLOW  " TEventQueue::Size()       = %i" RESET_COLOR "\n",TEventQueue::Size());
   printf(DBLUE    " TWriteQueue::Size()       = %i" RESET_COLOR "\n",TWriteQueue::Size());
   printf(DGREEN   " fFragmentsIn/fAnalysisOut = %i / %i" RESET_COLOR "\n",fFragmentsIn,fAnalysisOut);  
   printf(GREEN    " std::thread::hardware_concurrency = %u" RESET_COLOR "\n",std::thread::hardware_concurrency());
   printf(DMAGENTA " ==========================================" RESET_COLOR "\n");
   return;
}

void TAnalysisTreeBuilder::Status() {
   TStopwatch w;
   w.Start();
   bool fragmentsDone = false;
   bool sortingDone = false;
   while(fPrintStatus) {
      printf(DYELLOW HIDE_CURSOR "%12i / %12ld " RESET_COLOR "/" DBLUE " %12i " RESET_COLOR "/" DCYAN " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR "/" DGREEN " %12i " RESET_COLOR
             "    processed fragments / # of fragments/ # of events / event queue size / write queue size / events written.\t%f seconds." SHOW_CURSOR "\r",
             fFragmentsIn, fEntries, fAnalysisIn, TEventQueue::Size(), TWriteQueue::Size(), fAnalysisOut, w.RealTime());
      //we insert a newline (thus preserving the last status), if we just finished getting all fragment, or finished removing fragments from the event queue
      if(!fragmentsDone && fFragmentsIn == fEntries && fEntries != 0) {
         printf("\n");
         fragmentsDone = true;
      }
      if(fragmentsDone && !sortingDone && TEventQueue::Size() == 0) {
         printf("\n");
         sortingDone = true;
      }
      w.Continue(); 
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   }
   printf(DYELLOW HIDE_CURSOR "%12i / %12ld " RESET_COLOR "/" DBLUE " %12i " RESET_COLOR "/" DCYAN " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR "/" DGREEN " %12i " RESET_COLOR
          "    processed fragments / # of fragments/ # of events / event queue size / write queue size / events written.\t%f seconds." SHOW_CURSOR "\n",
          fFragmentsIn, fEntries, fAnalysisIn, TEventQueue::Size(), TWriteQueue::Size(), fAnalysisOut, w.RealTime());

   return;
}


