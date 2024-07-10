#include "TFragmentChainLoop.h"

#include <chrono>
#include <thread>

#include "TClass.h"
#include "TFile.h"
#include "TThread.h"

#include "TDetector.h"
#include "TGRSIint.h"
#include "TFragment.h"

TFragmentChainLoop* TFragmentChainLoop::Get(std::string name, TChain* chain)
{
   if(name.length() == 0) {
      name = "chain_loop";
   }

   auto* loop = static_cast<TFragmentChainLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      if((chain == nullptr) && (gFragment == nullptr)) {
         return nullptr;
      }
      if(chain == nullptr) {
         chain = gFragment;
      }
      loop = new TFragmentChainLoop(name, chain);
   }
   return loop;
}

TFragmentChainLoop::TFragmentChainLoop(std::string name, TChain* chain)
   : StoppableThread(std::move(name)), fEntriesTotal(chain->GetEntries()),
     fInputChain(chain), fFragment(nullptr), fSelfStopping(true)
{
   SetupChain();
}

TFragmentChainLoop::~TFragmentChainLoop() = default;

void TFragmentChainLoop::ClearQueue()
{
   for(const auto& outQueue : fOutputQueues) {
      while(outQueue->Size() != 0u) {
         std::shared_ptr<const TFragment> event;
         outQueue->Pop(event);
      }
   }
}

int TFragmentChainLoop::SetupChain()
{
   if(fInputChain == nullptr) {
      return 0;
   }

   fInputChain->SetBranchAddress("TFragment", &fFragment);
   return 0;
}

void TFragmentChainLoop::Restart()
{
   fItemsPopped = 0;
}

void TFragmentChainLoop::OnEnd()
{
   for(const auto& outQueue : fOutputQueues) {
      outQueue->SetFinished();
   }
}

bool TFragmentChainLoop::Iteration()
{
   if(static_cast<int64_t>(fItemsPopped) >= fEntriesTotal) {
      if(fSelfStopping) {
         return false;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      return true;
   }

   std::shared_ptr<TFragment> frag = std::make_shared<TFragment>();
   fInputChain->GetEntry(fItemsPopped++);
   *frag = *fFragment;
   frag->SetEntryNumber();
   for(const auto& outQueue : fOutputQueues) {
      outQueue->Push(frag);
   }
   fInputSize = fEntriesTotal - fItemsPopped;   // this way fInputSize+fItemsPopped gives the total number of entries

   return true;
}
