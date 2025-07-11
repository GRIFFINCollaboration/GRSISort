#ifndef AngularCorrelationHelper_h
#define AngularCorrelationHelper_h

#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TGriffinAngles.h"
#include "TGRSIHelper.h"

class AngularCorrelationHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<AngularCorrelationHelper> {
private:
   int              fNofMixedEvents{10};
   double           fGriffinDistance{145.};
   bool             fFolding{false};
   bool             fGrouping{false};
   bool             fAddback{true};
   bool             fSingleCrystal{false};
   std::vector<int> fExcludedDetectors;
   std::vector<int> fExcludedCrystals;

   double fPrompt{200.};           // Maximum absolute time difference for prompt gamma-gamma
   double fTimeRandomLow{400.};    // Minimum time difference for gamma-gamma time random background
   double fTimeRandomHigh{600.};   // Maximum time difference for gamma-gamma time random background

   Long64_t fCycleLength{0};
   double   fCycleTimeLow{-1.};    // Minimum time in the cycle in s (if both are -1 we ignore it)
   double   fCycleTimeHigh{-1.};   // Maximum time in the cycle in s (if both are -1 we ignore it)

   int    fBins{3000};
   double fMinEnergy{0.};
   double fMaxEnergy{3000.};

   TGriffinAngles* fAngles{nullptr};

   std::map<unsigned int, std::deque<TGriffin*>>    fGriffinDeque;
   std::map<unsigned int, std::deque<TGriffinBgo*>> fBgoDeque;

   bool ExcludeDetector(int detector)
   {
      return std::binary_search(fExcludedDetectors.begin(), fExcludedDetectors.end(), detector);
   }
   bool ExcludeCrystal(int arraynumber)
   {
      return std::binary_search(fExcludedCrystals.begin(), fExcludedCrystals.end(), arraynumber);
   }

   bool GoodCycleTime(double timeInCycle)
   {
      // check if the timeInCycle (in microseconds!) falls within the limits
      // this could be done much smarter, but this might be easier to read
      // if no limits are set we always are within the limits
      if(fCycleTimeLow == -1. && fCycleTimeHigh == -1.) {
         return true;
      }
      if(fCycleTimeLow == -1. && timeInCycle / 1e6 < fCycleTimeHigh) {
         return true;
      }
      if(fCycleTimeLow < timeInCycle / 1e6 && fCycleTimeHigh == -1.) {
         return true;
      }
      if(fCycleTimeLow < timeInCycle / 1e6 && timeInCycle / 1e6 < fCycleTimeHigh) {
         return true;
      }
      return false;
   }

public:
   explicit AngularCorrelationHelper(TList* list)
      : TGRSIHelper(list)
   {
      Prefix("AngularCorrelation");

      if(fUserSettings != nullptr) {
         fNofMixedEvents  = fUserSettings->GetInt("NumberOfMixedEvents", 10);
         fGriffinDistance = fUserSettings->GetDouble("GriffinDistance", 145.);
         fAddback         = fUserSettings->GetBool("Addback", true);
         fSingleCrystal   = fUserSettings->GetBool("SingleCrystal", true);
         fFolding         = fUserSettings->GetBool("Folding", false);
         fGrouping        = fUserSettings->GetBool("Grouping", false);

         try {
            fExcludedDetectors = fUserSettings->GetIntVector("ExcludedDetector", true);   // be quiet if we don't find this
         } catch(std::out_of_range&) {
            // do nothing, we simply don't have any detectors to exclude
         }
         try {
            fExcludedCrystals = fUserSettings->GetIntVector("ExcludedCrystal", true);   // be quiet if we don't find this
         } catch(std::out_of_range&) {
            // do nothing, we simply don't have any crystals to exclude
         }

         fPrompt         = fUserSettings->GetDouble("MaxPromptTime", 200.);
         fTimeRandomLow  = fUserSettings->GetDouble("TimeRandom.Low", 400.);
         fTimeRandomHigh = fUserSettings->GetDouble("TimeRandom.High", 600.);

         fCycleTimeLow  = fUserSettings->GetDouble("CycleTime.Low", -1.);
         fCycleTimeHigh = fUserSettings->GetDouble("CycleTime.High", -1.);

         fBins      = fUserSettings->GetInt("NumberOfBins", 3000);
         fMinEnergy = fUserSettings->GetDouble("MinimumEnergy", 0.);
         fMaxEnergy = fUserSettings->GetDouble("MaximumEnergy", 3000.);
      } else {
         std::cout << "No user settings provided, using default settings: ";
      }
      if(fSingleCrystal && !fAddback) {
         std::cerr << "Error, can't use single crystal method if not using addback! Setting addback to true!" << std::endl;
         fAddback = true;
      }
      std::cout << std::boolalpha << "# of mixed events " << fNofMixedEvents << ", distance " << fGriffinDistance << " mm, single crystal " << fSingleCrystal << ", addback " << fAddback << ", folding " << fFolding << ", and grouping " << fGrouping << std::endl;

      fAngles = new TGriffinAngles(fGriffinDistance, fFolding, fGrouping, fAddback);
      fAngles->Print();

      // Setup calls CreateHistograms, which uses the stored angle combinations, so we need those set before
      Setup();
   }

   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      return d->Book<TGriffin, TGriffinBgo>(std::move(*this), {"TGriffin", "TGriffinBgo"});
   }

   void CreateHistograms(unsigned int slot) override;
   void Exec(unsigned int slot, TGriffin& fGriffin, TGriffinBgo& fGriffinBgo);
};

#endif

extern "C" AngularCorrelationHelper* CreateHelper(TList* list) { return new AngularCorrelationHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }
