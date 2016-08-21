#include "TAnalysisHistLoop.h"

#include "TFile.h"

#include "TGRSIint.h"
#include "TGRSIOptions2.h"
#include "TPreserveGDirectory.h"
#include "GValue.h"
#include "TChannel.h"

TAnalysisHistLoop * TAnalysisHistLoop::Get(std::string name) {
  if(name.length()==0)
    name = "histo_loop";
  TAnalysisHistLoop *loop = dynamic_cast<TAnalysisHistLoop*>(StoppableThread::Get(name));
  if(!loop)
    loop = new TAnalysisHistLoop(name);
  return loop;
}

TAnalysisHistLoop::TAnalysisHistLoop(std::string name)
  : StoppableThread(name),
    output_file(0), output_filename("last.root"),
    input_queue(std::make_shared<ThreadsafeQueue<TUnpackedEvent*> >()),
    output_queue(std::make_shared<ThreadsafeQueue<TUnpackedEvent*> >()) {
  LoadLibrary(TGRSIOptions2::Get()->AnalysisHistogramLib());
}

TAnalysisHistLoop::~TAnalysisHistLoop() {
  CloseFile();
}

void TAnalysisHistLoop::ClearQueue() {
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

bool TAnalysisHistLoop::Iteration() {
  TUnpackedEvent* event = NULL;
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

void TAnalysisHistLoop::ClearHistograms() {
  compiled_histograms.ClearHistograms();
}

void TAnalysisHistLoop::OpenFile() {
  TPreserveGDirectory preserve;
  output_file = TGRSIint::instance()->OpenRootFile(output_filename,
                                                   "RECREATEONLINE");
  compiled_histograms.SetDefaultDirectory(output_file);
}

void TAnalysisHistLoop::CloseFile() {
  Write();

  if(output_file){
    output_file->Close();
    output_file = 0;
    output_filename = "last.root";
  }
}

void TAnalysisHistLoop::Write() {
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

void TAnalysisHistLoop::LoadLibrary(std::string library) {
  compiled_histograms.Load(library, "MakeAnalysisHistograms");
}

std::string TAnalysisHistLoop::GetLibraryName() const {
  return compiled_histograms.GetLibraryName();
}

TList* TAnalysisHistLoop::GetObjects() {
  return compiled_histograms.GetObjects();
}

TList* TAnalysisHistLoop::GetGates() {
  return compiled_histograms.GetGates();
}

void TAnalysisHistLoop::SetOutputFilename(const std::string& name){
  output_filename = name;
}

std::string TAnalysisHistLoop::GetOutputFilename() const {
  return output_filename;
}

void TAnalysisHistLoop::AddCutFile(TFile* cut_file) {
  compiled_histograms.AddCutFile(cut_file);
}
