#ifndef TimeWalkHelper_h
#define TimeWalkHelper_h

// Header file for the classes stored in the TTree if any.
#include "TGriffin.h"
#include "TSceptar.h"
#include "TGRSIHelper.h"

class TimeWalkHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<TimeWalkHelper> {
public:
   explicit TimeWalkHelper(TList* list) : TGRSIHelper(list)
   {
      Prefix("TimeWalk");   // Changes prefix of output file
      Setup();
   }
   // These functions are expected to exist
   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      return d->Book<TGriffin, TSceptar>(std::move(*this), {"TGriffin", "TSceptar"});
   }
   void CreateHistograms(unsigned int slot) override;
   void Exec(unsigned int slot, TGriffin& grif, TSceptar& scep);
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" TimeWalkHelper* CreateHelper(TList* list) { return new TimeWalkHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

#endif
