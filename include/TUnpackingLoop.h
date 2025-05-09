#ifndef TUNPACKINGLOOP_H
#define TUNPACKINGLOOP_H

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TUnpackingLoop
///
/// This loop parses raw events into fragments.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include <memory>
#include "ThreadsafeQueue.h"
#endif

#include "StoppableThread.h"
#include "TRawEvent.h"
#include "TFragment.h"
#include "TEpicsFrag.h"
#include "TDataParser.h"

class TUnpackingLoop : public StoppableThread {
public:
   static TUnpackingLoop* Get(std::string name = "");
   TUnpackingLoop(const TUnpackingLoop&)                = delete;
   TUnpackingLoop(TUnpackingLoop&&) noexcept            = delete;
   TUnpackingLoop& operator=(const TUnpackingLoop&)     = delete;
   TUnpackingLoop& operator=(TUnpackingLoop&&) noexcept = delete;
   ~TUnpackingLoop();

   void SetNoWaveForms(bool temp = true) { fParser->SetNoWaveForms(temp); }
   void SetRecordDiag(bool temp = true) { fParser->SetRecordDiag(temp); }

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TRawEvent>>>& InputQueue()
   {
      return fInputQueue;
   }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& AddGoodOutputQueue(size_t maxSize = 50000)
   {
      return fParser->AddGoodOutputQueue(maxSize);
   }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TBadFragment>>>& BadOutputQueue()
   {
      return fParser->BadOutputQueue();
   }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TEpicsFrag>>>& ScalerOutputQueue()
   {
      return fParser->ScalerOutputQueue();
   }
#endif

   bool Iteration() override;

   void ClearQueue() override;

   size_t GetItemsPushed() override { return fParser->ItemsPushed(); }
   size_t GetItemsPopped() override { return 0; }    // fParser.GoodOutputQueue()->ItemsPopped(); }
   size_t GetItemsCurrent() override { return 0; }   // fParser.GoodOutputQueue()->Size();        }
   size_t GetRate() override { return 0; }

   std::string EndStatus() override;

private:
#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TRawEvent>>> fInputQueue;
#endif

   TDataParser* fParser;
   int64_t      fFragsReadFromRaw;
   int64_t      fGoodFragsRead;

   explicit TUnpackingLoop(std::string name);
};

/*! @} */
#endif /* _TUNPACKINGLOOP_H_ */
