#define ExampleEventSelector_cxx
// The class definition in ExampleEventSelector.h has been generated automatically
#include "ExampleEventSelector.h"

<<<<<<< HEAD
void ExampleEventSelector::CreateHistograms() {
	//Define Histograms
   fH1["gE"] 	= new TH1D("gE","#gamma Singles",12000,0,3000);
   fH1["gE_b"] = new TH1D("gE_b","#gamma Singles in rough #beta coincidence",12000,0,3000);
	fH1["aE"]	= new TH1D("aE", "Addback Singles", 12000,0,3000);
	fH2["ggE"] 	= new TH2F("ggE","#gamma #gamma Coincidence",6000,0,3000,6000,0,3000);

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
      fH1["gE"]->Fill(fGrif->GetGriffinHit(i)->GetEnergy());
		//Loop over all sceptar hits
		for(auto j = 0; j < fScep->GetMultiplicity(); ++j){
			if(PromptCoincidence(fGrif->GetGriffinHit(i),fScep->GetSceptarHit(j))){
         	fH1["gE_b"]->Fill(fGrif->GetGriffinHit(i)->GetEnergy());
			}
		}
   }
}
