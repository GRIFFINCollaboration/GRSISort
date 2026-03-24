#include "TEventBuildingLoop.h"

#include "TGRSIOptions.h"
#include "TSortingDiagnostics.h"
#include "TRunInfo.h"

#include <chrono>
#include <thread>
#include <fstream>

TEventBuildingLoop* TEventBuildingLoop::Get(std::string name, EBuildMode mode, uint64_t buildWindow)
{
   if(name.empty()) {
      name = "build_loop";
   }

   auto* loop = static_cast<TEventBuildingLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      loop = new TEventBuildingLoop(name, mode, buildWindow);
   }
   return loop;
}

TEventBuildingLoop::TEventBuildingLoop(std::string name, EBuildMode mode, uint64_t buildWindow)
   : StoppableThread(std::move(name)), fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TFragment>>>()),
     fOutputQueue(std::make_shared<ThreadsafeQueue<std::vector<std::shared_ptr<TFragment>>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TFragment>>>()), fBuildMode(mode),
     fSortingDepth(10000), fBuildWindow(buildWindow), fPreviousSortingDepthError(false), fSkipInputSort(TGRSIOptions::Get()->SkipInputSort())
{
   std::cout << DYELLOW << (fSkipInputSort ? "Not sorting " : "Sorting ") << "input by time: ";
   switch(fBuildMode) {
   case EBuildMode::kTime:
      fOrdered = decltype(fOrdered)([](const std::pair<std::shared_ptr<TFragment>, Long64_t>& a, const std::pair<std::shared_ptr<TFragment>, Long64_t>& b) {
         return a.first->GetTime() < b.first->GetTime();
      });
      std::cout << DYELLOW << "sorting by time, using build window of " << fBuildWindow << "!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kTimestamp:
      fOrdered = decltype(fOrdered)([](const std::pair<std::shared_ptr<TFragment>, Long64_t>& a, const std::pair<std::shared_ptr<TFragment>, Long64_t>& b) {
         return a.first->GetTimeStampNs() < b.first->GetTimeStampNs();
      });
      std::cout << DYELLOW << "sorting by timestamp, using build window of " << fBuildWindow << "!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kTriggerId:
      fOrdered = decltype(fOrdered)([](const std::pair<std::shared_ptr<TFragment>, Long64_t>& a, const std::pair<std::shared_ptr<TFragment>, Long64_t>& b) {
         return a.first->GetTriggerId() < b.first->GetTriggerId();
      });
      std::cout << DYELLOW << "sorting by trigger ID!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kSkip:
      // no need for ordering, always return true
      fOrdered = decltype(fOrdered)([](const std::pair<std::shared_ptr<TFragment>, Long64_t>&, const std::pair<std::shared_ptr<TFragment>, Long64_t>&) {
         return true;
      });
      std::cout << DYELLOW << "not sorting!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kDefault:
      std::cout << "build mode was " << static_cast<int>(fBuildMode) << ", not " << static_cast<int>(EBuildMode::kTimestamp) << ", or " << static_cast<int>(EBuildMode::kTriggerId) << std::endl;
      throw std::runtime_error("Error in event building loop, no build mode selected. Maybe because no custom run info was loaded?");
      break;
   }

   fOffsetFile = Form("offset%05d.txt", TRunInfo::RunNumber());
   // if this is not the first sub-run, try to read the offset file
   if(TRunInfo::SubRunNumber() > 0) {
      std::ifstream file(fOffsetFile);
      file >> fDaqTimeStampOffset;
      std::cout << "Using previously determined daq time offset " << fDaqTimeStampOffset << " (read from " << fOffsetFile << ")" << std::endl;
      file.close();
   }

   auto* ppg    = TPPG::Get();
   fCycleLength = ppg->OdbCycleLength();

   // the beam off where we collect data is the last step, so we add up all duration until we get there
   for(size_t i = 0; i < ppg->OdbPPGSize(); ++i) {
      if(ppg->OdbPPGCode(i) == 0xc008) {
         fTapeMoveLength = ppg->OdbDuration(i);
      }
      if(ppg->OdbPPGCode(i) == 0xc002) {
         fBackgroundLength = ppg->OdbDuration(i);
      }
      if(ppg->OdbPPGCode(i) == 0xc001) {
         fImplantDaqOnLength = ppg->OdbDuration(i);
      }
      if(ppg->OdbPPGCode(i) == 0x8001) {
         fImplantDaqOffLength = ppg->OdbDuration(i);
      }
      if(ppg->OdbPPGCode(i) == 0x8004) {
         fDecayDaqOffLength = ppg->OdbDuration(i);
      }
   }
   std::cout << "Using cycle length " << static_cast<double>(fCycleLength) / 1e6 << ", tape move length " << static_cast<double>(fTapeMoveLength) / 1e6
             << ", background length " << static_cast<double>(fBackgroundLength) / 1e6 << ", implant length (DAQ on) " << static_cast<double>(fImplantDaqOnLength) / 1e6
             << ", implant length (DAQ off) " << static_cast<double>(fImplantDaqOffLength) / 1e6 << ", and decay (DAQ off) length "
             << static_cast<double>(fDecayDaqOffLength) / 1e6 << " to build events!" << std::endl;
}

