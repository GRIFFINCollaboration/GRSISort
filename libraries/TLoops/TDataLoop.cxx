#include "TDataLoop.h"

#include <chrono>
#include <thread>
#include <utility>
#include <cstdio>
#include <sstream>

#include "TGRSIOptions.h"
#include "TString.h"
#include "TRawFile.h"
#include "TChannel.h"
#include "TRunInfo.h"

TDataLoop::TDataLoop(std::string name, TRawFile* source)
   : StoppableThread(name), fSource(source), fSelfStopping(true), fEventsRead(0),
     fOutputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TRawEvent>>>("midas_queue"))
{
}

TDataLoop::~TDataLoop()
{
   // delete source;
}

TDataLoop* TDataLoop::Get(std::string name, TRawFile* source)
{
   if(name.length() == 0) {
      name = "input_loop";
   }
   TDataLoop* loop = static_cast<TDataLoop*>(StoppableThread::Get(name));
   if((loop == nullptr) && (source != nullptr)) {
      loop = new TDataLoop(name, source);
   }
   return loop;
}

void TDataLoop::ClearQueue()
{
   std::shared_ptr<TRawEvent> event;
   while(fOutputQueue->Size() != 0u) {
      fOutputQueue->Pop(event);
   }
}

void TDataLoop::ReplaceSource(TRawFile* new_source)
{
   std::lock_guard<std::mutex> lock(fSourceMutex);
   // delete source;
   fSource = new_source;
}

void TDataLoop::ResetSource()
{
   std::cerr<<"Reset not implemented for TRawFile"<<std::endl;
   // std::lock_guard<std::mutex> lock(fSourceMutex);
   // source->Reset();
}

void TDataLoop::OnEnd()
{
   fOutputQueue->SetFinished();
}

bool TDataLoop::Iteration()
{
   std::shared_ptr<TRawEvent> evt = fSource->NewEvent();
   int                        bytesRead = 0;
   {
      std::lock_guard<std::mutex> lock(fSourceMutex);
		bytesRead   = fSource->Read(evt);
		fItemsPopped = fSource->GetBytesRead() / 1000;
		fInputSize = fSource->GetFileSize() / 1000 - fItemsPopped; // this way fInputSize+fItemsPopped give the file size
		++fEventsRead;
		if(TGRSIOptions::Get()->Downscaling() > 1) {
			// if we use downscaling we skip n-1 events without updating bytesRead
			// that way all further checks work as usual on the single event we read
			fSource->Skip(TGRSIOptions::Get()->Downscaling()-1);
			fItemsPopped = fSource->GetBytesRead() / 1000;
			fInputSize = fSource->GetFileSize() / 1000 - fItemsPopped; // this way fInputSize+fItemsPopped give the file size
			fEventsRead += TGRSIOptions::Get()->Downscaling()-1;
		}
   }

   if(bytesRead <= 0 && fSelfStopping) {
      // Error, and no point in trying again.
      return false;
   }
   if(bytesRead > 0) {
      // A good event was returned
      fOutputQueue->Push(evt);
		if(fEventsRead == TGRSIOptions::Get()->NumberOfEvents()) {
			return false;
		}
      return true;
   }
   // Nothing returned this time, but I might get something next time.
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return true;
}

