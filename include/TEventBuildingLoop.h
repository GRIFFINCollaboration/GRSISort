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
  std::shared_ptr<ThreadsafeQueue<TFragment*> >& InputQueue() { return input_queue; }
  std::shared_ptr<ThreadsafeQueue<std::vector<TFragment*> > >& OutputQueue() { return output_queue; }
#endif

  bool Iteration();

  virtual void ClearQueue();

  size_t GetItemsPushed()  { return output_queue->ItemsPushed(); }
  size_t GetItemsPopped()  { return output_queue->ItemsPopped(); }
  size_t GetItemsCurrent() { return output_queue->Size();        }
  size_t GetRate()         { return 0; }

  void SetBuildWindow(long clock_ticks) { build_window = clock_ticks; }
  long GetBuildWindow() const { return build_window; }

  void SetSortDepth(int num_events) { sorting_depth = num_events; }
  int GetSortDepth() const { return sorting_depth; }


private:
  TEventBuildingLoop(std::string name, EBuildMode mode);
  TEventBuildingLoop(const TEventBuildingLoop& other);
  TEventBuildingLoop& operator=(const TEventBuildingLoop& other);

  void CheckBuildCondition(TFragment*);
  void CheckTimestampCondition(TFragment*);
  void CheckTriggerIdCondition(TFragment*);

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> > input_queue;
  std::shared_ptr<ThreadsafeQueue<std::vector<TFragment*> > > output_queue;
#endif

  EBuildMode build_mode;
  int sorting_depth;
  long build_window;

  std::vector<TFragment*> next_event;

#ifndef __CINT__
  std::multiset<TFragment*,
                std::function<bool(TFragment*,TFragment*)> > ordered;
#endif

  ClassDef(TEventBuildingLoop, 0);
};

#endif /* _TBUILDINGLOOP_H_ */
