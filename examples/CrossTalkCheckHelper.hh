#ifndef CROSSTALKCHECKHELPER_HH
#define CROSSTALKCHECKHELPER_HH

#include "TGRSIHelper.h"
#include "TGRSIOptions.h"

#include "TGriffin.h"
#include "TGriffinBgo.h"

// This is a custom action which respects a well defined interface. It supports parallelism,
// in the sense that it behaves correctly if implicit multi threading is enabled.
// Note the plural: in presence of a MT execution, internally more than a single TList is created.
// The detector types in the specifcation of Book must match those in the call to it as well as those in the Exec function (and be in the same order)!

class CrossTalkCheckHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<CrossTalkCheckHelper> {
public:
   // constructor sets the prefix (which is used for the output file as well)
   // and calls Setup which in turn also calls CreateHistograms
   explicit CrossTalkCheckHelper(TList* list)
      : TGRSIHelper(list)
   {
      if(fUserSettings != nullptr) {
         fEnergyBins = fUserSettings->GetInt("Energy.Bins", fEnergyBins);
         fLowEnergy  = fUserSettings->GetDouble("Energy.Low", fLowEnergy);
         fHighEnergy = fUserSettings->GetDouble("Energy.High", fHighEnergy);
      }
      Prefix("CrossTalkCheck");
      Setup();
   }

   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      std::cout << "Using options:" << std::endl;
      TGRSIOptions::Get()->Print();
      if(TGRSIOptions::Get()->NumberOfEvents() > 0) {
         return d->Range(TGRSIOptions::Get()->NumberOfEvents()).Book<TGriffin, TGriffinBgo>(std::move(*this), {"TGriffin", "TGriffinBgo"});
      }
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
   int    fEnergyBins{10000};
   double fLowEnergy{0.};
   double fHighEnergy{2000.};
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" CrossTalkCheckHelper* CreateHelper(TList* list) { return new CrossTalkCheckHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

#endif
