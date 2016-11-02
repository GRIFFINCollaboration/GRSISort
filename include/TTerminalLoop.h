#ifndef _TTERMINALLOOP_H_
#define _TTERMINALLOOP_H_

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TTerminalLoop
/// 
/// Template for loops to terminate queues.
///
////////////////////////////////////////////////////////////////////////////////

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"

template<typename T>
class TTerminalLoop : public StoppableThread {
	public:
		static TTerminalLoop* Get(std::string name="") {
			if(name.length()==0){
				name = "terminal_loop";
			}

			StoppableThread* thread = StoppableThread::Get(name);
			if(!thread){
				thread = new TTerminalLoop(name);
			}

			return dynamic_cast<TTerminalLoop*>(thread);
		}

		~TTerminalLoop() { }

#ifndef __CINT__
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<T> > >& InputQueue() { return fInputQueue; }

		virtual void ClearQueue() {
			while(fInputQueue->Size()){
				std::shared_ptr<T> event;
				fInputQueue->Pop(event);
			}
		}
#endif

		virtual size_t GetItemsPopped() { return 0; }
		virtual size_t GetItemsPushed() { return 0; }
		virtual size_t GetItemsCurrent() { return 0; }
		virtual size_t GetRate() { return 0; }

	protected:
#ifndef __CINT__
		bool Iteration() {
			std::shared_ptr<T> event;
			fInputQueue->Pop(event);

			if(event) {
				return true;
			} else if(fInputQueue->IsFinished()) {
				return false;
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				return true;
			}
		}
#endif

	private:
		TTerminalLoop(std::string name)
			: StoppableThread(name),
			fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<T> > >()) { }

#ifndef __CINT__
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<T> > > fInputQueue;
#endif
};

/*! @} */
#endif /* _TTERMINALLOOP_H_ */
