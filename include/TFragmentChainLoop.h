#ifndef _TFRAGMENTCHAINLOOP_H_
#define _TFRAGMENTCHAINLOOP_H_

#ifndef __CINT__
#include <atomic>
#endif

#include <map>

#include "TChain.h"
#include "TClass.h"

#include "TUnpackingLoop.h"
#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TFragment.h"

class TFragmentChainLoop : public StoppableThread {
public:
  static TFragmentChainLoop* Get(std::string name="",TChain *chain=0);
  virtual ~TFragmentChainLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> >& OutputQueue() { return output_queue; }
#endif

  size_t GetItemsPushed()  { return fEntriesRead;   }
  size_t GetItemsPopped()  { return 0; }
  size_t GetItemsCurrent() { return fEntriesTotal;      }
  size_t GetRate()         { return 0; }

  virtual std::string Status();
  virtual void ClearQueue();

  virtual void OnEnd();

  void SetSelfStopping(bool self_stopping) { fSelfStopping = self_stopping; }
  bool GetSelfStopping() const { return fSelfStopping; }
  void Restart();

protected:
  bool Iteration();

private:
  TFragmentChainLoop(std::string name, TChain *chain);

#ifndef __CINT__
  std::atomic_long fEntriesRead;
#endif
  long fEntriesTotal;

  TChain *input_chain;
  TFragment** address;
#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> > output_queue;
#endif

  bool fSelfStopping;

  int SetupChain();
  std::map<TClass*, TDetector**> det_map;

  ClassDef(TFragmentChainLoop, 0);
};


#endif /* _TWRITELOOP_H_ */
