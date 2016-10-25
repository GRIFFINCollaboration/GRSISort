#ifndef _TEVENTBUILDINGLOOP_H_
#define _TEVENTBUILDINGLOOP_H_

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
  enum EBuildMode {
    kTimestamp,
    kTriggerId
  };

  static TEventBuildingLoop *Get(std::string name="", EBuildMode mode=kTimestamp);
  virtual ~TEventBuildingLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > >& InputQueue() { return fInputQueue; }
  std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<TFragment> > > >& OutputQueue() { return fOutputQueue; }
#endif

  bool Iteration();

  virtual void ClearQueue();

  size_t GetItemsPushed()  { return fOutputQueue->ItemsPushed(); }
  size_t GetItemsPopped()  { return fOutputQueue->ItemsPopped(); }
  size_t GetItemsCurrent() { return fOutputQueue->Size();        }
  size_t GetRate()         { return 0; }

  void SetBuildWindow(long clock_ticks) { fBuildWindow = clock_ticks; }
  unsigned long GetBuildWindow() const { return fBuildWindow; }

  void SetSortDepth(int num_events) { fSortingDepth = num_events; }
  unsigned int GetSortDepth() const { return fSortingDepth; }


private:
  TEventBuildingLoop(std::string name, EBuildMode mode);
  TEventBuildingLoop(const TEventBuildingLoop& other);
  TEventBuildingLoop& operator=(const TEventBuildingLoop& other);

#ifndef __CINT__
  void CheckBuildCondition(std::shared_ptr<TFragment>);
  void CheckTimestampCondition(std::shared_ptr<TFragment>);
  void CheckTriggerIdCondition(std::shared_ptr<TFragment>);

  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > > fInputQueue;
  std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<TFragment> > > > fOutputQueue;
#endif

  EBuildMode fBuildMode;
  unsigned int fSortingDepth;
  long fBuildWindow;
  bool fPreviousSortingDepthError;

#ifndef __CINT__
  std::vector<std::shared_ptr<TFragment> > fNextEvent;

  std::multiset<std::shared_ptr<TFragment>,
                std::function<bool(std::shared_ptr<TFragment>,std::shared_ptr<TFragment>)> > fOrdered;
#endif

  ClassDef(TEventBuildingLoop, 0);
};

#endif /* _TBUILDINGLOOP_H_ */
