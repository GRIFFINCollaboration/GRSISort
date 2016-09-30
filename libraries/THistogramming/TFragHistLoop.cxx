#include "TFragHistLoop.h"

#include "TFile.h"

#include "TGRSIint.h"
#include "TGRSIOptions.h"
#include "TPreserveGDirectory.h"
#include "GValue.h"
#include "TChannel.h"

TFragHistLoop * TFragHistLoop::Get(std::string name) {
  if(name.length()==0)
    name = "histo_loop";
  TFragHistLoop *loop = dynamic_cast<TFragHistLoop*>(StoppableThread::Get(name));
  if(!loop)
    loop = new TFragHistLoop(name);
  return loop;
}

TFragHistLoop::TFragHistLoop(std::string name)
  : StoppableThread(name),
    output_file(0), output_filename("last.root"),
    input_queue(std::make_shared<ThreadsafeQueue<TFragment*> >()),
    output_queue(std::make_shared<ThreadsafeQueue<TFragment*> >()) {
  LoadLibrary(TGRSIOptions::Get()->FragmentHistogramLib());
}

TFragHistLoop::~TFragHistLoop() {
  CloseFile();
}

void TFragHistLoop::ClearQueue() {
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

bool TFragHistLoop::Iteration() {
  TFragment* event = NULL;
  input_queue->Pop(event);

  if(event) {
    if(!output_file){
      OpenFile();
    }

    compiled_histograms.Fill(*event);
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

void TFragHistLoop::ClearHistograms() {
  compiled_histograms.ClearHistograms();
}

void TFragHistLoop::OpenFile() {
  TPreserveGDirectory preserve;
  output_file = TGRSIint::instance()->OpenRootFile(output_filename,
                                                   "RECREATEONLINE");
  output_file->SetTitle("Fragment Histograms");
  compiled_histograms.SetDefaultDirectory(output_file);
}

void TFragHistLoop::CloseFile() {
  Write();

  if(output_file){
    output_file->Close();
    output_file = 0;
    output_filename = "last.root";
  }
}

void TFragHistLoop::Write() {
  if(GetOutputFilename() == "/dev/null") {
    return;
  }

  TPreserveGDirectory preserve;
  if(output_file){
    output_file->cd();
    compiled_histograms.Write();
    if(GValue::Size()) {
      GValue::Get()->Write();
      printf(BLUE "\t%i GValues written to file %s" RESET_COLOR "\n",GValue::Size(),gDirectory->GetName());
    }
    if(TChannel::GetNumberOfChannels()) {
      TChannel::GetDefaultChannel()->Write();
      printf(BLUE "\t%i TChannels written to file %s" RESET_COLOR "\n",
             TChannel::GetNumberOfChannels(),gDirectory->GetName());
    }
  }
}

void TFragHistLoop::LoadLibrary(std::string library) {
  compiled_histograms.Load(library, "MakeFragmentHistograms");
}

std::string TFragHistLoop::GetLibraryName() const {
  return compiled_histograms.GetLibraryName();
}

TList* TFragHistLoop::GetObjects() {
  return compiled_histograms.GetObjects();
}

TList* TFragHistLoop::GetGates() {
  return compiled_histograms.GetGates();
}

void TFragHistLoop::SetOutputFilename(const std::string& name){
  output_filename = name;
}

std::string TFragHistLoop::GetOutputFilename() const {
  return output_filename;
}

void TFragHistLoop::AddCutFile(TFile* cut_file) {
  compiled_histograms.AddCutFile(cut_file);
}
