#define GriffinKValueSelector_cxx
// The class definition in GriffinKValueSelector.h has been generated automatically
#include "GriffinKValueSelector.h"

int maxKValue = 720;

void GriffinKValueSelector::CreateHistograms()
{
	// 1D spectra of energy
   fH1["hE"] =      new TH1D("hE", "Energy of all hits;energy [keV];counts/0.5 keV", 4000, 0., 2000.);
   fH1["hE_noPU"] = new TH1D("hE_noPU", "Energy (no pileup);energy [keV];counts/0.5 keV", 4000, 0., 2000.);
   fH1["hE_2h"] =   new TH1D("hE_2h", "Energy (pileup of 2 hits);energy [keV];counts/0.5 keV", 4000, 0., 2000.);
   fH1["hE_3h"] =   new TH1D("hE_3h", "Energy (pileup of 3 hits);energy [keV];counts/0.5 keV", 4000, 0., 2000.);
   fH1["hE_2hc"] =  new TH1D("hE_2hc", "Energy (pileup of 2 hits with only 2 integration windows);energy [keV];counts/0.5 keV", 4000, 0., 2000.);
	// 2D spectra of energy vs. k-value
   fH2["hKP"] =       new TH2D("hKP", "K-value vs. # of pileups;# of pileups;k-value", 50, -32.5, 17.5, maxKValue, 0., maxKValue);
   fH2["hEK_noPU"] =  new TH2D("hEK_noPU", "Energy vs. k-value (no pileup);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2["hEK_1h2"] =   new TH2D("hEK_1h2", "Energy vs. k-value (first of two piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2["hEK_2h2"] =   new TH2D("hEK_2h2", "Energy vs. k-value (second of two piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2["hEK_1h3"] =   new TH2D("hEK_1h3", "Energy vs. k-value (first of three piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2["hEK_2h3"] =   new TH2D("hEK_2h3", "Energy vs. k-value (second of three piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2["hEK_3h3"] =   new TH2D("hEK_3h3", "Energy vs. k-value (third of three piled-up hits);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2["hEK_1h2c"] =  new TH2D("hEK_1h2c", "Energy vs. k-value (first of two piled-up hits with only 2 integration windows);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);
   fH2["hEK_2h2c"] =  new TH2D("hEK_2h2c", "Energy vs. k-value (second of two piled-up hits with only 2 integration windows);k-value;energy [keV]", maxKValue, 0., maxKValue, 2000, 0., 2000.);

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

void GriffinKValueSelector::FillHistograms()
{
	if(fFragment->GetDetectorType() == 0) { //GRIFFIN detector
		fH1.at("hE")->Fill(fFragment->GetEnergy());
		fH2.at("hKP")->Fill(fFragment->GetNumberOfPileups(), fFragment->GetKValue());
		if(fFragment->GetNumberOfPileups() > 0) {
			fH1.at("hE_noPU")->Fill(fFragment->GetEnergy());
			fH2.at("hEK_noPU")->Fill(fFragment->GetKValue(), fFragment->GetEnergy());
		} else if(fFragment->GetNumberOfPileups() == -20) {
			fH1.at("hE_2h")->Fill(fFragment->GetEnergy());
			fH2.at("hEK_1h2")->Fill(fFragment->GetKValue(), fFragment->GetEnergy());
		} else if(fFragment->GetNumberOfPileups() == -21) {
			fH1.at("hE_2h")->Fill(fFragment->GetEnergy());
			fH2.at("hEK_2h2")->Fill(fFragment->GetKValue(), fFragment->GetEnergy());
		} else if(fFragment->GetNumberOfPileups() == -30) {
			fH1.at("hE_3h")->Fill(fFragment->GetEnergy());
			fH2.at("hEK_1h3")->Fill(fFragment->GetKValue(), fFragment->GetEnergy());
		} else if(fFragment->GetNumberOfPileups() == -31) {
			fH1.at("hE_3h")->Fill(fFragment->GetEnergy());
			fH2.at("hEK_2h3")->Fill(fFragment->GetKValue(), fFragment->GetEnergy());
		} else if(fFragment->GetNumberOfPileups() == -32) {
			fH1.at("hE_3h")->Fill(fFragment->GetEnergy());
			fH2.at("hEK_3h3")->Fill(fFragment->GetKValue(), fFragment->GetEnergy());
		} else if(fFragment->GetNumberOfPileups() == -200) {
			fH1.at("hE_2hc")->Fill(fFragment->GetEnergy());
			fH2.at("hEK_1h2c")->Fill(fFragment->GetKValue(), fFragment->GetEnergy());
		} else if(fFragment->GetNumberOfPileups() == -201) {
			fH1.at("hE_2hc")->Fill(fFragment->GetEnergy());
			fH2.at("hEK_2h2c")->Fill(fFragment->GetKValue(), fFragment->GetEnergy());
		}
	}
}
