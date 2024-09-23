#ifndef TDATALOOP_H
#define TDATALOOP_H

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TDataLoop
///
/// This loop reads raw events from a raw file.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>
#endif

#include <cstdlib>
#include <iostream>
#include <map>

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TRawFile.h"
#include "TRawEvent.h"

class TDataLoop : public StoppableThread {
public:
   static TDataLoop* Get(std::string name = "", TRawFile* source = nullptr);
   TDataLoop(const TDataLoop&)                = delete;
   TDataLoop(TDataLoop&&) noexcept            = delete;
   TDataLoop& operator=(const TDataLoop&)     = delete;
   TDataLoop& operator=(TDataLoop&&) noexcept = delete;
   ~TDataLoop()                               = default;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TRawEvent>>>& OutputQueue()
   {
      return fOutputQueue;
   }
#endif

   const TRawFile& GetSource() const
   {
      return *fSource;
   }

   void ClearQueue() override;

   bool Iteration() override;
   void OnEnd() override;

   size_t GetItemsPushed() override { return fOutputQueue->ItemsPushed(); }
   size_t GetItemsPopped() override { return fOutputQueue->ItemsPopped(); }
   size_t GetItemsCurrent() override { return fOutputQueue->Size(); }
   size_t GetRate() override { return 0; }

   void ReplaceSource(TRawFile* new_source);

   void SetSelfStopping(bool self_stopping) { fSelfStopping = self_stopping; }
   bool GetSelfStopping() const { return fSelfStopping; }

private:
   TDataLoop(std::string name, TRawFile* source);
   TDataLoop();

   TRawFile* fSource;
   bool      fSelfStopping;
   size_t    fEventsRead;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TRawEvent>>> fOutputQueue;
   std::mutex                                                   fSourceMutex;
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TDataLoop, 0)   // NOLINT(readability-else-after-return)
   /// \endcond
};

/*! @} */
#endif /* _TDATALOOP_H_ */
