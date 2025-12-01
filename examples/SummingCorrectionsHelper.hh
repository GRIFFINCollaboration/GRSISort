#ifndef SUMMINGCORRECTIONSHELPER_HH
#define SUMMINGCORRECTIONSHELPER_HH

#include "TGRSIHelper.h"

#include "TGriffin.h"
#include "TGriffinBgo.h"

// This is a custom action which respects a well defined interface. It supports parallelism,
// in the sense that it behaves correctly if implicit multi threading is enabled.
// Note the plural: in presence of a MT execution, internally more than a single TList is created.
// The detector types in the specifcation of Book must match those in the call to it as well as those in the Exec function (and be in the same order)!

class SummingCorrectionsHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<SummingCorrectionsHelper> {
public:
   // constructor sets the prefix (which is used for the output file as well)
   // and calls Setup which in turn also calls CreateHistograms
   explicit SummingCorrectionsHelper(TList* list)
      : TGRSIHelper(list)
   {
      if(fUserSettings != nullptr) {
         fKValueGriffin = fUserSettings->GetInt("KValue.Griffin", fKValueGriffin);
         try {
            fPileUpRejection = fUserSettings->GetBool("PileUpRejection", true);
         } catch(std::exception&) {}
         fMinimumGriffinEnergy = fUserSettings->GetDouble("MinimumGriffinEnergy", fMinimumGriffinEnergy);
         fMaxCfd               = fUserSettings->GetDouble("MaximumCfd", fMaxCfd);
         fEnergyBins1D         = fUserSettings->GetInt("Energy.Bins.1D", fEnergyBins1D);
         fEnergyBins2D         = fUserSettings->GetInt("Energy.Bins.2D", fEnergyBins2D);
         fLowEnergy            = fUserSettings->GetDouble("Energy.Low", fLowEnergy);
         fHighEnergy           = fUserSettings->GetDouble("Energy.High", fHighEnergy);
         fPromptLow            = fUserSettings->GetDouble("Timing.GriffinGriffin.Prompt.Low", fPromptLow);
         fPromptHigh           = fUserSettings->GetDouble("Timing.GriffinGriffin.Prompt.High", fPromptHigh);
         fCoincidentLow        = fUserSettings->GetDouble("Timing.GriffinGriffin.Coincident.Low", fCoincidentLow);
         fCoincidentHigh       = fUserSettings->GetDouble("Timing.GriffinGriffin.Coincident.High", fCoincidentHigh);
         fTimeRandomLow        = fUserSettings->GetDouble("Timing.GriffinGriffin.TimeRandom.Low", fTimeRandomLow);
         fTimeRandomHigh       = fUserSettings->GetDouble("Timing.GriffinGriffin.TimeRandom.High", fTimeRandomHigh);
         fTimeRandomRatio      = fUserSettings->GetDouble("Timing.GriffinGriffin.TimeRandom.Ratio", fTimeRandomRatio);
         fEnergyGateLow        = fUserSettings->GetDouble("Energy.Gate.Low", fEnergyGateLow);
         fEnergyGateHigh       = fUserSettings->GetDouble("Energy.Gate.High", fEnergyGateHigh);
         for(int i = 0;; ++i) {
            double tmpLow  = 0.;
            double tmpHigh = 0.;
            try {
               tmpLow  = fUserSettings->GetDouble(Form("Energy.Background.%d.Low", i), true);
               tmpHigh = fUserSettings->GetDouble(Form("Energy.Background.%d.High", i), true);
            } catch(std::out_of_range& e) {
               break;
            }
            fEnergyBackground.emplace_back(tmpLow, tmpHigh);
         }
         fEnergyRatio = fUserSettings->GetDouble("Energy.Gate.Ratio", fEnergyRatio);
      }
      // if we have a time random gate and no ratio, calculate it
      if(fTimeRandomRatio == 0. && fTimeRandomLow < fTimeRandomHigh) {
         fTimeRandomRatio = (fCoincidentHigh - fCoincidentLow) / (fTimeRandomHigh - fTimeRandomLow);
      }
      // if we have a energy background gate(s) and no ratio, calculate it
      if(fEnergyRatio == 0. && !fEnergyBackground.empty()) {
         double tmpBg = 0.;
         for(const auto& iter : fEnergyBackground) {
            tmpBg += iter.second - iter.first;
         }
         if(tmpBg != 0.) {
            fEnergyRatio = (fEnergyGateHigh - fEnergyGateLow) / tmpBg;
         }
      }
      Prefix("SummingCorrectionsHelper");
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
   void EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list) override {}

private:
   // any constants that are set in the CreateHistograms function and used in the Exec function can be stored here
   // or any other settings
   // some variables to easily change range and binning for multiple histograms at once
   int    fEnergyBins1D{10000};
   int    fEnergyBins2D{2000};
   double fLowEnergy{0.};
   double fHighEnergy{2000.};

