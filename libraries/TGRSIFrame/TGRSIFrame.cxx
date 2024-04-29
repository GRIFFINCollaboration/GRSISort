#include "TGRSIFrame.h"
#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,14,0)

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "TFile.h"
#include "TChain.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "TRunInfo.h"
#include "TPPG.h"
#include "TDataFrameLibrary.h"

TGRSIFrame::TGRSIFrame()
{
	// this assumes the options have been set from argc and argv before!
	fOptions = TGRSIOptions::Get();

#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 24, 0)
	// this increases RDF's verbosity level as long as the `fVerbosity` variable is in scope, i.e. until TGRSIFrame is destroyed
	if(fOptions->Debug()) {
		fVerbosity = new ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);
	}
#endif

	// check if we have a tree-name, otherwise get it from the first input file
	std::string treeName = fOptions->TreeName();
	if(treeName.empty()) {
		TFile check(fOptions->RootInputFiles()[0].c_str());
		if(check.Get("AnalysisTree") != nullptr) treeName = "AnalysisTree";
		else if(check.Get("FragmentTree") != nullptr) treeName = "FragmentTree";
		check.Close();
	}
	if(treeName.empty()) {
		std::stringstream str;
		str<<"Failed to find 'AnalysisTree' or 'FragmentTree' in '"<<fOptions->RootInputFiles()[0]<<"', either provide a different tree name via --tree-name flag or check input file"<<std::endl;
		throw std::runtime_error(str.str());
	}

	// only enable multi threading if number of threads isn't zero
	if(fOptions->GetMaxWorkers() > 0) {
		ROOT::EnableImplicitMT(fOptions->GetMaxWorkers());
	}

	auto chain = new TChain(treeName.c_str());

	fPpg = new TPPG;

	// loop over input files, add them to the chain, and read the runinfo and calibration from them
	for(const auto& fileName : fOptions->RootInputFiles()) {
		if(chain->Add(fileName.c_str(), 0) >= 1) { // setting nentries parameter to zero make TChain load the file header and return a 1 if the file was opened successfully
			TFile* file = TFile::Open(fileName.c_str());
			TRunInfo::AddCurrent();
			auto ppg = static_cast<TPPG*>(file->Get("PPG"));
			if(ppg != nullptr) {
				fPpg->Add(ppg);
			}
			TChannel::ReadCalFromFile(file);
			file->Close();
		} else {
			std::cout<<"Failed to open '"<<fileName<<"'"<<std::endl;
		}
	}

	std::cout<<"Looped over "<<chain->GetNtrees()<<"/"<<fOptions->RootInputFiles().size()<<" files and got:"<<std::endl;
	TRunInfo::Get()->Print();
	fPpg->Print("odb");

	fTotalEntries = chain->GetEntries();

	fDataFrame = new ROOT::RDataFrame(*chain);

	// create an input list to pass to the helper
	auto inputList = new TList;
	inputList->Add(fPpg);
	inputList->Add(TRunInfo::Get());
	inputList->Add(fOptions->AnalysisOptions());
	inputList->Add(fOptions->UserSettings());
	int i = 0;
   for(const auto& valFile : fOptions->ValInputFiles()) {
      inputList->Add(new TNamed(Form("valFile%d", i++), valFile.c_str()));
   }
   i = 0;
   for(const auto& calFile : fOptions->CalInputFiles()) {
      inputList->Add(new TNamed(Form("calFile%d", i++), calFile.c_str()));
   }
   i = 0;
   for(const auto& cutFile : fOptions->InputCutFiles()) {
      inputList->Add(new TNamed(Form("cutFile%d", i++), cutFile.c_str()));
   }

	/// Try to load an external library with the correct function in it.
	/// If that library does not exist, try to compile it.
	/// To handle all that we use the class TDataFrameLibrary (very similar to TParserLibrary)
	auto helper = TDataFrameLibrary::Get()->CreateHelper(inputList);
	fOutputPrefix = helper->Prefix();
	// this actually moves the helper to the data frame, so from here on "helper" doesn't refer to the object we created anymore
	// aka don't use helper after this!
	fOutput = helper->Book(fDataFrame);
}

