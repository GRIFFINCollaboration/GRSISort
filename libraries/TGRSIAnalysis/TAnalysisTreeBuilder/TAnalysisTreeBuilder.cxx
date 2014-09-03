
#include "TAnalysisTreeBuilder.h"
#include "TGRSIOptions.h"

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


TChain *TAnalysisTreeBuilder::fFragmentChain = 0;
TTree  *TAnalysisTreeBuilder::fCurrentFragTree = 0;
TFile  *TAnalysisTreeBuilder::fCurrentFragFile = 0;
TGRSIRunInfo *TAnalysisTreeBuilder::fCurrentRunInfo = 0;

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
   TList *list = fCurrentFragTree->GetUserInfo();
   TIter iter(list);
   while(TObject *obj = iter.Next()) {
      if(!obj->InheritsFrom("TChannel"))
         continue;
      TChannel *chan = (TChannel*)obj;
      int number = chan->GetUserInfoNumber(); // I should need to do this.. 
      chan->SetNumber(number);
      TChannel::AddChannel(chan,"save");
   }
   if(!TGRSIOptions::GetInputCal().empty()) {
      TChannel::ReadCalFile(TGRSIOptions::GetInputCal().at(0).c_str());
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
      SortFragmentTree();
      printf("\n");
      i+=(nentries-10);
   }
   printf("Finished chain sort.\n");
}

void TAnalysisTreeBuilder::SetupFragmentTree() {

   fCurrentFragFile = fCurrentFragTree->GetCurrentFile();
   fCurrentRunInfo  = (TGRSIRunInfo*)fCurrentFragFile->Get("TGRSIRunInfo");
   if(fCurrentRunInfo) {
      TGRSIRunInfo::SetInfoFromFile(fCurrentRunInfo);
      fCurrentRunInfo->Print();
   }

}






