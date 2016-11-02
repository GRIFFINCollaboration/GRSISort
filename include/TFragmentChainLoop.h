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
		static TFragmentChainLoop* Get(std::string name="",TChain *chain=0);
		virtual ~TFragmentChainLoop();

#ifndef __CINT__
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > >& AddOutputQueue() { 
			fOutputQueues.push_back(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment> > >());
			return fOutputQueues.back(); 
		}
#endif

		size_t GetItemsPushed()  { return fItemsPopped; }
		size_t GetItemsPopped()  { return fItemsPopped; }
		size_t GetItemsCurrent() { return fEntriesTotal; }
		size_t GetRate()         { return 0; }

		virtual void ClearQueue();

		virtual void OnEnd();

		void SetSelfStopping(bool self_stopping) { fSelfStopping = self_stopping; }
		bool GetSelfStopping() const { return fSelfStopping; }
		void Restart();

	protected:
		bool Iteration();

	private:
		TFragmentChainLoop(std::string name, TChain *chain);

		long fEntriesTotal;

		TChain *fInputChain;
#ifndef __CINT__
		std::shared_ptr<const TFragment> fFragment;
		std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > > > fOutputQueues;
#endif

		bool fSelfStopping;

		int SetupChain();
		std::map<TClass*, TDetector**> fDetMap;

		//ClassDef(TFragmentChainLoop, 0);
};

/*! @} */
#endif /* _TWRITELOOP_H_ */
