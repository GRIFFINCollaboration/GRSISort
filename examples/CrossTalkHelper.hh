#ifndef CrossTalk_h
#define CrossTalk_h

// Header file for the classes stored in the TTree if any.
#include "TGRSIHelper.h"
#include "TGriffin.h"
#include "TGriffinBgo.h"

class CrossTalkHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<CrossTalkHelper> {
public:
	CrossTalkHelper(TList* list) : TGRSIHelper(list) {
		Prefix("Crosstalk");
		Setup();
	}
	ROOT::RDF::RResultPtr<TList> Book(ROOT::RDataFrame* d) override {
      return d->Book<TGriffin, TGriffinBgo>(std::move(*this), {"TGriffin", "TGriffinBgo"});
	}
	void          CreateHistograms(unsigned int slot);
	void          Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo);
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" CrossTalkHelper* CreateHelper(TList* list) { return new CrossTalkHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

#endif
