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
		std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment> > > >& InputQueue() { return fInputQueue; }
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent> > >& AddOutputQueue(size_t maxSize = 50000) {
			std::stringstream name; name<<"event_queue_"<<fOutputQueues.size();
			fOutputQueues.push_back(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent> > >(name.str(), maxSize));
			return fOutputQueues.back(); 
		}
#endif

		bool Iteration();
		virtual void ClearQueue();

		size_t GetItemsPushed()  { return fOutputQueues.back()->ItemsPushed(); } //this should work fine as all loops are always filled at the same time
		size_t GetItemsPopped()  { return 0; }//fOutputQueue->ItemsPopped(); }
		size_t GetItemsCurrent() { return 0; }//fOutputQueue->Size();        }
		size_t GetRate()         { return 0; }

	private:
		TDetBuildingLoop(std::string name);
		TDetBuildingLoop(const TDetBuildingLoop& other);
		TDetBuildingLoop& operator=(const TDetBuildingLoop& other);

#ifndef __CINT__
		std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment> > > > fInputQueue;
		std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent> > > > fOutputQueues;
#endif

		ClassDef(TDetBuildingLoop, 0);
		};

#endif /* _TDETBUILDINGLOOP_H_ */
