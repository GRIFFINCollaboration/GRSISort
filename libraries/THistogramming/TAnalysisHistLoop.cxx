#include "TAnalysisHistLoop.h"

#include "TFile.h"

#include "TGRSIint.h"
#include "TGRSIOptions.h"
#include "TPreserveGDirectory.h"
#include "GValue.h"
#include "TChannel.h"

TAnalysisHistLoop * TAnalysisHistLoop::Get(std::string name) {
  if(name.length()==0)
    name = "histo_loop";
  TAnalysisHistLoop *loop = static_cast<TAnalysisHistLoop*>(StoppableThread::Get(name));
  if(!loop)
    loop = new TAnalysisHistLoop(name);
  return loop;
}

TAnalysisHistLoop::TAnalysisHistLoop(std::string name)
  : StoppableThread(name),
    fOutputFile(nullptr), fOutputFilename("last.root"),
    fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent> > >()) {
  LoadLibrary(TGRSIOptions::Get()->AnalysisHistogramLib());
}

TAnalysisHistLoop::~TAnalysisHistLoop() {
  CloseFile();
}

void TAnalysisHistLoop::ClearQueue() {
  while(fInputQueue->Size()){
    std::shared_ptr<TUnpackedEvent> event;
    fInputQueue->Pop(event);
  }
}

bool TAnalysisHistLoop::Iteration() {
  std::shared_ptr<TUnpackedEvent> event;
  fInputQueue->Pop(event);

  if(event) {
    if(!fOutputFile){
      OpenFile();
    }

    fCompiledHistograms.Fill(event);
    return true;
  } else if(fInputQueue->IsFinished()) {
    return false;
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return true;
  }
}

void TAnalysisHistLoop::ClearHistograms() {
  fCompiledHistograms.ClearHistograms();
}

void TAnalysisHistLoop::OpenFile() {
  TPreserveGDirectory preserve;
  fOutputFile = TGRSIint::instance()->OpenRootFile(fOutputFilename,
                                                   "RECREATEONLINE");
  fOutputFile->SetTitle("Analysis Histograms");
  fCompiledHistograms.SetDefaultDirectory(fOutputFile);
}

void TAnalysisHistLoop::CloseFile() {
  Write();

  if(fOutputFile){
    fOutputFile->Close();
    fOutputFile = 0;
    fOutputFilename = "last.root";
  }
}

void TAnalysisHistLoop::Write() {
  if(GetOutputFilename() == "/dev/null") {
    return;
  }

  TPreserveGDirectory preserve;
  if(fOutputFile){
    fOutputFile->cd();
    fCompiledHistograms.Write();
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
  fCompiledHistograms.Load(library, "MakeAnalysisHistograms");
}

std::string TAnalysisHistLoop::GetLibraryName() const {
  return fCompiledHistograms.GetLibraryName();
}

TList* TAnalysisHistLoop::GetObjects() {
  return fCompiledHistograms.GetObjects();
}

TList* TAnalysisHistLoop::GetGates() {
  return fCompiledHistograms.GetGates();
}

void TAnalysisHistLoop::SetOutputFilename(const std::string& name){
  fOutputFilename = name;
}

std::string TAnalysisHistLoop::GetOutputFilename() const {
  return fOutputFilename;
}

void TAnalysisHistLoop::AddCutFile(TFile* cut_file) {
  fCompiledHistograms.AddCutFile(cut_file);
}
