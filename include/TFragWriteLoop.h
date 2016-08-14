#ifndef _TWRITELOOP_H_
#define _TWRITELOOP_H_

#include <map>

#include "TClass.h"
#include "TTree.h"

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TFragment.h"

class TFragWriteLoop : public StoppableThread {
public:
  static TFragWriteLoop* Get(std::string name="", std::string output_filename="");

  virtual ~TFragWriteLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> >& InputQueue() { return input_queue; }
  std::shared_ptr<ThreadsafeQueue<TFragment*> >& OutputQueue() { return output_queue; }
#endif

  virtual void ClearQueue();

  void Write();

  size_t GetItemsPushed()  { return items_handled; }
  size_t GetItemsPopped()  { return 0; }
  size_t GetItemsCurrent() { return 0;      }
  size_t GetRate()         { return 0; }

protected:
  bool Iteration();

private:
  TFragWriteLoop(std::string name, std::string output_file);
  void WriteEvent(TFragment& event);

  TFragment** address;
  TFile* output_file;
  TTree* event_tree;

  size_t items_handled;

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> > input_queue;
  std::shared_ptr<ThreadsafeQueue<TFragment*> > output_queue;
#endif

  ClassDef(TFragWriteLoop, 0);
};


#endif /* _TWRITELOOP_H_ */