   // Pile-up Rejection - default k-values for not piled-up hits
   int    fKValueGriffin{379};
   bool   fPileUpRejection{true};      // If true, pile-up is rejected
   double fMinimumGriffinEnergy{0.};   // reject hits below this energy
   double fMaxCfd{20.};

   // Coincidences Gates
   double fPromptLow{-250.};
   double fPromptHigh{250.};

   // settings for branching ratio summing corrections
   // timing
   double fCoincidentLow{-250.};
   double fCoincidentHigh{250.};
   double fTimeRandomLow{500.};
   double fTimeRandomHigh{1500.};
   double fTimeRandomRatio{0.};
   // energies
   double                                 fEnergyGateLow{0.};
   double                                 fEnergyGateHigh{0.};
   std::vector<std::pair<double, double>> fEnergyBackground;
   double                                 fEnergyRatio{0.};

   // ==========  functions that fill histograms  ==========
   void FillEfficiencyHistograms(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo);
   void FillBranchingRatioHistograms(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo);

   // ==========  functions for timing conditions  ==========
   // Griffin-Griffin
   // Prompt is applied to the 180 degree hits
   bool Prompt(TGriffinHit* h1, TGriffinHit* h2) const
   {
      return fPromptLow < h1->GetTime() - h2->GetTime() && h1->GetTime() - h2->GetTime() < fPromptHigh;
   }
   // Coincident and TimeRandom are used for a gate on a third gamma ray if an energy gate has been provided
   bool Coincident(TGriffinHit* h1, TGriffinHit* h2) const
   {
      return fCoincidentLow < h1->GetTime() - h2->GetTime() && h1->GetTime() - h2->GetTime() < fCoincidentHigh;
   }
   bool TimeRandom(TGriffinHit* h1, TGriffinHit* h2) const
   {
      return fTimeRandomLow < std::abs(h1->GetTime() - h2->GetTime()) && std::abs(h1->GetTime() - h2->GetTime()) < fTimeRandomHigh;
   }

   bool Gate(TGriffinHit* h1) const
   {
      return fEnergyGateLow < h1->GetEnergy() && h1->GetEnergy() < fEnergyGateHigh;
   }
   bool Background(TGriffinHit* h1) const
   {
      return std::any_of(fEnergyBackground.begin(), fEnergyBackground.end(), [h1](std::pair<double, double> iter) { return iter.first < h1->GetEnergy() && h1->GetEnergy() < iter.second; });
   }

   // general check for good CFD
   bool GoodCfd(TDetectorHit* h1) const
   {
      return h1->GetTime() - static_cast<double>(h1->GetTimeStampNs()) < fMaxCfd;
   }

   // ==========  functions for pileup rejection, and minimum energy  ==========
   bool Reject(TGriffinHit* hit) const
   {
      return ((fPileUpRejection && hit->GetKValue() != fKValueGriffin) || hit->GetEnergy() < fMinimumGriffinEnergy);
   }
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" SummingCorrectionsHelper* CreateHelper(TList* list) { return new SummingCorrectionsHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }

#endif
