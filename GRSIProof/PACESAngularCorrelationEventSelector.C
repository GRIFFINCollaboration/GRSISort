#define PACESAngularCorrelationEventSelector_cxx
// The class definition in PACESAngularCorrelationEventSelector.h has been generated automatically
#include "PACESAngularCorrelationEventSelector.h"
//DEFS
std::vector<double> xtmp, ytmp;
double gePrompt = 400.;		//ns
double geRandom1 = 400.;	//ns
double geRandom2 = 2000.;	//ns
int gx=0, gy=500, gbins=500;
int px=0, py=500, pbins=500;
int event_mixing_depth = 11;
int check, lgsize; 
std::vector<TGriffin> lastgrif;

//TIMING CONDITIONS ::
bool CoincidenceCondition(TGriffinHit* hit_one, TPacesHit* hit_two){	
   return TMath::Abs(hit_one->GetTime() - hit_two->GetTime()) < gePrompt; 
}
bool BGCondition(TGriffinHit* hit_one, TPacesHit* hit_two){		
   return (TMath::Abs(hit_one->GetTime() - hit_two->GetTime()) < geRandom2) && (TMath::Abs(hit_one->GetTime() - hit_two->GetTime()) > geRandom1); 
}

void PACESAngularCorrelationEventSelector::CreateHistograms() {

	//Angle mapping diagostics - very important
	fH2["PACESAngleMap_Index"] = new TH2D("PACESAngleMap_Index","Correlated #gamma-e^{-} coincidence angle index vs map index; Index; Index",320,0,320,320,0,320);
	fH2["PACESAngleMap_Bin"] = new TH2D("PACESAngleMap_Bin","Correlated #gamma-e^{-} coincidence angle index vs angle bin; Index; Angle Bin",320,0,320,70,0,70);
	//Timing
        fH1["geCFD"] = new TH1D("geCFD","#gamma-PACES time difference; [ns]; Counts per 2 ns",2500,-2500,2500);
	fH1["geCFDMixed"] = new TH1D("geCFDMixed","#gamma-PACES time difference (Event Mixed); Time difference [ns]; Counts per 1000 ns",1000,0,1000000);
	//e-y matrices

	//for(int i = 0; i < static_cast<int>(fPACESAngleCombinations.size()); ++i) {
	for(int i = 0; i < (maxanglebin+1); ++i) {
           fH2[Form("gammaElectron%i",i)] = new TH2D(Form("gammaElectron%i",i),
           Form("index%i #gamma-e^{-}, |#Deltat_{#gamma-e^{-}}| < %.1f; GRIFFIN Energy (keV); PACES Energy (keV)",i,gePrompt),gbins,gx,gy,pbins,px,py);
           fH2[Form("gammaElectronBG%i", i)] = new TH2D(Form("gammaElectronBG%i", i), 
           Form("index%i #gamma-e^{-}, #Deltat_{#gamma-e^{-}} = %.1f - %.1f; GRIFFIN Energy (keV); PACES Energy (keV)",i,geRandom1,geRandom2),gbins,gx,gy,pbins,px,py);
           fH2[Form("gammaElectronMixed%d", i)] = new TH2D(Form("gammaElectronMixed%d", i), 
           Form("index%i: #gamma-#e^{-} (Event Mixed); GRIFFIN Energy (keV); PACES Energy (keV)",i),gbins,gx,gy,pbins,px,py);
	}

	for(auto it : fH1) {
		GetOutputList()->Add(it.second);
	}
	for(auto it : fH2) {
		GetOutputList()->Add(it.second);
	}
}

