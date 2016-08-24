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
#include "TXMLOdb.h"

class TMidasFile;

class TDataLoop : public StoppableThread  {
public:
  static TDataLoop *Get(std::string name="", TMidasFile* source=0);
  virtual ~TDataLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TMidasEvent> >& OutputQueue() { return output_queue; }
#endif

  const TMidasFile& GetSource() const { return *fSource; }

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

  TMidasFile* fSource;
  bool fSelfStopping;

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TMidasEvent> > output_queue;
  std::mutex source_mutex;
#endif

  TXMLOdb* fOdb;

  void SetFileOdb(char* data, int size);
  void SetTIGOdb();
  void SetGRIFFOdb();

  ClassDef(TDataLoop, 0);
};

#endif /* _TDATALOOP_H_ */
