#include "AngularCorrelationHelper.hh"

double gbLow  = -50.;  // min. time difference gamma-beta
double gbHigh = 500.;  // max. time difference gamma-beta
double ggHigh = 400.;  // max. absolute time difference gamma-gamma
double bgLow  = 1000.; // min. time difference gamma-gamma background
double bgHigh = 2000.; // max. time difference gamma-gamma background

void AngularCorrelationHelper::CreateHistograms(unsigned int slot)
{
   // for each angle (and the sum) we want
   // for single crystal and addback
   // with and without coincident betas
   // coincident and time-random gamma-gamma
   for(int i = 0; i < static_cast<int>(fAngleCombinations.size()); ++i) {
      fH2[slot][Form("gammaGamma%d", i)] =
         new TH2D(Form("gammaGamma%d", i), Form("%.1f^{o}: #gamma-#gamma, |#Deltat_{#gamma-#gamma}| < %.1f",
                                                fAngleCombinations[i].first, ggHigh),
                  2000, 0., 2000., 2000, 0., 2000.);
      fH2[slot][Form("gammaGammaBeta%d", i)] = new TH2D(
         Form("gammaGammaBeta%d", i),
         Form("%.1f^{o}: #gamma-#gamma, |#Deltat_{#gamma-#gamma}| < %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f",
              fAngleCombinations[i].first, ggHigh, gbLow, gbHigh),
         2000, 0., 2000., 2000, 0., 2000.);
      fH2[slot][Form("gammaGammaBG%d", i)] =
         new TH2D(Form("gammaGammaBG%d", i), Form("%.1f^{o}: #gamma-#gamma, #Deltat_{#gamma-#gamma} = %.1f - %.1f",
                                                  fAngleCombinations[i].first, bgLow, bgHigh),
                  2000, 0., 2000., 2000, 0., 2000.);
      fH2[slot][Form("gammaGammaBetaBG%d", i)] = new TH2D(
         Form("gammaGammaBetaBG%d", i),
         Form("%.1f^{o}: #gamma-#gamma, #Deltat_{#gamma-#gamma} = %.1f - %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f",
              fAngleCombinations[i].first, bgLow, bgHigh, gbLow, gbHigh),
         2000, 0., 2000., 2000, 0., 2000.);
   }
   for(int i = 0; i < static_cast<int>(fAngleCombinationsAddback.size()); ++i) {
      fH2[slot][Form("addbackAddback%d", i)] = new TH2D(
         Form("addbackAddback%d", i), Form("%.1f^{o}: #gamma-#gamma with addback, |#Deltat_{#gamma-#gamma}| < %.1f",
                                           fAngleCombinationsAddback[i].first, ggHigh),
         2000, 0., 2000., 2000, 0., 2000.);
      fH2[slot][Form("addbackAddbackBeta%d", i)] = new TH2D(
         Form("addbackAddbackBeta%d", i), Form("%.1f^{o}: #gamma-#gamma with addback, |#Deltat_{#gamma-#gamma}| < "
                                               "%.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f",
                                               fAngleCombinationsAddback[i].first, ggHigh, gbLow, gbHigh),
         2000, 0., 2000., 2000, 0., 2000.);
      fH2[slot][Form("addbackAddbackBG%d", i)] =
         new TH2D(Form("addbackAddbackBG%d", i),
                  Form("%.1f^{o}: #gamma-#gamma with addback, #Deltat_{#gamma-#gamma} = %.1f - %.1f",
                       fAngleCombinationsAddback[i].first, bgLow, bgHigh),
                  2000, 0., 2000., 2000, 0., 2000.);
      fH2[slot][Form("addbackAddbackBetaBG%d", i)] = new TH2D(
         Form("addbackAddbackBetaBG%d", i), Form("%.1f^{o}: #gamma-#gamma with addback, #Deltat_{#gamma-#gamma} = %.1f "
                                                 "- %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f",
                                                 fAngleCombinationsAddback[i].first, bgLow, bgHigh, gbLow, gbHigh),
         2000, 0., 2000., 2000, 0., 2000.);
   }
   fH2[slot]["gammaGamma"] = new TH2D("gammaGamma", Form("#gamma-#gamma, |#Deltat_{#gamma-#gamma}| < %.1f", ggHigh), 2000, 0.,
                                2000., 2000, 0., 2000.);
   fH2[slot]["gammaGammaBeta"] = new TH2D(
      "gammaGammaBeta", Form("#gamma-#gamma, |#Deltat_{#gamma-#gamma}| < %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f",
                             ggHigh, gbLow, gbHigh),
      2000, 0., 2000., 2000, 0., 2000.);
   fH2[slot]["gammaGammaBG"] =
      new TH2D("gammaGammaBG", Form("#gamma-#gamma, #Deltat_{#gamma-#gamma} = %.1f - %.1f", bgLow, bgHigh), 2000, 0.,
               2000., 2000, 0., 2000.);
   fH2[slot]["gammaGammaBetaBG"] =
      new TH2D("gammaGammaBetaBG",
               Form("#gamma-#gamma, #Deltat_{#gamma-#gamma} = %.1f - %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f", bgLow,
                    bgHigh, gbLow, gbHigh),
               2000, 0., 2000., 2000, 0., 2000.);
   fH2[slot]["addbackAddback"] =
      new TH2D("addbackAddback", Form("#gamma-#gamma with addback, |#Deltat_{#gamma-#gamma}| < %.1f", ggHigh), 2000, 0.,
               2000., 2000, 0., 2000.);
   fH2[slot]["addbackAddbackBeta"] = new TH2D(
      "addbackAddbackBeta",
      Form("#gamma-#gamma with addback, |#Deltat_{#gamma-#gamma}| < %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f", ggHigh,
           gbLow, gbHigh),
      2000, 0., 2000., 2000, 0., 2000.);
   fH2[slot]["addbackAddbackBG"] = new TH2D(
      "addbackAddbackBG", Form("#gamma-#gamma with addback, #Deltat_{#gamma-#gamma} = %.1f - %.1f", bgLow, bgHigh),
      2000, 0., 2000., 2000, 0., 2000.);
   fH2[slot]["addbackAddbackBetaBG"] = new TH2D(
      "addbackAddbackBetaBG",
      Form("#gamma-#gamma with addback, #Deltat_{#gamma-#gamma} = %.1f - %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f",
           bgLow, bgHigh, gbLow, gbHigh),
      2000, 0., 2000., 2000, 0., 2000.);
   // plus hitpatterns for gamma-gamma and beta-gamma for single crystals
   fH2[slot]["gammaGammaHP"] = new TH2D("gammaGammaHP", "#gamma-#gamma hit pattern", 65, 0., 65., 65, 0., 65.);
   fH2[slot]["betaGammaHP"]  = new TH2D("betaGammaHP", "#beta-#gamma hit pattern", 21, 0., 21., 65, 0., 65.);
   fH2[slot]["addbackAddbackHP"] =
      new TH2D("addbackAddbackHP", "#gamma-#gamma hit pattern with addback", 65, 0., 65., 65, 0., 65.);
   fH2[slot]["betaAddbackHP"] = new TH2D("betaAddbackHP", "#beta-#gamma hit pattern with addback", 21, 0., 21., 65, 0., 65.);

   // same for event mixing
   for(int i = 0; i < static_cast<int>(fAngleCombinations.size()); ++i) {
      fH2[slot][Form("gammaGammaMixed%d", i)] =
         new TH2D(Form("gammaGammaMixed%d", i), Form("%.1f^{o}: #gamma-#gamma", fAngleCombinations[i].first), 2000, 0.,
                  2000., 2000, 0., 2000.);
      fH2[slot][Form("gammaGammaBetaMixed%d", i)] = new TH2D(
         Form("gammaGammaBetaMixed%d", i), Form("%.1f^{o}: #gamma-#gamma, #Deltat_{#gamma-#beta} = %.1f - %.1f",
                                                fAngleCombinations[i].first, gbLow, gbHigh),
         2000, 0., 2000., 2000, 0., 2000.);
   }
   for(int i = 0; i < static_cast<int>(fAngleCombinationsAddback.size()); ++i) {
      fH2[slot][Form("addbackAddbackMixed%d", i)] =
         new TH2D(Form("addbackAddbackMixed%d", i),
                  Form("%.1f^{o}: #gamma-#gamma with addback", fAngleCombinationsAddback[i].first), 2000, 0., 2000.,
                  2000, 0., 2000.);
      fH2[slot][Form("addbackAddbackBetaMixed%d", i)] =
         new TH2D(Form("addbackAddbackBetaMixed%d", i),
                  Form("%.1f^{o}: #gamma-#gamma with addback, #Deltat_{#gamma-#beta} = %.1f - %.1f",
                       fAngleCombinationsAddback[i].first, gbLow, gbHigh),
                  2000, 0., 2000., 2000, 0., 2000.);
   }
   fH2[slot]["gammaGammaMixed"] = new TH2D("gammaGammaMixed", "#gamma-#gamma", 2000, 0., 2000., 2000, 0., 2000.);
   fH2[slot]["gammaGammaBetaMixed"] =
      new TH2D("gammaGammaBetaMixed", Form("#gamma-#gamma, #Deltat_{#gamma-#beta} = %.1f - %.1f", gbLow, gbHigh), 2000,
               0., 2000., 2000, 0., 2000.);
   fH2[slot]["addbackAddbackMixed"] =
      new TH2D("addbackAddbackMixed", "#gamma-#gamma with addback", 2000, 0., 2000., 2000, 0., 2000.);
   fH2[slot]["addbackAddbackBetaMixed"] =
      new TH2D("addbackAddbackBetaMixed",
               Form("#gamma-#gamma with addback, #Deltat_{#gamma-#beta} = %.1f - %.1f", gbLow, gbHigh), 2000, 0., 2000.,
               2000, 0., 2000.);
   // plus hitpatterns for gamma-gamma and beta-gamma for single crystals
   fH2[slot]["gammaGammaHPMixed"] = new TH2D("gammaGammaHPMixed", "#gamma-#gamma hit pattern", 65, 0., 65., 65, 0., 65.);
   fH2[slot]["betaGammaHPMixed"]  = new TH2D("betaGammaHPMixed", "#beta-#gamma hit pattern", 21, 0., 21., 65, 0., 65.);
   fH2[slot]["addbackAddbackHPMixed"] =
      new TH2D("addbackAddbackHPMixed", "#gamma-#gamma hit pattern with addback", 65, 0., 65., 65, 0., 65.);
   fH2[slot]["betaAddbackHPMixed"] =
      new TH2D("betaAddbackHPMixed", "#beta-#gamma hit pattern with addback", 21, 0., 21., 65, 0., 65.);

   // additionally 1D spectra of gammas
   // for single crystal and addback
   // with and without coincident betas
   fH1[slot]["gammaEnergy"]     = new TH1D("gammaEnergy", "#gamma Singles", 12000, 0, 3000);
   fH1[slot]["gammaEnergyBeta"] = new TH1D("gammaEnergyBeta", "#gamma singles in rough #beta coincidence", 12000, 0, 3000);
   fH1[slot]["addbackEnergy"]   = new TH1D("addbackEnergy", "#gamma singles with addback", 12000, 0, 3000);
   fH1[slot]["addbackEnergyBeta"] = new TH1D("addbackEnergyBeta", "#gamma singles with addback in rough #beta coincidence", 12000, 0, 3000);

   // and timing spectra for gamma-gamma and beta-gamma
   fH1[slot]["gammaGammaTiming"]     = new TH1D("gammaGammaTiming", "#Deltat_{#gamma-#gamma}", 3000, 0., 3000.);
   fH1[slot]["betaGammaTiming"]      = new TH1D("betaGammaTiming", "#Deltat_{#beta-#gamma}", 2000, -1000., 1000.);
   fH1[slot]["addbackAddbackTiming"] = new TH1D("addbackAddbackTiming", "#Deltat_{#addback-#addback}", 2000, 0., 3000.);
   fH1[slot]["betaAddbackTiming"]    = new TH1D("betaAddbackTiming", "#Deltat_{#beta-#gamma}", 2000, -1000., 1000.);
}

