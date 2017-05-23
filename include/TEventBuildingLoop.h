#ifndef _TEVENTBUILDINGLOOP_H_
#define _TEVENTBUILDINGLOOP_H_

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
   enum EBuildMode { kTimestamp, kTriggerId };

   static TEventBuildingLoop* Get(std::string name = "", EBuildMode mode = kTimestamp);
   virtual ~TEventBuildingLoop();

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>&              InputQueue() { return fInputQueue; }
   std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>>& OutputQueue()
   {
      return fOutputQueue;
   }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& OutOfOrderQueue() { return fOutOfOrderQueue; }
#endif

   bool Iteration();

   virtual void ClearQueue();

   size_t GetItemsPushed() { return fOutputQueue->ItemsPushed(); }
   size_t GetItemsPopped() { return fOutputQueue->ItemsPopped(); }
   size_t GetItemsCurrent() { return fOutputQueue->Size(); }
   size_t GetRate() { return 0; }

   void SetBuildWindow(long clock_ticks) { fBuildWindow = clock_ticks; }
   unsigned long            GetBuildWindow() const { return fBuildWindow; }

   void SetSortDepth(int num_events) { fSortingDepth = num_events; }
   unsigned int          GetSortDepth() const { return fSortingDepth; }

   std::string EndStatus();

private:
   TEventBuildingLoop(std::string name, EBuildMode mode);
   TEventBuildingLoop(const TEventBuildingLoop& other);
   TEventBuildingLoop& operator=(const TEventBuildingLoop& other);

#ifndef __CINT__
   bool CheckBuildCondition(std::shared_ptr<const TFragment>);
   bool CheckTimestampCondition(std::shared_ptr<const TFragment>);
   bool CheckTriggerIdCondition(std::shared_ptr<const TFragment>);

   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>              fInputQueue;
   std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment>>>> fOutputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>              fOutOfOrderQueue;
#endif

   EBuildMode   fBuildMode;
   unsigned int fSortingDepth;
   long         fBuildWindow;
   bool         fPreviousSortingDepthError;

#ifndef __CINT__
   std::vector<std::shared_ptr<const TFragment>> fNextEvent;

   std::multiset<std::shared_ptr<const TFragment>,
                 std::function<bool(std::shared_ptr<const TFragment>, std::shared_ptr<const TFragment>)>>
      fOrdered;
#endif

   ClassDef(TEventBuildingLoop, 0);
};

/*! @} */
#endif /* _TBUILDINGLOOP_H_ */
