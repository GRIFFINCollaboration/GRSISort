#include "SummingCorrectionsHelper.hh"

void SummingCorrectionsHelper::CreateHistograms(unsigned int slot)
{
   // four histograms for each type:
   // - 1D singles (2D matrix for branching ratios),
   // - prompt 2D matrix (unsuppressed on y-axis for suppressed data),
   // - same as 1D projection on x-axis (but with 1D binning), and
   // - sum matrix (sum on x-axis)
   // latter three are for detectors at 180 degree from another
   // 180 degree means for addback that the detectors are opposite, so the crystal angles are 158 degrees and larger (for 145 mm distance)!

   // unsuppressed spectra
   fH1[slot]["griffinE"]            = new TH1F("griffinE", Form("Unsuppressed griffin energy;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinE"]     = new TH2F("griffinGriffinE", "Unsuppressed griffin-griffin energy @ 180^{o};energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH1[slot]["griffinGriffinEProj"] = new TH1F("griffinGriffinEProj", Form("Unsuppressed griffin energy with hit in crystal @ 180^{o};energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinESum"]  = new TH2F("griffinGriffinESum", "Unsuppressed griffin energy vs griffin sum energy @ 180^{o};sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   // suppressed spectra
   fH1[slot]["griffinESupp"]            = new TH1F("griffinESupp", Form("Suppressed griffin energy;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEMixed"]    = new TH2F("griffinGriffinEMixed", "Unsuppressed/suppressed griffin-griffin energy @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH1[slot]["griffinGriffinESuppProj"] = new TH1F("griffinGriffinESuppProj", Form("Suppressed griffin energy with hit in crystal @ 180^{o};energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinESuppSum"]  = new TH2F("griffinGriffinESuppSum", "Suppressed griffin energy vs griffin sum energy @ 180^{o};sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   // unsuppressed addback spectra
   fH1[slot]["griffinEAddback"]            = new TH1F("griffinEAddback", Form("Unsuppressed griffin addback energy;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEAddback"]     = new TH2F("griffinGriffinEAddback", "Unsuppressed griffin-griffin addback @ 180^{o};energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH1[slot]["griffinGriffinEAddbackProj"] = new TH1F("griffinGriffinEAddbackProj", Form("Unsuppressed griffin addback energy with hit in detector @ 180^{o};energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEAddbackSum"]  = new TH2F("griffinGriffinEAddbackSum", "Unsuppressed griffin addback vs griffin sum energy @ 180^{o};sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   // suppressed addback spectra
   fH1[slot]["griffinESuppAddback"]            = new TH1F("griffinESuppAddback", Form("Suppressed griffin addback energy;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEMixedAddback"]    = new TH2F("griffinGriffinEMixedAddback", "Unsuppressed/suppressed griffin-griffin addback @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH1[slot]["griffinGriffinESuppAddbackProj"] = new TH1F("griffinGriffinESuppAddbackProj", Form("Suppressed griffin addback energy with hit in detector @ 180^{o};energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinESuppAddbackSum"]  = new TH2F("griffinGriffinESuppAddbackSum", "Suppressed griffin addback vs sum energy @ 180^{o};sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   // suppressed single crystal addback spectra
   fH1[slot]["griffinESingleCrystal"]             = new TH1F("griffinESingleCrystal", Form("Suppressed griffin addback energy single crystal method;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinEMixedSingleCrystal"] = new TH2F("griffinGriffinEMixedSingleCrystal", "Unsuppressed/suppressed griffin-griffin addback @ 180^{o};suppressed energy [keV];unsuppressed energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
   fH1[slot]["griffinGriffinESingleCrystalProj"]  = new TH1F("griffinGriffinESingleCrystalProj", Form("Suppressed griffin addback energy single crystal method with hit in detector @ 180^{o};energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins1D), fEnergyBins1D, fLowEnergy, fHighEnergy);
   fH2[slot]["griffinGriffinESingleCrystalSum"]   = new TH2F("griffinGriffinESingleCrystalSum", "Suppressed griffin addback vs sum energy @ 180^{o}, single crystal method;sum energy [keV];energy [keV]", fEnergyBins2D, fLowEnergy, fHighEnergy, fEnergyBins2D, fLowEnergy, fHighEnergy);
}

void SummingCorrectionsHelper::Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo)
{
   if(fEnergyGateLow < fEnergyGateHigh) {
      FillBranchingRatioHistograms(slot, grif, grifBgo);
   } else {
      FillEfficiencyHistograms(slot, grif, grifBgo);
   }
}

void SummingCorrectionsHelper::FillEfficiencyHistograms(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo)
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

   // loop over unsuppressed griffin hits
   for(int g1 = 0; g1 < grif.GetMultiplicity(); ++g1) {
      auto* grif1 = grif.GetGriffinHit(g1);
      if(Reject(grif1) || !GoodCfd(grif1)) { continue; }
      fH1[slot].at("griffinE")->Fill(grif1->GetEnergy());
      for(int g2 = g1 + 1; g2 < grif.GetMultiplicity(); ++g2) {
         auto* grif2 = grif.GetGriffinHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
            if(Prompt(grif1, grif2)) {
               // fill every histogram "twice" because of the way we loop
               fH2[slot].at("griffinGriffinE")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at("griffinGriffinE")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
               fH1[slot].at("griffinGriffinEProj")->Fill(grif1->GetEnergy());
               fH1[slot].at("griffinGriffinEProj")->Fill(grif2->GetEnergy());
               fH2[slot].at("griffinGriffinESum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
               fH2[slot].at("griffinGriffinESum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif2->GetEnergy());
            }
         }
      }
   }

   // loop over suppressed griffin hits
   for(int g1 = 0; g1 < grif.GetSuppressedMultiplicity(&grifBgo); ++g1) {
      auto* grif1 = grif.GetSuppressedHit(g1);
      if(Reject(grif1) || !GoodCfd(grif1)) { continue; }
      fH1[slot].at("griffinESupp")->Fill(grif1->GetEnergy());
      for(int g2 = g1 + 1; g2 < grif.GetSuppressedMultiplicity(&grifBgo); ++g2) {
         auto* grif2 = grif.GetSuppressedHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
            if(Prompt(grif1, grif2)) {
               fH2[slot].at("griffinGriffinESuppSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
      for(int g2 = 0; g2 < grif.GetMultiplicity(); ++g2) {
         if(g1 == g2) { continue; }   // this is wrong? maybe check energy and time instead?
         auto* grif2 = grif.GetGriffinHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
            if(Prompt(grif1, grif2)) {
               fH2[slot].at("griffinGriffinEMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH1[slot].at("griffinGriffinESuppProj")->Fill(grif1->GetEnergy());
            }
         }
      }
   }

   // loop over unsuppressed griffin addback hits
   for(int g1 = 0; g1 < grif.GetAddbackMultiplicity(); ++g1) {
      auto* grif1 = grif.GetAddbackHit(g1);
      if(Reject(grif1) || !GoodCfd(grif1)) { continue; }
      fH1[slot].at("griffinEAddback")->Fill(grif1->GetEnergy());
      for(int g2 = g1 + 1; g2 < grif.GetAddbackMultiplicity(); ++g2) {
         auto* grif2 = grif.GetAddbackHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
            if(Prompt(grif1, grif2)) {
               // fill every histogram "twice" because of the way we loop
               fH2[slot].at("griffinGriffinEAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at("griffinGriffinEAddback")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
               fH1[slot].at("griffinGriffinEAddbackProj")->Fill(grif1->GetEnergy());
               fH1[slot].at("griffinGriffinEAddbackProj")->Fill(grif2->GetEnergy());
               fH2[slot].at("griffinGriffinEAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
               fH2[slot].at("griffinGriffinEAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif2->GetEnergy());
            }
         }
      }
   }

   // loop over suppressed griffin addback hits
   for(int g1 = 0; g1 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g1) {
      auto* grif1 = grif.GetSuppressedAddbackHit(g1);
      if(Reject(grif1) || !GoodCfd(grif1)) { continue; }
      fH1[slot].at("griffinESuppAddback")->Fill(grif1->GetEnergy());
      for(int g2 = g1 + 1; g2 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g2) {
         auto* grif2 = grif.GetSuppressedAddbackHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
            if(Prompt(grif1, grif2)) {
               fH2[slot].at("griffinGriffinESuppAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
      for(int g2 = 0; g2 < grif.GetAddbackMultiplicity(); ++g2) {
         if(g1 == g2) { continue; }
         auto* grif2 = grif.GetAddbackHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
            if(Prompt(grif1, grif2)) {
               fH2[slot].at("griffinGriffinEMixedAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH1[slot].at("griffinGriffinESuppAddbackProj")->Fill(grif1->GetEnergy());
            }
         }
      }
   }

   // loop over suppressed griffin addback hits, single crystal method
   for(int g1 = 0; g1 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g1) {
      auto* grif1 = grif.GetSuppressedAddbackHit(g1);
      if(Reject(grif1) || !GoodCfd(grif1) || grif.GetNSuppressedAddbackFrags(g1) > 1) { continue; }
      fH1[slot].at("griffinESingleCrystal")->Fill(grif1->GetEnergy());
      for(int g2 = g1 + 1; g2 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g2) {
         auto* grif2 = grif.GetSuppressedAddbackHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2) || grif.GetNSuppressedAddbackFrags(g2) > 1) { continue; }
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
            if(Prompt(grif1, grif2)) {
               fH2[slot].at("griffinGriffinESingleCrystalSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
      for(int g2 = 0; g2 < grif.GetAddbackMultiplicity(); ++g2) {
         if(g1 == g2) { continue; }
         auto* grif2 = grif.GetAddbackHit(g2);
         if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
         if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
            if(Prompt(grif1, grif2)) {
               fH2[slot].at("griffinGriffinEMixedSingleCrystal")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH1[slot].at("griffinGriffinESingleCrystalProj")->Fill(grif1->GetEnergy());
            }
         }
      }
   }
}

void SummingCorrectionsHelper::FillBranchingRatioHistograms(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo)
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

   // we do direct background corrections in the histograms, i.e.
   // we subtract time-random gate and coincident Compton background, and add time-random Compton background

   // loop over unsuppressed griffin hits
   for(int g0 = 0; g0 < grif.GetMultiplicity(); ++g0) {
      auto* grif0 = grif.GetGriffinHit(g0);
      if(Reject(grif0) || !GoodCfd(grif0)) { continue; }
      for(int g1 = 0; g1 < grif.GetMultiplicity(); ++g1) {
         if(g1 == g0) { continue; }
         auto* grif1 = grif.GetGriffinHit(g1);
         if(Reject(grif1) || !GoodCfd(grif1)) { continue; }
         if(Coincident(grif0, grif1) && Gate(grif0)) {
            // coincident gate
            fH1[slot].at("griffinE")->Fill(grif1->GetEnergy());
            for(int g2 = g1 + 1; g2 < grif.GetMultiplicity(); ++g2) {
               auto* grif2 = grif.GetGriffinHit(g2);
               if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
               if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
                  if(Prompt(grif1, grif2)) {
                     fH2[slot].at("griffinGriffinE")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
                     fH2[slot].at("griffinGriffinE")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
                     fH2[slot].at("griffinGriffinESum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
                  }
               }
            }
         } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
            // time-random gate - subtract with time random weight
            fH1[slot].at("griffinE")->Fill(grif1->GetEnergy(), -fTimeRandomRatio);
            for(int g2 = g1 + 1; g2 < grif.GetMultiplicity(); ++g2) {
               auto* grif2 = grif.GetGriffinHit(g2);
               if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
               if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
                  if(Prompt(grif1, grif2)) {
                     fH2[slot].at("griffinGriffinE")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fTimeRandomRatio);
                     fH2[slot].at("griffinGriffinE")->Fill(grif2->GetEnergy(), grif1->GetEnergy(), -fTimeRandomRatio);
                     fH2[slot].at("griffinGriffinESum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fTimeRandomRatio);
                  }
               }
            }
         } else if(Coincident(grif0, grif1) && Background(grif0)) {
            // coincident Compton background - subtract with energy gate weight
            fH1[slot].at("griffinE")->Fill(grif1->GetEnergy(), -fEnergyRatio);
            for(int g2 = g1 + 1; g2 < grif.GetMultiplicity(); ++g2) {
               auto* grif2 = grif.GetGriffinHit(g2);
               if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
               if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
                  if(Prompt(grif1, grif2)) {
                     fH2[slot].at("griffinGriffinE")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fEnergyRatio);
                     fH2[slot].at("griffinGriffinE")->Fill(grif2->GetEnergy(), grif1->GetEnergy(), -fEnergyRatio);
                     fH2[slot].at("griffinGriffinESum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fEnergyRatio);
                  }
               }
            }
         } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
            // time random Compton background - add with time random weight time energy gate weight
            fH1[slot].at("griffinE")->Fill(grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
            for(int g2 = g1 + 1; g2 < grif.GetMultiplicity(); ++g2) {
               auto* grif2 = grif.GetGriffinHit(g2);
               if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
               if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
                  if(Prompt(grif1, grif2)) {
                     fH2[slot].at("griffinGriffinE")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                     fH2[slot].at("griffinGriffinE")->Fill(grif2->GetEnergy(), grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                     fH2[slot].at("griffinGriffinESum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                  }
               }
            }
         }
      }
   }

   // loop over suppressed griffin hits
   for(int g0 = 0; g0 < grif.GetAddbackMultiplicity(); ++g0) {
      auto* grif0 = grif.GetAddbackHit(g0);
      if(Reject(grif0) || !GoodCfd(grif0)) { continue; }
      for(int g1 = 0; g1 < grif.GetSuppressedMultiplicity(&grifBgo); ++g1) {
         if(g1 == g0) { continue; }
         auto* grif1 = grif.GetSuppressedHit(g1);
         if(Reject(grif1) || !GoodCfd(grif1)) { continue; }
         if(Coincident(grif0, grif1) && Gate(grif0)) {
            // coincident gate
            fH1[slot].at("griffinESupp")->Fill(grif1->GetEnergy());
         } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
            // time-random gate - subtract with time random weight
            fH1[slot].at("griffinESupp")->Fill(grif1->GetEnergy(), -fTimeRandomRatio);
         } else if(Coincident(grif0, grif1) && Background(grif0)) {
            // coincident Compton background - subtract with energy gate weight
            fH1[slot].at("griffinESupp")->Fill(grif1->GetEnergy(), -fEnergyRatio);
         } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
            // time random Compton background - add with time random weight time energy gate weight
            fH1[slot].at("griffinESupp")->Fill(grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
         }
         for(int g2 = g1 + 1; g2 < grif.GetSuppressedMultiplicity(&grifBgo); ++g2) {
            auto* grif2 = grif.GetSuppressedHit(g2);
            if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
            if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
               if(Prompt(grif1, grif2)) {
                  if(Coincident(grif0, grif1) && Gate(grif0)) {
                     // coincident gate
                     fH2[slot].at("griffinGriffinESuppSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
                  } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
                     // time-random gate - subtract with time random weight
                     fH2[slot].at("griffinGriffinESuppSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fTimeRandomRatio);
                  } else if(Coincident(grif0, grif1) && Background(grif0)) {
                     // coincident Compton background - subtract with energy gate weight
                     fH2[slot].at("griffinGriffinESuppSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fEnergyRatio);
                  } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
                     // time random Compton background - add with time random weight time energy gate weight
                     fH2[slot].at("griffinGriffinESuppSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                  }
               }
            }
         }
         for(int g2 = 0; g2 < grif.GetMultiplicity(); ++g2) {
            if(g1 == g2) { continue; }
            auto* grif2 = grif.GetGriffinHit(g2);
            if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
            if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
               if(Prompt(grif1, grif2)) {
                  if(Coincident(grif0, grif1) && Gate(grif0)) {
                     // coincident gate
                     fH2[slot].at("griffinGriffinEMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
                  } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
                     // time-random gate - subtract with time random weight
                     fH2[slot].at("griffinGriffinEMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fTimeRandomRatio);
                  } else if(Coincident(grif0, grif1) && Background(grif0)) {
                     // coincident Compton background - subtract with energy gate weight
                     fH2[slot].at("griffinGriffinEMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fEnergyRatio);
                  } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
                     // time random Compton background - add with time random weight time energy gate weight
                     fH2[slot].at("griffinGriffinEMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                  }
               }
            }
         }
      }
   }

   // loop over unsuppressed griffin addback hits
   for(int g0 = 0; g0 < grif.GetAddbackMultiplicity(); ++g0) {
      auto* grif0 = grif.GetAddbackHit(g0);
      if(Reject(grif0) || !GoodCfd(grif0)) { continue; }
      for(int g1 = 0; g1 < grif.GetAddbackMultiplicity(); ++g1) {
         if(g1 == g0) { continue; }
         auto* grif1 = grif.GetAddbackHit(g1);
         if(Reject(grif1) || !GoodCfd(grif1)) { continue; }
         if(Coincident(grif0, grif1) && Gate(grif0)) {
            // coincident gate
            fH1[slot].at("griffinEAddback")->Fill(grif1->GetEnergy());
            for(int g2 = g1 + 1; g2 < grif.GetAddbackMultiplicity(); ++g2) {
               auto* grif2 = grif.GetAddbackHit(g2);
               if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
               if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
                  if(Prompt(grif1, grif2)) {
                     fH2[slot].at("griffinGriffinEAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
                     fH2[slot].at("griffinGriffinEAddback")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
                     fH2[slot].at("griffinGriffinEAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
                  }
               }
            }
         } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
            // time-random gate - subtract with time random weight
            fH1[slot].at("griffinEAddback")->Fill(grif1->GetEnergy(), -fTimeRandomRatio);
            for(int g2 = g1 + 1; g2 < grif.GetAddbackMultiplicity(); ++g2) {
               auto* grif2 = grif.GetAddbackHit(g2);
               if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
               if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
                  if(Prompt(grif1, grif2)) {
                     fH2[slot].at("griffinGriffinEAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fTimeRandomRatio);
                     fH2[slot].at("griffinGriffinEAddback")->Fill(grif2->GetEnergy(), grif1->GetEnergy(), -fTimeRandomRatio);
                     fH2[slot].at("griffinGriffinEAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fTimeRandomRatio);
                  }
               }
            }
         } else if(Coincident(grif0, grif1) && Background(grif0)) {
            // coincident Compton background - subtract with energy gate weight
            fH1[slot].at("griffinEAddback")->Fill(grif1->GetEnergy(), -fEnergyRatio);
            for(int g2 = g1 + 1; g2 < grif.GetAddbackMultiplicity(); ++g2) {
               auto* grif2 = grif.GetAddbackHit(g2);
               if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
               if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
                  if(Prompt(grif1, grif2)) {
                     fH2[slot].at("griffinGriffinEAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fEnergyRatio);
                     fH2[slot].at("griffinGriffinEAddback")->Fill(grif2->GetEnergy(), grif1->GetEnergy(), -fEnergyRatio);
                     fH2[slot].at("griffinGriffinEAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fEnergyRatio);
                  }
               }
            }
         } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
            // time random Compton background - add with time random weight time energy gate weight
            fH1[slot].at("griffinEAddback")->Fill(grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
            for(int g2 = g1 + 1; g2 < grif.GetAddbackMultiplicity(); ++g2) {
               auto* grif2 = grif.GetAddbackHit(g2);
               if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
               if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
                  if(Prompt(grif1, grif2)) {
                     fH2[slot].at("griffinGriffinEAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                     fH2[slot].at("griffinGriffinEAddback")->Fill(grif2->GetEnergy(), grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                     fH2[slot].at("griffinGriffinEAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                  }
               }
            }
         }
      }
   }

   // loop over suppressed griffin addback hits
   for(int g0 = 0; g0 < grif.GetAddbackMultiplicity(); ++g0) {
      auto* grif0 = grif.GetAddbackHit(g0);
      if(Reject(grif0) || !GoodCfd(grif0)) { continue; }
      for(int g1 = 0; g1 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g1) {
         if(g1 == g0) { continue; }
         auto* grif1 = grif.GetSuppressedAddbackHit(g1);
         if(Reject(grif1) || !GoodCfd(grif1)) { continue; }
         if(Coincident(grif0, grif1) && Gate(grif0)) {
            // coincident gate
            fH1[slot].at("griffinESuppAddback")->Fill(grif1->GetEnergy());
         } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
            // time-random gate - subtract with time random weight
            fH1[slot].at("griffinESuppAddback")->Fill(grif1->GetEnergy(), -fTimeRandomRatio);
         } else if(Coincident(grif0, grif1) && Background(grif0)) {
            // coincident Compton background - subtract with energy gate weight
            fH1[slot].at("griffinESuppAddback")->Fill(grif1->GetEnergy(), -fEnergyRatio);
         } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
            // time random Compton background - add with time random weight time energy gate weight
            fH1[slot].at("griffinESuppAddback")->Fill(grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
         }
         for(int g2 = g1 + 1; g2 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g2) {
            auto* grif2 = grif.GetSuppressedAddbackHit(g2);
            if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
            if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
               if(Prompt(grif1, grif2)) {
                  if(Coincident(grif0, grif1) && Gate(grif0)) {
                     // coincident gate
                     fH2[slot].at("griffinGriffinESuppAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
                  } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
                     // time-random gate - subtract with time random weight
                     fH2[slot].at("griffinGriffinESuppAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fTimeRandomRatio);
                  } else if(Coincident(grif0, grif1) && Background(grif0)) {
                     // coincident Compton background - subtract with energy gate weight
                     fH2[slot].at("griffinGriffinESuppAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fEnergyRatio);
                  } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
                     // time random Compton background - add with time random weight time energy gate weight
                     fH2[slot].at("griffinGriffinESuppAddbackSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                  }
               }
            }
         }
         for(int g2 = 0; g2 < grif.GetAddbackMultiplicity(); ++g2) {
            if(g1 == g2) { continue; }
            auto* grif2 = grif.GetAddbackHit(g2);
            if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
            if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
               if(Prompt(grif1, grif2)) {
                  if(Coincident(grif0, grif1) && Gate(grif0)) {
                     // coincident gate
                     fH2[slot].at("griffinGriffinEMixedAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
                  } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
                     // time-random gate - subtract with time random weight
                     fH2[slot].at("griffinGriffinEMixedAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fTimeRandomRatio);
                  } else if(Coincident(grif0, grif1) && Background(grif0)) {
                     // coincident Compton background - subtract with energy gate weight
                     fH2[slot].at("griffinGriffinEMixedAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fEnergyRatio);
                  } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
                     // time random Compton background - add with time random weight time energy gate weight
                     fH2[slot].at("griffinGriffinEMixedAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                  }
               }
            }
         }
      }
   }

   // loop over suppressed griffin addback hits, single crystal method
   for(int g0 = 0; g0 < grif.GetAddbackMultiplicity(); ++g0) {
      auto* grif0 = grif.GetAddbackHit(g0);
      if(Reject(grif0) || !GoodCfd(grif0)) { continue; }
      for(int g1 = 0; g1 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g1) {
         if(g1 == g0) { continue; }
         auto* grif1 = grif.GetSuppressedAddbackHit(g1);
         if(Reject(grif1) || !GoodCfd(grif1) || grif.GetNSuppressedAddbackFrags(g1) > 1) { continue; }
         if(Coincident(grif0, grif1) && Gate(grif0)) {
            // coincident gate
            fH1[slot].at("griffinESingleCrystal")->Fill(grif1->GetEnergy());
         } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
            // time-random gate - subtract with time random weight
            fH1[slot].at("griffinESingleCrystal")->Fill(grif1->GetEnergy(), -fTimeRandomRatio);
         } else if(Coincident(grif0, grif1) && Background(grif0)) {
            // coincident Compton background - subtract with energy gate weight
            fH1[slot].at("griffinESingleCrystal")->Fill(grif1->GetEnergy(), -fEnergyRatio);
         } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
            // time random Compton background - add with time random weight time energy gate weight
            fH1[slot].at("griffinESingleCrystal")->Fill(grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
         }
         for(int g2 = g1 + 1; g2 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g2) {
            auto* grif2 = grif.GetSuppressedAddbackHit(g2);
            if(Reject(grif2) || !GoodCfd(grif2) || grif.GetNSuppressedAddbackFrags(g2) > 1) { continue; }
            if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 179.) {
               if(Prompt(grif1, grif2)) {
                  if(Coincident(grif0, grif1) && Gate(grif0)) {
                     // coincident gate
                     fH2[slot].at("griffinGriffinESingleCrystalSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy());
                  } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
                     // time-random gate - subtract with time random weight
                     fH2[slot].at("griffinGriffinESingleCrystalSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fTimeRandomRatio);
                  } else if(Coincident(grif0, grif1) && Background(grif0)) {
                     // coincident Compton background - subtract with energy gate weight
                     fH2[slot].at("griffinGriffinESingleCrystalSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), -fEnergyRatio);
                  } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
                     // time random Compton background - add with time random weight time energy gate weight
                     fH2[slot].at("griffinGriffinESingleCrystalSum")->Fill(grif1->GetEnergy() + grif2->GetEnergy(), grif1->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                  }
               }
            }
         }
         for(int g2 = 0; g2 < grif.GetAddbackMultiplicity(); ++g2) {
            if(g1 == g2) { continue; }
            auto* grif2 = grif.GetAddbackHit(g2);
            if(Reject(grif2) || !GoodCfd(grif2)) { continue; }
            if(grif1->GetPosition().Angle(grif2->GetPosition()) / TMath::Pi() * 180. > 157.) {
               if(Prompt(grif1, grif2)) {
                  if(Coincident(grif0, grif1) && Gate(grif0)) {
                     // coincident gate
                     fH2[slot].at("griffinGriffinEMixedSingleCrystal")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
                  } else if(TimeRandom(grif0, grif1) && Gate(grif0)) {
                     // time-random gate - subtract with time random weight
                     fH2[slot].at("griffinGriffinEMixedSingleCrystal")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fTimeRandomRatio);
                  } else if(Coincident(grif0, grif1) && Background(grif0)) {
                     // coincident Compton background - subtract with energy gate weight
                     fH2[slot].at("griffinGriffinEMixedSingleCrystal")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), -fEnergyRatio);
                  } else if(TimeRandom(grif0, grif1) && Background(grif0)) {
                     // time random Compton background - add with time random weight time energy gate weight
                     fH2[slot].at("griffinGriffinEMixedSingleCrystal")->Fill(grif1->GetEnergy(), grif2->GetEnergy(), fTimeRandomRatio * fEnergyRatio);
                  }
               }
            }
         }
      }
   }
}
