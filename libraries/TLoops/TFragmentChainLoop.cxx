#include "TFragmentChainLoop.h"

#include <chrono>
#include <thread>

#include "TClass.h"
#include "TFile.h"
#include "TThread.h"

#include "TDetector.h"
#include "GRootCommands.h"
#include "TFragment.h"

TFragmentChainLoop* TFragmentChainLoop::Get(std::string name,TChain *chain){
  if(name.length()==0){
    name = "chain_loop";
  }

  StoppableThread* thread = StoppableThread::Get(name);
  if(!thread){
    if(!chain && !gFragment){
      return 0;
    } else if(!chain) {
      chain = gFragment;
    }
    thread = new TFragmentChainLoop(name,chain);
  }
  return dynamic_cast<TFragmentChainLoop*>(thread);
}

TFragmentChainLoop::TFragmentChainLoop(std::string name, TChain *chain)
  : StoppableThread(name),
    fEntriesRead(0), fEntriesTotal(chain->GetEntries()),
    input_chain(chain),
    output_queue(std::make_shared<ThreadsafeQueue<TFragment*> >()),
    fSelfStopping(true) {
  SetupChain();
}

TFragmentChainLoop::~TFragmentChainLoop() { }

void TFragmentChainLoop::ClearQueue() {
  while(output_queue->Size()){
    TFragment* event = NULL;
    output_queue->Pop(event);
    if(event){
      delete event;
    }
  }
}

int TFragmentChainLoop::SetupChain() {
  if(!input_chain) {
    return 0;
  }

  address = new TFragment*;
  *address = NULL;
  input_chain->SetBranchAddress("TFragment",address);
  return 0;
}

std::string TFragmentChainLoop::Status() {
  return Form("Event: %ld / %ld", long(fEntriesRead), fEntriesTotal);
}


void TFragmentChainLoop::Restart() {
  fEntriesRead = 0;
}

void TFragmentChainLoop::OnEnd() {
  output_queue->SetFinished();
}

bool TFragmentChainLoop::Iteration() {
  if(fEntriesRead >= fEntriesTotal){
    if(fSelfStopping) {
      return false;
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      return true;
    }
  }

  TFragment* frag = new TFragment;
  *address = frag;
  input_chain->GetEntry(fEntriesRead++);
  frag->SetEntryNumber();
  output_queue->Push(frag);
  return true;
}