TEventBuildingLoop::~TEventBuildingLoop() = default;

void TEventBuildingLoop::ClearQueue()
{
   std::shared_ptr<TFragment> singleEvent;
   while(fInputQueue->Size() != 0u) {
      fInputQueue->Pop(singleEvent);
   }

   std::vector<std::shared_ptr<TFragment>> event;
   while(fOutputQueue->Size() != 0u) {
      fOutputQueue->Pop(event);
   }
}

bool TEventBuildingLoop::Iteration()
{
   // Pull something off of the input queue.
   std::shared_ptr<TFragment> inputFragment = nullptr;
   InputSize(fInputQueue->Pop(inputFragment, 0));
   if(InputSize() < 0) {
      InputSize(0);
   }

   if(inputFragment != nullptr) {
      Long64_t originalTimeStamp = inputFragment->GetTimeStamp();
      CheckWrapAround(inputFragment);
      IncrementItemsPopped();
      if(!fSkipInputSort) {
         fOrdered.insert(std::make_pair(inputFragment, originalTimeStamp));
         if(fOrdered.size() < fSortingDepth) {
            // Got a new event, but we want to have more to sort
            return true;
         }
         // Got a new event, and we have enough to sort.
      }
   } else {
      if(!fInputQueue->IsFinished()) {
         // If the parent is live, wait for it
         std::this_thread::sleep_for(std::chrono::milliseconds(1000));
         return true;
      }
      if(fOrdered.empty()) {
         // Parent is dead, and we have passed on all events
         // check if last event needs to be pushed
         if(!fNextEvent.empty()) {
            fOutputQueue->Push(fNextEvent);
         }
         fOutputQueue->SetFinished();
         return false;
      }
      // Parent is dead, but we still have items.
      // Continue through the function to process them.
   }

   // We have data, and we want to add it to the next fragment;
   std::shared_ptr<TFragment> nextFragment;
   if(!fSkipInputSort) {
      nextFragment = (*fOrdered.begin()).first;
      fOrdered.erase(fOrdered.begin());
   } else {
      nextFragment = inputFragment;
   }

   if(CheckBuildCondition(nextFragment)) {
      fNextEvent.push_back(nextFragment);
   }

   return true;
}

bool TEventBuildingLoop::CheckBuildCondition(const std::shared_ptr<TFragment>& frag)
{
   switch(fBuildMode) {
   case EBuildMode::kTime: return CheckTimeCondition(frag); break;
   case EBuildMode::kTimestamp: return CheckTimestampCondition(frag); break;
   case EBuildMode::kTriggerId: return CheckTriggerIdCondition(frag); break;
   case EBuildMode::kSkip:
      // always push the current "event" (single fragment) on and clear it
      fOutputQueue->Push(fNextEvent);
      fNextEvent.clear();
      return true;
      break;
   default: return false;
   }
   return false;   // we should never reach this statement!
}

