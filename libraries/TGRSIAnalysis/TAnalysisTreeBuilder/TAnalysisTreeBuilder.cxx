
#include "TAnalysisTreeBuilder.h"

bool TEventQueue::lock = false;
std::queue<std::vector<TFragment>*> TEventQueue::fEventQueue;

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

TAnalysisTreeBuilder::TAnalysisTreeBuilder() { }

TAnalysisTreeBuilder::~TAnalysisTreeBuilder() { }








