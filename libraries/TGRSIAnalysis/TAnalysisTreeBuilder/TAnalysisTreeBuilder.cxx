
#include "Globals.h"

#include <TVirtualIndex.h>
#include <TTreeIndex.h>

#include "TAnalysisTreeBuilder.h"
#include "TGRSIOptions.h"

#include <TStopwatch.h>

#include <TGRSIOptions.h>


//#include "TSharcData.h"


bool TEventQueue::lock = false;
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
   while(lock) { }
   SetLock();
   fEventQueue.push(event);
   UnsetLock();
   return;
}

std::vector<TFragment> *TEventQueue::Pop() {
   std::vector<TFragment> *temp;
   while(lock) { }
   SetLock();
   temp = fEventQueue.front();
   fEventQueue.pop();
   UnsetLock();
   return temp;
}

int TEventQueue::Size() {
   int temp;
   while(lock) { }
   SetLock();
   temp = fEventQueue.size();
   UnsetLock();
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


const size_t TAnalysisTreeBuilder::MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)4; // 4 GB //20000000000

TChain *TAnalysisTreeBuilder::fFragmentChain = 0;
TTree  *TAnalysisTreeBuilder::fCurrentFragTree = 0;
TFile  *TAnalysisTreeBuilder::fCurrentFragFile = 0;
TTree  *TAnalysisTreeBuilder::fCurrentAnalysisTree = 0;
TFile  *TAnalysisTreeBuilder::fCurrentAnalysisFile = 0;
TGRSIRunInfo *TAnalysisTreeBuilder::fCurrentRunInfo = 0;

TFragment *TAnalysisTreeBuilder::fCurrentFragPtr = 0;

TTigress    *TAnalysisTreeBuilder::tigress = 0;    
TSharc      *TAnalysisTreeBuilder::sharc   = 0;    
TTriFoil    *TAnalysisTreeBuilder::triFoil = 0;
//TRf         *TAnalysisTreeBuilder::rf      = 0;     
TCSM        *TAnalysisTreeBuilder::csm     = 0;    
//TSpice      *TAnalysisTreeBuilder::spice   = 0;  
//TS3         *TAnalysisTreeBuilder::s3      = 0;
//TTip        *TAnalysisTreeBuilder::tip     = 0;    

TGriffin    *TAnalysisTreeBuilder::griffin = 0;
//TSceptar    *TAnalysisTreeBuilder::Sceptar = 0;
//TPaces      *TAnalysisTreeBuilder::Paces   = 0;  
//TDante      *TAnalysisTreeBuilder::Dante   = 0;  
//TZeroDegree *TAnalysisTreeBuilder::ZeroDegree = 0;
//TDescant    *TAnalysisTreeBuilder::Descant = 0;




TAnalysisTreeBuilder::TAnalysisTreeBuilder() { }

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

      //if(fCurrentRunInfo->MajorIndex().length()>0 && fCurrentRunInfo->MajorIndex().compare("TimeStampHigh")==0) 
      if(fCurrentRunInfo->Griffin())
         SortFragmentTreeByTimeStamp();
      else
         SortFragmentTree();
      

      CloseAnalysisFile();
      printf("\n");
      i+=(nentries-10);
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
         event->push_back(*fCurrentFragPtr);
      }
      if(event->empty()) {
         delete event;
      } else {
         TEventQueue::Get()->Add(event);
      }
      if((j%10000)==0 || j==major_max) {
         printf("\tprocessing event " CYAN "%i " RESET_COLOR "/" DBLUE " %li" RESET_COLOR "         \r",j,major_max);
      }
   }
   printf("\n");
   fCurrentFragTree->DropBranchFromCache(fCurrentFragTree->GetBranch("TFragment"),true);
   return;
}