bool TEventBuildingLoop::CheckTimeCondition(const std::shared_ptr<TFragment>& frag)
{
   double time       = frag->GetTime();
   double eventStart = (!fNextEvent.empty() ? (TGRSIOptions::AnalysisOptions()->StaticWindow() ? fNextEvent[0]->GetTime()
                                                                                               : fNextEvent.back()->GetTime())
                                            : time);

   // save time every <BuildWindow> fragments
   if(frag->GetEntryNumber() % (TGRSIOptions::Get()->SortDepth()) == 0) {
      TSortingDiagnostics::Get()->AddTime(eventStart);
   }
   if(time > eventStart + static_cast<double>(fBuildWindow) || time < eventStart - static_cast<double>(fBuildWindow)) {
      fOutputQueue->Push(fNextEvent);
      fNextEvent.clear();
   }

   if(time < eventStart) {
      TSortingDiagnostics::Get()->OutOfTimeOrder(time, eventStart, frag->GetEntryNumber());
      if(!fPreviousSortingDepthError) {
         std::cerr.precision(12);
         std::cerr << std::endl
                   << "Sorting depth of " << fSortingDepth << " was insufficient. time: " << std::setw(16) << time
                   << " Last: " << std::setw(16) << eventStart << " \n"
                   << "Not all events were built correctly" << std::endl;
         std::cerr << "Please increase sort depth with --sort-depth=N, if needed" << std::endl;
         fPreviousSortingDepthError = true;
      }
      if(TGRSIOptions::Get()->SeparateOutOfOrder()) {
         fOutOfOrderQueue->Push(frag);
         return false;
      }
   }

   return true;
}

bool TEventBuildingLoop::CheckTimestampCondition(const std::shared_ptr<TFragment>& frag)
{
   uint64_t timestamp  = frag->GetTimeStampNs();
   uint64_t eventStart = (!fNextEvent.empty() ? (TGRSIOptions::AnalysisOptions()->StaticWindow() ? fNextEvent[0]->GetTimeStampNs()
                                                                                                 : fNextEvent.back()->GetTimeStampNs())
                                              : timestamp);

   // save timestamp every <BuildWindow> fragments
   if(frag->GetEntryNumber() % (TGRSIOptions::Get()->SortDepth()) == 0) {
      TSortingDiagnostics::Get()->AddTimeStamp(eventStart);
   }
   if(timestamp > eventStart + fBuildWindow || timestamp < eventStart - fBuildWindow) {
      fOutputQueue->Push(fNextEvent);
      fNextEvent.clear();
   }

   if(timestamp < eventStart) {
      TSortingDiagnostics::Get()->OutOfOrder(timestamp, eventStart, frag->GetEntryNumber());
      if(!fPreviousSortingDepthError) {
         std::cerr << std::endl
                   << "Sorting depth of " << fSortingDepth << " was insufficient. timestamp: " << std::setw(16) << timestamp
                   << " Last: " << std::setw(16) << eventStart << " \n"
                   << "Not all events were built correctly" << std::endl;
         std::cerr << "Please increase sort depth with --sort-depth=N, if needed" << std::endl;
         fPreviousSortingDepthError = true;
      }
      if(TGRSIOptions::Get()->SeparateOutOfOrder()) {
         fOutOfOrderQueue->Push(frag);
         return false;
      }
   }

   return true;
}

bool TEventBuildingLoop::CheckTriggerIdCondition(const std::shared_ptr<TFragment>& frag)
{
   int64_t triggerId        = frag->GetTriggerId();
   int64_t currentTriggerId = (!fNextEvent.empty() ? fNextEvent[0]->GetTriggerId() : triggerId);

   // save trigger id every <BuildWindow> fragments
   if(frag->GetEntryNumber() % (TGRSIOptions::Get()->SortDepth()) == 0) {
      TSortingDiagnostics::Get()->AddTimeStamp(currentTriggerId);
   }

   if(triggerId != currentTriggerId) {
      fOutputQueue->Push(fNextEvent);
      fNextEvent.clear();
   }

   if(triggerId < currentTriggerId) {
      TSortingDiagnostics::Get()->OutOfOrder(triggerId, currentTriggerId, frag->GetEntryNumber());
      if(!fPreviousSortingDepthError) {
         std::cerr << std::endl
                   << "Sorting depth of " << fSortingDepth << " was insufficient.\n"
                   << "Not all events were built correctly" << std::endl;
         std::cerr << "Trigger id #" << triggerId << " was incorrectly sorted before "
                   << "trigger id #" << currentTriggerId << std::endl;
         std::cerr << "Please increase sort depth with --sort-depth=N, if needed" << std::endl;
         fPreviousSortingDepthError = true;
      }
      if(TGRSIOptions::Get()->SeparateOutOfOrder()) {
         fOutOfOrderQueue->Push(frag);
         return false;
      }
   }

   return true;
}

