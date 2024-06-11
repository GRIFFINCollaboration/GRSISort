#include "TFragWriteLoop.h"

#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"
#include "TROOT.h"

#include "GValue.h"
#include "TChannel.h"
#include "TRunInfo.h"
#include "TGRSIOptions.h"
#include "TThread.h"
#include "TTreeFillMutex.h"
#include "TParsingDiagnostics.h"

#include "TBadFragment.h"
#include "TScalerQueue.h"

TFragWriteLoop* TFragWriteLoop::Get(std::string name, std::string fOutputFilename)
{
   if(name.length() == 0) {
      name = "write_loop";
   }

   TFragWriteLoop* loop = static_cast<TFragWriteLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      if(fOutputFilename.length() == 0) {
         fOutputFilename = "temp.root";
      }
      loop = new TFragWriteLoop(name, fOutputFilename);
   }
   return loop;
}

TFragWriteLoop::TFragWriteLoop(std::string name, std::string fOutputFilename)
   : StoppableThread(name), fOutputFile(nullptr), fEventTree(nullptr), fBadEventTree(nullptr), fScalerTree(nullptr),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>()),
     fBadInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TBadFragment>>>()),
     fScalerInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TEpicsFrag>>>())
{
   if(fOutputFilename != "/dev/null") {
      TThread::Lock();

      fOutputFile = new TFile(fOutputFilename.c_str(), "RECREATE");
      if(fOutputFile == nullptr || !fOutputFile->IsOpen()) {
         throw std::runtime_error(Form("Failed to open \"%s\"\n", fOutputFilename.c_str()));
      }

      fEventTree    = new TTree("FragmentTree", "FragmentTree");
      fEventAddress = new TFragment;
      fEventTree->Branch("TFragment", &fEventAddress);

      fBadEventTree    = new TTree("BadFragmentTree", "BadFragmentTree");
      fBadEventAddress = new TBadFragment;
      fBadEventTree->Branch("TBadFragment", &fBadEventAddress);

      fScalerTree    = new TTree("EpicsTree", "EpicsTree");
      fScalerAddress = nullptr;
      fScalerTree->Branch("TEpicsFrag", &fScalerAddress);

      TThread::UnLock();
   }
}

TFragWriteLoop::~TFragWriteLoop()
{
   Write();
}

void TFragWriteLoop::ClearQueue()
{
   while(fInputQueue->Size() != 0u) {
      std::shared_ptr<const TFragment> event;
      fInputQueue->Pop(event);
   }
}

std::string TFragWriteLoop::EndStatus()
{
   std::stringstream ss;
   // ss<<"\r"<<Name()<<":\t"<<std::setw(8)<<GetItemsPushed()<<"/"<<(fInputSize>0 ?
   // fInputSize+GetItemsPushed():GetItemsPushed())<<std::endl;
   ss<<std::endl
     <<Name()<<": "<<std::setw(8)<<fItemsPopped<<"/"<<fItemsPopped + fInputSize<<", "
     <<fEventTree->GetEntries()<<" good fragments, "<<fBadEventTree->GetEntries()<<" bad fragments"
     <<std::endl;
   return ss.str();
}

bool TFragWriteLoop::Iteration()
{
   std::shared_ptr<const TFragment> event;
   fInputSize = fInputQueue->Pop(event, 0);
   if(fInputSize < 0) {
      fInputSize = 0;
   }

   std::shared_ptr<const TBadFragment> badEvent;
   fBadInputQueue->Pop(badEvent, 0);

   std::shared_ptr<TEpicsFrag> scaler;
   fScalerInputQueue->Pop(scaler, 0);

   bool hasAnything    = event || badEvent || scaler;
   bool allParentsDead = (fInputQueue->IsFinished() && fBadInputQueue->IsFinished() && fScalerInputQueue->IsFinished());

   if(event != nullptr) {
      WriteEvent(event);
      ++fItemsPopped;
   }

   if(badEvent != nullptr) {
      WriteBadEvent(badEvent);
   }

   if(scaler != nullptr) {
      WriteScaler(scaler);
   }

   if(hasAnything) {
      return true;
   }
   if(allParentsDead) {
      return false;
   }
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return true;
}

