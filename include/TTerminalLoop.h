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

template <typename T>
class TTerminalLoop : public StoppableThread {
public:
   static TTerminalLoop* Get(std::string name = "")
   {
      if(name.length() == 0) {
         name = "terminal_loop";
      }

      StoppableThread* thread = StoppableThread::Get(name);
      if(!thread) {
         thread = new TTerminalLoop(name);
      }

      return dynamic_cast<TTerminalLoop*>(thread);
   }

   ~TTerminalLoop() override = default;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<T>>>& InputQueue()
   {
      return fInputQueue;
   }

   void ClearQueue() override
   {
      while(fInputQueue->Size()) {
         std::shared_ptr<T> event;
         fInputQueue->Pop(event);
      }
   }
#endif

   size_t GetItemsPopped() override
   {
      return 0;
   }
   size_t GetItemsPushed() override { return 0; }
   size_t GetItemsCurrent() override { return 0; }
   size_t GetRate() override { return 0; }

protected:
#ifndef __CINT__
   bool Iteration() override
   {
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
      : StoppableThread(name), fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<T>>>())
   {
   }

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<T>>> fInputQueue;
#endif
};

/*! @} */
#endif /* _TTERMINALLOOP_H_ */
