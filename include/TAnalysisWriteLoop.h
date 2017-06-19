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
   static TAnalysisWriteLoop* Get(std::string name = "", std::string output_filename = "");

   virtual ~TAnalysisWriteLoop();

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>&  InputQueue() { return fInputQueue; }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& OutOfOrderQueue() { return fOutOfOrderQueue; }
#endif

   void ClearQueue() override;

   void Write();

   size_t GetItemsPushed() override { return fItemsPopped; }
   size_t GetItemsPopped() override { return 0; }
   size_t GetItemsCurrent() override { return 0; }
   size_t GetRate() override { return 0; }

   std::string EndStatus() override;

protected:
   bool Iteration() override;

private:
   TAnalysisWriteLoop(std::string name, std::string output_file);
   void AddBranch(TClass* cls);

   void WriteEvent(TUnpackedEvent& event);
   TFile* fOutputFile;
   TTree* fEventTree;

   TTree*     fOutOfOrderTree;
   TFragment* fOutOfOrderFrag;
#ifndef __CINT__
   std::map<TClass*, TDetector**> fDetMap;
   std::map<TClass*, TDetector*>  fDefaultDets;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>  fInputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>> fOutOfOrderQueue;
#endif

   ClassDefOverride(TAnalysisWriteLoop, 0);
};

/*! @} */
#endif /* _TWRITELOOP_H_ */
