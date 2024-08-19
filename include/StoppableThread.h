#ifndef STOPPABLETHREAD_H
#define STOPPABLETHREAD_H

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
   explicit StoppableThread(std::string name);
   StoppableThread(const StoppableThread&)                = delete;
   StoppableThread(StoppableThread&&) noexcept            = delete;
   StoppableThread& operator=(const StoppableThread&)     = delete;
   StoppableThread& operator=(StoppableThread&&) noexcept = delete;
   virtual ~StoppableThread();

   static void        SendStop();
   static void        StopAll();
   static bool        AnyThreadRunning();
   static std::string AnyThreadStatus();
   static std::string AllThreadProgress();
   static std::string AllThreadHeader();
   static std::string AllThreadStatus();

   static void PauseAll();
   static void ResumeAll();

   static StoppableThread*              Get(const std::string& name);
   static std::vector<StoppableThread*> GetAll();

   void Resume();
   void Pause();
   void Stop();
   bool IsPaused();
   bool IsRunning();
   void Join();

   virtual void        OnEnd() {}
   virtual std::string Status();
   virtual std::string Progress();
   virtual std::string EndStatus() { return {}; }
   std::string         Name() const { return fName; }

   virtual void ClearQueue() {}
   static void  ClearAllQueues();

   // protected:
   virtual bool Iteration() = 0;

   virtual size_t GetItemsPopped()  = 0;
   virtual size_t GetItemsPushed()  = 0;
   virtual size_t GetItemsCurrent() = 0;
   virtual size_t GetRate()         = 0;

   static int GetNThreads();

   static void Print();

   static void   ColumnWidth(size_t val) { fColumnWidth = val; }
   static void   StatusWidth(size_t val) { fStatusWidth = val; }
   static size_t ColumnWidth() { return fColumnWidth; }
   static size_t StatusWidth() { return fStatusWidth; }

   static void start_status_thread();
   static void stop_status_thread();
   static void join_status_thread();
   static void status_out_loop();
   static void status_out();

protected:
#ifndef __CINT__
   void                ItemsPopped(size_t val) { fItemsPopped = val; }
   void                InputSize(int64_t val) { fInputSize = val; }
   std::atomic_size_t& ItemsPopped() { return fItemsPopped; }
   std::atomic_long&   InputSize() { return fInputSize; }
   void                IncrementItemsPopped() { ++fItemsPopped; }
#endif
private:
#ifndef __CINT__
   std::atomic_size_t fItemsPopped{0};   ///< number of items popped from input queue
   std::atomic_long   fInputSize{0};     ///< number of items in the input (queue), only updated within Iteration(), so not
                                         ///< always fully up-to-date (signed to hold error from queue::pop)
#endif

   std::string fName;

   static size_t fColumnWidth;
   static size_t fStatusWidth;

   void Loop();

   static std::map<std::string, StoppableThread*> fThreadMap;

   static bool fStatusThreadOn;
#ifndef __CINT__
   static std::thread      fStatusThread;
   std::thread             fThread;
   std::atomic_bool        fRunning{false};
   std::atomic_bool        fForceStop{false};
   std::atomic_bool        fPaused{false};
   std::condition_variable fPausedWait;
   std::mutex              fPauseMutex;
#endif

   /// \cond CLASSIMP
   ClassDef(StoppableThread, 0)   // NOLINT
   /// \endcond
};

/*! @} */
#endif /* _STOPPABLETHREAD_H_ */
