#define ExampleFragmentSelector_cxx
// The class definition in ExampleFragmentSelector.h has been generated automatically
#include "ExampleFragmentSelector.h"

#include "TH2.h"

//need to define these histograms either globally or as members of the class
TH2F* hp_charge;
TH2F* hp_energy;

void ExampleFragmentSelector::CreateHistograms()
{
   hp_charge = new TH2F("hp_charge","Channel vs Charge",128,0,128,24000,0,12000);
   hp_energy = new TH2F("hp_energy","Channel vs Energy",128,0,128,16000,0,8000);
   GetOutputList()->AddAll(gDirectory->GetList());
}

void ExampleFragmentSelector::FillHistograms()
{
	if(fFragment->GetKValue() == 700){
   	hp_charge->Fill(fFragment->GetChannelNumber(),fFragment->Charge()/700.);
   	hp_energy->Fill(fFragment->GetChannelNumber(),fFragment->GetEnergy());
	}
}

