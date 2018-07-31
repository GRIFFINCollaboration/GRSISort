#include "TUnpackingLoop.h"

#include <chrono>
#include <thread>
#include <sstream>
#include <memory>

#include "TGRSIOptions.h"

#include <dlfcn.h>

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
	// try and open dynamic library
	if(TGRSIOptions::Get()->ParserLibrary().empty()) {
      throw std::runtime_error("No data parser library supplied, can't open parser!");
	}

	fHandle = dlopen(TGRSIOptions::Get()->ParserLibrary().c_str(), RTLD_LAZY);
	if(fHandle == nullptr) {
		std::ostringstream str;
		str<<"Failed to open data parser library '"<<TGRSIOptions::Get()->ParserLibrary()<<"': "<<dlerror()<<"!";
		throw std::runtime_error(str.str());
	}
	// try and get constructor and destructor functions from opened library
	fCreateDataParser  = (TDataParser* (*)())      dlsym(fHandle, "CreateParser");
	fDestroyDataParser = (void (*)(TDataParser*)) dlsym(fHandle, "DestroyParser");
	if(fCreateDataParser == nullptr || fDestroyDataParser == nullptr) {
		std::ostringstream str;
		str<<"Failed to find CreateParser, and/or DestroyParser functions in library '"<<TGRSIOptions::Get()->ParserLibrary()<<"'!";
		throw std::runtime_error(str.str());
	}
	// create new data parser
	fParser = fCreateDataParser();
}

TUnpackingLoop::~TUnpackingLoop() {
	fDestroyDataParser(fParser);
	dlclose(fHandle);
}

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
      fInputSize = 0;
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
	fParser->SetStatusVariables(&fItemsPopped, &fInputSize);
	fInputSize = error;//"error" is the return value of popping an event from the input queue (which returns the number of events left)
	++fItemsPopped;

   fFragsReadFromRaw += fParser->Process(event);
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
   ss<<fParser->OutputQueueStatus();
   return ss.str();
}
