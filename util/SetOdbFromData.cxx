#include <iostream>
#include <iomanip>

#include "TFile.h"

#include "Globals.h"
#include "TPPG.h"

int main(int argc, char** argv)
{
	if(argc == 1) {
		std::cerr<<"Usage: "<<argv[0]<<" <list of files>"<<std::endl;
		return 1;
	}

	// loop over all file names provided
	for(int i = 1; i < argc; ++i) {
		// open file
		TFile f(argv[i], "update");
		if(!f.IsOpen()) {
			std::cerr<<DYELLOW<<"Failed to open file "<<argv[i]<<RESET_COLOR<<std::endl;
			continue;
		}

		// get TPPG
		TPPG* ppg = static_cast<TPPG*>(f.Get("TPPG"));
		if(ppg == nullptr) {
			std::cerr<<DRED<<"Failed to find TPPG in "<<argv[i]<<" maybe this is a source run?"<<RESET_COLOR<<std::endl;
			f.Close();
			continue;
		}

		// check if ODB matches data, if so we're done
		if(ppg->OdbMatchesData()) {
			std::cout<<BLUE<<argv[i]<<": ODB already matches data, skipping it."<<RESET_COLOR<<std::endl;
			f.Close();
			continue;
		}

		// check if ODB is empty
		if(ppg->MapIsEmpty()) {
			std::cout<<DBLUE<<argv[i]<<": PPG is empty, probably a source run, skipping it."<<RESET_COLOR<<std::endl;
			f.Close();
			continue;
		}

		// set odb from data
		ppg->SetOdbFromData();

		// check that odb matches data, otherwise we have a problem
		if(!ppg->OdbMatchesData()) {
			std::cerr<<RED<<"ODB doesn't match data after we set it from the data!? Please check file "<<RESET_COLOR<<argv[i]<<std::endl;
			ppg->Print();
			f.Close();
			continue;
		}

		// write ppg and close file
		std::cout<<GREEN<<argv[i]<<" writing ODB set from data:"<<RESET_COLOR<<std::endl;
		ppg->Write();
		f.Close();
	}

	return 0;
}
