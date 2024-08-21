#ifndef ComptonPolarimetryHelper_h
#define ComptonPolarimetryHelper_h

#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TGriffinAngles.h"
#include "TGRSIHelper.h"

class ComptonPolarimetryHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<ComptonPolarimetryHelper> {
private:
   double           fGriffinDistance{145.};
   std::vector<int> fExcludedDetectors;
   std::vector<int> fExcludedCrystals;

   bool   fUseTimestamps{false};   // Whether to use timestamps of CFD corrected time for prompt gate
   double fPrompt{200.};           // Maximum absolute time difference for prompt gamma-gamma

   int    fBins{3000};
   double fMinEnergy{0.};
   double fMaxEnergy{3000.};
   int    fXiBins{180};
   int    fThetaBins{180};

   // not using vectors here because we need to ensure anyway that there are exactly two gates
   std::array<double, 2> fGammaGateLow  = {1170., 1329.};
   std::array<double, 2> fGammaGateHigh = {1176., 1335.};

   std::map<unsigned int, std::deque<TGriffin*>> fGriffinDeque;

   bool ExcludeDetector(int detector) const
   {
      return std::binary_search(fExcludedDetectors.begin(), fExcludedDetectors.end(), detector);
   }
   bool ExcludeCrystal(int arraynumber) const
   {
      return std::binary_search(fExcludedCrystals.begin(), fExcludedCrystals.end(), arraynumber);
   }

   double TimeDiff(TGriffinHit* grif1, TGriffinHit* grif2) const
   {
      if(fUseTimestamps) {
         return TMath::Abs(grif1->GetTimeStampNs() - grif2->GetTimeStampNs());
      } else {
         return TMath::Abs(grif1->GetTime() - grif2->GetTime());
      }
   }

   bool Coincident(TGriffinHit* grif1, TGriffinHit* grif2) const
   {
      if(fUseTimestamps) {
         return TMath::Abs(grif1->GetTimeStampNs() - grif2->GetTimeStampNs()) < fPrompt;
      } else {
         return TMath::Abs(grif1->GetTime() - grif2->GetTime()) < fPrompt;
      }
   }

   int CheckEnergy(double energy, int index = -1) const
   {
      // if index is -1 return index of gamma gate this energy falls into or return -1
      if(index < 0 || index > 1) {
         for(index = 0; index < 2; ++index) {
            if(fGammaGateLow[index] < energy && energy < fGammaGateHigh[index]) { return index; }
         }
         // we only reach here if the energy doesn't fall in either gate
         return -1;
      }
      // else check if energy falls in gamma gate indicated by the index (or return -1)
      if(fGammaGateLow[index] < energy && energy < fGammaGateHigh[index]) { return index; }
      return -1;
   }

public:
   explicit ComptonPolarimetryHelper(TList* list)
      : TGRSIHelper(list)
   {
      Prefix("ComptonPolarimetry");

      if(fUserSettings != nullptr) {
         fGriffinDistance   = fUserSettings->GetDouble("GriffinDistance", 145.);
         fExcludedDetectors = fUserSettings->GetIntVector("ExcludedDetector", true);   // be quiet if we don't find this
         fExcludedCrystals  = fUserSettings->GetIntVector("ExcludedCrystal", true);    // be quiet if we don't find this

         fUseTimestamps = fUserSettings->GetBool("UseTimestamps", false);
         fPrompt        = fUserSettings->GetDouble("MaxPromptTime", 200.);

         fBins             = fUserSettings->GetInt("NumberOfBins", 3000);
         fMinEnergy        = fUserSettings->GetDouble("MinimumEnergy", 0.);
         fMaxEnergy        = fUserSettings->GetDouble("MaximumEnergy", 3000.);
         fXiBins           = fUserSettings->GetInt("NumberOfXiBins", 181);
         fThetaBins        = fUserSettings->GetInt("NumberOfThetaBins", 181);
         fGammaGateLow[0]  = fUserSettings->GetDouble("GammaGate.0.Low", 1170.);
         fGammaGateHigh[0] = fUserSettings->GetDouble("GammaGate.0.High", 1176.);
         fGammaGateLow[1]  = fUserSettings->GetDouble("GammaGate.1.Low", 1329.);
         fGammaGateHigh[1] = fUserSettings->GetDouble("GammaGate.1.High", 1335.);
      } else {
         std::cout << "No user settings provided, using default settings: ";
      }
      std::cout << std::boolalpha << "distance " << fGriffinDistance << " mm, using " << (fUseTimestamps ? "timestamps" : "CFD corrected time") << ", gamma gate " << fGammaGateLow[0] << " - " << fGammaGateHigh[0] << " and " << fGammaGateLow[1] << " - " << fGammaGateHigh[1] << std::endl;

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

extern "C" ComptonPolarimetryHelper* CreateHelper(TList* list) { return new ComptonPolarimetryHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }
