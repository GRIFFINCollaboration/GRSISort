#define BasicPACESEventSelector_cxx
// The class definition in BasicPACESEventSelector.h has been generated automatically
#include "BasicPACESEventSelector.h"
//DEFS
std::vector<double> xtmp, ytmp;
int px=0, py=2000, pbins=4000;	//paces energy bins

void BasicPACESEventSelector::CreateHistograms() {

	fH1["pE"] = new TH1D("pE","PACES summed singles",pbins,px,py);
	for(int i=0;i<5;++i){
		fH1[Form("pE_%i",i)] = new TH1D(Form("pE_%i",i),Form("PACES %i singles; Energy (keV); Counts per 0.33 keV",i),pbins,px,py);
		fH2[Form("pE_NonLinearity%i",i)] = new TH2D(Form("pE_NonLinearity%i",i),Form("PACES %i energy vs residual; Energy (keV); Residual (keV)",i),pbins,px,py,400,-20,20);
	}

	for(auto it : fH1) {
		GetOutputList()->Add(it.second);
	}
	for(auto it : fH2) {
		GetOutputList()->Add(it.second);
	}
}

void BasicPACESEventSelector::FillHistograms() {
		int thisrun = TGRSIRunInfo::Get()->RunNumber();

		if(fPaces){
		for(auto i = 0; i < fPaces->GetMultiplicity(); ++i){

		//PACES numbering system begins at #1 for experiments circa October 2017 and at #0 preceding this time.
		//An offset is applied in the former case so that all PACES channels fall in the range 0-4.
		int pdet;
		if(thisrun<=runRef){
		   pdet = fPaces->GetPacesHit(i)->GetDetector();
		} else {
		   pdet = fPaces->GetPacesHit(i)->GetDetector()-1;
		}
		double pEn = fPaces->GetPacesHit(i)->GetEnergy();
		//non-linearity correction>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		for (int j = 0; j < dp; j++){
		   xtmp.push_back (x[pdet][j]); ytmp.push_back (y[pdet][j]);
		}

		TGraph *tmpgraph = new TGraph(xtmp.size(), &(xtmp[0]), &(ytmp[0]));
		double corr = tmpgraph->Eval(pEn);
		delete tmpgraph; xtmp.clear(); ytmp.clear();

		double pEnc = pEn-corr;
		fH1[Form("pE_%i",pdet)]->Fill(pEnc);
		fH2[Form("pE_NonLinearity%i",pdet)]->Fill(pEn,corr);
		fH1["pE"]->Fill(pEnc);
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		}
		}
}
