#include "TAnalysisWriteLoop.h"

#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"
#include "TMessage.h"
#include "TSocket.h"
#include "TMemFile.h"
#include "TFileMerger.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TFileCacheWrite.h"
#include "TROOT.h"
#include "THashTable.h"

#include "GValue.h"
#include "TChannel.h"
#include "TRunInfo.h"
#include "TGRSIOptions.h"
#include "TTreeFillMutex.h"
#include "TSortingDiagnostics.h"

TAnalysisWriteLoop* TAnalysisWriteLoop::Get(std::string name, std::string outputFilename)
{
   if(name.length() == 0) {
      name = "write_loop";
   }

   auto* loop = static_cast<TAnalysisWriteLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      if(outputFilename.length() == 0) {
         outputFilename = "temp.root";
      }
      loop = new TAnalysisWriteLoop(name, outputFilename);
   }

   return loop;
}

TAnalysisWriteLoop::TAnalysisWriteLoop(std::string name, const std::string& outputFilename)
   : StoppableThread(std::move(name)),
     fOutputFile(TFile::Open(outputFilename.c_str(), "recreate")),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>())
{
   if(fOutputFile == nullptr || !fOutputFile->IsOpen()) {
      std::cerr << "Failed to open '" << outputFilename << "'" << std::endl;
      throw;
   }

   fEventTree = new TTree("AnalysisTree", "AnalysisTree");
   if(TGRSIOptions::Get()->SeparateOutOfOrder()) {
      fOutOfOrderTree = new TTree("OutOfOrderTree", "OutOfOrderTree");
      fOutOfOrderFrag = new TFragment;
      fOutOfOrderTree->Branch("Fragment", &fOutOfOrderFrag);
      fOutOfOrder = true;
   }
}

TAnalysisWriteLoop::~TAnalysisWriteLoop()
{
   for(auto& elem : fDetMap) {
      delete elem.second;
   }
}

void TAnalysisWriteLoop::ClearQueue()
{
   while(fInputQueue->Size() != 0u) {
      std::shared_ptr<TUnpackedEvent> event;
      fInputQueue->Pop(event);
   }
}

std::string TAnalysisWriteLoop::EndStatus()
{
   std::ostringstream str;
   str << Name() << ":\t" << std::setw(8) << ItemsPopped() << "/" << InputSize() + ItemsPopped() << ", "
       << "??? good events" << std::endl;
   return str.str();
}

void TAnalysisWriteLoop::OnEnd()
{
   Write();
}

bool TAnalysisWriteLoop::Iteration()
{
   std::shared_ptr<TUnpackedEvent> event;
   InputSize(fInputQueue->Pop(event));
   if(InputSize() < 0) {
      InputSize(0);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   } else {
      IncrementItemsPopped();
   }

   if(fOutOfOrder) {
      std::shared_ptr<const TFragment> frag;
      fOutOfOrderQueue->Pop(frag, 0);
      if(frag != nullptr) {
         *fOutOfOrderFrag = *frag;
         fOutOfOrderFrag->ClearTransients();
         std::lock_guard<std::mutex> lock(ttree_fill_mutex);
         fOutOfOrderTree->Fill();
      }
   }

   if(event != nullptr) {
      WriteEvent(event);
      return true;
   }

   return !(fInputQueue->IsFinished());
}

void TAnalysisWriteLoop::Write()
{
   if(fOutputFile != nullptr) {
      gROOT->cd();
      auto* options = TGRSIOptions::Get();
      auto* ppg     = TPPG::Get();
      auto* diag    = TSortingDiagnostics::Get();

      fOutputFile->cd();
      if(GValue::Size() != 0) {
         GValue::Get()->Write("Values", TObject::kOverwrite);
      }
      if(TChannel::GetNumberOfChannels() != 0) {
         TChannel::WriteToRoot();
      }
      TRunInfo::WriteToRoot(fOutputFile);
      TGRSIOptions::AnalysisOptions()->WriteToFile(fOutputFile);
      ppg->Write("PPG");

      if(options->WriteDiagnostics()) {
         diag->Write("SortingDiagnostics", TObject::kOverwrite);
      }

      fOutputFile->Write();
      delete fOutputFile;
      fOutputFile = nullptr;
   }
}

void TAnalysisWriteLoop::AddBranch(TClass* cls)
{
   if(fDetMap.count(cls) == 0u) {
      // This uses the ROOT dictionaries, so we need to lock the threads.
      TThread::Lock();

      // Make a default detector of that type.
      auto* det_p       = reinterpret_cast<TDetector*>(cls->New());
      fDefaultDets[cls] = det_p;

      // Add to our local map
      auto* det_pp = new TDetector*;
      *det_pp      = det_p;
      fDetMap[cls] = det_pp;

      // Make a new branch.
      TBranch* newBranch = fEventTree->Branch(cls->GetName(), cls->GetName(), det_pp);

      // Fill the new branch up to the point where the tree is filled.
      // Explanation:
      //   When TTree::Fill is called, it calls TBranch::Fill for each
      // branch, then increments the number of entries.  We may be
      // adding branches after other branches have already been filled.
      // If the branch 'x' has been filled 100 times before the branch
      // 'y' is created, then the next call to TTree::Fill will fill
      // entry 101 of 'x', but entry 1 of 'y', rather than entry
      // 101 of both.
      //   Therefore, we need to fill the new branch as many times as
      // TTree::Fill has been called before.
      std::lock_guard<std::mutex> lock(ttree_fill_mutex);
      for(int i = 0; i < fEventTree->GetEntries(); i++) {
         newBranch->Fill();
      }

      std::cout << "\r" << std::string(30, ' ') << "\r" << Name() << ": added \"" << cls->GetName() << R"(" branch, )" << det_pp << ", " << det_p << std::string(30, ' ') << std::endl;

      // Unlock after we are done.
      TThread::UnLock();
   }
}

void TAnalysisWriteLoop::WriteEvent(std::shared_ptr<TUnpackedEvent>& event)
{
   if(fEventTree != nullptr) {
      // Clear pointers from previous writes.
      // Note that we cannot just set this equal to nullptr,
      //   because ROOT would then construct a new object.
      // This contradicts the ROOT documentation for TBranchElement::SetAddress,
      //   which suggests that a new object would be constructed only when setting the address,
      //   not when filling the TTree.
      for(auto& elem : fDetMap) {
         (*elem.second)->Clear();
      }

      // Load current events
      for(const auto& det : event->GetDetectors()) {
         TClass* cls = det->IsA();
			// attempt to copy this detector into the detector map
			// if that fails (because the detector isn't in the map yet), create the branch and then copy this detector
         try {
            **fDetMap.at(cls) = *det;
         } catch(std::out_of_range& e) {
            AddBranch(cls);
            **fDetMap.at(cls) = *det;
         }
         (*fDetMap.at(cls))->ClearTransients();
      }

      // Fill
      std::lock_guard<std::mutex> lock(ttree_fill_mutex);
      fEventTree->Fill();
   }
}