void TFragWriteLoop::Write()
{
   if(fOutputFile != nullptr) {
      // get all singletons before switching to the output file
      gROOT->cd();
      TRunInfo*            runInfo            = TRunInfo::Get();
      TGRSIOptions*        options            = TGRSIOptions::Get();
      TPPG*                ppg                = TPPG::Get();
      TParsingDiagnostics* parsingDiagnostics = TParsingDiagnostics::Get();
      GValue*              gValues            = GValue::Get();

      fOutputFile->cd();
      fEventTree->Write(fEventTree->GetName(), TObject::kOverwrite);
      fBadEventTree->Write(fBadEventTree->GetName(), TObject::kOverwrite);
      fScalerTree->Write(fScalerTree->GetName(), TObject::kOverwrite);
      if(GValue::Size() != 0) {
         gValues->Write("Values", TObject::kOverwrite);
      }

      if(TChannel::GetNumberOfChannels() != 0) {
         TChannel::WriteToRoot();
      }

      runInfo->WriteToRoot(fOutputFile);
      options->WriteToFile(fOutputFile);
      ppg->Write("PPG");

      if(options->WriteDiagnostics()) {
         parsingDiagnostics->ReadPPG(ppg);   // this set's the cycle length from the PPG information
         parsingDiagnostics->Write("ParsingDiagnostics", TObject::kOverwrite);
      }

      if(!options->IgnoreScaler()) {
         std::cout<<"Starting to write dead time scalers"<<std::endl;
         auto         deadtimeQueue = TDeadtimeScalerQueue::Get();
         auto         scalerTree    = new TTree("DeadtimeScaler", "DeadtimeScaler");
         TScalerData* scalerData    = new TScalerData;
         scalerTree->Branch("ScalerData", &scalerData);
         while(deadtimeQueue->Size() > 0) {
            scalerData = deadtimeQueue->PopScaler();
            scalerTree->Fill();
         }
         scalerTree->Write();

         std::cout<<"Starting to write rate scalers"<<std::endl;
         auto rateQueue = TRateScalerQueue::Get();
         scalerTree     = new TTree("RateScaler", "RateScaler");
         scalerData     = new TScalerData;
         scalerTree->Branch("ScalerData", &scalerData);
         while(rateQueue->Size() > 0) {
            scalerData = rateQueue->PopScaler();
            scalerTree->Fill();
         }
         scalerTree->Write();
         std::cout<<"Done writing scaler trees"<<std::endl;
      }

      fOutputFile->Close();
      fOutputFile->Delete();
      gROOT->cd();
   }
}

void TFragWriteLoop::WriteEvent(const std::shared_ptr<const TFragment>& event)
{
   if(fEventTree != nullptr) {
      *fEventAddress = *event;
      fEventAddress->ClearTransients();
      std::lock_guard<std::mutex> lock(ttree_fill_mutex);
      fEventTree->Fill();
      // fEventAddress = nullptr;
   } else {
      std::cout<<__PRETTY_FUNCTION__<<": no fragment tree!"<<std::endl;
   }
}

void TFragWriteLoop::WriteBadEvent(const std::shared_ptr<const TBadFragment>& event)
{
   if(fBadEventTree != nullptr) {
      *fBadEventAddress = *static_cast<const TBadFragment*>(event.get());
      std::lock_guard<std::mutex> lock(ttree_fill_mutex);
      fBadEventTree->Fill();
   }
}

void TFragWriteLoop::WriteScaler(const std::shared_ptr<TEpicsFrag>& scaler)
{
   if(fScalerTree != nullptr) {
      fScalerAddress = scaler.get();
      std::lock_guard<std::mutex> lock(ttree_fill_mutex);
      fScalerTree->Fill();
      fScalerAddress = nullptr;
   }
}
