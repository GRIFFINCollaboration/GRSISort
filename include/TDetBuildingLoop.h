#ifndef _TDETBUILDINGLOOP_H_
#define _TDETBUILDINGLOOP_H_

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
  static TDetBuildingLoop *Get(std::string name="");
  virtual ~TDetBuildingLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<std::vector<TFragment*> > >& InputQueue() { return input_queue; }
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> >& OutputQueue() { return output_queue; }
#endif

  bool Iteration();
  virtual void ClearQueue();

  size_t GetItemsPushed()  { return output_queue->ItemsPushed(); }
  size_t GetItemsPopped()  { return output_queue->ItemsPopped(); }
  size_t GetItemsCurrent() { return output_queue->Size();        }
  size_t GetRate()         { return 0; }

private:
  TDetBuildingLoop(std::string name);
  TDetBuildingLoop(const TDetBuildingLoop& other);
  TDetBuildingLoop& operator=(const TDetBuildingLoop& other);

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<std::vector<TFragment*> > > input_queue;
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> > output_queue;
#endif

  ClassDef(TDetBuildingLoop, 0);
};

#endif /* _TUNPACKLOOP_H_ */
