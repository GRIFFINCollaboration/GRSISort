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

      // we delay the creating of histograms until we need them so that all the information needed is present
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
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

      // check our final run length and compare to the binning we chose
      if(TRunInfo::RunLength() > fRunLength) {
         std::cerr << Name() << ": got a final run length of " << TRunInfo::RunLength() << " s, instead of the assumed run length of " << fRunLength << " s, some diagnostics information will be missing!" << std::endl;
      } else if(TRunInfo::RunLength() < fRunLength / 2) {
         // TODO: if we have less than half the range, resize the histograms (maybe via a general function?)
      }
      // switch to the output file
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

bool TFragDiagnosticsLoop::CreateHistograms()
{
   std::cout << Name() << ": creating histograms using output file " << fOutputFile << std::endl;

   if(fOutputFile == nullptr || !fOutputFile->IsOpen()) {
      return false;
   }
   fOutputFile->cd();

   // at this point we have read the first midas event and have a run start, but not a run stop yet (we will only find that one out at the end of the run)
   // so for now we assume a run length of 3600 seconds
   fRunLength = (TRunInfo::RunLength() > 0. ? static_cast<int>(TRunInfo::RunLength()) : 3600);
   // start with 1 s binning
   int nofTimeBins = fRunLength;
   if(nofTimeBins < 1000) {
      // if we can, increase the binning to 100 ms binning
      nofTimeBins = fRunLength * 10;
   } else if(nofTimeBins > 10000) {
      // if we have to, decrease the binning to 10 s binning
      nofTimeBins = fRunLength / 10;
      std::cout << "Warning, run is more than 10000 s long (run length " << TRunInfo::RunLength() << " s), reducing binning to 10 s (number of bins " << nofTimeBins << ")!" << std::endl;
   }

   std::cout << Name() << ": creating histograms with " << nofTimeBins << " time bins from 0 to " << fRunLength << std::endl;

   fAccepted            = new TH2D("accepted", "Accepted Channel ID vs. Channel Address;Channel Address;Accepted Channel ID",
                                   TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), 10000, 0, 1e6);
   fLostNetworkPackets  = new TH1D("lostNetworkPackets", "lost network packets;time [s];lost network packets",
                                   nofTimeBins, 0., fRunLength);
   fLostChannelIds      = new TH2D("lostChannelIds", "Lost Channel Id vs. Channel Number;Channel Number;Lost Channel Id",
                                   TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), 10000, 0, 1e6);
   fLostAcceptedIds     = new TH2D("lostAcceptedIds", "Lost Accepted Channel Id vs. Channel Number;Channel Number;Lost Accepted Channel Id",
                                   TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), 10000, 0, 1e6);
   fLostChannelIdsTime  = new TH2D("lostChannelIdsTime", "Lost Channel Id time vs. Channel Number;Channel Number;time [s]",
                                   TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), nofTimeBins, 0., fRunLength);
   fLostAcceptedIdsTime = new TH2D("lostAcceptedIdsTime", "Lost Accepted Channel Id time vs. Channel Number;Channel Number;time [s]",
                                   TChannel::GetNumberOfChannels(), 0, static_cast<Double_t>(TChannel::GetNumberOfChannels()), nofTimeBins, 0., fRunLength);

   int bin = 1;
   for(const auto& channel : *TChannel::GetChannelMap()) {
      fAccepted->GetXaxis()->SetBinLabel(bin, Form("0x%04x", channel.first));
      fLostChannelIds->GetXaxis()->SetBinLabel(bin, Form("0x%04x", channel.first));
      fLostAcceptedIds->GetXaxis()->SetBinLabel(bin, Form("0x%04x", channel.first));
      fLostChannelIdsTime->GetXaxis()->SetBinLabel(bin, Form("0x%04x", channel.first));
      fLostAcceptedIdsTime->GetXaxis()->SetBinLabel(bin, Form("0x%04x", channel.first));
      ++bin;
   }

   return true;
}

