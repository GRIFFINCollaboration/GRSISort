#include "TUnpackingLoop.h"

#include <chrono>
#include <thread>
#include <sstream>
#include <memory>

#include "TGRSIOptions.h"
#include "TParserLibrary.h"

TUnpackingLoop* TUnpackingLoop::Get(std::string name)
{
   if(name.length() == 0) {
      name = "unpacking_loop";
   }

   auto* loop = static_cast<TUnpackingLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      loop = new TUnpackingLoop(name);
   }
   return loop;
}

TUnpackingLoop::TUnpackingLoop(std::string name)
   : StoppableThread(std::move(name)), fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TRawEvent>>>()),
     fFragsReadFromRaw(0), fGoodFragsRead(0)
{
   // try and open dynamic library
   if(TGRSIOptions::Get()->ParserLibrary().empty()) {
      throw std::runtime_error("No data parser library supplied, can't open parser!");
   }

   // create new data parser
   fParser = TParserLibrary::Get()->CreateDataParser();
}

TUnpackingLoop::~TUnpackingLoop() = default;

void TUnpackingLoop::ClearQueue()
{
   std::shared_ptr<TRawEvent> singleEvent;
   while(fInputQueue->Size() != 0u) {
      fInputQueue->Pop(singleEvent);
   }

   fParser->ClearQueue();
}

bool TUnpackingLoop::Iteration()
{
   std::shared_ptr<TRawEvent> event;
   int                        error = fInputQueue->Pop(event);
   if(error < 0) {
      InputSize(0);
      if(fInputQueue->IsFinished()) {
         // Source is dead, push the last event and stop.
         fParser->SetFinished();
         BadOutputQueue()->SetFinished();
         ScalerOutputQueue()->SetFinished();
         return false;
      }
      // Wait for the source to give more data.
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      return true;
   }
   fParser->SetStatusVariables(&ItemsPopped(), &InputSize());
   InputSize(error);   //"error" is the return value of popping an event from the input queue (which returns the number of events left)
   IncrementItemsPopped();

   fFragsReadFromRaw += fParser->Process(event);
   fGoodFragsRead += event->GoodFrags();

   return true;
}

std::string TUnpackingLoop::EndStatus()
{
   std::ostringstream status;
   if(fFragsReadFromRaw > 0) {
      status << "\r" << Name() << ":\t" << fGoodFragsRead << " good fragments out of " << fFragsReadFromRaw
             << " fragments => " << 100. * static_cast<double>(fGoodFragsRead) / static_cast<double>(fFragsReadFromRaw) << "% passed" << std::endl;
   } else {
      status << "\rno fragments read from midas => none parsed!" << std::endl;
   }
   status << fParser->OutputQueueStatus();
   return status.str();
}
