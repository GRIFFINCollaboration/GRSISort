#ifndef GriffinKValueHelper_h
#define GriffinKValueHelper_h

// Header file for the classes stored in the TTree if any.
#include "TFragment.h"
#include "TGRSIHelper.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class GriffinKValueHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<GriffinKValueHelper> {
public:
   explicit GriffinKValueHelper(TList* list)
		: TGRSIHelper(list)
   {
      Prefix("GriffinKValue");
      Setup();
   }
   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      return d->Book<TFragment>(std::move(*this), {"TFragment"});
   }
   void CreateHistograms(unsigned int slot) override;
   void Exec(unsigned int sloti, TFragment& frag);
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" GriffinKValueHelper* CreateHelper(TList* list) { return new GriffinKValueHelper(list); } // NOLINT(misc-definitions-in-headers)

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; } // NOLINT(misc-definitions-in-headers)

#endif
