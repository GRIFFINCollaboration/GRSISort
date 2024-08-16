#include "ComptonPolarimetryHelper.hh"

void ComptonPolarimetryHelper::CreateHistograms(unsigned int slot)
{
   // set up the queue for event mixing by filling it with empty events
   // we chose 10 here as a reasonable compromise between statistics and computing power/memory needed
   for(int i = 0; i < 10; ++i) {
      fGriffinDeque[slot].emplace_back(new TGriffin);
   }

   fH2[slot]["xiThetaDetCount"] = new TH2D("xiThetaDetCount",
               "Possible #xi Angles in GRIFFIN Array for coincidence angle #theta (measured from clover faces);Experimental Angle #xi (#circ);Coincidence Angle #theta (#circ);Counts",
               fXiBins, 0., 180.000001, fThetaBins, 0., 180.000001);
   fH2[slot]["xiThetaDet"] = new TH2D("xiThetaDet",
               "Measured #xi Angles in GRIFFIN Array for coincidence angle #theta (measured from clover faces);Experimental Angle #xi (#circ);Coincidence Angle #theta (#circ);Counts",
               fXiBins, 0., 180.000001, fThetaBins, 0., 180.000001);
   fH2[slot]["xiThetaDetMixed"] = new TH2D("xiThetaDetMixed",
               "Measured #xi Angles in GRIFFIN Array for mixed angle #theta (measured from clover faces);Experimental Angle #xi (#circ);Coincidence Angle #theta (#circ);Counts",
               fXiBins, 0., 180.000001, fThetaBins, 0., 180.000001);
   fH2[slot]["xiThetaCryCount"] = new TH2D("xiThetaCryCount",
               "Possible #xi Angles in GRIFFIN Array for coincidence angle #theta (measured from crystal positions);Experimental Angle #xi (#circ);Coincidence Angle #theta (#circ);Counts",
               fXiBins, 0., 180.000001, fThetaBins, 0., 180.000001);
   fH2[slot]["xiThetaCry"] = new TH2D("xiThetaCry",
               "Measured #xi Angles in GRIFFIN Array for coincidence angle #theta (measured from crystal positions);Experimental Angle #xi (#circ);Coincidence Angle #theta (#circ);Counts",
               fXiBins, 0., 180.000001, fThetaBins, 0., 180.000001);
   fH2[slot]["xiThetaCryMixed"] = new TH2D("xiThetaCryMixed",
               "Measured #xi Angles in GRIFFIN Array for mixed angle #theta (measured from crystal positions);Experimental Angle #xi (#circ);Coincidence Angle #theta (#circ);Counts",
               fXiBins, 0., 180.000001, fThetaBins, 0., 180.000001);

   fH1[slot]["gammaSingles"]   = new TH1D("gammaSingles", "#gamma singles (All Events);Energy [keV]", fBins, fMinEnergy, fMaxEnergy);
   fH2[slot]["gammagamma"]     = new TH2D("gammagamma", "#gamma - #gamma (All Events);Energy [keV];Energy [keV]", fBins,
                                          fMinEnergy, fMaxEnergy, fBins, fMinEnergy, fMaxEnergy);
   fH2[slot]["gammaCrystal"]   = new TH2D("gammaCrystal", "#gamma Crystal (All Events);Energy [keV];Crystal Number",
                                          fBins, fMinEnergy, fMaxEnergy, 64, 0, 64);
   fH2[slot]["xiCrystalCount"] = new TH2D("xiCrystalCount", "Possible #xi Crystal (All Events);#xi;Crystal Number",
                                          fXiBins, 0.0, 180.000001, 64, 0, 64);
   fH2[slot]["thetaCrystalCount"] =
      new TH2D("thetaCrystalCount", "Possible #theta Crystal (All Events);#theta;Crystal Number", fThetaBins, 0.0,
               180.000001, 64, 0, 64);
   fH2[slot]["xiCrystal"] =
      new TH2D("xiCrystal", "#xi Crystal (All Events);#xi;Crystal Number", fXiBins, 0.0, 180.000001, 64, 0, 64);
   fH2[slot]["thetaCrystal"] = new TH2D("thetaCrystal", "#theta Crystal (All Events);#theta;Crystal Number", fThetaBins,
                                        0.0, 180.000001, 64, 0, 64);
   fH2[slot]["gammaXi_g1"] =
      new TH2D("gammaXi_g1", "#xi - #gamma (Only #gamma_{1} from Triple Coincidence);#xi;Energy [keV]", fXiBins, 0.0,
               180.000001, fBins, fMinEnergy, fMaxEnergy);
   fH2[slot]["gammaXi_g2"] =
      new TH2D("gammaXi_g2", "#xi - #gamma (Only #gamma_{2} from Triple Coincidence);#xi;Energy [keV]", fXiBins, 0.0,
               180.000001, fBins, fMinEnergy, fMaxEnergy);
   fH1[slot]["gammaSingles_g1"] =
      new TH1D("gammaSingles_g1", "#gamma singles (Only #gamma_{1} from Triple Coincidence);Energy [keV]", fBins,
               fMinEnergy, fMaxEnergy);
   fH1[slot]["gammaSingles_g2"] =
      new TH1D("gammaSingles_g2", "#gamma singles (Only #gamma_{2} from Triple Coincidence);Energy [keV]", fBins,
               fMinEnergy, fMaxEnergy);
   fH1[slot]["gammaSingles_g3"] =
      new TH1D("gammaSingles_g3", "#gamma singles (Only #gamma_{3} from Triple Coincidence);Energy [keV]", fBins,
               fMinEnergy, fMaxEnergy);
   fH1[slot]["ggTimeDiff_g1g2"] = new TH1D("ggTimeDiff_g1g2", "#gamma_{1}-#gamma_{2} time difference", 300, 0, 300);
   fH1[slot]["ggTimeDiff_g1g3"] = new TH1D("ggTimeDiff_g1g3", "#gamma_{1}-#gamma_{3} time difference", 300, 0, 300);
   fH1[slot]["ggTimeDiff_g2g3"] = new TH1D("ggTimeDiff_g2g3", "#gamma_{2}-#gamma_{3} time difference", 300, 0, 300);

   // fill histograms with counts of occurance of angles (only once for the first slot)
   if(slot == 0) {
		//TODO rewrite as detector and crystal loops
      for(int one = 0; one < 64; ++one) {
         // loop over each crystal
         if(ExcludeDetector(one / 4 + 1) || ExcludeCrystal(one)) { continue; }
         TVector3 v1 = TGriffin::GetPosition(one / 4 + 1, one % 4, fGriffinDistance);
         TVector3 d1 = TGriffin::GetPosition(one / 4 + 1, 5, fGriffinDistance);
         for(int two = (one / 4) * 4; two < (one / 4 + 1) * 4; ++two) {
            // loop over the other crystals in the same detector as one
            if(one == two) { continue; }
            TVector3 v2 = TGriffin::GetPosition(two / 4 + 1, two % 4, fGriffinDistance);
            for(int three = 0; three < 64; ++three) {
               // loop over all crystals not in the same detector as one
               if(three / 4 == one / 4) { continue; }
               if(ExcludeDetector(three / 4 + 1) || ExcludeCrystal(three)) { continue; }
               TVector3 v3 = TGriffin::GetPosition(three / 4 + 1, three % 4, fGriffinDistance);
               TVector3 d3 = TGriffin::GetPosition(three / 4 + 1, 5, fGriffinDistance);

               TVector3 n1    = v3.Cross(v1);
               TVector3 n2    = v1.Cross(v2);
               double   xi    = n1.Angle(n2) * TMath::RadToDeg();
               double   theta = v3.Angle(v1) * TMath::RadToDeg();

               std::cout << one << ", " << two << ", " << three << ": " << xi << ", "
                         << d1.Angle(d3) * TMath::RadToDeg() << ", " << theta << std::endl;
               fH2[slot]["xiThetaDetCount"]->Fill(xi, d1.Angle(d3) * TMath::RadToDeg());
               fH2[slot]["xiThetaCryCount"]->Fill(xi, theta);
               fH2[slot]["xiCrystalCount"]->Fill(xi, one);
               fH2[slot]["thetaCrystalCount"]->Fill(theta, one);
            }
         }
      }
   }
}

