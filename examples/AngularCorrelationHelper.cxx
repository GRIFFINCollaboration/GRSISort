#include "AngularCorrelationHelper.hh"

void AngularCorrelationHelper::CreateHistograms(unsigned int slot)
{
   // set up the queue for event mixing by filling it with empty events
   // we chose 10 here as a reasonable compromise between statistics and computing power/memory needed
   for(int i = 0; i < 10; ++i) {
      fGriffinDeque[slot].emplace_back(new TGriffin);
      fBgoDeque[slot].emplace_back(new TGriffinBgo);
   }

   std::string conditions = fAddback ? "using addback" : "without addback";
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
   for(auto g1 = 0; g1 < (fAddback ? fGriffin.GetSuppressedAddbackMultiplicity(&fGriffinBgo) : fGriffin.GetSuppressedMultiplicity(&fGriffinBgo)); ++g1) {
      auto grif1 = (fAddback ? fGriffin.GetSuppressedAddbackHit(g1) : fGriffin.GetSuppressedHit(g1));
      if(ExcludeDetector(grif1->GetDetector()) || ExcludeCrystal(grif1->GetArrayNumber())) continue;

      for(auto g2 = 0; g2 < (fAddback ? fGriffin.GetSuppressedAddbackMultiplicity(&fGriffinBgo) : fGriffin.GetSuppressedMultiplicity(&fGriffinBgo)); ++g2) {
         if(g1 == g2) continue;
         auto grif2 = (fAddback ? fGriffin.GetSuppressedAddbackHit(g2) : fGriffin.GetSuppressedHit(g2));
         if(ExcludeDetector(grif2->GetDetector()) || ExcludeCrystal(grif2->GetArrayNumber())) continue;

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
            fH2[slot].at(Form("AngularCorrelation%d", angleIndex))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
         } else if(fTimeRandomLow <= ggTime && ggTime <= fTimeRandomHigh) {
            fH2[slot].at("AngularCorrelationBG")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("AngularCorrelationBG%d", angleIndex))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
         }
      }

      // Event mixing: loop over all stored events for this thread/slot and use them as the "second" gamma ray
      for(auto l = 0; l < fGriffinDeque[slot].size(); ++l) {
         auto fLastGriffin = fGriffinDeque[slot][l];
         auto fLastBgo     = fBgoDeque[slot][l];
         for(auto g2 = 0; g2 < (fAddback ? fLastGriffin->GetSuppressedAddbackMultiplicity(fLastBgo) : fLastGriffin->GetSuppressedMultiplicity(fLastBgo)); ++g2) {
            auto grif2 = (fAddback ? fLastGriffin->GetSuppressedAddbackHit(g2) : fLastGriffin->GetSuppressedHit(g2));
            if(ExcludeDetector(grif2->GetDetector()) || ExcludeCrystal(grif2->GetArrayNumber())) continue;

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

   // Add the current event to the queue, delete the oldest event in the queue, and pop the pointer from the queue.
   fGriffinDeque[slot].emplace_back(new TGriffin(fGriffin));
   fBgoDeque[slot].emplace_back(new TGriffinBgo(fGriffinBgo));

   delete fGriffinDeque[slot].front();
   delete fBgoDeque[slot].front();

   fGriffinDeque[slot].pop_front();
   fBgoDeque[slot].pop_front();
}
