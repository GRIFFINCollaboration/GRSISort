#define ExampleEventSelector_cxx
// The class definition in ExampleEventSelector.h has been generated automatically
#include "ExampleEventSelector.h"

#include "TH1.h"

//need to define these histograms either globally or as members of the class
TH1D* gE;
TH1D* gE_b;

void ExampleEventSelector::CreateHistograms()
{
   gE = new TH1D("gE","#gamma Singles",12000,0,3000);
   gE_b = new TH1D("gE_b","#gamma Singles in rough #beta coincidence",12000,0,3000);
   GetOutputList()->AddAll(gDirectory->GetList());
}

void ExampleEventSelector::FillHistograms()
{
   for(auto i = 0; i < fGrif->GetMultiplicity(); ++i){
      gE->Fill(fGrif->GetGriffinHit(i)->GetEnergy());
      if(fScep->GetMultiplicity())
         gE_b->Fill(fGrif->GetGriffinHit(i)->GetEnergy());
   }


}