void ComptonPolarimetryHelper::Exec(unsigned int slot, TGriffin& fGriffin, TGriffinBgo& fGriffinBgo)
{
   for(auto g1 = 0; g1 < fGriffin.GetMultiplicity(); ++g1) {
      auto grif1 = fGriffin.GetGriffinHit(g1);
      if(ExcludeDetector(grif1->GetDetector()) || ExcludeCrystal(grif1->GetArrayNumber())) { continue; }

      fH1[slot].at("gammaSingles")->Fill(grif1->GetEnergy());
      fH2[slot].at("gammaCrystal")->Fill(grif1->GetEnergy(), grif1->GetArrayNumber() - 1);
      for(auto g2 = 0; g2 < fGriffin.GetMultiplicity(); ++g2) {
         if(g1 == g2) { continue; }
         auto grif2 = fGriffin.GetGriffinHit(g2);
         if(ExcludeDetector(grif2->GetDetector()) || ExcludeCrystal(grif2->GetArrayNumber())) { continue; }

         fH2[slot].at("gammagamma")->Fill(grif1->GetEnergy(), grif2->GetEnergy());

         // skip hits not in the same detector, in the same crystal, or the wrong order of energies (the reverse will be
         // looped over again), or not coincident
         if(grif1->GetDetector() != grif2->GetDetector() || grif1->GetCrystal() == grif2->GetCrystal() ||
            grif1->GetEnergy() < grif2->GetEnergy() || !Coincident(grif1, grif2)) {
				continue;
			}

         // check that the sum energy of the two hits matches one of our gamma gates
         int index = CheckEnergy(grif1->GetEnergy() + grif2->GetEnergy());
         if(index < 0) { continue; }

         TVector3 v1 = grif1->GetPosition(fGriffinDistance);
         TVector3 v2 = grif2->GetPosition(fGriffinDistance);
         TVector3 d1 = TGriffin::GetPosition(grif1->GetDetector(), 5, fGriffinDistance);

         for(auto g3 = 0; g3 < fGriffin.GetMultiplicity(); ++g3) {
            if(g1 == g3 || g2 == g3) { continue; }
            auto grif3 = fGriffin.GetGriffinHit(g3);
            if(ExcludeDetector(grif3->GetDetector()) || ExcludeCrystal(grif3->GetArrayNumber())) { continue; }
            // skip hits in the same detector as hits 1 and 2
            if(grif1->GetDetector() == grif3->GetDetector()) { continue; }
            // check that the energy of this gamma ray matches the other gamma gate
            if(CheckEnergy(grif3->GetEnergy(), 1 - index) < 0) { continue; }
            // fill time histograms
            // check that 1 and 3 are coincident
            fH1[slot].at("ggTimeDiff_g1g2")->Fill(TimeDiff(grif1, grif2));
            fH1[slot].at("ggTimeDiff_g1g3")->Fill(TimeDiff(grif1, grif3));
            fH1[slot].at("ggTimeDiff_g2g3")->Fill(TimeDiff(grif2, grif3));
            if(!Coincident(grif1, grif3)) { continue; }

            TVector3 v3 = grif3->GetPosition(fGriffinDistance);
            TVector3 d3 = TGriffin::GetPosition(grif3->GetDetector(), 5, fGriffinDistance);

            TVector3 n1 = v3.Cross(v1);
            TVector3 n2 = v1.Cross(v2);

            double xi    = n1.Angle(n2) * TMath::RadToDeg();
            double theta = v3.Angle(v1) * TMath::RadToDeg();

            fH2[slot].at("xiThetaCry")->Fill(xi, theta);
            fH2[slot].at("xiThetaDet")->Fill(xi, d1.Angle(d3) * TMath::RadToDeg());
            fH2[slot].at("xiCrystal")->Fill(xi, grif1->GetArrayNumber() - 1);
            fH2[slot].at("thetaCrystal")->Fill(theta, grif1->GetArrayNumber() - 1);

            fH2[slot].at("gammaXi_g1")->Fill(xi, grif1->GetEnergy());
            fH2[slot].at("gammaXi_g2")->Fill(xi, grif2->GetEnergy());
            fH1[slot].at("gammaSingles_g1")->Fill(grif1->GetEnergy());
            fH1[slot].at("gammaSingles_g2")->Fill(grif2->GetEnergy());
            fH1[slot].at("gammaSingles_g3")->Fill(grif3->GetEnergy());
         }   // for g3

         // Event mixing: loop over all stored events for this thread/slot and use them as the "third" gamma ray
         for(auto l = 0; l < fGriffinDeque[slot].size(); ++l) {
            auto fLastGriffin = fGriffinDeque[slot][l];
            for(auto g3 = 0; g3 < fLastGriffin->GetMultiplicity(); ++g3) {
               if(g1 == g3 || g2 == g3) { continue; }
               auto grif3 = fLastGriffin->GetGriffinHit(g3);
               if(ExcludeDetector(grif3->GetDetector()) || ExcludeCrystal(grif3->GetArrayNumber())) { continue; }
               // skip hits in the same detector as hits 1 and 2
               if(grif1->GetDetector() == grif3->GetDetector()) { continue; }
               // check that the energy of this gamma ray matches the other gamma gate
               if(CheckEnergy(grif3->GetEnergy(), 1 - index) < 0) { continue; }

               TVector3 v3 = grif3->GetPosition(fGriffinDistance);
               TVector3 d3 = TGriffin::GetPosition(grif3->GetDetector(), 5, fGriffinDistance);

               TVector3 n1 = v3.Cross(v1);
               TVector3 n2 = v1.Cross(v2);

               double xi    = n1.Angle(n2) * TMath::RadToDeg();
               double theta = v3.Angle(v1) * TMath::RadToDeg();

               fH2[slot].at("xiThetaCryMixed")->Fill(xi, theta);
               fH2[slot].at("xiThetaDetMixed")->Fill(xi, d1.Angle(d3) * TMath::RadToDeg());
            }
         }
      }   // for g2
   }      // for g1

   // Add the current event to the queue, delete the oldest event in the queue, and pop the pointer from the queue.
   fGriffinDeque[slot].emplace_back(new TGriffin(fGriffin));

   delete fGriffinDeque[slot].front();

   fGriffinDeque[slot].pop_front();
}