void TAnalysisTreeBuilder::SortFragmentTreeByTimeStamp() {

   TFragment *currentFrag = 0;
   TFragment *oldFrag = new TFragment;

   fCurrentFragTree->SetBranchAddress("TFragment",&currentFrag);

   TTreeIndex *index = (TTreeIndex*)fCurrentFragTree->GetTreeIndex();

   long entries = index->GetN();
   Long64_t *indexvalues = index->GetIndex();
   int major_max = fCurrentFragTree->GetMaximum("TimeStampHigh");

   std::vector<TFragment> *event = new std::vector<TFragment>;

   fCurrentFragTree->GetEntry(indexvalues[0]);
   *oldFrag = *currentFrag;

   
   for(int x=1;x<entries;x++) {
      if(fCurrentFragTree->GetEntry(indexvalues[x]) == -1 ) {  //major,minor) == -1) {
         printf(DRED "FIRE!!!" RESET_COLOR  "\n");
         continue;
      }
      if(indexvalues[x] == indexvalues[x-1]) {
         printf(DRED "REAL FIRE!!! x = %i, indexvalues[x] = %lu, indexvalues[x-1] = %lu" RESET_COLOR  "\n", x, indexvalues[x], indexvalues[x-1]);
         printf("currentFrag->MidasId = %i   oldFrag->MidasId = %i\n",currentFrag->MidasId, oldFrag->MidasId);
         continue;
      } 
      event->push_back(*oldFrag);
      //printf("major = %d, minor = %d\n", major, minor);
      //oldFrag->Print();
      //currentFrag->Print();
      //printf("================================================================================\n");
         //printf("coincident event: event->size() = %lu\n",event->size());
         //printf("non-coincident event: event->size() = %lu\n",event->size());
         //if(event->size()==0) {
            //event->push_back(*currentFrag);
         //}
         //if(event->size() > 1) {
            //printf("================================================================================\n");
            //for(size_t i = 0; i < event->size(); ++i) {
               //event->at(i).Print();
            //}
         //}
         
      if(abs(oldFrag->GetTimeStamp()-currentFrag->GetTimeStamp()) > 200) {  // 2 micro-sec.
         TEventQueue::Get()->Add(event);
         event = new std::vector<TFragment>;
      }
      *oldFrag = *currentFrag;
      if((x%10000)==0 ) {
         printf("\tprocessing fragment " CYAN "%i " RESET_COLOR "/" DBLUE " %li" RESET_COLOR "         \r",x,entries);
       }
   }
   printf("\n");
   fCurrentFragTree->DropBranchFromCache(fCurrentFragTree->GetBranch("TFragment"),true);
   return;                                                                                                                                                                            return;
}







