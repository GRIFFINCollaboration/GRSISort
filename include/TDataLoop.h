#ifndef _TDATALOOP_H_
#define _TDATALOOP_H_

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
#include "TMidasEvent.h"

class TMidasFile;

class TDataLoop : public StoppableThread  {
public:
  static TDataLoop *Get(std::string name="", TMidasFile* source=0);
  virtual ~TDataLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TMidasEvent> >& OutputQueue() { return output_queue; }
#endif

  const TMidasFile& GetSource() const { return *source; }

  std::string Status();
  virtual void ClearQueue();

  virtual bool Iteration();
  virtual void OnEnd();

  size_t GetItemsPushed()  { return output_queue->ItemsPushed(); }
  size_t GetItemsPopped()  { return output_queue->ItemsPopped(); }
  size_t GetItemsCurrent() { return output_queue->Size();        }
  size_t GetRate()         { return 0; }

  void ReplaceSource(TMidasFile* new_source);
  void ResetSource();

  void SetSelfStopping(bool self_stopping) { fSelfStopping = self_stopping; }
  bool GetSelfStopping() const { return fSelfStopping; }

private:
  TDataLoop(std::string name,TMidasFile* source);
  TDataLoop();
  TDataLoop(const TDataLoop& other);
  TDataLoop& operator=(const TDataLoop& other);

  TMidasFile* source;

  bool fSelfStopping;

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TMidasEvent> > output_queue;
  std::mutex source_mutex;
#endif

  ClassDef(TDataLoop, 0);
};

#endif /* _TDATALOOP_H_ */
