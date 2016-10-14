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
		std::shared_ptr<ThreadsafeQueue<TFragment*> >& InputQueue() { return fInputQueue; }
		std::shared_ptr<ThreadsafeQueue<TFragment*> >& BadInputQueue() { return fBadInputQueue; }
		std::shared_ptr<ThreadsafeQueue<TEpicsFrag*> >& ScalerInputQueue() { return fScalerInputQueue; }
		std::shared_ptr<ThreadsafeQueue<TFragment*> >& OutputQueue() { return fOutputQueue; }
#endif

		virtual void ClearQueue();

		void Write();

		size_t GetItemsPushed()  { return fOutputQueue->ItemsPushed(); }
		size_t GetItemsPopped()  { return fOutputQueue->ItemsPopped(); }
		size_t GetItemsCurrent() { return fOutputQueue->Size(); }
		size_t GetRate()         { return 0; }

		std::string Status();
		std::string EndStatus();

	protected:
		bool Iteration();

	private:
		TFragWriteLoop(std::string name, std::string outputFile);
		void WriteEvent(TFragment& event);
		void WriteBadEvent(TFragment& event);
		void WriteScaler(TEpicsFrag& scaler);

		TFile* fOutputFile;

		TTree* fEventTree;
		TTree* fBadEventTree;
		TTree* fScalerTree;

		TFragment* fEventAddress;
		TFragment* fBadEventAddress;
		TEpicsFrag* fScalerAddress;

		size_t fItemsHandled;
		int fInputQueueSize;

#ifndef __CINT__
		std::shared_ptr<ThreadsafeQueue<TFragment*> > fInputQueue;
		std::shared_ptr<ThreadsafeQueue<TFragment*> > fBadInputQueue;
		std::shared_ptr<ThreadsafeQueue<TEpicsFrag*> > fScalerInputQueue;
		std::shared_ptr<ThreadsafeQueue<TFragment*> > fOutputQueue;
#endif

		ClassDef(TFragWriteLoop, 0);
};


#endif /* _TWRITELOOP_H_ */
