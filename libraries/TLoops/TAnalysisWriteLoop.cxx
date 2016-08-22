#include "TAnalysisWriteLoop.h"

#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"

#include "GValue.h"
#include "TChannel.h"
#include "TGRSIRunInfo.h"
#include "TTreeFillMutex.h"

TAnalysisWriteLoop* TAnalysisWriteLoop::Get(std::string name, std::string output_filename){
  if(name.length()==0){
    name = "write_loop";
  }

  StoppableThread* thread = StoppableThread::Get(name);
  if(!thread){
    if(output_filename.length()==0){
      output_filename = "temp.root";
    }
    thread = new TAnalysisWriteLoop(name,output_filename);
  }

  return dynamic_cast<TAnalysisWriteLoop*>(thread);
}

TAnalysisWriteLoop::TAnalysisWriteLoop(std::string name, std::string output_filename)
  : StoppableThread(name),
    output_file(NULL), event_tree(NULL),
    input_queue(std::make_shared<ThreadsafeQueue<TUnpackedEvent*> >()),
    output_queue(std::make_shared<ThreadsafeQueue<TUnpackedEvent*> >()) {

  if(output_filename != "/dev/null"){
    //TPreserveGDirectory preserve;
    output_file = new TFile(output_filename.c_str(),"RECREATE");
    event_tree = new TTree("AnalysisTree","AnalysisTree");
  }
}

TAnalysisWriteLoop::~TAnalysisWriteLoop() {
  for(auto& elem : det_map) {
    delete elem.second;
  }

  for(auto& elem : default_dets) {
    delete elem.second;
  }

  Write();

}

void TAnalysisWriteLoop::ClearQueue() {
  while(input_queue->Size()){
    TUnpackedEvent* event = NULL;
    input_queue->Pop(event);
    if(event){
      delete event;
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

bool TAnalysisWriteLoop::Iteration() {
  TUnpackedEvent* event = NULL;
  input_queue->Pop(event);

  if(event) {
    WriteEvent(*event);
    output_queue->Push(event);
    return true;
  } else if(input_queue->IsFinished()) {
    output_queue->SetFinished();
    return false;
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return true;
  }
}

void TAnalysisWriteLoop::Write() {

  if(output_file){
    output_file->cd();

    event_tree->Write(event_tree->GetName(), TObject::kOverwrite);
    if(GValue::Size()) {
      GValue::Get()->Write();
    }
    if(TChannel::GetNumberOfChannels()) {
      TChannel::GetDefaultChannel()->Write();
    }
    TGRSIRunInfo::Get()->WriteToRoot(output_file);
    TPPG::Get()->Write();

    output_file->Close();
    output_file->Delete();
  }
}

void TAnalysisWriteLoop::AddBranch(TClass* cls){
  if(!det_map.count(cls)){
    // This uses the ROOT dictionaries, so we need to lock the threads.
    TThread::Lock();

    // Make a default detector of that type.
    TDetector* det_p = (TDetector*)cls->New();
    default_dets[cls] = det_p;

    // Make the TDetector**
    TDetector** det_pp = new TDetector*;
    *det_pp = det_p;
    det_map[cls] = det_pp;

    // Make a new branch.
    TBranch* new_branch = event_tree->Branch(cls->GetName(), cls->GetName(), det_pp);

    // Fill the new branch up to the point where the tree is filled.
    // Explanation:
    //   When TTree::Fill is called, it calls TBranch::Fill for each
    // branch, then increments the number of entries.  We may be
    // adding branches after other branches have already been filled.
    // If the S800 branch has been filled 100 times before the Gretina
    // branch is created, then the next call to TTree::Fill will fill
    // entry 101 of S800, but entry 1 of Gretina, rather than entry
    // 101 of both.
    //   Therefore, we need to fill the new branch as many times as
    // TTree::Fill has been called before.
    std::lock_guard<std::mutex> lock(ttree_fill_mutex);
    for(int i=0; i<event_tree->GetEntries(); i++){
      new_branch->Fill();
    }

    std::cout << "\r" << std::string(30,' ')
              << "\rAdded \"" << cls->GetName() << "\" branch" << std::endl;

    // Unlock after we are done.
    TThread::UnLock();
  }
}

void TAnalysisWriteLoop::WriteEvent(TUnpackedEvent& event) {
  if(event_tree){
    // Clear pointers from previous writes.
    // Note that we cannot just set this equal to NULL,
    //   because ROOT would then construct a new object.
    // This contradicts the ROOT documentation for TBranchElement::SetAddress,
    //   which suggests that a new object would be constructed only when setting the address,
    //   not when filling the TTree.
    for(auto& elem : det_map){
      *elem.second = default_dets[elem.first];
    }

    // Load current events
    for(auto det : event.GetDetectors()) {
      TClass* cls = det->IsA();
      try{
        *det_map.at(cls) = det;
      } catch (std::out_of_range& e) {
        AddBranch(cls);
        *det_map.at(cls) = det;
      }
    }

    // Fill
    std::lock_guard<std::mutex> lock(ttree_fill_mutex);
    event_tree->Fill();
  }
}
