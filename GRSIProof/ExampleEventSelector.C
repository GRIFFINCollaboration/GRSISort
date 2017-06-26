#define ExampleEventSelector_cxx
// The class definition in ExampleEventSelector.h has been generated automatically
#include "ExampleEventSelector.h"

void ExampleEventSelector::CreateHistograms()
{
   fH1["gE"]   = new TH1D("gE", "#gamma Singles", 12000, 0, 3000);
   fH1["gE_b"] = new TH1D("gE_b", "#gamma Singles in rough #beta coincidence", 12000, 0, 3000);

   for(auto it : fH1) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fH2) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fHSparse) {
      GetOutputList()->Add(it.second);
   }
}

void ExampleEventSelector::FillHistograms()
{
   for(auto i = 0; i < fGrif->GetMultiplicity(); ++i) {
      fH1["gE"]->Fill(fGrif->GetGriffinHit(i)->GetEnergy());
      if(fScep->GetMultiplicity()) fH1["gE_b"]->Fill(fGrif->GetGriffinHit(i)->GetEnergy());
   }
}
