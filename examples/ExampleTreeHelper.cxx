#include "ExampleTreeHelper.hh"

void ExampleTreeHelper::CreateHistograms(unsigned int slot)
{
   // define Trees
   Tree()[slot].emplace("coinc", new TTree("coinc", "coincident events"));
   Tree()[slot]["timebg"] = new TTree("timebg", "time random events");

   fSuppressedAddback2[slot] = new double[3];
   // set branch addresses for output tree (these can be different for different trees)
   // four coincident gammas a,b,c,d will be saved as a,b,c, a,b,d, a,c,d, and b,c,d
   Tree()[slot]["coinc"]->Branch("energy", &(fSuppressedAddback2[slot]), "energy[3]/D");
   // Tree()[slot]["coinc"]->Branch("timing", &(fBetaGammaTiming[slot]));
   Tree()[slot]["coinc"]->Branch("mult", &(fGriffinMultiplicity[slot]), "mult/I");

   Tree()[slot]["timebg"]->Branch("energy", &(fSuppressedAddback[slot]));
   Tree()[slot]["timebg"]->Branch("timing", &(fBetaGammaTiming[slot]));

   // We can also create histograms at the same time, maybe for some diagnostics or simple checks
   H1()[slot]["asE"]         = new TH1D("asE", "suppressed Addback Singles", 12000, 0, 3000);
   H2()[slot]["sceptarMult"] = new TH2D("sceptarMult", "Sceptar multiplicity vs. Griffin multiplicity", 64, 0, 64, 40, 0, 40);
   H2()[slot]["zdsMult"]     = new TH2D("zdsMult", "ZeroDegree multiplicity vs. Griffin multiplicity", 64, 0, 64, 10, 0, 10);
}

bool PromptCoincidence(TGriffinHit* g, TZeroDegreeHit* z)
{
   // Check if hits are less then 300 ns apart.
   return std::fabs(g->GetTime() - z->GetTime()) < 300.;
}

bool PromptCoincidence(TGriffinHit* g, TSceptarHit* s)
{
   // Check if hits are less then 300 ns apart.
   return std::fabs(g->GetTime() - s->GetTime()) < 300.;
}

bool PromptCoincidence(TGriffinHit* g1, TGriffinHit* g2)
{
   // Check if hits are less then 500 ns apart.
   return std::fabs(g1->GetTime() - g2->GetTime()) < 500.;
}

void ExampleTreeHelper::Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo, TZeroDegree& zds, TSceptar& scep)
{
   // we could check multiplicities here and skip events where we do not have at least
   // three suppressed addback energies and a beta, but we want to fill some general
   // histograms without these cuts.

   // clear the vectors and other variables
   fSuppressedAddback[slot].resize(3, 0.);
   fBetaGammaTiming[slot].resize(3, -1e6);
   for(int i = 0; i < 3; ++i) fSuppressedAddback2[slot][i] = 0.;

   H2()[slot].at("sceptarMult")->Fill(grif.GetSuppressedAddbackMultiplicity(&grifBgo), scep.GetMultiplicity());
   H2()[slot].at("zdsMult")->Fill(grif.GetSuppressedAddbackMultiplicity(&grifBgo), zds.GetMultiplicity());

   // Loop over all suppressed addback Griffin Hits
   fGriffinMultiplicity[slot] = grif.GetSuppressedAddbackMultiplicity(&grifBgo);
   if(fGriffinMultiplicity[slot] < 3) { return; }
   for(auto i = 0; i < fGriffinMultiplicity[slot]; ++i) {
      auto grif1 = grif.GetSuppressedAddbackHit(i);
      H1()[slot].at("asE")->Fill(grif1->GetEnergy());
      fSuppressedAddback[slot][0]  = grif1->GetEnergy();
      fSuppressedAddback2[slot][0] = grif1->GetEnergy();
      for(auto j = i + 1; j < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++j) {
         auto grif2                   = grif.GetSuppressedAddbackHit(j);
         fSuppressedAddback[slot][1]  = grif2->GetEnergy();
         fSuppressedAddback2[slot][1] = grif2->GetEnergy();
         for(auto k = j + 1; k < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++k) {
            auto grif3                   = grif.GetSuppressedAddbackHit(k);
            fSuppressedAddback[slot][2]  = grif3->GetEnergy();
            fSuppressedAddback2[slot][2] = grif3->GetEnergy();
            // we now have three suppressed addback hits i, j, and k so now we need a coincident beta-tag
            bool foundBeta = false;
            for(auto b = 0; b < zds.GetMultiplicity(); ++b) {
               auto beta = zds.GetZeroDegreeHit(b);
               if(b == 0) {
                  // use the time of the first beta as reference in case we don't find a coincident beta
                  fBetaGammaTiming[slot][0] = grif1->GetTime() - beta->GetTime();
                  fBetaGammaTiming[slot][1] = grif2->GetTime() - beta->GetTime();
                  fBetaGammaTiming[slot][2] = grif3->GetTime() - beta->GetTime();
               }
               if(PromptCoincidence(grif1, beta) && PromptCoincidence(grif2, beta) && PromptCoincidence(grif3, beta)) {
                  foundBeta                 = true;
                  fBetaGammaTiming[slot][0] = grif1->GetTime() - beta->GetTime();
                  fBetaGammaTiming[slot][1] = grif2->GetTime() - beta->GetTime();
                  fBetaGammaTiming[slot][2] = grif3->GetTime() - beta->GetTime();
                  break;
               }
            }
            // only check sceptar if we haven't found a zds yet
            for(auto b = 0; !foundBeta && b < scep.GetMultiplicity(); ++b) {
               auto beta = scep.GetSceptarHit(b);
               if(b == 0) {
                  // use the time of the first beta as reference in case we don't find a coincident beta
                  fBetaGammaTiming[slot][0] = grif1->GetTime() - beta->GetTime();
                  fBetaGammaTiming[slot][1] = grif2->GetTime() - beta->GetTime();
                  fBetaGammaTiming[slot][2] = grif3->GetTime() - beta->GetTime();
               }
               if(PromptCoincidence(grif1, beta) && PromptCoincidence(grif2, beta) && PromptCoincidence(grif3, beta)) {
                  foundBeta                 = true;
                  fBetaGammaTiming[slot][0] = grif1->GetTime() - beta->GetTime();
                  fBetaGammaTiming[slot][1] = grif2->GetTime() - beta->GetTime();
                  fBetaGammaTiming[slot][2] = grif3->GetTime() - beta->GetTime();
                  break;
               }
            }

            if(foundBeta) {
               Tree()[slot].at("coinc")->Fill();
            } else {
               Tree()[slot].at("timebg")->Fill();
            }
         }
      }
   }
}
