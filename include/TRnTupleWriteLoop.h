#ifndef _TRNTUPLEWRITELOOP_H_
#define _TRNTUPLEWRITELOOP_H_

/** \addtogroup Loops
 *  @{
 */

#include <future>
#include <vector>

#include "TClass.h"
#include "ROOT/RField.hxx"
#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleModel.hxx"

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TUnpackedEvent.h"

////////////////////////////////////////////////////////////////////////////////
///
/// \class TRnTupleWriteLoop
///
/// This loop writes built events to file using the experiment TRnTuple.
///
////////////////////////////////////////////////////////////////////////////////

class TRnTupleWriteLoop : public StoppableThread {
public:
   static TRnTupleWriteLoop* Get(std::string name = "", std::string output_filename = "");

   ~TRnTupleWriteLoop() override;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>&  InputQueue() { return fInputQueue; }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& OutOfOrderQueue() { return fOutOfOrderQueue; }
#endif

   void ClearQueue() override;

   void Write();

   size_t GetItemsPushed() override { return ItemsPopped(); }
   size_t GetItemsPopped() override { return 0; }
   size_t GetItemsCurrent() override { return 0; }
   size_t GetRate() override { return 0; }

   std::string EndStatus() override;
	void OnEnd() override;

protected:
   bool Iteration() override;

private:
   TRnTupleWriteLoop(std::string name, std::string output_filename);
   void AddBranch(TClass* cls);
	void WriteEvent(std::shared_ptr<TUnpackedEvent>& event);

	std::unique_ptr<ROOT::Experimental::RNTupleWriter> fRNTupleWriter;
	//std::unique_ptr<ROOT::Experimental::RNTupleModel> fRNTupleModel;
   //TTree* fOutOfOrderTree;
   //TFragment* fOutOfOrderFrag;
	//bool fOutOfOrder;
#ifndef __CINT__
	std::map<TClass*, TDetector**> fDetMap;
   std::map<TClass*, TDetector*>  fDefaultDets;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>  fInputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>> fOutOfOrderQueue;
#endif

   ClassDefOverride(TRnTupleWriteLoop, 0);
};

/*! @} */
#endif /* _TWRITELOOP_H_ */
