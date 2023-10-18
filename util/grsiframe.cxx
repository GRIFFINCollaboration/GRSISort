#include <iostream>
#include <string>
#include <vector>

#include "TStyle.h"
#include "TFile.h"
#include "TChain.h"
#include "ROOT/RDataFrame.hxx"
#include "TEnv.h"
#include "TSystem.h"
#include "TStopwatch.h"

#include "TGRSIOptions.h"
#include "TChannel.h"
#include "TParserLibrary.h"
#include "TGRSIFrame.h"

int main(int argc, char** argv)
{
	auto stopwatch = new TStopwatch;

	TGRSIOptions* opt = nullptr;
	try {
		std::string grsi_path = getenv("GRSISYS"); // Finds the GRSISYS path to be used by other parts of the grsisort code
		if(grsi_path.length() > 0) {
			grsi_path += "/";
		}
		// Read in grsirc in the GRSISYS directory to set user defined options on grsisort startup
		grsi_path += ".grsirc";
		gEnv->ReadFile(grsi_path.c_str(), kEnvChange);
		opt = TGRSIOptions::Get(argc, argv);
		if(opt->ShouldExit()) {
			return 0;
		}
	} catch(ParseError& e) {
		return 1;
	}

	// load parser library if provided
	if(!opt->ParserLibrary().empty()) {
		try {
			TParserLibrary::Get()->Load();
		} catch(std::runtime_error& e) {
			// if we failed to load the library, we quit
			std::cerr<<DRED<<e.what()<<RESET_COLOR<<std::endl;
			return 1;
		}
	} else {
		std::cout<<"No parser library set!"<<std::endl;
		return 1;
	}

	// check that we have input files and a macro
	if(opt->RootInputFiles().empty()) {
		std::cout<<"No input files provided!"<<std::endl;
		return 1;
	}
	if(opt->DataFrameLibrary().empty()) {
		std::cout<<"No dataframe library (*.cxx file or *.so library) provided!"<<std::endl;
		return 1;
	}

	// this reads and compiles the user code
	TGRSIFrame frame;
	// run it and write the results
	frame.Run();

	// print time it took to run
   double realTime = stopwatch->RealTime();
   int    hour     = static_cast<int>(realTime / 3600);
   realTime -= hour * 3600;
   int min = static_cast<int>(realTime / 60);
   realTime -= min * 60;
   std::cout<<DMAGENTA<<std::endl
      <<"Done after "<<hour<<":"<<std::setfill('0')<<std::setw(2)<<min<<":"
      <<std::setprecision(3)<<std::fixed<<realTime<<" h:m:s"
      <<RESET_COLOR<<std::endl;

	return 0;
}
