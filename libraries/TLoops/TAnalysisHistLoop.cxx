#include <utility>

#include "TAnalysisHistLoop.h"

#include "TFile.h"

#include "TGRSIint.h"
#include "TGRSIOptions.h"
#include "TPreserveGDirectory.h"
#include "GValue.h"
#include "TChannel.h"

TAnalysisHistLoop* TAnalysisHistLoop::Get(std::string name)
{
   if(name.length() == 0) {
      name = "histo_loop";
   }
   auto* loop = static_cast<TAnalysisHistLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      loop = new TAnalysisHistLoop(name);
   }
   return loop;
}

TAnalysisHistLoop::TAnalysisHistLoop(std::string name)
   : StoppableThread(std::move(name)), fOutputFile(nullptr), fOutputFilename("last.root"),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>())
{
   LoadLibrary(TGRSIOptions::Get()->AnalysisHistogramLib());
}

TAnalysisHistLoop::~TAnalysisHistLoop()
{
   CloseFile();
}

void TAnalysisHistLoop::ClearQueue()
{
   while(fInputQueue->Size() != 0u) {
      std::shared_ptr<TUnpackedEvent> event;
      fInputQueue->Pop(event);
   }
}

bool TAnalysisHistLoop::Iteration()
{
   std::shared_ptr<TUnpackedEvent> event;
   InputSize(fInputQueue->Pop(event));

   if(event) {
      if(fOutputFile == nullptr) {
         OpenFile();
      }

      fCompiledHistograms.Fill(event);
      IncrementItemsPopped();
      return true;
   }
   if(fInputQueue->IsFinished()) {
      return false;
   }
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return true;
}

void TAnalysisHistLoop::ClearHistograms()
{
   fCompiledHistograms.ClearHistograms();
}

void TAnalysisHistLoop::OpenFile()
{
   TPreserveGDirectory preserve;
   fOutputFile = TGRSIint::instance()->OpenRootFile(fOutputFilename, "RECREATEONLINE");
   fOutputFile->SetTitle("Analysis Histograms");
   fCompiledHistograms.SetDefaultDirectory(fOutputFile);
}

void TAnalysisHistLoop::CloseFile()
{
   Write();

   if(fOutputFile != nullptr) {
      fOutputFile->Close();
      fOutputFile     = nullptr;
      fOutputFilename = "last.root";
   }
}

void TAnalysisHistLoop::Write()
{
   if(GetOutputFilename() == "/dev/null") {
      return;
   }

   TPreserveGDirectory preserve;
   if(fOutputFile != nullptr) {
      fOutputFile->cd();
      fCompiledHistograms.Write();
      if(GValue::Size() != 0) {
         GValue::Get()->Write();
         std::cout << BLUE << "\t" << GValue::Size() << " GValues written to file " << gDirectory->GetName() << RESET_COLOR << std::endl;
      }
      if(TChannel::GetNumberOfChannels() != 0) {
         TChannel::GetDefaultChannel()->Write();
         std::cout << BLUE << "\t" << TChannel::GetNumberOfChannels() << " TChannels written to file " << gDirectory->GetName() << RESET_COLOR << std::endl;
      }
   }
}

void TAnalysisHistLoop::LoadLibrary(const std::string& library)
{
   fCompiledHistograms.Load(library, "MakeAnalysisHistograms");
}

std::string TAnalysisHistLoop::GetLibraryName() const
{
   return fCompiledHistograms.GetLibraryName();
}

TList* TAnalysisHistLoop::GetObjects()
{
   return fCompiledHistograms.GetObjects();
}

TList* TAnalysisHistLoop::GetGates()
{
   return fCompiledHistograms.GetGates();
}

void TAnalysisHistLoop::SetOutputFilename(const std::string& name)
{
   fOutputFilename = name;
}

std::string TAnalysisHistLoop::GetOutputFilename() const
{
   return fOutputFilename;
}

void TAnalysisHistLoop::AddCutFile(TFile* cut_file)
{
   fCompiledHistograms.AddCutFile(cut_file);
}
