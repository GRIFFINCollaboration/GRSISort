#ifndef _TUNPACKINGLOOP_H_
#define _TUNPACKINGLOOP_H_

#ifndef __CINT__
#include <memory>
#endif

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TRawEvent.h"

class TUnpackingLoop : public StoppableThread {
public:
  static TUnpackingLoop *Get(std::string name="");
  virtual ~TUnpackingLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TRawEvent> >& InputQueue() { return input_queue; }
  std::shared_ptr<ThreadsafeQueue<TFragment> >& OutputQueue() { return output_queue; }
#endif

  bool Iteration();

  virtual void ClearQueue();

  size_t GetItemsPushed()  { return output_queue->ItemsPushed(); }
  size_t GetItemsPopped()  { return output_queue->ItemsPopped(); }
  size_t GetItemsCurrent() { return output_queue->Size();        }
  size_t GetRate()         { return 0; }

private:
  TUnpackingLoop(std::string name);
  TUnpackingLoop(const TUnpackingLoop& other);
  TUnpackingLoop& operator=(const TUnpackingLoop& other);

  bool ProcessEPICS  (uint32_t* ptr,int& dSize,TDataParser::EBank bank, TMidasEvent* mEvent);
  bool ProcessTIGRESS(uint32_t* ptr,int& dSize,TDataParser::EBank bank, TMidasEvent* mEvent);
  bool ProcessGRIFFIN(uint32_t* ptr,int& dSize,TDataParser::EBank bank, TMidasEvent* mEvent);

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TMidasEvent> > input_queue;
  std::shared_ptr<ThreadsafeQueue<TFragment> > output_queue;
#endif

  ClassDef(TUnpackingLoop, 0);
};

#endif /* _TUNPACKINGLOOP_H_ */
