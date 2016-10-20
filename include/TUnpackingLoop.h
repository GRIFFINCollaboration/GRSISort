#ifndef _TUNPACKINGLOOP_H_
#define _TUNPACKINGLOOP_H_

#ifndef __CINT__
#include <memory>
#endif

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TMidasEvent.h"
#include "TFragment.h"
#include "TEpicsFrag.h"
#include "TDataParser.h"

class TUnpackingLoop : public StoppableThread {
public:
  static TUnpackingLoop *Get(std::string name="");
  virtual ~TUnpackingLoop();

  void SetNoWaveForms(bool temp = true) { parser.SetNoWaveForms(temp); }
  void SetRecordDiag(bool temp = true) { parser.SetRecordDiag(temp); }

	//std::string Status(); TODO: Might be able to do something here with Midas events
	std::string EndStatus();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TMidasEvent> >& InputQueue() { return input_queue; }
  std::shared_ptr<ThreadsafeQueue<TFragment*> >&
  GoodOutputQueue() { return parser.GoodOutputQueue(); }

  std::shared_ptr<ThreadsafeQueue<TFragment*> >&
  BadOutputQueue() { return parser.BadOutputQueue(); }

  std::shared_ptr<ThreadsafeQueue<TEpicsFrag*> >&
  ScalerOutputQueue() { return parser.ScalerOutputQueue(); }
#endif

  bool Iteration();

  virtual void ClearQueue();

  size_t GetItemsPushed()  { return parser.GoodOutputQueue()->ItemsPushed(); }
  size_t GetItemsPopped()  { return parser.GoodOutputQueue()->ItemsPopped(); }
  size_t GetItemsCurrent() { return parser.GoodOutputQueue()->Size();        }
  size_t GetRate()         { return 0; }

private:
#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TMidasEvent> > input_queue;
#endif
  
  TDataParser parser;
  long fFragsReadFromMidas;
  long fGoodFragsRead;

  TUnpackingLoop(std::string name);
  TUnpackingLoop(const TUnpackingLoop& other);
  TUnpackingLoop& operator=(const TUnpackingLoop& other);

  bool ProcessMidasEvent(TMidasEvent* event);
  bool ProcessEPICS  (float* ptr,int& dSize,TMidasEvent* mEvent);
  bool ProcessTIGRESS(uint32_t* ptr,int& dSize,TMidasEvent* mEvent);
  bool ProcessGRIFFIN(uint32_t* ptr,int& dSize,TDataParser::EBank bank, TMidasEvent* mEvent);


  ClassDef(TUnpackingLoop, 0);
};

#endif /* _TUNPACKINGLOOP_H_ */
