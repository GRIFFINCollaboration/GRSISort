#define ExampleFragmentSelector_cxx
// The class definition in ExampleFragmentSelector.h has been generated automatically
#include "ExampleFragmentSelector.h"

#include "TH2.h"

//need to define these histograms either globally or as members of the class
TH2F* hp_chargelow;
TH2F* hp_energylow;
TH2F* hp_chargehigh;
TH2F* hp_energyhigh;

void ExampleFragmentSelector::CreateHistograms()
{
   hp_chargelow = new TH2F("hp_chargelow","Channel vs Charge",129,0,129,24000,0,12000);
   hp_chargehigh = new TH2F("hp_chargehigh","Channel vs Charge",129,0,129,24000,0,12000);
   hp_energylow = new TH2F("hp_energylow","Channel vs Energy",129,0,129,20000,0,10000);
   hp_energyhigh = new TH2F("hp_energyhigh","Channel vs Energy",129,0,129,20000,0,10000);
   GetOutputList()->AddAll(gDirectory->GetList());
}

void ExampleFragmentSelector::FillHistograms()
{
	if(fFragment->GetChannelNumber() < 64 && fFragment->GetKValue() == 700){
   	hp_chargelow->Fill(fFragment->GetChannelNumber(),fFragment->Charge()/700.);
   	hp_energylow->Fill(fFragment->GetChannelNumber(),fFragment->GetEnergy());
	}
	else if(fFragment->GetChannelNumber() >=64 && fFragment->GetKValue() == 700) {
   	hp_chargehigh->Fill(fFragment->GetChannelNumber(),fFragment->Charge()/700.);
   	hp_energyhigh->Fill(fFragment->GetChannelNumber(),fFragment->GetEnergy());
	}
}

