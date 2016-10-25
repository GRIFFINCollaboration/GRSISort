#ifndef _TUNPACKINGLOOP_H_
#define _TUNPACKINGLOOP_H_

#ifndef __CINT__
#include <memory>
#include "ThreadsafeQueue.h"
#endif

#include "StoppableThread.h"
#include "TMidasEvent.h"
#include "TFragment.h"
#include "TEpicsFrag.h"
#include "TDataParser.h"

class TUnpackingLoop : public StoppableThread {
public:
  static TUnpackingLoop *Get(std::string name="");
  virtual ~TUnpackingLoop();

  void SetNoWaveForms(bool temp = true) { fParser.SetNoWaveForms(temp); }
  void SetRecordDiag(bool temp = true)  { fParser.SetRecordDiag(temp); }

	std::string EndStatus();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TMidasEvent> >&                  InputQueue()                               { return fInputQueue; }
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > >&  AddGoodOutputQueue(size_t maxSize = 50000) { return fParser.AddGoodOutputQueue(maxSize); }
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment> > >&  BadOutputQueue()                           { return fParser.BadOutputQueue(); }
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TEpicsFrag> > >& ScalerOutputQueue()                        { return fParser.ScalerOutputQueue(); }
#endif

  bool Iteration();

  virtual void ClearQueue();

  size_t GetItemsPushed()  { return fParser.ItemsPushed(); }
  size_t GetItemsPopped()  { return 0; }//fParser.GoodOutputQueue()->ItemsPopped(); }
  size_t GetItemsCurrent() { return 0; }//fParser.GoodOutputQueue()->Size();        }
  size_t GetRate()         { return 0; }

private:
#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TMidasEvent> > fInputQueue;
#endif
  
  TDataParser fParser;
  long fFragsReadFromMidas;
  long fGoodFragsRead;

  TUnpackingLoop(std::string name);
  TUnpackingLoop(const TUnpackingLoop& other);
  TUnpackingLoop& operator=(const TUnpackingLoop& other);

  bool ProcessMidasEvent(TMidasEvent* event);
  bool ProcessEPICS  (float* ptr,int& dSize,TMidasEvent* mEvent);
  bool ProcessTIGRESS(uint32_t* ptr,int& dSize,TMidasEvent* mEvent);
  bool ProcessGRIFFIN(uint32_t* ptr,int& dSize,TDataParser::EBank bank, TMidasEvent* mEvent);


  //ClassDef(TUnpackingLoop, 0);
};

#endif /* _TUNPACKINGLOOP_H_ */
