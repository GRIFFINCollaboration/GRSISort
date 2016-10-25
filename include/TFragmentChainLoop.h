#ifndef _TFRAGMENTCHAINLOOP_H_
#define _TFRAGMENTCHAINLOOP_H_

#ifndef __CINT__
#include <atomic>
#include <memory>
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
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > >& AddOutputQueue() { 
     fOutputQueues.push_back(std::make_shared<ThreadsafeQueue<std::shared_ptr<TFragment> > >());
     return fOutputQueues.back(); 
  }
#endif

  size_t GetItemsPushed()  { return fEntriesRead; }
  size_t GetItemsPopped()  { return 0; }
  size_t GetItemsCurrent() { return fEntriesTotal; }
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

  TChain *fInputChain;
#ifndef __CINT__
  std::shared_ptr<TFragment> fFragment;
  std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > > > fOutputQueues;
#endif

  bool fSelfStopping;

  int SetupChain();
  std::map<TClass*, TDetector**> fDetMap;

  //ClassDef(TFragmentChainLoop, 0);
};


#endif /* _TWRITELOOP_H_ */
