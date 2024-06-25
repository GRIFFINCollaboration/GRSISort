#include "TDetBuildingLoop.h"

#include <chrono>
#include <thread>

#include "TUnpackedEvent.h"

ClassImp(TDetBuildingLoop)

TDetBuildingLoop* TDetBuildingLoop::Get(std::string name)
{
   if(name.length() == 0) {
      name = "unpack_loop";
   }
   auto* loop = static_cast<TDetBuildingLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      loop = new TDetBuildingLoop(name);
   }
   return loop;
}

TDetBuildingLoop::TDetBuildingLoop(std::string name)
   : StoppableThread(std::move(name)),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>>())
{
}

TDetBuildingLoop::~TDetBuildingLoop() = default;

bool TDetBuildingLoop::Iteration()
{
   std::vector<std::shared_ptr<const TFragment>> frags;

   fInputSize = fInputQueue->Pop(frags);
   if(fInputSize < 0) {
      fInputSize = 0;
   }

   if(frags.empty()) {
      if(fInputQueue->IsFinished()) {
         for(const auto& outQueue : fOutputQueues) {
            outQueue->SetFinished();
         }
         return false;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      return true;
   }
   ++fItemsPopped;

   std::shared_ptr<TUnpackedEvent> outputEvent = std::make_shared<TUnpackedEvent>();
   outputEvent->SetRawData(frags);
   outputEvent->Build();
   for(const auto& outQueue : fOutputQueues) {
      outQueue->Push(outputEvent);
   }

   return true;
}

void TDetBuildingLoop::ClearQueue()
{
   std::vector<std::shared_ptr<const TFragment>> rawEvent;
   while(fInputQueue->Size() != 0u) {
      fInputQueue->Pop(rawEvent);
   }

   for(const auto& outQueue : fOutputQueues) {
      while(outQueue->Size() != 0u) {
         std::shared_ptr<TUnpackedEvent> event;
         outQueue->Pop(event);
      }
   }
}
