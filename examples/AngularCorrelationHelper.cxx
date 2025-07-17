#include "AngularCorrelationHelper.hh"

void AngularCorrelationHelper::CreateHistograms(unsigned int slot)
{
   // set up the queue for event mixing by filling it with empty events
   // we chose 10 here as a reasonable compromise between statistics and computing power/memory needed
   for(int i = 0; i < fNofMixedEvents; ++i) {
      fGriffinDeque[slot].emplace_back(new TGriffin);
      fBgoDeque[slot].emplace_back(new TGriffinBgo);
   }

   // try and get the cycle length if we have a PPG provided
   // but only if either the upper or the lower limit of the cycle time is set
   if(fCycleTimeLow >= 0. || fCycleTimeHigh >= 0.) {
      if(fPpg != nullptr) {
         // the ODB cycle length is in microseconds!
         fCycleLength = fPpg->OdbCycleLength();
         if(slot == 0) {
            std::stringstream str;
            str << "Got ODB cycle length " << fCycleLength << " us = " << fCycleLength / 1e6 << " s" << std::endl;
            std::cout << str.str();
         }
      } else if(slot == 0) {
         std::stringstream str;
         str << DRED << "No ppg provided, won't be able to apply cycle cut!" << RESET_COLOR << std::endl;
         std::cout << str.str();
      }
   }

   std::string conditions = fAddback ? "using addback" : "without addback";
   conditions += fSingleCrystal ? " single crystal" : "";
   conditions += fFolding ? ", folded around 90^{o}" : "";
   conditions += fGrouping ? ", grouped" : "";

   for(auto angle = fAngles->begin(); angle != fAngles->end(); ++angle) {
      int i                                           = std::distance(fAngles->begin(), angle);
      fH2[slot][Form("AngularCorrelation%d", i)]      = new TH2D(Form("AngularCorrelation%d", i), Form("%.1f^{o}: Suppressed #gamma-#gamma %s, |#Deltat_{#gamma-#gamma}| < %.1f", *angle, conditions.c_str(), fPrompt), fBins, fMinEnergy, fMaxEnergy, fBins, fMinEnergy, fMaxEnergy);
      fH2[slot][Form("AngularCorrelationBG%d", i)]    = new TH2D(Form("AngularCorrelationBG%d", i), Form("%.1f^{o}: Suppressed #gamma-#gamma %s, |#Deltat_{#gamma-#gamma}| = %.1f - %.1f", *angle, conditions.c_str(), fTimeRandomLow, fTimeRandomHigh), fBins, fMinEnergy, fMaxEnergy, fBins, fMinEnergy, fMaxEnergy);
      fH2[slot][Form("AngularCorrelationMixed%d", i)] = new TH2D(Form("AngularCorrelationMixed%d", i), Form("%.1f^{o}: Event mixed suppressed #gamma-#gamma %s", *angle, conditions.c_str()), fBins, fMinEnergy, fMaxEnergy, fBins, fMinEnergy, fMaxEnergy);
   }

   fH2[slot]["AngularCorrelation"]      = new TH2D("AngularCorrelation", Form("Suppressed #gamma-#gamma %s, |#Deltat_{#gamma-#gamma}| < %.1f", conditions.c_str(), fPrompt), fBins, fMinEnergy, fMaxEnergy, fBins, fMinEnergy, fMaxEnergy);
   fH2[slot]["AngularCorrelationBG"]    = new TH2D("AngularCorrelationBG", Form("Suppressed #gamma-#gamma %s, #Deltat_{#gamma-#gamma} = %.1f - %.1f", conditions.c_str(), fTimeRandomLow, fTimeRandomHigh), fBins, fMinEnergy, fMaxEnergy, fBins, fMinEnergy, fMaxEnergy);
   fH2[slot]["AngularCorrelationMixed"] = new TH2D("AngularCorrelationMixed", Form("Event mixed suppressed #gamma-#gamma %s", conditions.c_str()), fBins, fMinEnergy, fMaxEnergy, fBins, fMinEnergy, fMaxEnergy);

   // for the first slot we also write the griffin angles
   if(slot == 0) {
      fObject[slot]["GriffinAngles"] = fAngles;
   }
}

