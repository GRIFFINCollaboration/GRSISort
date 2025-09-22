#ifndef TESTHELPER_HH
#define TESTHELPER_HH

#include "TGRSIHelper.h"

#include "TGriffin.h"
#include "TGriffinBgo.h"

// This is a custom action which respects a well defined interface. It supports parallelism,
// in the sense that it behaves correctly if implicit multi threading is enabled.
// Note the plural: in presence of a MT execution, internally more than a single TList is created.
// The detector types in the specifcation of Book must match those in the call to it as well as those in the Exec function (and be in the same order)!

class DirectoryHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<DirectoryHelper> {
public:
   // constructor sets the prefix (which is used for the output file as well)
   // and calls Setup which in turn also calls CreateHistograms
   explicit DirectoryHelper(TList* list)
      : TGRSIHelper(list)
   {
      if(fUserSettings != nullptr) {
         fEnergyBins = fUserSettings->GetInt("LaBr3.Threshold", fEnergyBins);
         fEnergyLow  = fUserSettings->GetDouble("LaBr3.Threshold", fEnergyLow);
         fEnergyHigh = fUserSettings->GetDouble("LaBr3.Threshold", fEnergyHigh);
      }
      Prefix("DirectoryHelper");
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
   std::map<uint16_t, std::vector<int64_t>> fLastTS;
   std::map<uint16_t, std::vector<int64_t>> fLastSuppressedTS;
   std::map<uint16_t, std::vector<double>>  fLastTime;
   std::map<uint16_t, std::vector<double>>  fLastSuppressedTime;
   std::map<uint16_t, std::vector<int64_t>> fLastTSNoPileup;
   std::map<uint16_t, std::vector<int64_t>> fLastSuppressedTSNoPileup;
   std::map<uint16_t, std::vector<double>>  fLastTimeNoPileup;
   std::map<uint16_t, std::vector<double>>  fLastSuppressedTimeNoPileup;

   // some variables to easily change range and binning for multiple histograms at once
   int    fEnergyBins = 10000;
   double fEnergyLow  = 0.;
   double fEnergyHigh = 2000.;
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" DirectoryHelper* CreateHelper(TList* list) { return new DirectoryHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

#endif
