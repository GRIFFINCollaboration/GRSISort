#include "TFragWriteLoop.h"

#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"

#include "GValue.h"
#include "TChannel.h"

TFragWriteLoop* TFragWriteLoop::Get(std::string name, std::string output_filename){
  if(name.length()==0){
    name = "write_loop";
  }

  StoppableThread* thread = StoppableThread::Get(name);
  if(!thread){
    if(output_filename.length()==0){
      output_filename = "temp.root";
    }
    thread = new TFragWriteLoop(name,output_filename);
  }

  return dynamic_cast<TFragWriteLoop*>(thread);
}

TFragWriteLoop::TFragWriteLoop(std::string name, std::string output_filename)
  : StoppableThread(name),
    output_file(NULL), event_tree(NULL),
    items_handled(0),
    input_queue(std::make_shared<ThreadsafeQueue<TFragment*> >()),
    output_queue(std::make_shared<ThreadsafeQueue<TFragment*> >()) {

  if(output_filename != "/dev/null"){
    //TPreserveGDirectory preserve;
    output_file = new TFile(output_filename.c_str(),"RECREATE");
    event_tree = new TTree("FragmentTree","FragmentTree");
    //scaler_tree = new TTree("ScalerTree","ScalerTree");
  }

  address = new TFragment*;
  *address = NULL;
}

TFragWriteLoop::~TFragWriteLoop() {
  delete address;

  if(output_file){
    output_file->cd();
    event_tree->Write(event_tree->GetName(), TObject::kOverwrite);
    if(GValue::Size())
      GValue::Get()->Write();
    if(TChannel::GetNumberOfChannels())
      TChannel::GetDefaultChannel()->Write();

    output_file->Close();
    output_file->Delete();
  }
}

void TFragWriteLoop::ClearQueue() {
  while(input_queue->Size()){
    TFragment* event = NULL;
    input_queue->Pop(event);
    if(event){
      delete event;
    }
  }

  while(output_queue->Size()){
    TFragment* event = NULL;
    output_queue->Pop(event);
    if(event){
      delete event;
    }
  }
}

bool TFragWriteLoop::Iteration() {
  TFragment* event = NULL;
  input_queue->Pop(event);

  if(event) {
    WriteEvent(*event);
    output_queue->Push(event);
    items_handled++;
    return true;
  } else if(input_queue->IsFinished()) {
    output_queue->SetFinished();
    return false;
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return true;
  }
}

void TFragWriteLoop::Write() {
  if(output_file){
    //TPreserveGDirectory preserve;
    output_file->cd();
    event_tree->Write();
  }
}

void TFragWriteLoop::WriteEvent(TFragment& event) {
  if(event_tree){
    *address = &event;
    event_tree->Fill();
    *address = NULL;
  }
}
