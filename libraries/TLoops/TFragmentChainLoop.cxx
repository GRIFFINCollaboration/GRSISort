#include "TFragmentChainLoop.h"

#include <chrono>
#include <thread>

#include "TClass.h"
#include "TFile.h"
#include "TThread.h"

#include "TDetector.h"
#include "GRootCommands.h"
#include "TFragment.h"

TFragmentChainLoop* TFragmentChainLoop::Get(std::string name, TChain *chain) {
  if(name.length() == 0){
    name = "chain_loop";
  }

  StoppableThread* thread = StoppableThread::Get(name);
  if(!thread) {
    if(!chain && !gFragment) {
      return 0;
    } else if(!chain) {
      chain = gFragment;
    }
    thread = new TFragmentChainLoop(name, chain);
  }
  return dynamic_cast<TFragmentChainLoop*>(thread);
}

TFragmentChainLoop::TFragmentChainLoop(std::string name, TChain *chain)
  : StoppableThread(name),
    fEntriesTotal(chain->GetEntries()),
    fInputChain(chain), fFragment(NULL), 
    fSelfStopping(true) {
  SetupChain();
}

TFragmentChainLoop::~TFragmentChainLoop() { }

void TFragmentChainLoop::ClearQueue() {
   for(auto outQueue : fOutputQueues) {
      while(outQueue->Size()){
         std::shared_ptr<const TFragment> event;
         outQueue->Pop(event);
      }
   }
}

int TFragmentChainLoop::SetupChain() {
   if(!fInputChain) {
      return 0;
   }

   fInputChain->SetBranchAddress("TFragment",&fFragment);
   return 0;
}

void TFragmentChainLoop::Restart() {
   fItemsPopped = 0;
}

void TFragmentChainLoop::OnEnd() {
   for(auto outQueue : fOutputQueues) {
      outQueue->SetFinished();
   }
}

bool TFragmentChainLoop::Iteration() {
   if(static_cast<long>(fItemsPopped) >= fEntriesTotal){
      if(fSelfStopping) {
         return false;
      } else {
         std::this_thread::sleep_for(std::chrono::milliseconds(1000));
         return true;
      }
   }

   std::shared_ptr<TFragment> frag = std::make_shared<TFragment>();
   fInputChain->GetEntry(fItemsPopped++);
   *frag = *fFragment;
   frag->SetEntryNumber();
   for(auto outQueue : fOutputQueues) {
      outQueue->Push(frag);
   }
	fInputSize = fEntriesTotal - fItemsPopped; //this way fInputSize+fItemsPopped gives the total number of entries

   return true;
}
