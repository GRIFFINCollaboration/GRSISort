#include "TFragDiagnosticsLoop.h"

#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"
#include "TROOT.h"
#include "TH2.h"

#include "GValue.h"
#include "TChannel.h"
#include "TRunInfo.h"
#include "TGRSIOptions.h"
#include "TTreeFillMutex.h"
#include "TParsingDiagnostics.h"

#include "TBadFragment.h"
#include "TScalerQueue.h"

TFragDiagnosticsLoop* TFragDiagnosticsLoop::Get(std::string name, std::string fOutputFilename)
{
   if(name.empty()) {
      name = "diag_loop";
   }

   auto* loop = static_cast<TFragDiagnosticsLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      if(fOutputFilename.empty()) {
         fOutputFilename = "temp.root";
      }
      loop = new TFragDiagnosticsLoop(name, fOutputFilename);
   }
   return loop;
}

TFragDiagnosticsLoop::TFragDiagnosticsLoop(std::string name, const std::string& fOutputFilename)
   : StoppableThread(std::move(name)),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>())
{
   if(fOutputFilename != "/dev/null") {
      TThread::Lock();

      fOutputFile = new TFile(fOutputFilename.c_str(), "RECREATE");
      if(fOutputFile == nullptr || !fOutputFile->IsOpen()) {
         throw std::runtime_error(Form("Failed to open \"%s\"\n", fOutputFilename.c_str()));
      }

      // start with 1 s binning
      int nofTimeBins = static_cast<int>(TRunInfo::RunLength());
      if(nofTimeBins < 1000) {
         // if we can, increase the binning to 100 ms binning
         nofTimeBins = static_cast<int>(TRunInfo::RunLength()*10.);
      } else if(nofTimeBins > 10000) {
         // if we have to, decrease the binning to 10 s binning
         nofTimeBins = static_cast<int>(TRunInfo::RunLength()/10.);
         std::cout << "Warning, run is more than 10000 s long (run length " << TRunInfo::RunLength() << " s), reducing binning to 10 s (number of bins " << nofTimeBins << ")!" << std::endl;
      }

      fAccepted = new TH2D("accepted", "Accepted Channel ID vs. Channel Address;Channel Address;Accepted Channel ID",
            TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), 10000, 0, 1e6);
      fLostNetworkPackets = new TH1D("lostNetworkPackets", "lost network packets;time [s];lost network packets",
            nofTimeBins, TRunInfo::RunStart(), TRunInfo::RunStop());
      fLostChannelIds = new TH2D("lostChannelIds", "Lost Channel Id vs. Channel Number;Channel Number;Lost Channel Id",
            TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), 10000, 0, 1e6);
      fLostAcceptedIds = new TH2D("lostAcceptedIds", "Lost Accepted Channel Id vs. Channel Number;Channel Number;Lost Accepted Channel Id",
            TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), 10000, 0, 1e6);
      fLostChannelIdsTime = new TH2D("lostChannelIdsTime", "Lost Channel Id time vs. Channel Number;Channel Number;time [s]",
            TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), nofTimeBins, TRunInfo::RunStart(), TRunInfo::RunStop());
      fLostAcceptedIdsTime = new TH2D("lostAcceptedIdsTime", "Lost Accepted Channel Id time vs. Channel Number;Channel Number;time [s]",
            TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), nofTimeBins, TRunInfo::RunStart(), TRunInfo::RunStop());


      int bin = 1;
      for(const auto& channel : *TChannel::GetChannelMap()) {
         fAccepted->GetXaxis()->SetBinLabel(bin, Form("0x%04x", channel.first));
         ++bin;
      }
      TThread::UnLock();
   }
}

TFragDiagnosticsLoop::~TFragDiagnosticsLoop()
{
   Write();
}

void TFragDiagnosticsLoop::ClearQueue()
{
   while(fInputQueue->Size() != 0u) {
      std::shared_ptr<const TFragment> event;
      fInputQueue->Pop(event);
   }
}

std::string TFragDiagnosticsLoop::EndStatus()
{
   std::ostringstream str;
   str << std::endl
       << Name() << ": " << std::setw(8) << ItemsPopped() << "/" << ItemsPopped() + InputSize() << std::endl;
   return str.str();
}

