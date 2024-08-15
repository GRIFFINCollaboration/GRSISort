#include "EfficiencyHelper.hh"

// Coincidences Gates
double promptLow  = -250.;
double promptHigh = 250.;
bool   PromptCoincidence(TGriffinHit* h1, TGriffinHit* h2)
{   // Griffin-Griffin
   return promptLow <= h2->GetTime() - h1->GetTime() && h2->GetTime() - h1->GetTime() <= promptHigh;
}
double bgLow  = 500.;
double bgHigh = 1500.;
bool   TimeRandom(TGriffinHit* h1, TGriffinHit* h2)
{
   return (bgLow <= std::fabs(h1->GetTime() - h2->GetTime()) && std::fabs(h1->GetTime() - h2->GetTime()) <= bgHigh);
}

void EfficiencyHelper::CreateHistograms(unsigned int slot)
{
   // some variables to easily change range and binning for multiple histograms at once
   int    energyBins   = 10000;
   int    energyBins2D = 2000;
   double lowEnergy    = 0.;
   double highEnergy   = 2000.;

   // five histograms for each type: 1D singles, coincident and time random 2D matrix (unsuppressed on y-axis for suppressed data)
   // and coincident and time random E_1 vs E_1+E_2, all 2D histograms are at 180 degree

   // unsuppressed spectra
   H1()[slot]["griffinE"]                   = new TH1F("griffinE", Form("Unsuppressed griffin energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinE180Coinc"]    = new TH2F("griffinGriffinE180Coinc", "Unsuppressed griffin-griffin energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinE180Bg"]       = new TH2F("griffinGriffinE180Bg", "Unsuppressed griffin-griffin energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinESum180Coinc"] = new TH2F("griffinGriffinESum180Coinc", "Unsuppressed griffin energy vs griffin sum energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinESum180Bg"]    = new TH2F("griffinGriffinESum180Bg", "Unsuppressed griffin energy vs griffin sum energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   // suppressed spectra
   H1()[slot]["griffinESupp"]                   = new TH1F("griffinESupp", Form("Suppressed griffin energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinESuppSum180Coinc"] = new TH2F("griffinGriffinESuppSum180Coinc", "Suppressed griffin energy vs griffin sum energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinESuppSum180Bg"]    = new TH2F("griffinGriffinESuppSum180Bg", "Suppressed griffin energy vs griffin sum energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinEMixed180Coinc"]   = new TH2F("griffinGriffinEMixed180Coinc", "Unsuppressed/suppressed griffin-griffin energy @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinEMixed180Bg"]      = new TH2F("griffinGriffinEMixed180Bg", "Unsuppressed/suppressed griffin-griffin energy @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   // unsuppressed addback spectra
   H1()[slot]["griffinEAddback"]                   = new TH1F("griffinEAddback", Form("Unsuppressed griffin addback energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinEAddback180Coinc"]    = new TH2F("griffinGriffinEAddback180Coinc", "Unsuppressed griffin-griffin addback @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinEAddback180Bg"]       = new TH2F("griffinGriffinEAddback180Bg", "Unsuppressed griffin-griffin addback @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinEAddbackSum180Coinc"] = new TH2F("griffinGriffinEAddbackSum180Coinc", "Unsuppressed griffin addback vs griffin sum energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinEAddbackSum180Bg"]    = new TH2F("griffinGriffinEAddbackSum180Bg", "Unsuppressed griffin addback vs griffin sum energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   // suppressed addback spectra
   H1()[slot]["griffinESuppAddback"]                   = new TH1F("griffinESuppAddback", Form("Suppressed griffin addback energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinESuppAddbackSum180Coinc"] = new TH2F("griffinGriffinESuppAddbackSum180Coinc", "Suppressed griffin addback vs sum energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinESuppAddbackSum180Bg"]    = new TH2F("griffinGriffinESuppAddbackSum180Bg", "Suppressed griffin addback vs sum energy @ 180^{o};energy [keV];energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinEMixedAddback180Coinc"]   = new TH2F("griffinGriffinEMixedAddback180Coinc", "Unsuppressed/suppressed griffin-griffin addback @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
   H2()[slot]["griffinGriffinEMixedAddback180Bg"]      = new TH2F("griffinGriffinEMixedAddback180Bg", "Unsuppressed/suppressed griffin-griffin addback @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", energyBins2D, lowEnergy, highEnergy, energyBins2D, lowEnergy, highEnergy);
}

void EfficiencyHelper::Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo)
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

   // loop over unsuppressed griffin hits
   for(int g = 0; g < grif.GetMultiplicity(); ++g) {
      auto grif1 = grif.GetGriffinHit(g);
      H1()[slot].at("griffinE")->Fill(grif1->GetEnergy());
      for(int g2 = g + 1; g2 < grif.GetMultiplicity(); ++g2) {
         auto grif2 = grif.GetGriffinHit(g2);
         if(grif1->GetPosition().Angle(grif2->GetPosition()) * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               H2()[slot].at("griffinGriffinE180Coinc")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               H2()[slot].at("griffinGriffinE180Coinc")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
               H2()[slot].at("griffinGriffinESum180Coinc")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            } else if(TimeRandom(grif1, grif2)) {
               H2()[slot].at("griffinGriffinE180Bg")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               H2()[slot].at("griffinGriffinE180Bg")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
               H2()[slot].at("griffinGriffinESum180Bg")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
   }

   // loop over suppressed griffin hits
   for(int g = 0; g < grif.GetSuppressedMultiplicity(&grifBgo); ++g) {
      auto grif1 = grif.GetSuppressedHit(g);
      H1()[slot].at("griffinESupp")->Fill(grif1->GetEnergy());
      for(int g2 = g + 1; g2 < grif.GetSuppressedMultiplicity(&grifBgo); ++g2) {
         auto grif2 = grif.GetSuppressedHit(g2);
         if(grif1->GetPosition().Angle(grif2->GetPosition()) * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               H2()[slot].at("griffinGriffinESuppSum180Coinc")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            } else if(TimeRandom(grif1, grif2)) {
               H2()[slot].at("griffinGriffinESuppSum180Bg")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
      for(int g2 = 0; g2 < grif.GetMultiplicity(); ++g2) {
         if(g == g2) continue;
         auto grif2 = grif.GetGriffinHit(g2);
         if(grif1->GetPosition().Angle(grif2->GetPosition()) * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               H2()[slot].at("griffinGriffinEMixed180Coinc")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            } else if(TimeRandom(grif1, grif2)) {
               H2()[slot].at("griffinGriffinEMixed180Bg")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            }
         }
      }
   }

   // loop over unsuppressed griffin addback hits
   for(int g = 0; g < grif.GetAddbackMultiplicity(); ++g) {
      auto grif1 = grif.GetAddbackHit(g);
      H1()[slot].at("griffinEAddback")->Fill(grif1->GetEnergy());
      for(int g2 = g + 1; g2 < grif.GetAddbackMultiplicity(); ++g2) {
         auto grif2 = grif.GetAddbackHit(g2);
         if(grif1->GetPosition().Angle(grif2->GetPosition()) * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               H2()[slot].at("griffinGriffinEAddback180Coinc")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               H2()[slot].at("griffinGriffinEAddback180Coinc")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
               H2()[slot].at("griffinGriffinEAddbackSum180Coinc")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            } else if(TimeRandom(grif1, grif2)) {
               H2()[slot].at("griffinGriffinEAddback180Bg")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               H2()[slot].at("griffinGriffinEAddback180Bg")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
               H2()[slot].at("griffinGriffinEAddbackSum180Bg")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
   }

   // loop over suppressed griffin addback hits
   for(int g = 0; g < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g) {
      auto grif1 = grif.GetSuppressedAddbackHit(g);
      H1()[slot].at("griffinESuppAddback")->Fill(grif1->GetEnergy());
      for(int g2 = g + 1; g2 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g2) {
         auto grif2 = grif.GetSuppressedAddbackHit(g2);
         if(grif1->GetPosition().Angle(grif2->GetPosition()) * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               H2()[slot].at("griffinGriffinESuppAddbackSum180Coinc")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            } else if(TimeRandom(grif1, grif2)) {
               H2()[slot].at("griffinGriffinESuppAddbackSum180Bg")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
      for(int g2 = 0; g2 < grif.GetAddbackMultiplicity(); ++g2) {
         if(g == g2) continue;
         auto grif2 = grif.GetAddbackHit(g2);
         if(grif1->GetPosition().Angle(grif2->GetPosition()) * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               H2()[slot].at("griffinGriffinEMixedAddback180Coinc")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            } else if(TimeRandom(grif1, grif2)) {
               H2()[slot].at("griffinGriffinEMixedAddback180Bg")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            }
         }
      }
   }
}

void EfficiencyHelper::EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list)
{
   std::cout << std::endl;
   for(auto obj : list->at("")) {
      std::string name  = obj->GetName();
      size_t      coinc = name.rfind("Coinc");
      if(coinc == name.length() - 5) {
         auto timeRandom = static_cast<TH2*>(list->at("").FindObject(name.replace(coinc, 5, "Bg").c_str()));
         if(timeRandom == nullptr) {
            std::cout << "Failed to find \"" << name.replace(coinc, 5, "Bg") << "\" after finding \"" << name << "\" in list:" << std::endl;
            list->at("").Print();
            continue;
         }
         double factor = (promptHigh - promptLow) / (2. * (bgHigh - bgLow));   // factor two for BG window because we use absolute time here, might be wrong for asymmetric histograms?
         std::cout << "Found \"" << timeRandom->GetName() << "\" after finding \"" << obj->GetName() << "\", creating corrected spectrum using factor " << factor << std::endl;
         auto corrected = static_cast<TH2*>(obj->Clone(name.replace(coinc, 5, "Corr").c_str()));
         corrected->Add(timeRandom, -factor);
         list->at("").Add(corrected);
      }
   }
}
