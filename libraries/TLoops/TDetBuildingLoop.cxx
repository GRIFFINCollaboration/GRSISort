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
  : StoppableThread(name),
    input_queue(std::make_shared<ThreadsafeQueue<std::vector<TFragment*> > >()),
    output_queue(std::make_shared<ThreadsafeQueue<TUnpackedEvent*> >()) { }

TDetBuildingLoop::~TDetBuildingLoop() { }

bool TDetBuildingLoop::Iteration(){
  std::vector<TFragment*> frags;

  input_queue->Pop(frags);
  if(frags.size() == 0){
    if(input_queue->IsFinished()) {
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
  while(input_queue->Size()) {
    input_queue->Pop(raw_event);
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
