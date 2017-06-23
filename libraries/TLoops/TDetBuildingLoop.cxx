#include "TDetBuildingLoop.h"

#include <chrono>
#include <thread>

#include "TUnpackedEvent.h"

//#include "TMode3.h"

ClassImp(TDetBuildingLoop)

   TDetBuildingLoop* TDetBuildingLoop::Get(std::string name)
{
   if(name.length() == 0) {
      name = "unpack_loop";
   }
   TDetBuildingLoop* loop = static_cast<TDetBuildingLoop*>(StoppableThread::Get(name));
   if(!loop) {
      loop = new TDetBuildingLoop(name);
   }
   return loop;
}

TDetBuildingLoop::TDetBuildingLoop(std::string name)
   : StoppableThread(name),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>>())
{
}

TDetBuildingLoop::~TDetBuildingLoop()
= default;

bool TDetBuildingLoop::Iteration()
{
   std::vector<std::shared_ptr<const TFragment>> frags;

   fInputSize                    = fInputQueue->Pop(frags);
   if(fInputSize < 0) fInputSize = 0;

   if(frags.size() == 0) {
      if(fInputQueue->IsFinished()) {
         for(auto outQueue : fOutputQueues) {
            outQueue->SetFinished();
         }
         return false;
      } else {
         std::this_thread::sleep_for(std::chrono::milliseconds(1000));
         return true;
      }
   }
   ++fItemsPopped;

   std::shared_ptr<TUnpackedEvent> outputEvent = std::make_shared<TUnpackedEvent>();
   for(auto frag : frags) {
      // passes ownership of all TFragments, no need to delete here
      outputEvent->AddRawData(frag);
   }
   outputEvent->Build();
   for(auto outQueue : fOutputQueues) {
      outQueue->Push(outputEvent);
   }

   return true;
}

void TDetBuildingLoop::ClearQueue()
{
   std::vector<std::shared_ptr<const TFragment>> rawEvent;
   while(fInputQueue->Size()) {
      fInputQueue->Pop(rawEvent);
   }

   for(auto outQueue : fOutputQueues) {
      while(outQueue->Size()) {
         std::shared_ptr<TUnpackedEvent> event;
         outQueue->Pop(event);
      }
   }
}
