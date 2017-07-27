#define ExampleFragmentSelector_cxx
// The class definition in ExampleFragmentSelector.h has been generated automatically
#include "ExampleFragmentSelector.h"

void ExampleFragmentSelector::CreateHistograms()
{
   fH2["hp_charge"] = new TH2D("hp_charge", "#gamma Charge vs. Channel", 128, 0., 128., 12000, 0., 12000.);
   fH2["hp_energy"] = new TH2D("hp_energy", "#gamma Energy vs. Channel", 128, 0., 128., 16000, 0., 8000.);

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

void ExampleFragmentSelector::FillHistograms()
{
   fH2["hp_charge"]->Fill(fFragment->GetChannelNumber(), fFragment->GetCharge());
   fH2["hp_energy"]->Fill(fFragment->GetChannelNumber(), fFragment->GetEnergy());
}
