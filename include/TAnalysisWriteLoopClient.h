#ifndef T_ANALYSIS_WRITE_LOOP_CLIENT_HH
#define T_ANALYSIS_WRITE_LOOP_CLIENT_HH
#include <iostream>
#include <future>
#include "TFile.h"
#include "TTree.h"
#include "TBenchmark.h"
#include "TMessage.h"
#include "TSocket.h"
#include "TMemFile.h"
#include "TFileMerger.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TFileCacheWrite.h"
#include "TThread.h"
#include "TParallelMergingFile.h"
#include "TRandom.h"
#include "TError.h"
#include "TH2.h"

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TUnpackedEvent.h"
#include "TFragment.h"

////////////////////////////////////////////////////////////////////////////////
///
/// \class TAnalysisWriteLoopClient
///
/// The client(s) for the TAnalysisWriteLoop. Each gets a subset of event to 
/// write to file.
///
////////////////////////////////////////////////////////////////////////////////


class TAnalysisWriteLoopClient : public StoppableThread {
public:
	TAnalysisWriteLoopClient(std::string name, std::string outputFilename, Int_t localPort);
	~TAnalysisWriteLoopClient() override;

#ifndef __CINT__
	std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>> InputQueue() { return fInputQueue; }
	std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>> OutOfOrderQueue() { return fOutOfOrderQueue; }
	void OutOfOrderFragment(std::shared_ptr<const TFragment> frag);
#endif

	void ClearQueue() override;

   size_t GetItemsPushed() override { return fItemsPopped; }
   size_t GetItemsPopped() override { return 0; }
   size_t GetItemsCurrent() override { return 0; }
   size_t GetRate() override { return 0; }
	size_t GetInputSize() { return fInputSize; }

	void OnEnd() override;

protected:
	bool Iteration() override;

private:
	void AddBranch(TClass* cls);
	void WriteEvent(std::shared_ptr<TUnpackedEvent>& event);

	TParallelMergingFile* fOutputFile;
	TTree* fEventTree;
	TTree* fOutOfOrderTree;
	TFragment* fOutOfOrderFrag;
	bool fOutOfOrder;
	bool fFirstClient;
#ifndef __CINT__
	//static std::mutex fGlobalMapMutex;
	static std::map<TClass*, TDetector**> fGlobalDetMap;
	std::map<TClass*, TDetector**> fDetMap;
	std::map<TClass*, TDetector*>  fDefaultDets;
	std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>  fInputQueue;
	std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>> fOutOfOrderQueue;
#endif
};
#endif
