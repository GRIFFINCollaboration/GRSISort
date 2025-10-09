#include "TEventBuildingLoop.h"

#include "TGRSIOptions.h"
#include "TSortingDiagnostics.h"

#include <chrono>
#include <thread>

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
   : StoppableThread(std::move(name)), fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>()),
     fOutputQueue(std::make_shared<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>()), fBuildMode(mode),
     fSortingDepth(10000), fBuildWindow(buildWindow), fPreviousSortingDepthError(false), fSkipInputSort(TGRSIOptions::Get()->SkipInputSort())
{
   std::cout << DYELLOW << (fSkipInputSort ? "Not sorting " : "Sorting ") << "input by time: ";
   switch(fBuildMode) {
   case EBuildMode::kTime:
      fOrdered = decltype(fOrdered)([](const std::shared_ptr<const TFragment>& a, const std::shared_ptr<const TFragment>& b) {
         return a->GetTime() < b->GetTime();
      });
      std::cout << DYELLOW << "sorting by time, using build window of " << fBuildWindow << "!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kTimestamp:
      fOrdered = decltype(fOrdered)([](const std::shared_ptr<const TFragment>& a, const std::shared_ptr<const TFragment>& b) {
         return a->GetTimeStampNs() < b->GetTimeStampNs();
      });
      std::cout << DYELLOW << "sorting by timestamp, using build window of " << fBuildWindow << "!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kTriggerId:
      fOrdered = decltype(fOrdered)([](const std::shared_ptr<const TFragment>& a, const std::shared_ptr<const TFragment>& b) {
         return a->GetTriggerId() < b->GetTriggerId();
      });
      std::cout << DYELLOW << "sorting by trigger ID!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kSkip:
      // no need for ordering, always return true
      fOrdered = decltype(fOrdered)([](const std::shared_ptr<const TFragment>&, const std::shared_ptr<const TFragment>&) {
         return true;
      });
      std::cout << DYELLOW << "not sorting!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kDefault:
      std::cout << "build mode was " << static_cast<int>(fBuildMode) << ", not " << static_cast<int>(EBuildMode::kTimestamp) << ", or " << static_cast<int>(EBuildMode::kTriggerId) << std::endl;
      throw std::runtime_error("Error in event building loop, no build mode selected. Maybe because no custom run info was loaded?");
      break;
   }
}

TEventBuildingLoop::~TEventBuildingLoop() = default;

void TEventBuildingLoop::ClearQueue()
{
   std::shared_ptr<const TFragment> singleEvent;
   while(fInputQueue->Size() != 0u) {
      fInputQueue->Pop(singleEvent);
   }

   std::vector<std::shared_ptr<const TFragment>> event;
   while(fOutputQueue->Size() != 0u) {
      fOutputQueue->Pop(event);
   }
}

bool TEventBuildingLoop::Iteration()
{
   // Pull something off of the input queue.
   std::shared_ptr<const TFragment> inputFragment = nullptr;
   InputSize(fInputQueue->Pop(inputFragment, 0));
   if(InputSize() < 0) {
      InputSize(0);
   }

   if(inputFragment != nullptr) {
      IncrementItemsPopped();
      if(!fSkipInputSort) {
         fOrdered.insert(inputFragment);
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
   std::shared_ptr<const TFragment> nextFragment;
   if(!fSkipInputSort) {
      nextFragment = *fOrdered.begin();
      fOrdered.erase(fOrdered.begin());
   } else {
      nextFragment = inputFragment;
   }

   if(CheckBuildCondition(nextFragment)) {
      fNextEvent.push_back(nextFragment);
   }

   return true;
}

bool TEventBuildingLoop::CheckBuildCondition(const std::shared_ptr<const TFragment>& frag)
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

bool TEventBuildingLoop::CheckTimeCondition(const std::shared_ptr<const TFragment>& frag)
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

bool TEventBuildingLoop::CheckTimestampCondition(const std::shared_ptr<const TFragment>& frag)
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

bool TEventBuildingLoop::CheckTriggerIdCondition(const std::shared_ptr<const TFragment>& frag)
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
