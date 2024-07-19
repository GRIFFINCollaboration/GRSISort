#ifndef TEVENTBUILDINGLOOP_H
#define TEVENTBUILDINGLOOP_H

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TEventBuildingLoop
///
/// This loop builds events (vectors of fragments) based on timestamps and a
/// build windows.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include <memory>
#include <functional>
#include <set>
#endif

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TFragment.h"

class TEventBuildingLoop : public StoppableThread {
public:
   enum class EBuildMode { kDefault,
                           kTime,
                           kTimestamp,
                           kTriggerId,
                           kSkip };

   static TEventBuildingLoop* Get(std::string name = "", EBuildMode mode = EBuildMode::kTimestamp, uint64_t buildWindow = 2000);
   ~TEventBuildingLoop() override;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& InputQueue()
   {
      return fInputQueue;
   }
   std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>>& OutputQueue()
   {
      return fOutputQueue;
   }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& OutOfOrderQueue() { return fOutOfOrderQueue; }
#endif

   bool Iteration() override;

   void ClearQueue() override;

   size_t GetItemsPushed() override { return fOutputQueue->ItemsPushed(); }
   size_t GetItemsPopped() override { return fOutputQueue->ItemsPopped(); }
   size_t GetItemsCurrent() override { return fOutputQueue->Size(); }
   size_t GetRate() override { return 0; }

   void     SetBuildWindow(uint64_t val) { fBuildWindow = val; }
   uint64_t GetBuildWindow() const { return fBuildWindow; }

   void         SetSortDepth(unsigned int val) { fSortingDepth = val; }
   unsigned int GetSortDepth() const { return fSortingDepth; }

   std::string EndStatus() override;

private:
   TEventBuildingLoop(std::string name, EBuildMode mode, uint64_t buildWindow);
   TEventBuildingLoop(const TEventBuildingLoop& other);
   TEventBuildingLoop& operator=(const TEventBuildingLoop& other);

#ifndef __CINT__
   bool CheckBuildCondition(const std::shared_ptr<const TFragment>&);
   bool CheckTimeCondition(const std::shared_ptr<const TFragment>&);
   bool CheckTimestampCondition(const std::shared_ptr<const TFragment>&);
   bool CheckTriggerIdCondition(const std::shared_ptr<const TFragment>&);

   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>              fInputQueue;
   std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>> fOutputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>              fOutOfOrderQueue;
#endif

   EBuildMode   fBuildMode;
   unsigned int fSortingDepth;
   uint64_t     fBuildWindow;
   bool         fPreviousSortingDepthError;
   bool         fSkipInputSort;

#ifndef __CINT__
   std::vector<std::shared_ptr<const TFragment>> fNextEvent;

   std::multiset<std::shared_ptr<const TFragment>,
                 std::function<bool(std::shared_ptr<const TFragment>, std::shared_ptr<const TFragment>)>>
      fOrdered;
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TEventBuildingLoop, 0) // NOLINT
   /// \endcond
};

/*! @} */
#endif /* _TBUILDINGLOOP_H_ */
