#ifndef AngularCorrelationHelper_h
#define AngularCorrelationHelper_h

#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TGriffinAngles.h"
#include "TGRSIHelper.h"

class AngularCorrelationHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<AngularCorrelationHelper> {
private:
	double fGriffinDistance{145.};
	bool fFolding{false};
	bool fGrouping{false};
	bool fAddback{true};

	TGriffinAngles* fAngles{nullptr};

	std::map<unsigned int, std::deque<TGriffin*>> fGriffinDeque;
	std::map<unsigned int, std::deque<TGriffinBgo*>> fBgoDeque;

public :
	AngularCorrelationHelper(TList* list) : TGRSIHelper(list) {
		Prefix("AngularCorrelation");

		if(fUserSettings != nullptr) {
			fGriffinDistance = fUserSettings->GetDouble("GriffinDistance");
			fAddback = fUserSettings->GetBool("Addback");
			fFolding = fUserSettings->GetBool("Folding");
			fGrouping = fUserSettings->GetBool("Grouping");
		} else {
			std::cout<<"No user settings provided, using default settings: ";
		}
		std::cout<<std::boolalpha<<"distance "<<fGriffinDistance<<" mm, addback "<<fAddback<<", folding "<<fFolding<<", and grouping "<<fGrouping<<std::endl;

		fAngles = new TGriffinAngles(fGriffinDistance, fFolding, fGrouping, fAddback);
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