void TAnalysisTreeBuilder::SetupFragmentTree() {

   fCurrentFragFile = fCurrentFragTree->GetCurrentFile();
   fCurrentRunInfo  = (TGRSIRunInfo*)fCurrentFragFile->Get("TGRSIRunInfo");
   if(fCurrentRunInfo) {
      TGRSIRunInfo::SetInfoFromFile(fCurrentRunInfo);
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
   printf("created ouput file: %s\n",fCurrentAnalysisFile->GetName());
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

   if(info->Tigress())   { tree->Branch("TTigress","TTigress",&tigress); } 
   if(info->Sharc())     { tree->Branch("TSharc","TSharc",&sharc); } 
   if(info->TriFoil())   { tree->Branch("TTriFoil","TTriFoil",&triFoil); } 
   //if(info->Rf())        { tree->Branch("TRf","TRf",&rf); } 
   if(info->CSM())       { tree->Branch("TCSM","TCSM",&csm); } 
   //if(info->Spice())     { tree->Branch("TSpice","TSpice",&spice); tree->SetBranch("TS3","TS3",&s3); } 
   //if(info->Tip())       { tree->Branch("TTip","TTip",&tip); } 

   if(info->Griffin())   { tree->Branch("TGriffin","TGriffin",&griffin); } 
   //if(info->Sceptar())   { tree->Branch("TSceptar","TSceptar",&sceptar); } 
   //if(info->Paces())     { tree->Branch("TPaces","TPaces",&paces); } 
   //if(info->Dante())     { tree->Branch("TDante","TDante",&dante); } 
   //if(info->ZeroDegree()){ tree->Branch("TZeroDegree","TZeroDegree",&zerodegree); } 
   //if(info->Descant())   { tree->Branch("TDescant","TDescant",&descant);
   return;  
}

void TAnalysisTreeBuilder::ClearActiveAnalysisTreeBranches() {

   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;
   TGRSIRunInfo *info = fCurrentRunInfo;
   TTree *tree = fCurrentAnalysisTree;

   if(info->Tigress())   { tigress->Clear(); } 
   if(info->Sharc())     { sharc->Clear(); } 
   if(info->TriFoil())   { triFoil->Clear(); } 
   //if(info->Rf())        { rf->Clear(); } 
   if(info->CSM())       { csm->Clear(); } 
   //if(info->Spice())     { spice->Clear(); s3->Clear(); } 
   //if(info->Tip())       { tip->Clear(); } 

   if(info->Griffin())   { griffin->Clear(); } 
   //if(info->Sceptar())   { sceptar->Clear(); } 
   //if(info->Paces())     { paces->Clear(); } 
   //if(info->Dante())     { dante->Clear(); } 
   //if(info->ZeroDegree()){ zerodegree->Clear(); } 
   //if(info->Descant())   { descant->Clear();
}

void TAnalysisTreeBuilder::BuildActiveAnalysisTreeBranches() {

   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;
   TGRSIRunInfo *info = fCurrentRunInfo;
   TTree *tree = fCurrentAnalysisTree;

   if(info->Tigress())   { tigress->BuildHits(); } 
   if(info->Sharc())     { sharc->BuildHits(); } 
   if(info->TriFoil())   { triFoil->BuildHits(); } 
   //if(info->Rf())        { rf->Clear(); } 
   if(info->CSM())       { csm->BuildHits(); } 
   //if(info->Spice())     { spice->Clear(); s3->Clear(); } 
   //if(info->Tip())       { tip->Clear(); } 

   if(info->Griffin())   { griffin->BuildHits(); } 
   //if(info->Sceptar())   { sceptar->Clear(); } 
   //if(info->Paces())     { paces->Clear(); } 
   //if(info->Dante())     { dante->Clear(); } 
   //if(info->ZeroDegree()){ zerodegree->Clear(); } 
   //if(info->Descant())   { descant->Clear();
}

void TAnalysisTreeBuilder::FillAnalysisTree() {
   if(!fCurrentAnalysisTree)
      return;
   fCurrentAnalysisTree->Fill();
  
   return;
}

void TAnalysisTreeBuilder::CloseAnalysisFile() {
   if(!fCurrentAnalysisFile)
      return;

   ///******************************////
   ///******************************////
   // this will be removed and put into a seperate thread later.

   printf("Event Queue has %i events waiting to be processed.\n", TEventQueue::Size());
   int totalsize = TEventQueue::Size();
   TStopwatch w;
   while(TEventQueue::Size()) {
      w.Start();
      std::vector<TFragment> *event = TEventQueue::Pop();
			//printf("event = 0x%08x\n",event);

      ProcessEvent(event);

      if(TEventQueue::Size()%1500==0 || TEventQueue::Size()==0) {
         printf("\t\ton event %d/%d\t\t%f seconds.\r",TEventQueue::Size(),totalsize,w.RealTime());
         w.Continue(); 
      }
      if(TEventQueue::Size() == 0) 
         break;
   }
   printf("\n");

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


void TAnalysisTreeBuilder::ProcessEvent(std::vector<TFragment> *event) {
   if(!event)
      return;

   MNEMONIC mnemonic;
   for(int i=0;i<event->size();i++) {
      //printf("ChannelAddress =0x%08x\t",event->at(i).ChannelAddress);

      TChannel *channel = TChannel::GetChannel(event->at(i).ChannelAddress);
      //printf("name: %s \n",channel->GetChannelName());

     
      if(!channel)
         continue;
      ClearMNEMONIC(&mnemonic);
      ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
		
			//printf("ChannelName = %s\n",channel->GetChannelName());

			if(mnemonic.system.compare("TI")==0) {
				tigress->FillData(&(event->at(i)),channel,&mnemonic);
			} else if(mnemonic.system.compare("SH")==0) {
				sharc->FillData(&(event->at(i)),channel,&mnemonic);
			//} else if(mnemonic.system.compare("TR")==0) {	
			//	FillData(&(event->at(i)),&mnemonic);
			//} else if(mnemonic.system.compare("RF")==0) {	
			//	FillData(&(event->at(i)),&mnemonic);
			} else if(mnemonic.system.compare("CS")==0) {	
				csm->FillData(&(event->at(i)),channel,&mnemonic);
			//} else if(mnemonic.system.compare("SP")==0) {	
			//	FillData(&(event->at(i)),&mnemonic);
			} else if(mnemonic.system.compare("GR")==0) {	
				griffin->FillData(&(event->at(i)),channel,&mnemonic);
			} //else if(mnemonic.system.compare("SC")==0) {	
			//	FillData(&(event->at(i)),&mnemonic);
			//} else if(mnemonic.system.compare("PA")==0) {	
			//	FillData(&(event->at(i)),&mnemonic);
			//} else if(mnemonic.system.compare("DA")==0) {	
			//	FillData(&(event->at(i)),&mnemonic);
			//} else if(mnemonic.system.compare("ZD")==0) {	
			//	FillData(&(event->at(i)),&mnemonic);
			//} else if(mnemonic.system.compare("DE")==0) {	
			//	FillData(&(event->at(i)),&mnemonic);
			//}


	}		
	
	BuildActiveAnalysisTreeBranches();
	FillAnalysisTree();
	ClearActiveAnalysisTreeBranches();	

  delete event;
   
}


















