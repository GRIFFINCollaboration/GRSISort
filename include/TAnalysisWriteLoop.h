#ifndef _TANALYSISWRITELOOP_H_
#define _TANALYSISWRITELOOP_H_

#include <map>

#include "TClass.h"
#include "TTree.h"

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TUnpackedEvent.h"

class TAnalysisWriteLoop : public StoppableThread {
public:
  static TAnalysisWriteLoop* Get(std::string name="", std::string output_filename="");

  virtual ~TAnalysisWriteLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent> > >& InputQueue() { return fInputQueue; }
#endif

  virtual void ClearQueue();

  void Write();

  size_t GetItemsPushed()  { return 0; }
  size_t GetItemsPopped()  { return 0; }
  size_t GetItemsCurrent() { return 0; }
  size_t GetRate()         { return 0; }

	std::string Status();
	std::string EndStatus();

protected:
  bool Iteration();

private:
  TAnalysisWriteLoop(std::string name, std::string output_file);
  void AddBranch(TClass* cls);

  void WriteEvent(TUnpackedEvent& event);
  TFile* fOutputFile;
  TTree* fEventTree;

  size_t fItemsHandled;

  int fInputQueueSize;

#ifndef __CINT__
  std::map<TClass*, TDetector**> fDetMap;
  std::map<TClass*, TDetector*> fDefaultDets;
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent> > > fInputQueue;
#endif

  ClassDef(TAnalysisWriteLoop, 0);
};


#endif /* _TWRITELOOP_H_ */
