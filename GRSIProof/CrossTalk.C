#define CrossTalk_cxx
// The class definition in CrossTalk.h has been generated automatically
#include "CrossTalk.h"

#include "TH1.h"

bool pileup_reject = true;

const double gg_time_low = -200.;
const double gg_time_high = 300.;

//Beta gamma histograms
const double gb_time_low = -250.;
const double gb_time_high = 350.;


bool Addback(TGriffinHit& one, TGriffinHit& two) {
		return ((one.GetDetector() == two.GetDetector()) &&
		(std::fabs(one.GetTime() - two.GetTime()) < 300.));
}

bool PromptCoincidence(TGriffinHit* one, TGriffinHit *two){
   
   return ((two->GetTime() - one->GetTime()) >= gg_time_low) && ((two->GetTime() - one->GetTime()) <= gg_time_high);

}


void CrossTalk::CreateHistograms() {
	fH2.clear();
	for(int det_num=1; det_num<=16; ++det_num){
		std::string aedet_str = Form("aEdet%d",det_num);
		std::string gedet_str = Form("gEdet%d",det_num);
		std::string ae2det_str = Form("aE2det%d",det_num);
		fH1[aedet_str] = new TH1D(Form("aEdet%d",det_num),Form("Addback detector %d",det_num),2000,0,2000);
		fH1[gedet_str] = new TH1D(Form("geEdet%d",det_num),Form("Singles detector %d",det_num),2000,0,2000);
		fH1[ae2det_str] = new TH1D(Form("aE2det%d",det_num),Form("Addback with 2 hits, detector %d",det_num),2000,0,2000);
		fH1["aMult"] = new TH1D("aMult","addback multilpicity",20,0,20);
		for(int crys_1=0; crys_1 <4; ++crys_1){
			for(int crys_2=crys_1+1;crys_2<4;++crys_2){
				std::string name_str = Form("det_%d_%d_%d",det_num,crys_1,crys_2);
			 	const char* hist_name = name_str.c_str();
				std::cout << "Creating histogram: " << hist_name;
				fH2[name_str] = new TH2F(hist_name,hist_name,2000,0,2000,2000,0,2000); 
				std::cout << " at address: " << fH2[hist_name] << std::endl;
			}
		}
	}

	fH2["gE_chan"] = new TH2D("gE_chan","gE_chan",65,0,65,8000,0,2000); 
	fH1["aE"] = new TH1D("aE", "Summed Addback", 8000,0,2000);
	fH1["gE"] = new TH1D("gE", "Summed Singles", 8000,0,2000);
	fH1["gEnoCT"] = new TH1D("gEnoCT", "Singles, no CT correction", 8000,0,2000);
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

void CrossTalk::FillHistograms() {
	//find the multiplicity in each clover over the entire event
   //we do this because we want to force a multiplicity of 2
	Int_t det_multiplicity[17] = {0};
	for(auto gr1 = 0; gr1 < fGrif->GetMultiplicity(); ++gr1){
		++(det_multiplicity[fGrif->GetGriffinHit(gr1)->GetDetector()]);
	}
   for(auto gr1 = 0; gr1 < fGrif->GetMultiplicity(); ++gr1){
		if(pileup_reject && (fGrif->GetGriffinHit(gr1)->GetKValue() != 700)) continue; //This pileup number might have to change for other expmnts
		fH1[Form("gEdet%d",fGrif->GetGriffinHit(gr1)->GetDetector())]->Fill(fGrif->GetGriffinHit(gr1)->GetEnergy());
		fH2["gE_chan"]->Fill(fGrif->GetGriffinHit(gr1)->GetArrayNumber(),fGrif->GetGriffinHit(gr1)->GetEnergy());
		fH1["gE"]->Fill(fGrif->GetGriffinHit(gr1)->GetEnergy());
		fH1["gEnoCT"]->Fill(fGrif->GetGriffinHit(gr1)->GetNoCTEnergy());
		for(auto gr2 = gr1 + 1; gr2 < fGrif->GetMultiplicity(); ++gr2){
			if(pileup_reject && fGrif->GetGriffinHit(gr2)->GetKValue() != 700) continue; //This pileup number might have to change for other expmnts
			if((det_multiplicity[fGrif->GetGriffinHit(gr1)->GetDetector()] == 2) && Addback(*(fGrif->GetGriffinHit(gr1)), *(fGrif->GetGriffinHit(gr2)))){
				TGriffinHit *low_crys_hit, *high_crys_hit;
				if(fGrif->GetGriffinHit(gr1)->GetCrystal() < fGrif->GetGriffinHit(gr2)->GetCrystal()){
					low_crys_hit = fGrif->GetGriffinHit(gr1);
					high_crys_hit = fGrif->GetGriffinHit(gr2);
				}
				else{
					low_crys_hit = fGrif->GetGriffinHit(gr2);
					high_crys_hit = fGrif->GetGriffinHit(gr1);
				}
				fH2[Form("det_%d_%d_%d",low_crys_hit->GetDetector(),low_crys_hit->GetCrystal(),high_crys_hit->GetCrystal())]->Fill(low_crys_hit->GetNoCTEnergy(),high_crys_hit->GetNoCTEnergy());

			}
		}
   }
	
	for(auto gr1 = 0; gr1 < fGrif->GetAddbackMultiplicity(); ++gr1){
		if(pileup_reject && (fGrif->GetAddbackHit(gr1)->GetKValue() != 700)) continue; //This pileup number might have to change for other expmnts
		fH1["aE"]->Fill(fGrif->GetAddbackHit(gr1)->GetEnergy());
		fH1[Form("aEdet%d",fGrif->GetAddbackHit(gr1)->GetDetector())]->Fill(fGrif->GetAddbackHit(gr1)->GetEnergy());
		fH1["aMult"]->Fill(fGrif->GetNAddbackFrags(gr1));
		if(fGrif->GetNAddbackFrags(gr1) == 2)
			fH1[Form("aE2det%d",fGrif->GetAddbackHit(gr1)->GetDetector())]->Fill(fGrif->GetAddbackHit(gr1)->GetEnergy());
	}
}
