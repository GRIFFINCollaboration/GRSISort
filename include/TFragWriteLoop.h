#ifndef _TWRITELOOP_H_
#define _TWRITELOOP_H_

#include <map>

#include "TClass.h"
#include "TTree.h"

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TFragment.h"
#include "TEpicsFrag.h"

class TFragWriteLoop : public StoppableThread {
public:
  static TFragWriteLoop* Get(std::string name="", std::string output_filename="");

  virtual ~TFragWriteLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> >& InputQueue() { return input_queue; }
  std::shared_ptr<ThreadsafeQueue<TEpicsFrag*> >& ScalerInputQueue() { return scaler_input_queue; }
  std::shared_ptr<ThreadsafeQueue<TFragment*> >& OutputQueue() { return output_queue; }
#endif

  virtual void ClearQueue();

  void Write();

  size_t GetItemsPushed()  { return output_queue->ItemsPushed(); }
  size_t GetItemsPopped()  { return output_queue->ItemsPopped(); }
  size_t GetItemsCurrent() { return output_queue->Size(); }
  size_t GetRate()         { return 0; }

	std::string Status();
	std::string EndStatus();

protected:
  bool Iteration();

private:
  TFragWriteLoop(std::string name, std::string output_file);
  void WriteEvent(TFragment& event);
  void WriteScaler(TEpicsFrag& scaler);

  TFile* output_file;

  TFragment** event_address;
  TTree* event_tree;

  TEpicsFrag** scaler_address;
  TTree* scaler_tree;

  size_t items_handled;
  int fInputQueueSize;

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> > input_queue;
  std::shared_ptr<ThreadsafeQueue<TEpicsFrag*> > scaler_input_queue;
  std::shared_ptr<ThreadsafeQueue<TFragment*> > output_queue;
#endif

  ClassDef(TFragWriteLoop, 0);
};


#endif /* _TWRITELOOP_H_ */
