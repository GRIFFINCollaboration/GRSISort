#include "TDataParser.h"

#include "TChannel.h"
#include "Globals.h"

#include "TScalerQueue.h"

#include "TEpicsFrag.h"
#include "TParsingDiagnostics.h"

#include "Rtypes.h"

#include "TFragment.h"
#include "TBadFragment.h"

TGRSIOptions* TDataParser::fOptions = nullptr;

TDataParser::TDataParser()
   : fBadOutputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TBadFragment>>>("bad_frag_queue")),
     fScalerOutputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TEpicsFrag>>>("scaler_queue")),
     fNoWaveforms(false), fRecordDiag(true), fMaxTriggerId(1024 * 1024 * 16), fLastDaqId(0), fLastTriggerId(0),
     fLastNetworkPacket(0), fFragmentHasWaveform(false), fFragmentMap(fGoodOutputQueues, fBadOutputQueue),
     fItemsPopped(nullptr), fInputSize(nullptr)
{
   fChannel = new TChannel;
}

TDataParser::~TDataParser()
{
   delete fChannel;
}

void TDataParser::ClearQueue()
{
   std::shared_ptr<const TFragment> frag;
   for(const auto& outQueue : fGoodOutputQueues) {
      while(outQueue->Size() != 0u) {
         outQueue->Pop(frag);
      }
   }
   std::shared_ptr<const TBadFragment> badFrag;
   while(fBadOutputQueue->Size() != 0u) {
      fBadOutputQueue->Pop(badFrag);
   }
   std::shared_ptr<TEpicsFrag> epicsFrag;
   while(fScalerOutputQueue->Size() != 0u) {
      fScalerOutputQueue->Pop(epicsFrag);
   }
}

void TDataParser::SetFinished()
{
   for(const auto& outQueue : fGoodOutputQueues) {
      outQueue->SetFinished();
   }
   fBadOutputQueue->SetFinished();
   fScalerOutputQueue->SetFinished();
}

void TDataParser::Push(std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>>& queues,
                       const std::shared_ptr<TFragment>&                                                frag)
{
   frag->SetFragmentId(fFragmentIdMap[frag->GetTriggerId()]);
   fFragmentIdMap[frag->GetTriggerId()]++;
   frag->SetEntryNumber();
   for(const auto& queue : queues) {
      queue->Push(frag);
   }
}

void TDataParser::Push(ThreadsafeQueue<std::shared_ptr<const TBadFragment>>& queue, const std::shared_ptr<TBadFragment>& frag)
{
   frag->SetFragmentId(fFragmentIdMap[frag->GetTriggerId()]);
   fFragmentIdMap[frag->GetTriggerId()]++;
   frag->SetEntryNumber();
   queue.Push(frag);
}

std::string TDataParser::OutputQueueStatus()
{
   std::stringstream ss;
   ss << "********************************************************************************" << std::endl;
   for(const auto& queue : fGoodOutputQueues) {
      ss << queue->Name() << ": " << queue->ItemsPushed() << " pushed, " << queue->ItemsPopped() << " popped, "
         << queue->Size() << " left" << std::endl;
   }
   ss << "********************************************************************************" << std::endl;
   return ss.str();
}
