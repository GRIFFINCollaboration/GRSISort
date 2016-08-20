#include "TFragWriteLoop.h"

#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"

#include "GValue.h"
#include "TChannel.h"
#include "TGRSIRunInfo.h"

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
    scaler_input_queue(std::make_shared<ThreadsafeQueue<TEpicsFrag*> >()),
    output_queue(std::make_shared<ThreadsafeQueue<TFragment*> >()) {


  event_address = new TFragment*;
  *event_address = NULL;

  scaler_address = new TEpicsFrag*;
  *scaler_address = NULL;

  if(output_filename != "/dev/null"){
    //TPreserveGDirectory preserve;
    output_file = new TFile(output_filename.c_str(),"RECREATE");

    event_tree = new TTree("FragmentTree","FragmentTree");
    event_tree->Branch("TFragment",event_address);

    scaler_tree = new TTree("EpicsTree","EpicsTree");
    scaler_tree->Branch("TEpicsFrag",scaler_address);
  }

}

TFragWriteLoop::~TFragWriteLoop() {
  delete event_address;
  delete scaler_address;

  if(output_file){
    output_file->cd();
    event_tree->Write(event_tree->GetName(), TObject::kOverwrite);
    scaler_tree->Write(scaler_tree->GetName(), TObject::kOverwrite);
    if(GValue::Size()) {
      GValue::Get()->Write();
    }

    if(TChannel::GetNumberOfChannels()) {
      TChannel::GetDefaultChannel()->Write();
    }

    TGRSIRunInfo::Get()->WriteToRoot(output_file);

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
  input_queue->Pop(event,0);

  TEpicsFrag* scaler = NULL;
  scaler_input_queue->Pop(scaler,0);

  bool has_anything = event || scaler;
  bool all_parents_dead = (input_queue->IsFinished() &&
                           scaler_input_queue->IsFinished());

  if(event) {
    WriteEvent(*event);
    output_queue->Push(event);
    items_handled++;
  }

  if(scaler) {
    WriteScaler(*scaler);
    delete scaler;
  }

  if(has_anything) {
    return true;
  } else if(all_parents_dead) {
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
    event_tree->Write(event_tree->GetName(), TObject::kOverwrite);
    scaler_tree->Write(scaler_tree->GetName(), TObject::kOverwrite);
  }
}

void TFragWriteLoop::WriteEvent(TFragment& event) {
  if(event_tree){
    *event_address = &event;
    event_tree->Fill();
    *event_address = NULL;
  }
}

void TFragWriteLoop::WriteScaler(TEpicsFrag& scaler) {
  if(scaler_tree){
    *scaler_address = &scaler;
    scaler_tree->Fill();
    *scaler_address = NULL;
  }
}


