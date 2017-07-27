#include <utility>

#include "TFragHistLoop.h"

#include "TFile.h"

#include "TGRSIint.h"
#include "TGRSIOptions.h"
#include "TPreserveGDirectory.h"
#include "GValue.h"
#include "TChannel.h"

TFragHistLoop* TFragHistLoop::Get(std::string name)
{
   if(name.length() == 0) {
      name = "histo_loop";
   }
   TFragHistLoop* loop = static_cast<TFragHistLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      loop = new TFragHistLoop(name);
   }
   return loop;
}

TFragHistLoop::TFragHistLoop(std::string name)
   : StoppableThread(name), fOutputFile(nullptr), fOutputFilename("last.root"),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>())
{
   LoadLibrary(TGRSIOptions::Get()->FragmentHistogramLib());
}

TFragHistLoop::~TFragHistLoop()
{
   CloseFile();
}

void TFragHistLoop::ClearQueue()
{
   while(fInputQueue->Size() != 0u) {
      std::shared_ptr<const TFragment> event;
      fInputQueue->Pop(event);
   }
}

bool TFragHistLoop::Iteration()
{
   std::shared_ptr<const TFragment> event;
   fInputSize = fInputQueue->Pop(event);

   if(event) {
      if(fOutputFile == nullptr) {
         OpenFile();
      }

      fCompiledHistograms.Fill(event);
      ++fItemsPopped;
      return true;
   }
   if(fInputQueue->IsFinished()) {
      return false;
   }
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return true;
}

void TFragHistLoop::ClearHistograms()
{
   fCompiledHistograms.ClearHistograms();
}

void TFragHistLoop::OpenFile()
{
   TPreserveGDirectory preserve;
   fOutputFile = TGRSIint::instance()->OpenRootFile(fOutputFilename, "RECREATEONLINE");
   fOutputFile->SetTitle("Fragment Histograms");
   fCompiledHistograms.SetDefaultDirectory(fOutputFile);
}

void TFragHistLoop::CloseFile()
{
   Write();

   if(fOutputFile != nullptr) {
      fOutputFile->Close();
      fOutputFile     = nullptr;
      fOutputFilename = "last.root";
   }
}

void TFragHistLoop::Write()
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
         printf(BLUE "\t%i GValues written to file %s" RESET_COLOR "\n", GValue::Size(), gDirectory->GetName());
      }
      if(TChannel::GetNumberOfChannels() != 0) {
         TChannel::GetDefaultChannel()->Write();
         printf(BLUE "\t%i TChannels written to file %s" RESET_COLOR "\n", TChannel::GetNumberOfChannels(),
                gDirectory->GetName());
      }
   }
}

void TFragHistLoop::LoadLibrary(std::string library)
{
   fCompiledHistograms.Load(std::move(library), "MakeFragmentHistograms");
}

std::string TFragHistLoop::GetLibraryName() const
{
   return fCompiledHistograms.GetLibraryName();
}

TList* TFragHistLoop::GetObjects()
{
   return fCompiledHistograms.GetObjects();
}

TList* TFragHistLoop::GetGates()
{
   return fCompiledHistograms.GetGates();
}

void TFragHistLoop::SetOutputFilename(const std::string& name)
{
   fOutputFilename = name;
}

std::string TFragHistLoop::GetOutputFilename() const
{
   return fOutputFilename;
}

void TFragHistLoop::AddCutFile(TFile* cut_file)
{
   fCompiledHistograms.AddCutFile(cut_file);
}
