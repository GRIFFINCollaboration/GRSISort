#include "TEventBuildingLoop.h"

#include "TGRSIOptions.h"
#include "TSortingDiagnostics.h"

#include <chrono>
#include <thread>

ClassImp(TEventBuildingLoop)

TEventBuildingLoop* TEventBuildingLoop::Get(std::string name, EBuildMode mode)
{
   if(name.length() == 0) {
      name = "build_loop";
   }

   TEventBuildingLoop* loop = static_cast<TEventBuildingLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      loop = new TEventBuildingLoop(name, mode);
   }
   return loop;
}

TEventBuildingLoop::TEventBuildingLoop(std::string name, EBuildMode mode)
   : StoppableThread(name), fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>()),
     fOutputQueue(std::make_shared<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>()), fBuildMode(mode),
     fSortingDepth(10000), fBuildWindow(200), fPreviousSortingDepthError(false)
{

   switch(fBuildMode) {
   case kTimestamp:
      fOrdered = decltype(fOrdered)([](std::shared_ptr<const TFragment> a, std::shared_ptr<const TFragment> b) {
         return a->GetTimeStamp() < b->GetTimeStamp();
      });
      break;

   case kTriggerId:
      fOrdered = decltype(fOrdered)([](std::shared_ptr<const TFragment> a, std::shared_ptr<const TFragment> b) {
         return a->GetTriggerId() < b->GetTriggerId();
      });
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

   if(input_frag) {
      ++fItemsPopped;
      fOrdered.insert(input_frag);
      if(fOrdered.size() < fSortingDepth) {
         // Got a new event, but we want to have more to sort
         return true;
      }
      // Got a new event, and we have enough to sort.

   } else {
      if(!fInputQueue->IsFinished()) {
         // If the parent is live, wait for it
         std::this_thread::sleep_for(std::chrono::milliseconds(1000));
         return true;
      }
      if(fOrdered.empty()) {
         // Parent is dead, and we have passed on all events
         if(static_cast<unsigned int>(!fNextEvent.empty()) != 0u) {
            fOutputQueue->Push(fNextEvent);
         }
         fOutputQueue->SetFinished();
         return false;
      }
      // Parent is dead, but we still have items.
      // Continue through the function to process them.
   }

   // We have data, and we want to add it to the next fragment;
   std::shared_ptr<const TFragment> next_fragment = *fOrdered.begin();
   fOrdered.erase(fOrdered.begin());
   if(CheckBuildCondition(next_fragment)) {
      fNextEvent.push_back(next_fragment);
   }

   return true;
}

bool TEventBuildingLoop::CheckBuildCondition(const std::shared_ptr<const TFragment>& frag)
{
   switch(fBuildMode) {
   case kTimestamp: return CheckTimestampCondition(frag); break;

   case kTriggerId: return CheckTriggerIdCondition(frag); break;
   }
   return false; // we should never reach this statement!
}

bool TEventBuildingLoop::CheckTimestampCondition(const std::shared_ptr<const TFragment>& frag)
{
   long timestamp   = frag->GetTimeStamp();
   long event_start = (static_cast<unsigned int>(!fNextEvent.empty()) != 0u
                          ? (TGRSIOptions::Get()->AnalysisOptions()->StaticWindow() ? fNextEvent[0]->GetTimeStamp()
                                                                                    : fNextEvent.back()->GetTimeStamp())
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
         std::cerr<<std::endl
                  <<"Sorting depth of "<<fSortingDepth<<" was insufficient. timestamp: "<<timestamp
                  <<" Last: "<<event_start<<" \n"
                  <<"Not all events were built correctly"<<std::endl;
         std::cerr<<"Please increase sort depth with --sort-depth=N"<<std::endl;
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
      (static_cast<unsigned int>(!fNextEvent.empty()) != 0u ? fNextEvent[0]->GetTriggerId() : trigger_id);

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
         std::cerr<<std::endl
                  <<"Sorting depth of "<<fSortingDepth<<" was insufficient.\n"
                  <<"Not all events were built correctly"<<std::endl;
         std::cerr<<"Trigger id #"<<trigger_id<<" was incorrectly sorted before "
                  <<"trigger id #"<<current_trigger_id<<std::endl;
         std::cerr<<"Please increase sort depth with --sort-depth=N"<<std::endl;
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
   ss<<fInputQueue->Name()<<": "<<fItemsPopped<<"/"<<fInputQueue->ItemsPopped()<<" items popped"
     <<std::endl;

   return ss.str();
}