bool TFragDiagnosticsLoop::Iteration()
{
   std::shared_ptr<const TFragment> event;
   InputSize(fInputQueue->Pop(event, 0));
   if(InputSize() < 0) {
      InputSize(0);
   }

   if(event != nullptr) {
      Process(event);
      IncrementItemsPopped();
      return true;
   }

   if(fInputQueue->IsFinished()) {
      return false;
   }
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return true;
}

void TFragDiagnosticsLoop::Write()
{
   if(fOutputFile != nullptr) {
      // get all singletons before switching to the output file
      gROOT->cd();
      TGRSIOptions*        options            = TGRSIOptions::Get();
      TPPG*                ppg                = TPPG::Get();
      TParsingDiagnostics* parsingDiagnostics = TParsingDiagnostics::Get();
      GValue*              gValues            = GValue::Get();

      fOutputFile->cd();

      fAccepted->Write();
      fLostNetworkPackets->Write();
      fLostChannelIds->Write();
      fLostAcceptedIds->Write();
      fLostChannelIdsTime->Write();
      fLostAcceptedIdsTime->Write();

      if(GValue::Size() != 0) {
         gValues->Write("Values", TObject::kOverwrite);
      }

      if(TChannel::GetNumberOfChannels() != 0) {
         TChannel::WriteToRoot();
      }

      TRunInfo::WriteToRoot(fOutputFile);
      TGRSIOptions::WriteToFile(fOutputFile);
      ppg->Write("PPG");

      if(options->WriteDiagnostics()) {
         parsingDiagnostics->ReadPPG(ppg);   // this set's the cycle length from the PPG information
         parsingDiagnostics->Write("ParsingDiagnostics", TObject::kOverwrite);
      }

      fOutputFile->Close();
      fOutputFile->Delete();
      gROOT->cd();
   }
}