void TGRSIFrame::Run()
{
	// get runinfo and get run and sub-run number
	auto runInfo = TRunInfo::Get();

	// get output file name
	std::string outputFileName = Form("%s%s.root", fOutputPrefix.c_str(), runInfo->CreateLabel(true).c_str());
	std::cout<<"Writing to "<<outputFileName<<std::endl;

	TFile outputFile(outputFileName.c_str(), "recreate");

#if ROOT_VERSION_CODE < ROOT_VERSION(6,30,0)
	std::string progressBar;
	const auto barWidth = 100;
	if(!fOptions->Debug()) {
		// create a progress bar with percentage
		auto entries = fDataFrame->Count();
		std::mutex barMutex; // Only one thread at a time can lock a mutex. Let's use this to avoid concurrent printing.
		const auto everyN = fTotalEntries/barWidth;
		//entries.OnPartialResultSlot(everyN, [&everyN, &fTotalEntries = fTotalEntries, &barWidth, &progressBar, &barMutex](unsigned int /*slot*/, ULong64_t& /*partialList*/) {
		entries.OnPartialResultSlot(everyN, [&everyN, &fTotalEntries = fTotalEntries, &progressBar, &barMutex](unsigned int /*slot*/, ULong64_t& /*partialList*/) {
				std::lock_guard<std::mutex> l(barMutex); // lock_guard locks the mutex at construction, releases it at destruction
				static int counter = 1;
				progressBar.push_back('#');
				// re-print the line with the progress bar
				std::cout<<"\r["<<std::left<<std::setw(barWidth)<<progressBar<<' '<<std::setw(3)<<(counter*everyN*100)/fTotalEntries<<" %]"<<std::flush;
				++counter;
				});
	}
#else
	ROOT::RDF::Experimental::AddProgressBar(*fDataFrame);
#endif

	if(fOutput != nullptr) {
		// accessing the result from Book causes the actual processing of the helper
		// so we try and catch any exception
		try {
			for(auto& list : *fOutput) {
				// try and switch to the directory this list should be written to
				if(gDirectory->GetDirectory(list.first.c_str()) && gDirectory->cd(list.first.c_str())) {
					list.second.Write();
				} else {
					// directory this list should be written to doesn't exist, so create it
					gDirectory->mkdir(list.first.c_str());
					if(gDirectory->cd(list.first.c_str())) {
						list.second.Write();
					} else {
						std::cout<<"Error, failed to find or create path "<<list.first<<", writing into "<<gDirectory->GetPath()<<std::endl;
					}
				}
				// switch back to topmost directory
				while(gDirectory->GetDirectory("..")) { gDirectory->cd(".."); }
			}
#if ROOT_VERSION_CODE < ROOT_VERSION(6,30,0)
         std::cout<<"\r["<<std::left<<std::setw(barWidth)<<progressBar<<' '<<"100 %]"<<std::flush;
#endif
		} catch(TGRSIMapException<std::string>& e) {
			std::cout<<DRED<<"Exception in "<<__PRETTY_FUNCTION__<<": "<<e.detail()<<RESET_COLOR<<std::endl;
			throw e;
		}
	} else {
		std::cout<<"Error, output list is nullptr!"<<std::endl;
	}
	runInfo->Write();
	if(fPpg != nullptr) {
		fPpg->Write();
	} else {
      std::cerr<<"failed to find TPPG, can't write it!"<<std::endl;
   }
   fOptions->AnalysisOptions()->WriteToFile(&outputFile);
	fOptions->UserSettings()->Write("UserSettings", TObject::kOverwrite);
   TChannel::WriteToRoot();
   outputFile.Close();
   std::cout<<"Closed '"<<outputFile.GetName()<<"'"<<std::endl;
}

void DummyFunctionToLocateTGRSIFrameLibrary() {
	// does nothing
}
#endif
