#ifndef _TUNPACKINGLOOP_H_
#define _TUNPACKINGLOOP_H_

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
   enum class EDataType { kMidas,
                          kLst,
                          kTdr };

   static TUnpackingLoop* Get(std::string name = "");
   ~TUnpackingLoop() override;

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
   long         fFragsReadFromRaw;
   long         fGoodFragsRead;

   bool      fEvaluateDataType;
   EDataType fDataType;

   TUnpackingLoop(std::string name);
   TUnpackingLoop(const TUnpackingLoop& other);
   TUnpackingLoop& operator=(const TUnpackingLoop& other);

   // ClassDefOverride(TUnpackingLoop, 0);
};

/*! @} */
#endif /* _TUNPACKINGLOOP_H_ */
