#ifndef _TFRAGMENTCHAINLOOP_H_
#define _TFRAGMENTCHAINLOOP_H_

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TFragmentChainLoop
///
/// This loop reads fragments from a root-file with a FragmentTree.
///
////////////////////////////////////////////////////////////////////////////////

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
   static TFragmentChainLoop* Get(std::string name = "", TChain* chain = nullptr);
   ~TFragmentChainLoop() override;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& AddOutputQueue()
   {
      fOutputQueues.push_back(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>());
      return fOutputQueues.back();
   }
#endif

   size_t GetItemsPushed() override
   {
      return fItemsPopped;
   }
   size_t GetItemsPopped() override { return fItemsPopped; }
   size_t GetItemsCurrent() override { return fEntriesTotal; }
   size_t GetRate() override { return 0; }

   void ClearQueue() override;

   void OnEnd() override;

   void SetSelfStopping(bool self_stopping) { fSelfStopping = self_stopping; }
   bool GetSelfStopping() const { return fSelfStopping; }
   void Restart();

protected:
   bool Iteration() override;

private:
   TFragmentChainLoop(std::string name, TChain* chain);

   int64_t fEntriesTotal;

   TChain* fInputChain;
#ifndef __CINT__
   TFragment*                                                                      fFragment;
   std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>> fOutputQueues;
#endif

   bool fSelfStopping;

   int                            SetupChain();
   std::map<TClass*, TDetector**> fDetMap;

   // ClassDefOverride(TFragmentChainLoop, 0);
};

/*! @} */
#endif /* _TWRITELOOP_H_ */
