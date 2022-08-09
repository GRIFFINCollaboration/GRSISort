#include "GriffinKValueHelper.hh"

int maxKValue = 720;

void GriffinKValueHelper::CreateHistograms(unsigned int slot)
{
	// 1D spectra of energy
   fH1[slot]["hE"] =      new TH1D("hE", "Energy of all hits;energy [keV];counts/0.5 keV", 4000, 0., 2000.);
   fH1[slot]["hE_noPU"] = new TH1D("hE_noPU", "Energy (no pileup);energy [keV];counts/0.5 keV", 4000, 0., 2000.);
   fH1[slot]["hE_2h"] =   new TH1D("hE_2h", "Energy (pileup of 2 hits);energy [keV];counts/0.5 keV", 4000, 0., 2000.);
   fH1[slot]["hE_3h"] =   new TH1D("hE_3h", "Energy (pileup of 3 hits);energy [keV];counts/0.5 keV", 4000, 0., 2000.);
   fH1[slot]["hE_2hc"] =  new TH1D("hE_2hc", "Energy (pileup of 2 hits with only 2 integration windows);energy [keV];counts/0.5 keV", 4000, 0., 2000.);
	// 2D spectra of energy vs. k-value
   fH2[slot]["hKP"] =       new TH2D("hKP", "K-value vs. # of pileups;# of pileups;k-value", 50, -32.5, 17.5, maxKValue, 0., maxKValue);
   fH2[slot]["hEK_noPU"] =  new TH2D("hEK_noPU", "Energy vs. k-value (no pileup);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2[slot]["hEK_1h2"] =   new TH2D("hEK_1h2", "Energy vs. k-value (first of two piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2[slot]["hEK_2h2"] =   new TH2D("hEK_2h2", "Energy vs. k-value (second of two piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2[slot]["hEK_1h3"] =   new TH2D("hEK_1h3", "Energy vs. k-value (first of three piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2[slot]["hEK_2h3"] =   new TH2D("hEK_2h3", "Energy vs. k-value (second of three piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2[slot]["hEK_3h3"] =   new TH2D("hEK_3h3", "Energy vs. k-value (third of three piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2[slot]["hEK_1h2c"] =  new TH2D("hEK_1h2c", "Energy vs. k-value (first of two piled-up hits with only 2 integration windows);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2[slot]["hEK_2h2c"] =  new TH2D("hEK_2h2c", "Energy vs. k-value (second of two piled-up hits with only 2 integration windows);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
}

void GriffinKValueHelper::Exec(unsigned int slot, TFragment& frag)
{
	if(frag.GetDetectorType() == 0) { //GRIFFIN detector
		fH1[slot].at("hE")->Fill(frag.GetEnergy());
		fH2[slot].at("hKP")->Fill(frag.GetNumberOfPileups(), frag.GetKValue());
		if(frag.GetNumberOfPileups() > 0) {
			fH1[slot].at("hE_noPU")->Fill(frag.GetEnergy());
			fH2[slot].at("hEK_noPU")->Fill(frag.GetKValue(), frag.GetEnergy());
		} else if(frag.GetNumberOfPileups() == -20) {
			fH1[slot].at("hE_2h")->Fill(frag.GetEnergy());
			fH2[slot].at("hEK_1h2")->Fill(frag.GetKValue(), frag.GetEnergy());
		} else if(frag.GetNumberOfPileups() == -21) {
			fH1[slot].at("hE_2h")->Fill(frag.GetEnergy());
			fH2[slot].at("hEK_2h2")->Fill(frag.GetKValue(), frag.GetEnergy());
		} else if(frag.GetNumberOfPileups() == -30) {
			fH1[slot].at("hE_3h")->Fill(frag.GetEnergy());
			fH2[slot].at("hEK_1h3")->Fill(frag.GetKValue(), frag.GetEnergy());
		} else if(frag.GetNumberOfPileups() == -31) {
			fH1[slot].at("hE_3h")->Fill(frag.GetEnergy());
			fH2[slot].at("hEK_2h3")->Fill(frag.GetKValue(), frag.GetEnergy());
		} else if(frag.GetNumberOfPileups() == -32) {
			fH1[slot].at("hE_3h")->Fill(frag.GetEnergy());
			fH2[slot].at("hEK_3h3")->Fill(frag.GetKValue(), frag.GetEnergy());
		} else if(frag.GetNumberOfPileups() == -200) {
			fH1[slot].at("hE_2hc")->Fill(frag.GetEnergy());
			fH2[slot].at("hEK_1h2c")->Fill(frag.GetKValue(), frag.GetEnergy());
		} else if(frag.GetNumberOfPileups() == -201) {
			fH1[slot].at("hE_2hc")->Fill(frag.GetEnergy());
			fH2[slot].at("hEK_2h2c")->Fill(frag.GetKValue(), frag.GetEnergy());
		}
	}
}
