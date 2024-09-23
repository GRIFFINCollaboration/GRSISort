#ifndef ExampleFragmentHelper_h
#define ExampleFragmentHelper_h

// Header file for the classes stored in the TTree if any.
#include "TFragment.h"
#include "TGRSIHelper.h"

class ExampleFragmentHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<ExampleFragmentHelper> {
public:
   explicit ExampleFragmentHelper(TList* list)
      : TGRSIHelper(list)
   {
      Prefix("ExampleFragment");
      Setup();
   }
   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      return d->Book<TFragment>(std::move(*this), {"TFragment"});
   }

   void CreateHistograms(unsigned int slot) override;
   void Exec(unsigned int slot, TFragment& frag);
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" ExampleFragmentHelper* CreateHelper(TList* list) { return new ExampleFragmentHelper(list); }   // NOLINT(misc-definitions-in-headers)

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }   // NOLINT(misc-definitions-in-headers)

#endif
