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

//#include "Utilities.hh"


class TAnalysisWriteLoopClient {
	public:
		TAnalysisWriteLoopClient(const char*);

	private:
		TParallelMergingFile* fFile;
		TTree* fEventTree;
		TTree* fOutOfOrderTree;
};
#endif
