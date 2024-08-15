#include "ExampleFragmentHelper.hh"

void ExampleFragmentHelper::CreateHistograms(unsigned int slot)
{
   H2()[slot]["hp_charge"] = new TH2D("hp_charge", "#gamma Charge vs. Channel", 128, 0., 128., 12000, 0., 12000.);
   H2()[slot]["hp_energy"] = new TH2D("hp_energy", "#gamma Energy vs. Channel", 128, 0., 128., 16000, 0., 8000.);
}

void ExampleFragmentHelper::Exec(unsigned int slot, TFragment& frag)
{
   H2()[slot].at("hp_charge")->Fill(frag.GetChannelNumber(), frag.GetCharge());
   H2()[slot].at("hp_energy")->Fill(frag.GetChannelNumber(), frag.GetEnergy());
}