void TFragDiagnosticsLoop::Process(const std::shared_ptr<const TFragment>& event)
{
   auto timeStamp = event->GetTimeStamp();
   auto address = event->GetAddress();
   auto acceptedId = event->GetAcceptedChannelId();
   auto channelId = event->GetChannelId();
   auto networkPacket = event->GetNetworkPacketNumber();

   //---------------- this section deals with the rolling over of the AcceptedChannelId. -------------------//
   // if we do not have an entry in the fRolling map, it will be created as false
   if(!fRolling[address] && acceptedId > (fAcceptedMax - fRollingThreshold)) {
      ++fRollnum[address];
   }
   if(fRolling[address] && acceptedId > fRollingThreshold && acceptedId < (fAcceptedMax / 2)) {
      ++fRollnum[address];
   }
   if(!fRolling[address] && fRollnum[address] > fRollnumThreshold) {
      fRolling[address] = true;
      fRollnum[address] = 0;
   }
   if(fRolling[address] && fRollnum[address] > fRollnumThreshold) {
      fRolling[address] = false;
      fRollnum[address] = 0;
      ++fNofRollovers[address];
   }

   if(fRolling[address] && acceptedId < fRollingThreshold * 2) {
      acceptedId += fAcceptedMax;
   }
   acceptedId += fNofRollovers[address] * fAcceptedMax;

   // if we have a new network packet number
   if(networkPacket != 0 && fNetworkPacketNumber[0] != 0) {
      if(fNetworkPacketNumber[0] < fNetworkPacketNumber[1] && fNetworkPacketNumber[1] < fNetworkPacketNumber[2]) {
         for(int packet = fNetworkPacketNumber[0] + 1; packet < fNetworkPacketNumber[1]; ++packet) {
            fLostNetworkPackets->Fill(static_cast<double>(fNetworkPacketTimeStamp[1]) / 1e8);
         }
         // things look fine, so prepare for next time
         fNetworkPacketNumber[0]    = fNetworkPacketNumber[1];
         fNetworkPacketNumber[1]    = fNetworkPacketNumber[2];
         fNetworkPacketTimeStamp[0] = fNetworkPacketTimeStamp[1];
         fNetworkPacketTimeStamp[1] = fNetworkPacketTimeStamp[2];
      } else if(fNetworkPacketNumber[0] < fNetworkPacketNumber[2]) {
         std::cout << "found wrong network packet number 0x" << std::hex << fNetworkPacketNumber[1]
            << " (not between 0x" << fNetworkPacketNumber[0] << " and 0x" << fNetworkPacketNumber[2] << std::dec
            << ")" << std::endl;
         fNetworkPacketNumber[1]    = fNetworkPacketNumber[2];
         fNetworkPacketTimeStamp[1] = fNetworkPacketTimeStamp[2];
      }
      // these two statements used to be before the if statement, but that seemed wrong?
      fNetworkPacketNumber[2]    = networkPacket;
      fNetworkPacketTimeStamp[2] = timeStamp;
   } else if(networkPacket != 0 && fNetworkPacketNumber[0] == 0) {
      fNetworkPacketNumber[0]    = fNetworkPacketNumber[1];
      fNetworkPacketNumber[1]    = fNetworkPacketNumber[2];
      fNetworkPacketNumber[2]    = networkPacket;
      fNetworkPacketTimeStamp[0] = fNetworkPacketTimeStamp[1];
      fNetworkPacketTimeStamp[1] = fNetworkPacketTimeStamp[2];
      fNetworkPacketTimeStamp[2] = timeStamp;
   }

   // check if the "middle" channel ID is reasonable and fill all IDs we've missed between the first and middle ID
   if(fTimeStamps[address][0] != 0) {
      fTimeStamps[address][2]  = timeStamp;
      fChannelIds[address][2] = channelId;
      if(fTimeStamps[address][0] > 0 && fChannelIds[address][0] < fChannelIds[address][1] &&
            fChannelIds[address][1] < fChannelIds[address][2]) {
         for(int id = fChannelIds[address][0] + 1; id < fChannelIds[address][1]; ++id) {
            fLostChannelIds->Fill(Form("0x%04x", address), id, 1.);
            fLostChannelIdsTime->Fill(Form("0x%04x", address), static_cast<double>(fTimeStamps[address][1]) / 1e8, 1.);
         }
         fTimeStamps[address][0]  = fTimeStamps[address][1];
         fTimeStamps[address][1]  = fTimeStamps[address][2];
         fChannelIds[address][0] = fChannelIds[address][1];
         fChannelIds[address][1] = fChannelIds[address][2];
      } else {
         // now, we only want to overwrite the bad timestamp
         fTimeStamps[address][1]  = fTimeStamps[address][2];
         fChannelIds[address][1] = fChannelIds[address][2];
      }
   } else {
      fTimeStamps[address][0]          = fTimeStamps[address][1];
      fTimeStamps[address][1]          = fTimeStamps[address][2];
      fChannelIds[address][0]         = fChannelIds[address][1];
      fChannelIds[address][1]         = fChannelIds[address][2];
      fTimeStamps[address][2]          = timeStamp;
      fChannelIds[address][2]         = channelId;
      fAcceptedChannelIds[address][0] = fAcceptedChannelIds[address][1];
      fAcceptedChannelIds[address][1] = fAcceptedChannelIds[address][2];
      fAcceptedChannelIds[address][2] = acceptedId;
   }

   fAccepted->Fill(Form("0x%04x", address), static_cast<double>(acceptedId), 1.);

   // check if the "middle" accepted channel ID is reasonable and fill all IDs we've missed between the first and
   // middle ID
   if(fTimeStamps[address][0] != 0) {
      fAcceptedChannelIds[address][2] = acceptedId;
      if(fTimeStamps[address][0] > 0 && fAcceptedChannelIds[address][0] < fAcceptedChannelIds[address][1] &&
            fAcceptedChannelIds[address][1] < fAcceptedChannelIds[address][2]) {
         for(int id = fAcceptedChannelIds[address][0] + 1; id < fAcceptedChannelIds[address][1]; ++id) {
            fLostAcceptedIds->Fill(Form("0x%04x", address), id, 1.);
            fLostAcceptedIdsTime->Fill(Form("0x%04x", address), static_cast<double>(fTimeStamps[address][1]) / 1e8, 1.);
         }
         fAcceptedChannelIds[address][0] = fAcceptedChannelIds[address][1];
         fAcceptedChannelIds[address][1] = fAcceptedChannelIds[address][2];
      } else {
         fAcceptedChannelIds[address][1] = fAcceptedChannelIds[address][2];
      }
   }
}
