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
  std::shared_ptr<ThreadsafeQueue<T*> >& InputQueue() { return input_queue; }
#endif

  virtual void ClearQueue() {
    while(input_queue->Size()){
      T* event = NULL;
      input_queue->Pop(event);
      if(event){
        delete event;
      }
    }
  }

  virtual size_t GetItemsPopped() { return 0; }
  virtual size_t GetItemsPushed() { return 0; }
  virtual size_t GetItemsCurrent() { return 0; }
  virtual size_t GetRate() { return 0; }

protected:
  bool Iteration() {
    T* event = NULL;
    input_queue->Pop(event);

    if(event) {
      delete event;
      return true;
    } else if(input_queue->IsFinished()) {
      return false;
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      return true;
    }
  }

private:
  TTerminalLoop(std::string name)
    : StoppableThread(name),
      input_queue(std::make_shared<ThreadsafeQueue<T*> >()) { }

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<T*> > input_queue;
#endif
};

/*! @} */
#endif /* _TTERMINALLOOP_H_ */
