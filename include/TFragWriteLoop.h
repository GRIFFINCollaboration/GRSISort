#ifndef _TWRITELOOP_H_
#define _TWRITELOOP_H_

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TFragWriteLoop
///
/// This loop writes fragments to a root-file.
///
////////////////////////////////////////////////////////////////////////////////

#include <map>

#include "TClass.h"
#include "TTree.h"

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TFragment.h"
#include "TBadFragment.h"
#include "TEpicsFrag.h"

class TFragWriteLoop : public StoppableThread {
public:
   static TFragWriteLoop* Get(std::string name = "", std::string fOutputFilename = "");

   ~TFragWriteLoop() override;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& InputQueue() { return fInputQueue; }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TBadFragment>>>& BadInputQueue() { return fBadInputQueue; }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TEpicsFrag>>>&      ScalerInputQueue() { return fScalerInputQueue; }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& OutputQueue() { return fOutputQueue; }
#endif

   void ClearQueue() override;

   void Write();

   // there is no output queue for this loop, so we assume that all items handled (= all good fragments written)
   // are also the number of items popped and that we have no current items
   size_t GetItemsPushed() override { return fItemsPopped; }
   size_t GetItemsPopped() override { return fItemsPopped; }
   size_t GetItemsCurrent() override { return 0; }
   size_t GetRate() override { return 0; }

   std::string EndStatus() override;

protected:
   bool Iteration() override;

private:
   TFragWriteLoop(std::string name, std::string fOutputFilename);
#ifndef __CINT__
   void WriteEvent(const std::shared_ptr<const TFragment>& event);
   void WriteBadEvent(const std::shared_ptr<const TBadFragment>& event);
   void WriteScaler(const std::shared_ptr<TEpicsFrag>& scaler);
#endif

   TFile* fOutputFile;

   TTree* fEventTree;
   TTree* fBadEventTree;
   TTree* fScalerTree;

   TFragment*    fEventAddress;
   TBadFragment* fBadEventAddress;
   TEpicsFrag*   fScalerAddress;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>> fInputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TBadFragment>>> fBadInputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TEpicsFrag>>>      fScalerInputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>> fOutputQueue;
#endif

   ClassDefOverride(TFragWriteLoop, 0);
};

/*! @} */
#endif /* _TWRITELOOP_H_ */