std::string TEventBuildingLoop::EndStatus()
{
   std::ostringstream str;
   str << fInputQueue->Name() << ": " << ItemsPopped() << "/" << fInputQueue->ItemsPopped() << " items popped"
       << std::endl;

   return str.str();
}

void TEventBuildingLoop::CheckWrapAround(const std::shared_ptr<TFragment>& frag)
{
   /// We check the wrap around due to us turning the DAQ off during the implant
   /// and back on during the decay.
   /// For this we use the very first fragment to determine the offset of the DAQ timestamp
   /// (which is time in s from 01.1.1970), and then compare the difference between the timestamp in seconds
   /// and the offset corrected DAQ timestamp to calculate the wrap around.
   /// Comments below are outdated!
   /// If we are past the background plus both implants (ignoring the decay w/ DAQ off) we correct the timestamp
   /// by adding the background, both implants, and the decay w/ DAQ on plus (the difference between TS and corr. DAQ TS
   /// divided by the cycle length) times the cycle length. The latter ensure we only add full cycles.

   Long64_t timeStamp    = frag->GetTimeStampNs();

   // first we check that the time difference between this hit and the last (good) hit for this address is not larger than one cycle
   // otherwise this is probably leftover from the previous run and we don't do anything with it
   if(fLastTimeStamp.find(frag->GetAddress()) != fLastTimeStamp.end()) {
      if(timeStamp - fLastTimeStamp[frag->GetAddress()] > fCycleLength * 1000) {
         //std::cout << "Skipping hit for address " << hex(frag->GetAddress()) << " with timestamp " << hex(timeStamp) << " = " << timeStamp << " since last timestamp " << hex(fLastTimeStamp[frag->GetAddress()]) << " = " << fLastTimeStamp[frag->GetAddress()] << " is more than cycle length (" << fCycleLength << " = " << static_cast<double>(fCycleLength) / 1e6 << " s) earlier" << std::endl;
         return;
      }
   }
   time_t   daqTimeStamp = frag->GetDaqTimeStamp();

   if(fDaqTimeStampOffset == 0 && TRunInfo::SubRunNumber() == 0) {
      if(timeStamp > fCycleLength * 1000) {
         //std::cout << "Skipping hit for address " << hex(frag->GetAddress()) << " with timestamp " << hex(timeStamp) << " = " << timeStamp << " since it's larger than the cycle length " << fCycleLength << " = " << static_cast<double>(fCycleLength) / 1e6 << " s, DAQ time stamp offset not yet set!" << std::endl;
         return;
      }
      fDaqTimeStampOffset = daqTimeStamp - timeStamp / 1000000000;
      std::ofstream file(fOffsetFile);
      file << fDaqTimeStampOffset;
      file.close();
      std::cout << "Wrote daq time offset " << fDaqTimeStampOffset << " to " << fOffsetFile << " using timestamp " << timeStamp << " = " << hex(timeStamp) << " = " << static_cast<double>(timeStamp) / 1e9 << " s" << std::endl;
   }

   // this is a good hit, so we update the last time stamp (or if this is the first hit of this adddress, set it)
   fLastTimeStamp[frag->GetAddress()] = timeStamp;

   Long64_t offset = daqTimeStamp - fDaqTimeStampOffset - timeStamp / 1000000000;
   if(offset > (fImplantDaqOffLength + fDecayDaqOffLength) / 1000000 - 1) {   // -1 to account for uncertainty of offset between DAQ time (1 s precision) and timestamps
      timeStamp += (fTapeMoveLength + fBackgroundLength + fImplantDaqOnLength + fImplantDaqOffLength + fDecayDaqOffLength) * 1000 + (offset / (fCycleLength / 1000000)) * fCycleLength * 1000;
      frag->SetTimeStamp(timeStamp / frag->GetTimeStampUnit());
   }
}
