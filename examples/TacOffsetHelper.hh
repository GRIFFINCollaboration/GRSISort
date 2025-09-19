#ifndef TACOFFSETHELPER_HH
#define TACOFFSETHELPER_HH

#include "TGRSIHelper.h"

#include "TGriffin.h"
#include "TLaBr.h"
#include "TTAC.h"

// This is a custom action which respects a well defined interface. It supports parallelism,
// in the sense that it behaves correctly if implicit multi threading is enabled.
// Note the plural: in presence of a MT execution, internally more than a single TList is created.
// The detector types in the specifcation of Book must match those in the call to it as well as those in the Exec function (and be in the same order)!

class TacOffsetHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<TacOffsetHelper> {
public:
   // constructor sets the prefix (which is used for the output file as well)
   // and calls Setup which in turn also calls CreateHistograms
   explicit TacOffsetHelper(TList* list)
      : TGRSIHelper(list)
   {
      Prefix("TacOffsetHelper");
      Setup();
   }

   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      return d->Book<TGriffin, TTAC, TLaBr>(std::move(*this), {"TGriffin", "TTAC", "TLaBr"});
   }
   // this function creates and books all histograms
   void CreateHistograms(unsigned int slot) override;
   // this function gets called for every single event and fills the histograms
   void Exec(unsigned int slot, TGriffin& grif, TTAC& tac, TLaBr& labr);
   // this function is optional and is called after the output lists off all slots/workers have been merged
   void EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list) override;

private:
   // any constants that are set in the CreateHistograms function and used in the Exec function can be stored here
   // or any other settings
   std::array<double, 8> fOffset;
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" TacOffsetHelper* CreateHelper(TList* list) { return new TacOffsetHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

#endif