void AngularCorrelationHelper::Exec(unsigned int slot, TGriffin& fGriffin, TGriffinBgo& fGriffinBgo)
{
   bool eventInCycleCut = false;
   for(auto g1 = 0; g1 < (fAddback ? fGriffin.GetSuppressedAddbackMultiplicity(&fGriffinBgo) : fGriffin.GetSuppressedMultiplicity(&fGriffinBgo)); ++g1) {
      if(fSingleCrystal && fGriffin.GetNSuppressedAddbackFrags(g1) > 1) continue;
      auto grif1 = (fAddback ? fGriffin.GetSuppressedAddbackHit(g1) : fGriffin.GetSuppressedHit(g1));
      if(ExcludeDetector(grif1->GetDetector()) || ExcludeCrystal(grif1->GetArrayNumber())) continue;
      if(!GoodCycleTime(std::fmod(grif1->GetTime() / 1e3, fCycleLength))) continue;

      // we only get here if at least one hit of the event is within the cycle cut
      eventInCycleCut = true;

      for(auto g2 = g1 + 1; g2 < (fAddback ? fGriffin.GetSuppressedAddbackMultiplicity(&fGriffinBgo) : fGriffin.GetSuppressedMultiplicity(&fGriffinBgo)); ++g2) {
         if(fSingleCrystal && fGriffin.GetNSuppressedAddbackFrags(g2) > 1) continue;
         auto grif2 = (fAddback ? fGriffin.GetSuppressedAddbackHit(g2) : fGriffin.GetSuppressedHit(g2));
         if(ExcludeDetector(grif2->GetDetector()) || ExcludeCrystal(grif2->GetArrayNumber())) continue;
         if(!GoodCycleTime(std::fmod(grif2->GetTime() / 1e3, fCycleLength))) continue;

         // skip hits in the same detector when using addback, or in the same crystal when not using addback
         if(grif1->GetDetector() == grif2->GetDetector() && (fAddback || grif1->GetCrystal() == grif2->GetCrystal())) {
            continue;
         }

         // calculate the angle
         double angle = grif1->GetPosition(fGriffinDistance).Angle(grif2->GetPosition(fGriffinDistance)) * 180. / TMath::Pi();
         if(angle < fAngles->Rounding()) continue;
         if(fFolding && angle > 90.) angle = 180. - angle;

         // find the index of the angle
         auto angleIndex = fAngles->Index(angle);
         if(angleIndex < 0) continue;
         // check the timing to see if these are coincident or time-random hits
         double ggTime = TMath::Abs(grif2->GetTime() - grif1->GetTime());
         if(ggTime <= fPrompt) {
            fH2[slot].at("AngularCorrelation")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at("AngularCorrelation")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
            fH2[slot].at(Form("AngularCorrelation%d", angleIndex))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("AngularCorrelation%d", angleIndex))->Fill(grif2->GetEnergy(), grif1->GetEnergy());
         } else if(fTimeRandomLow <= ggTime && ggTime <= fTimeRandomHigh) {
            fH2[slot].at("AngularCorrelationBG")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at("AngularCorrelationBG")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
            fH2[slot].at(Form("AngularCorrelationBG%d", angleIndex))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("AngularCorrelationBG%d", angleIndex))->Fill(grif2->GetEnergy(), grif1->GetEnergy());
         }
      }

      // Event mixing: loop over all stored events for this thread/slot and use them as the "second" gamma ray
      for(auto l = 0; l < fGriffinDeque[slot].size(); ++l) {
         auto fLastGriffin = fGriffinDeque[slot][l];
         auto fLastBgo     = fBgoDeque[slot][l];
         for(auto g2 = 0; g2 < (fAddback ? fLastGriffin->GetSuppressedAddbackMultiplicity(fLastBgo) : fLastGriffin->GetSuppressedMultiplicity(fLastBgo)); ++g2) {
            auto grif2 = (fAddback ? fLastGriffin->GetSuppressedAddbackHit(g2) : fLastGriffin->GetSuppressedHit(g2));
            if(ExcludeDetector(grif2->GetDetector()) || ExcludeCrystal(grif2->GetArrayNumber())) continue;
            if(fSingleCrystal && fLastGriffin->GetNSuppressedAddbackFrags(g2) > 1) continue;
            if(!GoodCycleTime(std::fmod(grif2->GetTime() / 1e3, fCycleLength))) continue;

            // skip hits in the same detector when using addback, or in the same crystal when not using addback
            if(grif1->GetDetector() == grif2->GetDetector() && (fAddback || grif1->GetCrystal() == grif2->GetCrystal())) continue;

            double angle = grif1->GetPosition(fGriffinDistance).Angle(grif2->GetPosition(fGriffinDistance)) * 180. / TMath::Pi();
            if(angle < fAngles->Rounding()) continue;
            if(fFolding && angle > 90.) angle = 180. - angle;

            // find the index of the angle
            auto angleIndex = fAngles->Index(angle);
            if(angleIndex < 0) continue;
            // no point in checking the time here, the two hits are from different events
            fH2[slot].at("AngularCorrelationMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("AngularCorrelationMixed%d", angleIndex))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
         }
      }
   }

   if(eventInCycleCut) {
      // Add the current event to the queue, delete the oldest event in the queue, and pop the pointer from the queue.
      fGriffinDeque[slot].emplace_back(new TGriffin(fGriffin));
      fBgoDeque[slot].emplace_back(new TGriffinBgo(fGriffinBgo));

      delete fGriffinDeque[slot].front();
      delete fBgoDeque[slot].front();

      fGriffinDeque[slot].pop_front();
      fBgoDeque[slot].pop_front();
   }
}