void PACESAngularCorrelationEventSelector::FillHistograms() {
		int thisrun = TGRSIRunInfo::Get()->RunNumber();

		for(auto j = 0; j < fPaces->GetMultiplicity(); ++j){

		//PACES numbering system begins at #1 for experiments circa October 2017 and at #0 preceding this time.
		//An offset is applied in the former case so that all PACES channels fall in the range 0-4.
		int pdet;
		if(thisrun<=runRef){
		   pdet = fPaces->GetPacesHit(j)->GetDetector();
		} else {
		   pdet = fPaces->GetPacesHit(j)->GetDetector()-1;
		}
		double ptime = fPaces->GetPacesHit(j)->GetTime();
		double pEn = fPaces->GetPacesHit(j)->GetEnergy();
		//non-linearity correction>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		for (int j = 0; j < dp; j++){
		   xtmp.push_back (x[pdet][j]); ytmp.push_back (y[pdet][j]);
		}

		TGraph *tmpgraph = new TGraph(xtmp.size(), &(xtmp[0]), &(ytmp[0]));
		double corr = tmpgraph->Eval(pEn);
		delete tmpgraph; xtmp.clear(); ytmp.clear();

		double pEnc = pEn-corr;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//ELECTRON-GAMMA COINCIDENCE (CORRELATED)
			for(auto i = 0; i < fGrif->GetMultiplicity(); ++i){
			int gdet = fGrif->GetGriffinHit(i)->GetDetector();
			int gcry = fGrif->GetGriffinHit(i)->GetCrystal();
			double gtime1 = fGrif->GetGriffinHit(i)->GetTime();
			double gEn = fGrif->GetGriffinHit(i)->GetEnergy();

			fH1["geCFD"]->Fill(gtime1-ptime);

			double combo = (64*pdet)+(((gdet-1)*4)+gcry);
			auto index = fPACESAngleMap.lower_bound(combo-0.5);
			int whichindex = int(index->first);
			int whichbin = int(index->second);
			fH2["PACESAngleMap_Index"]->Fill(combo, whichindex);		//check correct index is retrieved 
			fH2["PACESAngleMap_Bin"]->Fill(whichindex, whichbin);		//check this index falls in correct bin

				//for(int k = 0; k < static_cast<int>(fPACESAngleCombinations.size()); ++k){
				for(int k = 0; k < (maxanglebin+1); ++k){
				   if(k==whichbin){
            			      if(CoincidenceCondition(fGrif->GetGriffinHit(i),fPaces->GetPacesHit(j))){
				      fH2[Form("gammaElectron%i",whichbin)]->Fill(gEn, pEnc);
				      }
            			      if(BGCondition(fGrif->GetGriffinHit(i),fPaces->GetPacesHit(j))){
				      fH2[Form("gammaElectronBG%i",whichbin)]->Fill(gEn, pEnc);
				      }
				   break;
				   }
				}
			}//end GrifMult

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//ELECTRON-GAMMA COINCIDENCE (UNCORRELATED/EVENT-MIXED)
			check = (int)lastgrif.size();
	     		if(check>=event_mixing_depth) {

			//loop over all 'last' events in the vector, except the most recent event:
			for(auto lg = 0; lg < (check-1); ++lg) {
			int multLG = lastgrif.at(lg).GetMultiplicity();

				for(auto i = 0; i < multLG; ++i){
	        		TGriffinHit *ghit = lastgrif.at(lg).GetGriffinHit(i);

				int gdet = ghit->GetDetector();
				int gcry = ghit->GetCrystal();
				double gtime1 = ghit->GetTime();
				double gEn = ghit->GetEnergy();

				fH1["geCFDMixed"]->Fill(gtime1-ptime);

				double combo = (64*pdet)+(((gdet-1)*4)+gcry);
				auto index = fPACESAngleMap.lower_bound(combo-0.5);
				int whichindex = int(index->first);
				int whichbin = int(index->second);
			
					//for(int k = 0; k < static_cast<int>(fPACESAngleCombinations.size()); ++k){
					for(int k = 0; k < (maxanglebin+1); ++k){
			  		   if(k==whichbin){
			      		   fH2[Form("gammaElectronMixed%i",k)]->Fill(gEn, pEnc);
			      		   break;
			   		   }
					}
				}//end GrifMult
			}//end LastGrif vector
			}//end event_mixing_depth restriction
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		}//end PacesMult

    //Maintain a vector containing the last (N=event_mixing_depth) GRIFFIN physics events
    lastgrif.push_back(*fGrif);
    lgsize = (int)lastgrif.size();
    if(lgsize>event_mixing_depth) {
      lastgrif.erase(lastgrif.begin());
    }
    //END
}
