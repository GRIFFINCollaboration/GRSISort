#ifndef TGRSIFRAME_H
#define TGRSIFRAME_H
#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,14,0)

#include <map>
#include <string>

#include "TList.h"

#include "TGRSIOptions.h"
#include "TPPG.h"
#include "ROOT/RDataFrame.hxx"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 24, 0)
#include "ROOT/RLogger.hxx"
#endif

/** \addtogroup Framing
 *  * @{
 *  */

class TGRSIFrame {
public:
   TGRSIFrame();

	void Run();

private:
	std::string fOutputPrefix{"default"};
	ROOT::RDF::RResultPtr<std::map<std::string, TList>> fOutput;

	TGRSIOptions* fOptions{nullptr};
	TPPG* fPpg{nullptr};

	ROOT::RDataFrame* fDataFrame{nullptr};
	Long64_t fTotalEntries{0};

#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 24, 0)
	ROOT::Experimental::RLogScopedVerbosity* fVerbosity{nullptr};
#endif
};

/*! @} */
void DummyFunctionToLocateTGRSIFrameLibrary();

#endif
#endif
