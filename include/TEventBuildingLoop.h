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
  enum EBuildMode {
    kTimestamp,
    kTriggerId
  };

  static TEventBuildingLoop *Get(std::string name="", EBuildMode mode=kTimestamp);
  virtual ~TEventBuildingLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> >& InputQueue() { return fInputQueue; }
  std::shared_ptr<ThreadsafeQueue<std::vector<TFragment*> > >& OutputQueue() { return fOutputQueue; }
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

  void CheckBuildCondition(TFragment*);
  void CheckTimestampCondition(TFragment*);
  void CheckTriggerIdCondition(TFragment*);

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> > fInputQueue;
  std::shared_ptr<ThreadsafeQueue<std::vector<TFragment*> > > fOutputQueue;
#endif

  EBuildMode fBuildMode;
  unsigned int fSortingDepth;
  long fBuildWindow;
  bool fPreviousSortingDepthError;

  std::vector<TFragment*> fNextEvent;

#ifndef __CINT__
  std::multiset<TFragment*,
                std::function<bool(TFragment*,TFragment*)> > fOrdered;
#endif

  ClassDef(TEventBuildingLoop, 0);
};

/*! @} */
#endif /* _TBUILDINGLOOP_H_ */
