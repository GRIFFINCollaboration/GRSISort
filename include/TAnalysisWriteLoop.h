#ifndef TANALYSISWRITELOOP_H
#define TANALYSISWRITELOOP_H

/** \addtogroup Loops
 *  @{
 */

#include <future>
#include <vector>

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
   static TAnalysisWriteLoop* Get(std::string name = "", std::string outputFilename = "");

   ~TAnalysisWriteLoop();

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>& InputQueue()
   {
      return fInputQueue;
   }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& OutOfOrderQueue() { return fOutOfOrderQueue; }
#endif

   void ClearQueue() override;

   void Write();

   size_t GetItemsPushed() override { return ItemsPopped(); }
   size_t GetItemsPopped() override { return 0; }
   size_t GetItemsCurrent() override { return 0; }
   size_t GetRate() override { return 0; }

   std::string EndStatus() override;
   void        OnEnd() override;

protected:
   bool Iteration() override;

private:
   TAnalysisWriteLoop(std::string name, const std::string& outputFilename);
	TAnalysisWriteLoop(const TAnalysisWriteLoop&) = delete;
	TAnalysisWriteLoop(TAnalysisWriteLoop&&) noexcept = delete;
	TAnalysisWriteLoop& operator=(const TAnalysisWriteLoop&) = delete;
	TAnalysisWriteLoop& operator=(TAnalysisWriteLoop&&) noexcept = delete;

   void AddBranch(TClass* cls);
   void WriteEvent(std::shared_ptr<TUnpackedEvent>& event);

   TFile*     fOutputFile;
   TTree*     fEventTree;
   TTree*     fOutOfOrderTree;
   TFragment* fOutOfOrderFrag;
   bool       fOutOfOrder;
#ifndef __CINT__
   std::map<TClass*, TDetector**>                                     fDetMap;
   std::map<TClass*, TDetector*>                                      fDefaultDets;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>  fInputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>> fOutOfOrderQueue;
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TAnalysisWriteLoop, 0) // NOLINT
   /// \endcond
};

/*! @} */
#endif /* _TWRITELOOP_H_ */
