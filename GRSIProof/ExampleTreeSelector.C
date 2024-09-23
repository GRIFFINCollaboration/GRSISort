#define ExampleTreeSelector_cxx
// The class definition in ExampleTreeSelector.h has been generated automatically
#include "ExampleTreeSelector.h"

void ExampleTreeSelector::CreateHistograms()
{
   // define Trees
   fTree["coinc"]  = new TTree("coinc", "coincident events");
   fTree["timebg"] = new TTree("timebg", "time random events");

   // set branch addresses for output tree (these can be different for different trees)
   // we save the entry number as well to detect possible double counting
   // four coincident gammas a,b,c,d will be saved as a,b,c, a,b,d, a,c,d, and b,c,d
   fTree["coinc"]->Branch("energy", &fSuppressedAddback);
   fTree["coinc"]->Branch("timing", &fBetaGammaTiming);
   fTree["coinc"]->Branch("entry", &fEntry, "entry/L");

   fTree["timebg"]->Branch("energy", &fSuppressedAddback);
   fTree["timebg"]->Branch("timing", &fBetaGammaTiming);
   fTree["timebg"]->Branch("entry", &fEntry, "entry/L");

   // We can also create histograms at the same time, maybe for some diagnostics or simple checks
   fH1["asE"]         = new TH1D("asE", "suppressed Addback Singles", 12000, 0, 3000);
   fH2["sceptarMult"] = new TH2D("sceptarMult", "Sceptar multiplicity vs. Griffin multiplicity", 64, 0, 64, 40, 0, 40);
   fH2["zdsMult"]     = new TH2D("zdsMult", "ZeroDegree multiplicity vs. Griffin multiplicity", 64, 0, 64, 10, 0, 10);

   // Send histograms to Output list to be added and written.
   for(auto it : fH1) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fH2) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fTree) {
      GetOutputList()->Add(it.second);
   }
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

void ExampleTreeSelector::FillHistograms()
{
   // we could check multiplicities here and skip events where we do not have at least
   // three suppressed addback energies and a beta, but we want to fill some general
   // histograms without these cuts.

   // clear the vectors and other variables
   fSuppressedAddback.resize(3, 0.);
   fBetaGammaTiming.resize(3, -1e6);

   fH2.at("sceptarMult")->Fill(fGrif->GetSuppressedAddbackMultiplicity(fGriffinBgo), fScep->GetMultiplicity());
   fH2.at("zdsMult")->Fill(fGrif->GetSuppressedAddbackMultiplicity(fGriffinBgo), fZds->GetMultiplicity());

   // Loop over all suppressed addback Griffin Hits
   for(auto i = 0; i < fGrif->GetSuppressedAddbackMultiplicity(fGriffinBgo); ++i) {
      auto grif1 = fGrif->GetSuppressedAddbackHit(i);
      fH1.at("asE")->Fill(grif1->GetEnergy());
      fSuppressedAddback[0] = grif1->GetEnergy();
      for(auto j = i + 1; j < fGrif->GetSuppressedAddbackMultiplicity(fGriffinBgo); ++j) {
         auto grif2            = fGrif->GetSuppressedAddbackHit(j);
         fSuppressedAddback[1] = grif2->GetEnergy();
         for(auto k = j + 1; k < fGrif->GetSuppressedAddbackMultiplicity(fGriffinBgo); ++k) {
            auto grif3            = fGrif->GetSuppressedAddbackHit(k);
            fSuppressedAddback[2] = grif3->GetEnergy();
            // we now have three suppressed addback hits i, j, and k so now we need a coincident beta-tag
            bool foundBeta = false;
            for(auto b = 0; b < fZds->GetMultiplicity(); ++b) {
               auto beta = fZds->GetZeroDegreeHit(b);
               if(b == 0) {
                  // use the time of the first beta as reference in case we don't find a coincident beta
                  fBetaGammaTiming[0] = grif1->GetTime() - beta->GetTime();
                  fBetaGammaTiming[1] = grif2->GetTime() - beta->GetTime();
                  fBetaGammaTiming[2] = grif3->GetTime() - beta->GetTime();
               }
               if(PromptCoincidence(grif1, beta) && PromptCoincidence(grif2, beta) && PromptCoincidence(grif3, beta)) {
                  foundBeta           = true;
                  fBetaGammaTiming[0] = grif1->GetTime() - beta->GetTime();
                  fBetaGammaTiming[1] = grif2->GetTime() - beta->GetTime();
                  fBetaGammaTiming[2] = grif3->GetTime() - beta->GetTime();
                  break;
               }
            }
            // only check sceptar if we haven't found a zds yet
            for(auto b = 0; !foundBeta && b < fScep->GetMultiplicity(); ++b) {
               auto beta = fScep->GetSceptarHit(b);
               if(b == 0) {
                  // use the time of the first beta as reference in case we don't find a coincident beta
                  fBetaGammaTiming[0] = grif1->GetTime() - beta->GetTime();
                  fBetaGammaTiming[1] = grif2->GetTime() - beta->GetTime();
                  fBetaGammaTiming[2] = grif3->GetTime() - beta->GetTime();
               }
               if(PromptCoincidence(grif1, beta) && PromptCoincidence(grif2, beta) && PromptCoincidence(grif3, beta)) {
                  foundBeta           = true;
                  fBetaGammaTiming[0] = grif1->GetTime() - beta->GetTime();
                  fBetaGammaTiming[1] = grif2->GetTime() - beta->GetTime();
                  fBetaGammaTiming[2] = grif3->GetTime() - beta->GetTime();
                  break;
               }
            }

            if(foundBeta) {
               fTree.at("coinc")->Fill();
            } else {
               fTree.at("timebg")->Fill();
            }
         }
      }
   }
}
