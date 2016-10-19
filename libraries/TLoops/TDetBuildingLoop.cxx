#include "TDetBuildingLoop.h"

#include <chrono>
#include <thread>

#include "TUnpackedEvent.h"

//#include "TMode3.h"

ClassImp(TDetBuildingLoop)

TDetBuildingLoop *TDetBuildingLoop::Get(std::string name) {
  if(name.length() == 0) {
    name = "unpack_loop";
  }
  TDetBuildingLoop *loop = (TDetBuildingLoop*)StoppableThread::Get(name);
  if(!loop) {
    loop = new TDetBuildingLoop(name);
  }
  return loop;
}

TDetBuildingLoop::TDetBuildingLoop(std::string name)
  : StoppableThread(name), fInputQueueSize(0), 
    fInputQueue(std::make_shared<ThreadsafeQueue<std::vector<TFragment*> > >()),
    output_queue(std::make_shared<ThreadsafeQueue<TUnpackedEvent*> >()) { }

TDetBuildingLoop::~TDetBuildingLoop() { }

bool TDetBuildingLoop::Iteration() {
  std::vector<TFragment*> frags;

  fInputQueueSize = fInputQueue->Pop(frags);
  if(frags.size() == 0){
    if(fInputQueue->IsFinished()) {
      output_queue->SetFinished();
      return false;
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      return true;
    }
  }

  TUnpackedEvent* output_event = new TUnpackedEvent;
  for(auto frag : frags) {
    // passes ownership of all TFragments, no need to delete here
    output_event->AddRawData(frag);
  }
  output_event->Build();
  output_queue->Push(output_event);
  return true;
}

void TDetBuildingLoop::ClearQueue() {
  std::vector<TFragment*> raw_event;
  while(fInputQueue->Size()) {
    fInputQueue->Pop(raw_event);
    for(auto frag : raw_event) {
      delete frag;
    }
  }

  while(output_queue->Size()){
    TUnpackedEvent* event = NULL;
    output_queue->Pop(event);
    if(event){
      delete event;
    }
  }
}

std::string TDetBuildingLoop::Status() {
	std::stringstream ss;
	ss<<Name()<<":\t"<<std::setw(8)<<GetItemsPushed()<<"/"<<(fInputQueueSize>0 ? fInputQueueSize+GetItemsPushed():GetItemsPushed());
	return ss.str();
}

std::string TDetBuildingLoop::EndStatus() {
	std::stringstream ss;
	ss<<"\r"<<Name()<<":\t"<<std::setw(8)<<GetItemsPushed()<<"/"<<(fInputQueueSize>0 ? fInputQueueSize+GetItemsPushed():GetItemsPushed())<<std::endl;;
	return ss.str();
}

