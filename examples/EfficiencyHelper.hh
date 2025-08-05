#ifndef EFFICIENCYHELPER_HH
#define EFFICIENCYHELPER_HH

#include "TGRSIHelper.h"

#include "TGriffin.h"
#include "TGriffinBgo.h"

// This is a custom action which respects a well defined interface. It supports parallelism,
// in the sense that it behaves correctly if implicit multi threading is enabled.
// Note the plural: in presence of a MT execution, internally more than a single TList is created.
// The detector types in the specifcation of Book must match those in the call to it as well as those in the Exec function (and be in the same order)!

class EfficiencyHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<EfficiencyHelper> {
public:
   // constructor sets the prefix (which is used for the output file as well)
   // and calls Setup which in turn also calls CreateHistograms
   explicit EfficiencyHelper(TList* list)
      : TGRSIHelper(list)
   {
		if(fUserSettings != nullptr) {
			fKValueGriffin = fUserSettings->GetInt("KValue.Griffin", fKValueGriffin);
			try {
				fPileUpRejection = fUserSettings->GetBool("PileUpRejection", true);
			} catch(std::exception&) {}
			fMinimumGriffinEnergy = fUserSettings->GetDouble("MinimumGriffinEnergy", fMinimumGriffinEnergy);
			fGgLow = fUserSettings->GetDouble("Timing.GriffinGriffin.Coincident.Low", fGgLow);
			fGgHigh = fUserSettings->GetDouble("Timing.GriffinGriffin.Coincident.High", fGgHigh);
			fGgBgLow = fUserSettings->GetDouble("Timing.GriffinGriffin.TimeRandom.Low", fGgBgLow);
			fGgBgHigh = fUserSettings->GetDouble("Timing.GriffinGriffin.TimeRandom.High", fGgBgHigh);
			fMaxCfd = fUserSettings->GetDouble("MaximumCfd", fMaxCfd);
		}
      Prefix("EfficiencyHelper");
      Setup();
   }

   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      return d->Book<TGriffin, TGriffinBgo>(std::move(*this), {"TGriffin", "TGriffinBgo"});
   }
   // this function creates and books all histograms
   void CreateHistograms(unsigned int slot) override;
   // this function gets called for every single event and fills the histograms
   void Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo);
   // this function is optional and is called after the output lists off all slots/workers have been merged
   void EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list) override;

private:
   // any constants that are set in the CreateHistograms function and used in the Exec function can be stored here
   // or any other settings
   Long64_t fCycleLength{0};
	// Pile-up Rejection - default k-values for not piled-up hits
	int fKValueGriffin{379};
	bool fPileUpRejection{true}; // If true, pile-up is rejected
	double fMinimumGriffinEnergy{0.}; // reject hits below this energy

	// Coincidences Gates
	double fGgLow{-250.};
	double fGgHigh{250.};
	double fGgBgLow{500.};
	double fGgBgHigh{1500.};
	double fMaxCfd{20.};

	// ==========  functions for timing conditions  ==========
	// Griffin-Griffin
	bool PromptCoincidence(TGriffinHit* h1, TGriffinHit* h2) {
		return fGgLow < h1->GetTime() - h2->GetTime() && h1->GetTime() - h2->GetTime() < fGgHigh; 
	}
	bool TimeRandom(TGriffinHit* h1, TGriffinHit* h2) {
		return fGgBgLow < std::fabs(h1->GetTime() - h2->GetTime()) && std::fabs(h1->GetTime() - h2->GetTime()) < fGgBgHigh;
	}

	// general check for good CFD
	bool GoodCfd(TDetectorHit* h1) {
		return h1->GetTime() - h1->GetTimeStampNs() < fMaxCfd;
	}

	// ==========  functions for pileup rejection, and minimum energy  ==========
	bool Reject(TGriffinHit* hit) {
		return ((fPileUpRejection && hit->GetKValue() != fKValueGriffin) || hit->GetEnergy() < fMinimumGriffinEnergy);
	}
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" EfficiencyHelper* CreateHelper(TList* list) { return new EfficiencyHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

#endif
