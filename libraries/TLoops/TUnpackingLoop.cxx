#include "TUnpackingLoop.h"

#include <chrono>
#include <thread>
#include <sstream>
#include <memory>

#include "TGRSIOptions.h"
#include "TLstEvent.h"
#include "TTdrEvent.h"
#include "TMidasEvent.h"

TUnpackingLoop* TUnpackingLoop::Get(std::string name)
{
   if(name.length() == 0) {
      name = "unpacking_loop";
   }

   TUnpackingLoop* loop = static_cast<TUnpackingLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      loop = new TUnpackingLoop(name);
   }
   return loop;
}

TUnpackingLoop::TUnpackingLoop(std::string name)
   : StoppableThread(name), fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TRawEvent>>>()),
     fFragsReadFromRaw(0), fGoodFragsRead(0), fEvaluateDataType(true), fDataType(EDataType::kMidas)
{
}

TUnpackingLoop::~TUnpackingLoop() = default;

void TUnpackingLoop::ClearQueue()
{
   std::shared_ptr<TRawEvent> singleEvent;
   while(fInputQueue->Size() != 0u) {
      fInputQueue->Pop(singleEvent);
   }

   fParser.ClearQueue();
}

bool TUnpackingLoop::Iteration()
{
   std::shared_ptr<TRawEvent> event;
   int                        error = fInputQueue->Pop(event);
   if(error < 0) {
      fInputSize = 0;
      if(fInputQueue->IsFinished()) {
         // Source is dead, push the last event and stop.
         fParser.SetFinished();
         BadOutputQueue()->SetFinished();
         ScalerOutputQueue()->SetFinished();
         return false;
      }
      // Wait for the source to give more data.
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      return true;
   }
   if(fEvaluateDataType) {
		if(event->IsA() == TLstEvent::Class()) {
			fDataType         = EDataType::kLst;
		} else if(event->IsA() == TTdrEvent::Class()) {
			fDataType = EDataType::kTdr;
		} else {
			fDataType = EDataType::kMidas;
		}
      fEvaluateDataType = false;
   }
   if(fDataType == EDataType::kLst) {// || fDataType == EDataType::kTdr) {
      fParser.SetStatusVariables(&fItemsPopped, &fInputSize);
   } else {
      fInputSize = error;//"error" is the return value of popping an event from the input queue (which returns the number of events left)
      ++fItemsPopped;
   }

   fFragsReadFromRaw += event->Process(fParser);
   fGoodFragsRead += event->GoodFrags();

   return true;
}

std::string TUnpackingLoop::EndStatus()
{
   std::stringstream ss;
   if(fFragsReadFromRaw > 0) {
      ss<<"\r"<<Name()<<":\t"<<fGoodFragsRead<<" good fragments out of "<<fFragsReadFromRaw
        <<" fragments => "<<(100. * fGoodFragsRead) / fFragsReadFromRaw<<"% passed"<<std::endl;
   } else {
      ss<<"\rno fragments read from midas => none parsed!"<<std::endl;
   }
   ss<<fParser.OutputQueueStatus();
   return ss.str();
}
