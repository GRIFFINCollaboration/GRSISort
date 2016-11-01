#ifndef _TANALYSISWRITELOOP_H_
#define _TANALYSISWRITELOOP_H_

/** \addtogroup Loops
 *  @{
 */

#include <map>

#include "TClass.h"
#include "TTree.h"

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TUnpackedEvent.h"

////////////////////////////////////////////////////////////////////////////////
///
/// \class TAnalysisWriteLoop
/// 
/// This loop writes built events to file
///
////////////////////////////////////////////////////////////////////////////////

class TAnalysisWriteLoop : public StoppableThread {
public:
  static TAnalysisWriteLoop* Get(std::string name="", std::string output_filename="");

  virtual ~TAnalysisWriteLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> >& InputQueue() { return input_queue; }
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> >& OutputQueue() { return output_queue; }
#endif

  virtual void ClearQueue();

  void Write();

  size_t GetItemsPushed()  { return OutputQueue()->ItemsPushed(); }
  size_t GetItemsPopped()  { return OutputQueue()->ItemsPopped(); }
  size_t GetItemsCurrent() { return OutputQueue()->Size();        }
  size_t GetRate()         { return 0; }

	std::string Status();
	std::string EndStatus();

protected:
  bool Iteration();

private:
  TAnalysisWriteLoop(std::string name, std::string output_file);
  void AddBranch(TClass* cls);

  void WriteEvent(TUnpackedEvent& event);
  TFile* output_file;
  TTree* event_tree;
  std::map<TClass*, TDetector**> det_map;
  std::map<TClass*, TDetector*> default_dets;

  size_t items_handled;

  int fInputQueueSize;

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> > input_queue;
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> > output_queue;
#endif

  ClassDef(TAnalysisWriteLoop, 0);
};

/*! @} */
#endif /* _TWRITELOOP_H_ */
