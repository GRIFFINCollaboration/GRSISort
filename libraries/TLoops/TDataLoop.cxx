#include "TDataLoop.h"

#include <chrono>
#include <thread>
#include <utility>

#include "TGRSIOptions2.h"
#include "TString.h"
#include "TMidasFile.h"


TDataLoop::TDataLoop(std::string name,TMidasFile* source)
  : StoppableThread(name),
    source(source), fSelfStopping(true),
    output_queue(std::make_shared<ThreadsafeQueue<TMidasEvent> >()) { }

TDataLoop::~TDataLoop(){
  //delete source;
}

TDataLoop *TDataLoop::Get(std::string name,TMidasFile* source) {
  if(name.length()==0)
    name = "input_loop";

  TDataLoop *loop = dynamic_cast<TDataLoop*>(StoppableThread::Get(name));
  if(!loop && source) {
    loop = new TDataLoop(name,source);
  }
  return loop;
}

void TDataLoop::ClearQueue() {
  TMidasEvent event;
  while(output_queue->Size()){
    output_queue->Pop(event);
  }
}

void TDataLoop::ReplaceSource(TMidasFile* new_source) {
  std::lock_guard<std::mutex> lock(source_mutex);
  //delete source;
  source = new_source;
}

void TDataLoop::ResetSource() {
  std::cerr << "Reset not implemented for TMidasFile" << std::endl;
  // std::lock_guard<std::mutex> lock(source_mutex);
  // source->Reset();
}

void TDataLoop::OnEnd() {
  output_queue->SetFinished();
}

bool TDataLoop::Iteration() {
  TMidasEvent evt;
  int bytes_read;
  {
    std::lock_guard<std::mutex> lock(source_mutex);
    bytes_read = source->Read(evt);
  }

  if(bytes_read < 0 && fSelfStopping){
    // Error, and no point in trying again.
    printf("finished sorting all input.\n");
    return false;
  } else if(bytes_read > 0){
    // A good event was returned
    output_queue->Push(evt);
    return true;
  } else {
    // Nothing returned this time, but I might get something next time.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return true;
  }
  return true;
}

std::string TDataLoop::Status() {
  return source->Status(TGRSIOptions2::Get()->LongFileDescription());
}
