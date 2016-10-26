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
		static TFragWriteLoop* Get(std::string name="", std::string outputFilename="");

		virtual ~TFragWriteLoop();

#ifndef __CINT__
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > >& InputQueue() { return fInputQueue; }
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > >& BadInputQueue() { return fBadInputQueue; }
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TEpicsFrag> > >& ScalerInputQueue() { return fScalerInputQueue; }
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > >& OutputQueue() { return fOutputQueue; }
#endif

		virtual void ClearQueue();

		void Write();

		// there is no output queue for this loop, so we assume that all items handled (= all good fragments written)
		// are also the number of items popped and that we have no current items
		size_t GetItemsPushed()  { return fItemsPopped; }
		size_t GetItemsPopped()  { return fItemsPopped; }
		size_t GetItemsCurrent() { return 0; }
		size_t GetRate()         { return 0; }

		std::string EndStatus();

	protected:
		bool Iteration();

	private:
		TFragWriteLoop(std::string name, std::string outputFile);
#ifndef __CINT__
		void WriteEvent(std::shared_ptr<TFragment> event);
		void WriteBadEvent(std::shared_ptr<TFragment> event);
		void WriteScaler(std::shared_ptr<TEpicsFrag> scaler);
#endif

		TFile* fOutputFile;

		TTree* fEventTree;
		TTree* fBadEventTree;
		TTree* fScalerTree;

		TFragment*  fEventAddress;
		TFragment*  fBadEventAddress;
		TEpicsFrag* fScalerAddress;

#ifndef __CINT__
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > > fInputQueue;
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > > fBadInputQueue;
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TEpicsFrag> > > fScalerInputQueue;
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > > fOutputQueue;
#endif

		ClassDef(TFragWriteLoop, 0);
};


#endif /* _TWRITELOOP_H_ */