void AngularCorrelationHelper::Exec(unsigned int slot, TGriffin& grif, TSceptar& scep)
{
   // without addback
   for(auto g1 = 0; g1 < grif.GetMultiplicity(); ++g1) {
      auto grif1 = grif.GetGriffinHit(g1);
      // check for coincident betas
      bool coincBeta = false;
      for(auto s = 0; s < scep.GetMultiplicity(); ++s) {
         auto scep1 = scep.GetSceptarHit(s);
         if(!coincBeta && gbLow <= grif1->GetTime() - scep1->GetTime() && grif1->GetTime() - scep1->GetTime() <= gbHigh)
            coincBeta = true;
         fH1[slot].at("betaGammaTiming")->Fill(scep1->GetTime() - grif1->GetTime());
         fH2[slot].at("betaGammaHP")->Fill(scep1->GetDetector(), grif1->GetArrayNumber());
      }
      fH1[slot].at("gammaEnergy")->Fill(grif1->GetEnergy());
      if(coincBeta) fH1[slot].at("gammaEnergyBeta")->Fill(grif1->GetEnergy());
      for(auto g2 = 0; g2 < grif.GetMultiplicity(); ++g2) {
         if(g1 == g2) continue;
         auto   grif2 = grif.GetGriffinHit(g2);
         double angle = grif1->GetPosition().Angle(grif2->GetPosition()) * 180. / TMath::Pi();
         if(angle < 0.0001) continue;
         auto   angleIndex = fAngleMap.lower_bound(angle - 0.0005);
         double ggTime     = TMath::Abs(grif1->GetTime() - grif2->GetTime());
         fH1[slot].at("gammaGammaTiming")->Fill(ggTime);
         fH2[slot].at("gammaGammaHP")->Fill(grif1->GetArrayNumber(), grif2->GetArrayNumber());

         if(ggTime < ggHigh) {
            fH2[slot].at("gammaGamma")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("gammaGamma%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            if(coincBeta) {
               fH2[slot].at("gammaGammaBeta")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at(Form("gammaGammaBeta%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            }
         } else if(bgLow < ggTime && ggTime < bgHigh) {
            fH2[slot].at("gammaGammaBG")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("gammaGammaBG%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            if(coincBeta) {
               fH2[slot].at("gammaGammaBetaBG")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at(Form("gammaGammaBetaBG%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            }
         }
      }
      // event mixing, we use the last event as second griffin
      for(auto g2 = 0; g2 < fLastGrif.GetMultiplicity(); ++g2) {
         if(g1 == g2) continue;
         auto   grif2 = fLastGrif.GetGriffinHit(g2);
         double angle = grif1->GetPosition().Angle(grif2->GetPosition()) * 180. / TMath::Pi();
         if(angle < 0.0001) continue;
         auto   angleIndex = fAngleMap.lower_bound(angle - 0.0005);
         double ggTime     = TMath::Abs(grif1->GetTime() - grif2->GetTime());
         fH2[slot].at("gammaGammaHPMixed")->Fill(grif1->GetArrayNumber(), grif2->GetArrayNumber());

         fH2[slot].at("gammaGammaMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
         fH2[slot].at(Form("gammaGammaMixed%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
         if(coincBeta) {
            fH2[slot].at("gammaGammaBetaMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("gammaGammaBetaMixed%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
         }
      }
   }
   // with addback
   for(auto g1 = 0; g1 < grif.GetAddbackMultiplicity(); ++g1) {
      auto grif1 = grif.GetAddbackHit(g1);
      // check for coincident betas
      bool coincBeta = false;
      for(auto s = 0; s < scep.GetMultiplicity(); ++s) {
         auto scep1 = scep.GetSceptarHit(s);
         if(!coincBeta && gbLow <= grif1->GetTime() - scep1->GetTime() && grif1->GetTime() - scep1->GetTime() <= gbHigh)
            coincBeta = true;
         fH1[slot].at("betaAddbackTiming")->Fill(scep1->GetTime() - grif1->GetTime());
         fH2[slot].at("betaAddbackHP")->Fill(scep1->GetDetector(), grif1->GetArrayNumber());
      }
      fH1[slot].at("addbackEnergy")->Fill(grif1->GetEnergy());
      if(coincBeta) fH1[slot].at("addbackEnergyBeta")->Fill(grif1->GetEnergy());
      for(auto g2 = 0; g2 < grif.GetAddbackMultiplicity(); ++g2) {
         if(g1 == g2) continue;
         auto   grif2 = grif.GetAddbackHit(g2);
         double angle = grif1->GetPosition().Angle(grif2->GetPosition()) * 180. / TMath::Pi();
         if(angle < 0.0001) continue;
         auto   angleIndex = fAngleMapAddback.lower_bound(angle - 0.0005);
         double ggTime     = TMath::Abs(grif1->GetTime() - grif2->GetTime());
         fH1[slot].at("addbackAddbackTiming")->Fill(ggTime);
         fH2[slot].at("addbackAddbackHP")->Fill(grif1->GetArrayNumber(), grif2->GetArrayNumber());

         if(ggTime < ggHigh) {
            fH2[slot].at("addbackAddback")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("addbackAddback%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            if(coincBeta) {
               fH2[slot].at("addbackAddbackBeta")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at(Form("addbackAddbackBeta%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            }
         } else if(bgLow < ggTime && ggTime < bgHigh) {
            fH2[slot].at("addbackAddbackBG")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("addbackAddbackBG%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            if(coincBeta) {
               fH2[slot].at("addbackAddbackBetaBG")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at(Form("addbackAddbackBetaBG%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            }
         }
      }
      // event mixing, we use the last event as second griffin
      for(auto g2 = 0; g2 < fLastGrif.GetAddbackMultiplicity(); ++g2) {
         if(g1 == g2) continue;
         auto   grif2 = fLastGrif.GetAddbackHit(g2);
         double angle = grif1->GetPosition().Angle(grif2->GetPosition()) * 180. / TMath::Pi();
         if(angle < 0.0001) continue;
         auto   angleIndex = fAngleMapAddback.lower_bound(angle - 0.0005);
         double ggTime     = TMath::Abs(grif1->GetTime() - grif2->GetTime());
         fH2[slot].at("addbackAddbackHPMixed")->Fill(grif1->GetArrayNumber(), grif2->GetArrayNumber());

         fH2[slot].at("addbackAddbackMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
         fH2[slot].at(Form("addbackAddbackMixed%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
         if(coincBeta) {
            fH2[slot].at("addbackAddbackBetaMixed")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
            fH2[slot].at(Form("addbackAddbackBetaMixed%d", angleIndex->second))->Fill(grif1->GetEnergy(), grif2->GetEnergy());
         }
      }
   }

   // update "last" event
   fLastGrif = grif;
   fLastScep = scep;
}
