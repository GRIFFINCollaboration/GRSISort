#ifndef _TDATALOOP_H_
#define _TDATALOOP_H_

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TDataLoop
///
/// This loop reads midas events from a midas file.
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
#include "TXMLOdb.h"

// class TMidasFile;

class TDataLoop : public StoppableThread {
public:
   static TDataLoop* Get(std::string name = "", TRawFile* source = nullptr);
   ~TDataLoop() override;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TRawEvent>>>& OutputQueue() { return fOutputQueue; }
#endif

   const TRawFile& GetSource() const { return *fSource; }

   void ClearQueue() override;

   bool Iteration() override;
   void OnEnd() override;

   size_t GetItemsPushed() override { return fOutputQueue->ItemsPushed(); }
   size_t GetItemsPopped() override { return fOutputQueue->ItemsPopped(); }
   size_t GetItemsCurrent() override { return fOutputQueue->Size(); }
   size_t GetRate() override { return 0; }

   void ReplaceSource(TRawFile* new_source);
   void ResetSource();

   void SetSelfStopping(bool self_stopping) { fSelfStopping = self_stopping; }
   bool                      GetSelfStopping() const { return fSelfStopping; }

private:
   TDataLoop(std::string name, TRawFile* source);
   TDataLoop();
   TDataLoop(const TDataLoop& other);
   TDataLoop& operator=(const TDataLoop& other);

   TRawFile* fSource;
   bool      fSelfStopping;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TRawEvent>>> fOutputQueue;
   std::mutex                                                   fSourceMutex;
#endif

#ifdef HAS_XML
   TXMLOdb* fOdb;
#endif

   void SetFileOdb(uint32_t time, char* data, int size);
   void SetRunInfo(uint32_t time);
   void SetEPICSOdb();
   void SetTIGOdb();
   void SetGRIFFOdb();

   ClassDefOverride(TDataLoop, 0);
};

/*! @} */
#endif /* _TDATALOOP_H_ */
