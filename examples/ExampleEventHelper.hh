// TODO: Replace all EXAMPLEVENT and ExampleEvent with the name you want to use for this helper action
#ifndef EXAMPLEEVENTHELPER_HH
#define EXAMPLEEVENTHELPER_HH

#include "TGRSIHelper.h"

// TODO: edit these include statments to match the detectors you want to use!
#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TZeroDegree.h"

// This is a custom action which respects a well defined interface. It supports parallelism,
// in the sense that it behaves correctly if implicit multi threading is enabled.
// Note the plural: in presence of a MT execution, internally more than a single TList is created.
// The detector types in the specifcation of Book must match those in the call to it as well as those in the Exec function (and be in the same order)!

class ExampleEventHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<ExampleEventHelper> {
public:
	// constructor sets the prefix (which is used for the output file as well)
	// and calls Setup which in turn also calls CreateHistograms
	ExampleEventHelper(TList* list) : TGRSIHelper(list) {
		Prefix("ExampleEventHelper");
		Setup();
	}

	ROOT::RDF::RResultPtr<TList> Book(ROOT::RDataFrame* d) override {
		// TODO: edit the template specification and branch names to match the detectors you want to use!
		return d->Book<TGriffin, TGriffinBgo, TZeroDegree>(std::move(*this), {"TGriffin", "TGriffinBgo", "TZeroDegree"});
	}
	// this function creates and books all histograms
	void CreateHistograms(unsigned int i);
	// this function gets called for every single event and fills the histograms
	// TODO: edit the function arguments to match the detectors you want to use!
	void Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo, TZeroDegree& zds);
	// this function is optional and is called after the output lists off all slots/workers have been merged
	void EndOfSort(std::shared_ptr<TList> list) override;

private:
	// any constants that are set in the CreateHistograms function and used in the Exec function can be stored here
	// or any other settings
	Long64_t fCycleLength{0};
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" ExampleEventHelper* CreateHelper(TList* list) { return new ExampleEventHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

#endif
