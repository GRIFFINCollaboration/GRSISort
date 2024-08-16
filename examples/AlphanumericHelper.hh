//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Oct 25 13:18:27 2016 by ROOT version 5.34/24
// from TTree FragmentTree/FragmentTree
// found on file: fragment07844_000.root
//////////////////////////////////////////////////////////

#ifndef AlphanumericHelper_h
#define AlphanumericHelper_h

#include "TGRSIHelper.h"
#include "TGriffin.h"

class AlphanumericHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<AlphanumericHelper> {

public:
   explicit AlphanumericHelper(TList* list)
      : TGRSIHelper(list)
   {
      Prefix("Alphanumeric");   // Changes prefix of output file
      Setup();
   }
   // These functions are expected to exist
   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      return d->Book<TGriffin>(std::move(*this), {"TGriffin"});
   }
   void CreateHistograms(unsigned int slot) override;
   void Exec(unsigned int slot, TGriffin& grif);
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" AlphanumericHelper* CreateHelper(TList* list) { return new AlphanumericHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

#endif
