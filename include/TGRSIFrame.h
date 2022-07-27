#ifndef TGRSIFRAME_H
#define TGRSIFRAME_H

#include <string>

#include "TList.h"

#include "TGRSIOptions.h"
#include "TPPG.h"
#include "ROOT/RDataFrame.hxx"

class TGRSIFrame {
public:
   TGRSIFrame();

	void Run();

private:
	std::string fOutputPrefix;
	ROOT::RDF::RResultPtr<TList> fOutput;

	TGRSIOptions* fOptions{nullptr};
	TPPG* fPpg{nullptr};

	ROOT::RDataFrame* fDataFrame{nullptr};
	Long64_t fTotalEntries{0};
};

void DummyFunctionToLocateTGRSIFrameLibrary();

#endif
