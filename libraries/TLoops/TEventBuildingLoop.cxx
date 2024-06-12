#include "TEventBuildingLoop.h"

#include "TGRSIOptions.h"
#include "TSortingDiagnostics.h"

#include <chrono>
#include <thread>

ClassImp(TEventBuildingLoop)

   TEventBuildingLoop* TEventBuildingLoop::Get(std::string name, EBuildMode mode, long buildWindow)
{
   if(name.length() == 0) {
      name = "build_loop";
   }

   TEventBuildingLoop* loop = static_cast<TEventBuildingLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      loop = new TEventBuildingLoop(name, mode, buildWindow);
   }
   return loop;
}

TEventBuildingLoop::TEventBuildingLoop(std::string name, EBuildMode mode, long buildWindow)
   : StoppableThread(name), fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>()),
     fOutputQueue(std::make_shared<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>()), fBuildMode(mode),
     fSortingDepth(10000), fBuildWindow(buildWindow), fPreviousSortingDepthError(false), fSkipInputSort(TGRSIOptions::Get()->SkipInputSort())
{
   std::cout << DYELLOW << (fSkipInputSort ? "Not sorting " : "Sorting ") << "input by time: ";
   switch(fBuildMode) {
   case EBuildMode::kTime:
      fOrdered = decltype(fOrdered)([](std::shared_ptr<const TFragment> a, std::shared_ptr<const TFragment> b) {
         return a->GetTime() < b->GetTime();
      });
      std::cout << DYELLOW << "sorting by time, using build window of " << fBuildWindow << "!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kTimestamp:
      fOrdered = decltype(fOrdered)([](std::shared_ptr<const TFragment> a, std::shared_ptr<const TFragment> b) {
         return a->GetTimeStampNs() < b->GetTimeStampNs();
      });
      std::cout << DYELLOW << "sorting by timestamp, using build window of " << fBuildWindow << "!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kTriggerId:
      fOrdered = decltype(fOrdered)([](std::shared_ptr<const TFragment> a, std::shared_ptr<const TFragment> b) {
         return a->GetTriggerId() < b->GetTriggerId();
      });
      std::cout << DYELLOW << "sorting by trigger ID!" << RESET_COLOR << std::endl;
      break;
   case EBuildMode::kSkip:
      // no need for ordering, always return true
      fOrdered = decltype(fOrdered)([](std::shared_ptr<const TFragment>, std::shared_ptr<const TFragment>) {
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
   std::shared_ptr<const TFragment> single_event;
   while(fInputQueue->Size() != 0u) {
      fInputQueue->Pop(single_event);
   }

   std::vector<std::shared_ptr<const TFragment>> event;
   while(fOutputQueue->Size() != 0u) {
      fOutputQueue->Pop(event);
   }
}

bool TEventBuildingLoop::Iteration()
{
   // Pull something off of the input queue.
   std::shared_ptr<const TFragment> input_frag = nullptr;
   fInputSize                                  = fInputQueue->Pop(input_frag, 0);
   if(fInputSize < 0) {
      fInputSize = 0;
   }

   if(input_frag != nullptr) {
      ++fItemsPopped;
      if(!fSkipInputSort) {
         fOrdered.insert(input_frag);
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
   std::shared_ptr<const TFragment> next_fragment;
   if(!fSkipInputSort) {
      next_fragment = *fOrdered.begin();
      fOrdered.erase(fOrdered.begin());
   } else {
      next_fragment = input_frag;
   }

   if(CheckBuildCondition(next_fragment)) {
      fNextEvent.push_back(next_fragment);
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
   double time        = frag->GetTime();
   double event_start = (!fNextEvent.empty() ? (TGRSIOptions::Get()->AnalysisOptions()->StaticWindow() ? fNextEvent[0]->GetTime()
                                                                                                       : fNextEvent.back()->GetTime())
                                             : time);

   // save time every <BuildWindow> fragments
   if(frag->GetEntryNumber() % (TGRSIOptions::Get()->SortDepth()) == 0) {
      TSortingDiagnostics::Get()->AddTime(event_start);
   }
   if(time > event_start + fBuildWindow || time < event_start - fBuildWindow) {
      // std::cout.precision(12);
      // std::cout<<std::setw(12)<<time<<", "<<std::setw(12)<<event_start<<", "<<std::setw(12)<<fBuildWindow<<"; "<<std::setw(12)<<fabs(time - event_start)<<", "<<std::setw(12)<<event_start + fBuildWindow<<", "<<std::setw(12)<<event_start - fBuildWindow<<std::endl;
      fOutputQueue->Push(fNextEvent);
      fNextEvent.clear();
   }

   if(time < event_start) {
      TSortingDiagnostics::Get()->OutOfTimeOrder(time, event_start, frag->GetEntryNumber());
      if(!fPreviousSortingDepthError) {
         std::cerr.precision(12);
         std::cerr << std::endl
                   << "Sorting depth of " << fSortingDepth << " was insufficient. time: " << std::setw(12) << time
                   << " Last: " << std::setw(12) << event_start << " \n"
                   << "Not all events were built correctly" << std::endl;
         std::cerr << "Please increase sort depth with --sort-depth=N" << std::endl;
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
   long timestamp   = frag->GetTimeStampNs();
   long event_start = (!fNextEvent.empty() ? (TGRSIOptions::Get()->AnalysisOptions()->StaticWindow() ? fNextEvent[0]->GetTimeStampNs()
                                                                                                     : fNextEvent.back()->GetTimeStampNs())
                                           : timestamp);

   // save timestamp every <BuildWindow> fragments
   if(frag->GetEntryNumber() % (TGRSIOptions::Get()->SortDepth()) == 0) {
      TSortingDiagnostics::Get()->AddTimeStamp(event_start);
   }
   if(timestamp > event_start + fBuildWindow || timestamp < event_start - fBuildWindow) {
      fOutputQueue->Push(fNextEvent);
      fNextEvent.clear();
   }

   if(timestamp < event_start) {
      TSortingDiagnostics::Get()->OutOfOrder(timestamp, event_start, frag->GetEntryNumber());
      if(!fPreviousSortingDepthError) {
         std::cerr << std::endl
                   << "Sorting depth of " << fSortingDepth << " was insufficient. timestamp: " << timestamp
                   << " Last: " << event_start << " \n"
                   << "Not all events were built correctly" << std::endl;
         std::cerr << "Please increase sort depth with --sort-depth=N" << std::endl;
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
   long trigger_id = frag->GetTriggerId();
   long current_trigger_id =
      (!fNextEvent.empty() ? fNextEvent[0]->GetTriggerId() : trigger_id);

   // save trigger id every <BuildWindow> fragments
   if(frag->GetEntryNumber() % (TGRSIOptions::Get()->SortDepth()) == 0) {
      TSortingDiagnostics::Get()->AddTimeStamp(current_trigger_id);
   }

   if(trigger_id != current_trigger_id) {
      fOutputQueue->Push(fNextEvent);
      fNextEvent.clear();
   }

   if(trigger_id < current_trigger_id) {
      TSortingDiagnostics::Get()->OutOfOrder(trigger_id, current_trigger_id, frag->GetEntryNumber());
      if(!fPreviousSortingDepthError) {
         std::cerr << std::endl
                   << "Sorting depth of " << fSortingDepth << " was insufficient.\n"
                   << "Not all events were built correctly" << std::endl;
         std::cerr << "Trigger id #" << trigger_id << " was incorrectly sorted before "
                   << "trigger id #" << current_trigger_id << std::endl;
         std::cerr << "Please increase sort depth with --sort-depth=N" << std::endl;
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
   std::stringstream ss;
   ss << fInputQueue->Name() << ": " << fItemsPopped << "/" << fInputQueue->ItemsPopped() << " items popped"
      << std::endl;

   return ss.str();
}
