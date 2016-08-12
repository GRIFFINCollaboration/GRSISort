#include "TUnpackingLoop.h"

#include <chrono>
#include <thread>

ClassImp(TUnpackingLoop)

TUnpackingLoop *TUnpackingLoop::Get(std::string name) {
  if(name.length()==0) {
    name = "build_loop";
  }

  TUnpackingLoop *loop = (TUnpackingLoop*)StoppableThread::Get(name);
  if(!loop) {
    loop = new TUnpackingLoop(name);
  }
  return loop;
}

TUnpackingLoop::TUnpackingLoop(std::string name)
  : StoppableThread(name),
    input_queue(std::make_shared<ThreadsafeQueue<TRawEvent> >()),
    output_queue(std::make_shared<ThreadsafeQueue<std::vector<TRawEvent> > >()) {

  SetBuildWindow(1000);
  event_start = 0;
}

TUnpackingLoop::~TUnpackingLoop() { }

void TUnpackingLoop::ClearQueue() {
  TRawEvent single_event;
  while(input_queue->Size()) {
    input_queue->Pop(single_event);
  }

  std::vector<TRawEvent> event;
  while(output_queue->Size()){
    output_queue->Pop(event);
  }
}

bool TUnpackingLoop::Iteration(){
  TRawEvent event;

  int error = input_queue->Pop(event);
  if(error<0) {
    if(input_queue->IsFinished()){
      // Source is dead, push the last event and stop.
       output_queue->SetFinished();
      return false;
    } else {
      // Wait for the source to give more data.
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      return true;
    }
  }


  ProcessMidasEvent(event);
  return true;
}
