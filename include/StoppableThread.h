#ifndef _STOPPABLETHREAD_H_
#define _STOPPABLETHREAD_H_

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class StoppableThread
///
/// Base-class for all loops/threads.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include <atomic>
#include <condition_variable>
#include <thread>
#endif

#include <sstream>
#include <iomanip>
#include <string>
#include <map>

#include "TObject.h"

class StoppableThread {
public:
   static void        SendStop();
   static void        StopAll();
   static bool        AnyThreadRunning();
   static std::string AnyThreadStatus();
   static std::string AllThreadProgress();
   static std::string AllThreadHeader();
   static std::string AllThreadStatus();

   static void PauseAll();
   static void ResumeAll();

   StoppableThread(std::string name);
   static StoppableThread* Get(std::string name);
   static std::vector<StoppableThread*> GetAll();
   virtual ~StoppableThread();

   void Resume();
   void Pause();
   void Stop();
   bool IsPaused();
   bool IsRunning();
   void Join();

   virtual void        OnEnd() {}
   virtual std::string Status();
   virtual std::string Progress();
   virtual std::string EndStatus() { return std::string(); } // std::stringstream ss; ss<<std::endl; return ss.str(); }
   std::string         Name() const { return fName; }

   virtual void ClearQueue() {}
	static void ClearAllQueues();

   // protected:
   virtual bool Iteration() = 0;

   virtual size_t GetItemsPopped()  = 0;
   virtual size_t GetItemsPushed()  = 0;
   virtual size_t GetItemsCurrent() = 0;
   virtual size_t GetRate()         = 0;

   static int GetNThreads();

   static void Print();

   static void ColumnWidth(size_t val) { fColumnWidth = val; }
   static void StatusWidth(size_t val) { fStatusWidth = val; }
   static size_t                  ColumnWidth() { return fColumnWidth; }
   static size_t                  StatusWidth() { return fStatusWidth; }

#ifndef __CINT__
   static std::thread status_thread;
#endif
   static bool status_thread_on;
   static void start_status_thread();
   static void stop_status_thread();
   static void join_status_thread();
   static void status_out_loop();
   static void status_out();

protected:
   static std::map<std::string, StoppableThread*> fThreadMap;

#ifndef __CINT__
   std::atomic_size_t fItemsPopped{}; ///< number of items popped from input queue
   std::atomic_long   fInputSize{};   ///< number of items in the input (queue), only updated within Iteration(), so not
                                    ///< always fully up-to-date (signed to hold error from queue::pop)
#endif

private:
   StoppableThread(const StoppableThread&) {}
   StoppableThread& operator=(const StoppableThread&) { return *this; }

   std::string   fName;
   static size_t fColumnWidth;
   static size_t fStatusWidth;

   void Loop();

#ifndef __CINT__
   std::thread             thread;
   std::atomic_bool        running{};
   std::atomic_bool        force_stop{};
   std::atomic_bool        paused{};
   std::condition_variable paused_wait;
   std::mutex              pause_mutex{};
#endif

   ClassDef(StoppableThread, 0);
};

/*! @} */
#endif /* _STOPPABLETHREAD_H_ */
