#ifndef TDETBUILDINGLOOP_H
#define TDETBUILDINGLOOP_H

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TDetBuildingLoop
///
/// This loop builds detectors from vectors of fragments.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include <condition_variable>
#include <mutex>
#include <queue>
#endif

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TFragment.h"

class TNSCLEvent;
class TGEBEvent;
class TUnpackedEvent;

class TDetector;

class TDetBuildingLoop : public StoppableThread {
public:
   static TDetBuildingLoop* Get(std::string name = "");
   TDetBuildingLoop(const TDetBuildingLoop&)                = delete;
   TDetBuildingLoop(TDetBuildingLoop&&) noexcept            = delete;
   TDetBuildingLoop& operator=(const TDetBuildingLoop&)     = delete;
   TDetBuildingLoop& operator=(TDetBuildingLoop&&) noexcept = delete;
   ~TDetBuildingLoop();

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>>& InputQueue()
   {
      return fInputQueue;
   }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>& AddOutputQueue(size_t maxSize = 50000)
   {
      std::stringstream name;
      name << "event_queue_" << fOutputQueues.size();
      fOutputQueues.push_back(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>(name.str(), maxSize));
      return fOutputQueues.back();
   }
#endif

   bool Iteration() override;
   void ClearQueue() override;

   size_t GetItemsPushed() override
   {
      if(!fOutputQueues.empty()) {
         return fOutputQueues.back()->ItemsPushed();
      }
      return std::numeric_limits<size_t>::max();
   }                                                 // this should work fine as all loops are always filled at the same time
   size_t GetItemsPopped() override { return 0; }    // fOutputQueue->ItemsPopped(); }
   size_t GetItemsCurrent() override { return 0; }   // fOutputQueue->Size();        }
   size_t GetRate() override { return 0; }

private:
   explicit TDetBuildingLoop(std::string name);

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>> fInputQueue;
   std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>>  fOutputQueues;
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TDetBuildingLoop, 0)   // NOLINT(readability-else-after-return)
   /// \endcond
};

/*! @} */
#endif /* _TDETBUILDINGLOOP_H_ */