void TFragDiagnosticsLoop::Process(const std::shared_ptr<const TFragment>& event)
{
   if(fAccepted == nullptr || fLostNetworkPackets == nullptr || fLostChannelIds == nullptr ||
      fLostAcceptedIds == nullptr || fLostChannelIdsTime == nullptr || fLostAcceptedIdsTime == nullptr) {
      if(!CreateHistograms()) {
         throw std::runtime_error(Name() + ": trying to create histograms failed, can't produce diagnostics!");
      }
   }
   auto timeStamp     = event->GetTimeStampNs();
   auto address       = event->GetAddress();
   auto acceptedId    = event->GetAcceptedChannelId();
   auto channelId     = event->GetChannelId();
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

   // check whether we had this channel before and if not, initialize all arrays
   if(fChannelIds.find(address) == fChannelIds.end()) {
      fChannelIds[address]         = {0, 0};
      fAcceptedChannelIds[address] = {0, 0};
      fTimeStamps[address]         = {0, 0};
   }

   // if we have a new network packet number
   if(networkPacket != 0 && fNetworkPacketNumber[0] != 0) {
      // we get here once we get the 3rd network packet number, so now
      // [1] is the last, [0] the second to last

      // we don't need to check that [0] is smaller than [1]
      if(fNetworkPacketNumber[1] < networkPacket) {
         // we get a new highest network packet number, so count all numbers between the second to last and the last one
         // before we update things
         for(int packet = fNetworkPacketNumber[0] + 1; packet < fNetworkPacketNumber[1]; ++packet) {
            fLostNetworkPackets->Fill(static_cast<double>(fNetworkPacketTimeStamp[1]) / 1e9);
         }
         // things look fine, so prepare for next time
         fNetworkPacketNumber[0]    = fNetworkPacketNumber[1];
         fNetworkPacketNumber[1]    = networkPacket;
         fNetworkPacketTimeStamp[0] = fNetworkPacketTimeStamp[1];
         fNetworkPacketTimeStamp[1] = timeStamp;
         // [1] is now the current packet number, [0] is the last packet number
      } else if(networkPacket < fNetworkPacketNumber[1]) {
         //std::cout << "found wrong network packet number " << hex(networkPacket, 8)
         //          << " (between " << hex(fNetworkPacketNumber[0], 8) << " and " << hex(fNetworkPacketNumber[1]) << ", 8)" << std::endl;
         // we update [1] to the current packet number here, not sure if that is the best way to deal with this situation?
         fNetworkPacketNumber[1]    = networkPacket;
         fNetworkPacketTimeStamp[1] = timeStamp;
      }
      // It seems that we can get the same packet number multiple times? But if that happens there's nothing to do so no need to check.
   } else if(networkPacket != 0) {
      // if [0] is 0, we haven't got 2 network packets numbers yet, so we just copy them through
      // that means after this [1] is the current, and [0] the last
      fNetworkPacketNumber[0]    = fNetworkPacketNumber[1];
      fNetworkPacketNumber[1]    = networkPacket;
      fNetworkPacketTimeStamp[0] = fNetworkPacketTimeStamp[1];
      fNetworkPacketTimeStamp[1] = timeStamp;
   }

   // check if the "middle" channel ID is reasonable and fill all IDs we've missed between the first and middle ID
   if(fChannelIds[address][0] != 0) {
      if(fChannelIds[address][0] < fChannelIds[address][1] && fChannelIds[address][1] < channelId) {
         for(int id = fChannelIds[address][0] + 1; id < fChannelIds[address][1]; ++id) {
            fLostChannelIds->Fill(Form("0x%04x", address), id, 1.);
            fLostChannelIdsTime->Fill(Form("0x%04x", address), static_cast<double>(fTimeStamps[address][1]) / 1e9, 1.);
         }
         fTimeStamps[address][0] = fTimeStamps[address][1];
         fChannelIds[address][0] = fChannelIds[address][1];
      }
      // we either got a good new timestamp and channel ID and have copied the information from [1] to [0]
      // or we had a bad timestamp that we overwrite now
      fTimeStamps[address][1] = timeStamp;
      fChannelIds[address][1] = channelId;
   } else {
      fTimeStamps[address][0] = fTimeStamps[address][1];
      fChannelIds[address][0] = fChannelIds[address][1];
      fTimeStamps[address][1] = timeStamp;
      fChannelIds[address][1] = channelId;
   }

   fAccepted->Fill(Form("0x%04x", address), static_cast<double>(acceptedId), 1.);

   // check if the "middle" accepted channel ID is reasonable and fill all IDs we've missed between the first and
   // middle ID
   if(fTimeStamps[address][0] != 0) {
      if(fAcceptedChannelIds[address][0] < fAcceptedChannelIds[address][1] && fAcceptedChannelIds[address][1] < acceptedId) {
         for(int id = fAcceptedChannelIds[address][0] + 1; id < fAcceptedChannelIds[address][1]; ++id) {
            fLostAcceptedIds->Fill(Form("0x%04x", address), id, 1.);
            fLostAcceptedIdsTime->Fill(Form("0x%04x", address), static_cast<double>(fTimeStamps[address][1]) / 1e9, 1.);
         }
         fAcceptedChannelIds[address][0] = fAcceptedChannelIds[address][1];
      }
      fAcceptedChannelIds[address][1] = acceptedId;
   }
}
