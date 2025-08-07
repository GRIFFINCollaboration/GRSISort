#include "EfficiencyHelper.hh"

void EfficiencyHelper::CreateHistograms(unsigned int slot)
{
   // three histograms for each type: 1D singles, coincident and time random 2D matrix (unsuppressed on y-axis for suppressed data)
   // all 2D histograms are at 180 degree
   // 180 degree means for addback that the detectors are opposite, so the crystal angles are 158 degrees and larger!
   // (for 145 mm distance)

   // unsuppressed spectra
   fH1[slot]["griffinE"]           = new TH1F("griffinE", Form("Unsuppressed griffin energy;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinE"]    = new TH2F("griffinGriffinE", "Unsuppressed griffin-griffin energy @ 180^{o};energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinESum"] = new TH2F("griffinGriffinESum", "Unsuppressed griffin energy vs griffin sum energy @ 180^{o};sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   // suppressed spectra
   fH1[slot]["griffinESupp"]           = new TH1F("griffinESupp", Form("Suppressed griffin energy;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinESuppSum"] = new TH2F("griffinGriffinESuppSum", "Suppressed griffin energy vs griffin sum energy @ 180^{o};sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEMixed"]   = new TH2F("griffinGriffinEMixed", "Unsuppressed/suppressed griffin-griffin energy @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   // unsuppressed addback spectra
   fH1[slot]["griffinEAddback"]           = new TH1F("griffinEAddback", Form("Unsuppressed griffin addback energy;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEAddback"]    = new TH2F("griffinGriffinEAddback", "Unsuppressed griffin-griffin addback @ 180^{o};energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEAddbackSum"] = new TH2F("griffinGriffinEAddbackSum", "Unsuppressed griffin addback vs griffin sum energy @ 180^{o};sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   // suppressed addback spectra
   fH1[slot]["griffinESuppAddback"]           = new TH1F("griffinESuppAddback", Form("Suppressed griffin addback energy;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinESuppAddbackSum"] = new TH2F("griffinGriffinESuppAddbackSum", "Suppressed griffin addback vs sum energy @ 180^{o};sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEMixedAddback"]   = new TH2F("griffinGriffinEMixedAddback", "Unsuppressed/suppressed griffin-griffin addback @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   // suppressed single crystal addback spectra
   fH1[slot]["griffinESingleCrystal"]             = new TH1F("griffinESingleCrystal", Form("Suppressed griffin addback energy single crystal method;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinESingleCrystalSum"]   = new TH2F("griffinGriffinESingleCrystalSum", "Suppressed griffin addback vs sum energy @ 180^{o}, single crystal method;sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEMixedSingleCrystal"] = new TH2F("griffinGriffinEMixedSingleCrystal", "Unsuppressed/suppressed griffin-griffin addback @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
}

void EfficiencyHelper::Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo)
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

   // loop over unsuppressed griffin hits
   for(int g = 0; g < grif.GetMultiplicity(); ++g) {
      auto grif1 = grif.GetGriffinHit(g);
      if(Reject(grif1) || !GoodCfd(grif1)) continue;
      fH1[slot].at("griffinE")->Fill(grif1->GetEnergy());
      for(int g2 = g + 1; g2 < grif.GetMultiplicity(); ++g2) {
         auto grif2 = grif.GetGriffinHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) continue;
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               fH2[slot].at("griffinGriffinE")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at("griffinGriffinE")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
               fH2[slot].at("griffinGriffinESum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
   }

   // loop over suppressed griffin hits
   for(int g = 0; g < grif.GetSuppressedMultiplicity(&grifBgo); ++g) {
      auto grif1 = grif.GetSuppressedHit(g);
      if(Reject(grif1) || !GoodCfd(grif1)) continue;
      fH1[slot].at("griffinESupp")->Fill(grif1->GetEnergy());
      for(int g2 = g + 1; g2 < grif.GetSuppressedMultiplicity(&grifBgo); ++g2) {
         auto grif2 = grif.GetSuppressedHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) continue;
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               fH2[slot].at("griffinGriffinESuppSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
      for(int g2 = 0; g2 < grif.GetMultiplicity(); ++g2) {
         if(g == g2) continue;
         auto grif2 = grif.GetGriffinHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) continue;
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               fH2[slot].at("griffinGriffinEMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            }
         }
      }
   }

   // loop over unsuppressed griffin addback hits
   for(int g = 0; g < grif.GetAddbackMultiplicity(); ++g) {
      auto grif1 = grif.GetAddbackHit(g);
      if(Reject(grif1) || !GoodCfd(grif1)) continue;
      fH1[slot].at("griffinEAddback")->Fill(grif1->GetEnergy());
      for(int g2 = g + 1; g2 < grif.GetAddbackMultiplicity(); ++g2) {
         auto grif2 = grif.GetAddbackHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) continue;
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
            if(PromptCoincidence(grif1, grif2)) {
               fH2[slot].at("griffinGriffinEAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at("griffinGriffinEAddback")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
               fH2[slot].at("griffinGriffinEAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
   }

   // loop over suppressed griffin addback hits
   for(int g = 0; g < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g) {
      auto grif1 = grif.GetSuppressedAddbackHit(g);
      if(Reject(grif1) || !GoodCfd(grif1)) continue;
      fH1[slot].at("griffinESuppAddback")->Fill(grif1->GetEnergy());
      for(int g2 = g + 1; g2 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g2) {
         auto grif2 = grif.GetSuppressedAddbackHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) continue;
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
            if(PromptCoincidence(grif1, grif2)) {
               fH2[slot].at("griffinGriffinESuppAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
      for(int g2 = 0; g2 < grif.GetAddbackMultiplicity(); ++g2) {
         if(g == g2) continue;
         auto grif2 = grif.GetAddbackHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) continue;
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
            if(PromptCoincidence(grif1, grif2)) {
               fH2[slot].at("griffinGriffinEMixedAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            }
         }
      }
   }

   // loop over suppressed griffin addback hits, single crystal method
   for(int g = 0; g < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g) {
      auto grif1 = grif.GetSuppressedAddbackHit(g);
      if(Reject(grif1) || !GoodCfd(grif1) || grif.GetNSuppressedAddbackFrags(g) > 1) continue;
      fH1[slot].at("griffinESingleCrystal")->Fill(grif1->GetEnergy());
      for(int g2 = g + 1; g2 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g2) {
         auto grif2 = grif.GetSuppressedAddbackHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2) || grif.GetNSuppressedAddbackFrags(g2) > 1) continue;
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               fH2[slot].at("griffinGriffinESingleCrystalSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
      for(int g2 = 0; g2 < grif.GetMultiplicity(); ++g2) {
         if(g == g2) continue;
         auto grif2 = grif.GetGriffinHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) continue;
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
            if(PromptCoincidence(grif1, grif2)) {
               fH2[slot].at("griffinGriffinEMixedSingleCrystal")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            }
         }
      }
   }
}
