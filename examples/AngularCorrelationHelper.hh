#ifndef AngularCorrelationHelper_h
#define AngularCorrelationHelper_h

#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TGriffinAngles.h"
#include "TGRSIHelper.h"

class AngularCorrelationHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<AngularCorrelationHelper> {
private:
	double fGriffinDistance = 145.;

	bool fFolding = true;
	bool fGrouping = true;
	bool fAddback = true;

	TGriffinAngles* fAngles = new TGriffinAngles(fGriffinDistance, fFolding, fGrouping, fAddback);

	std::map<unsigned int, std::deque<TGriffin*>> fGriffinDeque;
	std::map<unsigned int, std::deque<TGriffinBgo*>> fBgoDeque;

public :
	AngularCorrelationHelper(TList* list) : TGRSIHelper(list) {
		Prefix("AngularCorrelation");

		fAngles->Print();

		// Setup calls CreateHistograms, which uses the stored angle combinations, so we need those set before
		Setup();
	}

	ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override {
		return d->Book<TGriffin, TGriffinBgo>(std::move(*this), {"TGriffin", "TGriffinBgo"});
	}

	void CreateHistograms(unsigned int slot);
	void Exec(unsigned int slot, TGriffin& fGriffin, TGriffinBgo& fGriffinBgo);
};

#endif

extern "C" AngularCorrelationHelper* CreateHelper(TList* list) { return new AngularCorrelationHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

