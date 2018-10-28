#define ExampleEventSelector_cxx
// The class definition in ExampleEventSelector.h has been generated automatically
#include "ExampleEventSelector.h"

void ExampleEventSelector::CreateHistograms() {
	//Define Histograms
   fH1["gE"] 	= new TH1D("gE","#gamma Singles",12000,0,3000);
   fH1["gE_b"] = new TH1D("gE_b","#gamma Singles in rough #beta coincidence",12000,0,3000);
	fH1["aE"]	= new TH1D("aE", "Addback Singles", 12000,0,3000);
	fH2["ggE"] 	= new TH2F("ggE","#gamma #gamma Coincidence",8192,0,4096.,8192,0,4096.);

	//Send histograms to Output list to be added and written.
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

bool PromptCoincidence(TGriffinHit *g, TSceptarHit *s){
	//Check if hits are less then 300 ns apart.
	return std::fabs(g->GetTime() - s->GetTime()) < 300.;
}

bool PromptCoincidence(TGriffinHit *g1, TGriffinHit *g2){
	//Check if hits are less then 500 ns apart.
	return std::fabs(g1->GetTime() - g2->GetTime()) < 500.;
}

void ExampleEventSelector::FillHistograms() {
	//Loop over all Griffin Hits
   for(auto i = 0; i < fGrif->GetMultiplicity(); ++i){
		auto grif1 = fGrif->GetGriffinHit(i);
      fH1.at("gE")->Fill(grif1->GetEnergy());
		// second loop over all Griffin Hits
		for(auto j = 0; j < fGrif->GetMultiplicity(); ++j){
			if(i == j) continue;
			auto grif2 = fGrif->GetGriffinHit(j);
			fH2.at("ggE")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
		}
		//Loop over all sceptar hits
		for(auto j = 0; j < fScep->GetMultiplicity(); ++j){
			if(PromptCoincidence(grif1,fScep->GetSceptarHit(j))){
         	fH1.at("gE_b")->Fill(grif1->GetEnergy());
			}
		}
   }
   for(auto i = 0; i < fGrif->GetAddbackMultiplicity(); ++i){
		auto grif1 = fGrif->GetAddbackHit(i);
      fH1.at("aE")->Fill(grif1->GetEnergy());
	}
}
