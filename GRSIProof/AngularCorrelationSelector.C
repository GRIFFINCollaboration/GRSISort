#define AngularCorrelationSelector_cxx
// The class definition in AngularCorrelationSelector.h has been generated automatically
#include "AngularCorrelationSelector.h"

double gbLow = -50.; //min. time difference gamma-beta
double gbHigh = 500.; //max. time difference gamma-beta
double ggHigh = 400.; //max. absolute time difference gamma-gamma
double bgLow = 1000.; //min. time difference gamma-gamma background
double bgHigh = 2000.; //max. time difference gamma-gamma background

void AngularCorrelationSelector::CreateHistograms() {
	std::cout<<"creating histograms ..."<<std::endl;
	//for each angle (and the sum) we want
	//for single crystal and addback
	//with and without coincident betas
	//coincident and time-random gamma-gamma
	for(int i = 0; i < static_cast<int>(fAngleCombinations.size()); ++i) {
		fH2[Form("gammaGamma%d", i)] = new TH2D(Form("gammaGamma%d", i), Form("%.1f^{o}: #gamma-#gamma, |#Deltat_{#gamma-#gamma}| < %.1f", fAngleCombinations[i].first, ggHigh), 2000, 0., 2000., 2000, 0., 2000.);
		fH2[Form("gammaGammaBeta%d", i)] = new TH2D(Form("gammaGammaBeta%d", i), Form("%.1f^{o}: #gamma-#gamma, |#Deltat_{#gamma-#gamma}| < %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f", fAngleCombinations[i].first, ggHigh, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
		fH2[Form("gammaGammaBG%d", i)] = new TH2D(Form("gammaGammaBG%d", i), Form("%.1f^{o}: #gamma-#gamma, #Deltat_{#gamma-#gamma} = %.1f - %.1f", fAngleCombinations[i].first, bgLow, bgHigh), 2000, 0., 2000., 2000, 0., 2000.);
		fH2[Form("gammaGammaBetaBG%d", i)] = new TH2D(Form("gammaGammaBetaBG%d", i), Form("%.1f^{o}: #gamma-#gamma, #Deltat_{#gamma-#gamma} = %.1f - %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f", fAngleCombinations[i].first, bgLow, bgHigh, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	}	
	for(int i = 0; i < static_cast<int>(fAngleCombinationsAddback.size()); ++i) {
		fH2[Form("addbackAddback%d", i)] = new TH2D(Form("addbackAddback%d", i), Form("%.1f^{o}: #gamma-#gamma with addback, |#Deltat_{#gamma-#gamma}| < %.1f", fAngleCombinationsAddback[i].first, ggHigh), 2000, 0., 2000., 2000, 0., 2000.);
		fH2[Form("addbackAddbackBeta%d", i)] = new TH2D(Form("addbackAddbackBeta%d", i), Form("%.1f^{o}: #gamma-#gamma with addback, |#Deltat_{#gamma-#gamma}| < %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f", fAngleCombinationsAddback[i].first, ggHigh, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
		fH2[Form("addbackAddbackBG%d", i)] = new TH2D(Form("addbackAddbackBG%d", i), Form("%.1f^{o}: #gamma-#gamma with addback, #Deltat_{#gamma-#gamma} = %.1f - %.1f", fAngleCombinationsAddback[i].first, bgLow, bgHigh), 2000, 0., 2000., 2000, 0., 2000.);
		fH2[Form("addbackAddbackBetaBG%d", i)] = new TH2D(Form("addbackAddbackBetaBG%d", i), Form("%.1f^{o}: #gamma-#gamma with addback, #Deltat_{#gamma-#gamma} = %.1f - %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f", fAngleCombinationsAddback[i].first, bgLow, bgHigh, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	}
	fH2["gammaGamma"] = new TH2D("gammaGamma",Form("#gamma-#gamma, |#Deltat_{#gamma-#gamma}| < %.1f", ggHigh), 2000, 0., 2000., 2000, 0., 2000.);
	fH2["gammaGammaBeta"] = new TH2D("gammaGammaBeta",Form("#gamma-#gamma, |#Deltat_{#gamma-#gamma}| < %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f", ggHigh, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	fH2["gammaGammaBG"] = new TH2D("gammaGammaBG",Form("#gamma-#gamma, #Deltat_{#gamma-#gamma} = %.1f - %.1f", bgLow, bgHigh), 2000, 0., 2000., 2000, 0., 2000.);
	fH2["gammaGammaBetaBG"] = new TH2D("gammaGammaBetaBG",Form("#gamma-#gamma, #Deltat_{#gamma-#gamma} = %.1f - %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f", bgLow, bgHigh, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	fH2["addbackAddback"] = new TH2D("addbackAddback",Form("#gamma-#gamma with addback, |#Deltat_{#gamma-#gamma}| < %.1f", ggHigh), 2000, 0., 2000., 2000, 0., 2000.);
	fH2["addbackAddbackBeta"] = new TH2D("addbackAddbackBeta",Form("#gamma-#gamma with addback, |#Deltat_{#gamma-#gamma}| < %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f",ggHigh, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	fH2["addbackAddbackBG"] = new TH2D("addbackAddbackBG",Form("#gamma-#gamma with addback, #Deltat_{#gamma-#gamma} = %.1f - %.1f", bgLow, bgHigh), 2000, 0., 2000., 2000, 0., 2000.);
	fH2["addbackAddbackBetaBG"] = new TH2D("addbackAddbackBetaBG",Form("#gamma-#gamma with addback, #Deltat_{#gamma-#gamma} = %.1f - %.1f, #Deltat_{#gamma-#beta} = %.1f - %.1f", bgLow, bgHigh, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	//plus hitpatterns for gamma-gamma and beta-gamma for single crystals
	fH2["gammaGammaHP"] = new TH2D("gammaGammaHP","#gamma-#gamma hit pattern", 65, 0., 65., 65, 0., 65.);
	fH2["betaGammaHP"] = new TH2D("betaGammaHP","#beta-#gamma hit pattern", 21, 0., 21., 65, 0., 65.);
	fH2["addbackAddbackHP"] = new TH2D("addbackAddbackHP","#gamma-#gamma hit pattern with addback", 65, 0., 65., 65, 0., 65.);
	fH2["betaAddbackHP"] = new TH2D("betaAddbackHP","#beta-#gamma hit pattern with addback", 21, 0., 21., 65, 0., 65.);

	//same for event mixing
	for(int i = 0; i < static_cast<int>(fAngleCombinations.size()); ++i) {
		fH2[Form("gammaGammaMixed%d", i)] = new TH2D(Form("gammaGammaMixed%d", i), Form("%.1f^{o}: #gamma-#gamma", fAngleCombinations[i].first), 2000, 0., 2000., 2000, 0., 2000.);
		fH2[Form("gammaGammaBetaMixed%d", i)] = new TH2D(Form("gammaGammaBetaMixed%d", i), Form("%.1f^{o}: #gamma-#gamma, #Deltat_{#gamma-#beta} = %.1f - %.1f", fAngleCombinations[i].first, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	}	
	for(int i = 0; i < static_cast<int>(fAngleCombinationsAddback.size()); ++i) {
		fH2[Form("addbackAddbackMixed%d", i)] = new TH2D(Form("addbackAddbackMixed%d", i), Form("%.1f^{o}: #gamma-#gamma with addback", fAngleCombinationsAddback[i].first), 2000, 0., 2000., 2000, 0., 2000.);
		fH2[Form("addbackAddbackBetaMixed%d", i)] = new TH2D(Form("addbackAddbackBetaMixed%d", i), Form("%.1f^{o}: #gamma-#gamma with addback, #Deltat_{#gamma-#beta} = %.1f - %.1f", fAngleCombinationsAddback[i].first, gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	}
	fH2["gammaGammaMixed"] = new TH2D("gammaGammaMixed","#gamma-#gamma", 2000, 0., 2000., 2000, 0., 2000.);
	fH2["gammaGammaBetaMixed"] = new TH2D("gammaGammaBetaMixed",Form("#gamma-#gamma, #Deltat_{#gamma-#beta} = %.1f - %.1f", gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	fH2["addbackAddbackMixed"] = new TH2D("addbackAddbackMixed","#gamma-#gamma with addback", 2000, 0., 2000., 2000, 0., 2000.);
	fH2["addbackAddbackBetaMixed"] = new TH2D("addbackAddbackBetaMixed",Form("#gamma-#gamma with addback, #Deltat_{#gamma-#beta} = %.1f - %.1f", gbLow, gbHigh), 2000, 0., 2000., 2000, 0., 2000.);
	//plus hitpatterns for gamma-gamma and beta-gamma for single crystals
	fH2["gammaGammaHPMixed"] = new TH2D("gammaGammaHPMixed","#gamma-#gamma hit pattern", 65, 0., 65., 65, 0., 65.);
	fH2["betaGammaHPMixed"] = new TH2D("betaGammaHPMixed","#beta-#gamma hit pattern", 21, 0., 21., 65, 0., 65.);
	fH2["addbackAddbackHPMixed"] = new TH2D("addbackAddbackHPMixed","#gamma-#gamma hit pattern with addback", 65, 0., 65., 65, 0., 65.);
	fH2["betaAddbackHPMixed"] = new TH2D("betaAddbackHPMixed","#beta-#gamma hit pattern with addback", 21, 0., 21., 65, 0., 65.);

	//additionally 1D spectra of gammas
	//for single crystal and addback
	//with and without coincident betas
	fH1["gammaEnergy"] = new TH1D("gammaEnergy","#gamma Singles", 12000, 0, 3000);
	fH1["gammaEnergyBeta"] = new TH1D("gammaEnergyBeta","#gamma singles in rough #beta coincidence", 12000, 0, 3000);
	fH1["addbackEnergy"] = new TH1D("addbackEnergy","#gamma singles with addback", 12000, 0, 3000);
	fH1["addbackEnergyBeta"] = new TH1D("addbackEnergyBeta","#gamma singles with addback in rough #beta coincidence", 12000, 0, 3000);

	//and timing spectra for gamma-gamma and beta-gamma
	fH1["gammaGammaTiming"] = new TH1D("gammaGammaTiming","#Deltat_{#gamma-#gamma}", 3000, 0., 3000.);
	fH1["betaGammaTiming"] = new TH1D("betaGammaTiming","#Deltat_{#beta-#gamma}", 2000, -1000., 1000.);
	fH1["addbackAddbackTiming"] = new TH1D("addbackAddbackTiming","#Deltat_{#addback-#addback}", 2000, 0., 3000.);
	fH1["betaAddbackTiming"] = new TH1D("betaAddbackTiming","#Deltat_{#beta-#gamma}", 2000, -1000., 1000.);

	for(auto it : fH1) {
		GetOutputList()->Add(it.second);
	}
	for(auto it : fH2) {
		GetOutputList()->Add(it.second);
	}
	for(auto it : fHSparse) {
		GetOutputList()->Add(it.second);
	}
	std::cout<<"done"<<std::endl;
}

void AngularCorrelationSelector::FillHistograms() {
	//without addback
	for(auto g1 = 0; g1 < fGrif->GetMultiplicity(); ++g1) {
		auto grif1 = fGrif->GetGriffinHit(g1);
		//check for coincident betas
		bool coincBeta = false;
		for(auto s = 0; s < fScep->GetMultiplicity(); ++s) {
			auto scep = fScep->GetSceptarHit(s);
			if(!coincBeta && gbLow <= grif1->GetTime()-scep->GetTime() && grif1->GetTime()-scep->GetTime() <= gbHigh) coincBeta = true;
			fH1["betaGammaTiming"]->Fill(scep->GetTime()-grif1->GetTime());
			fH2["betaGammaHP"]->Fill(scep->GetDetector(), grif1->GetArrayNumber());
		}
		fH1["gammaEnergy"]->Fill(grif1->GetEnergy());
		if(coincBeta) fH1["gammaEnergyBeta"]->Fill(grif1->GetEnergy());
		for(auto g2 = 0; g2 < fGrif->GetMultiplicity(); ++g2) {
			if(g1 == g2) continue;
			auto grif2 = fGrif->GetGriffinHit(g2);
			double angle = grif1->GetPosition().Angle(grif2->GetPosition())*180./TMath::Pi();
			if(angle < 0.0001) continue;
			auto angleIndex = fAngleMap.lower_bound(angle-0.0005);
			double ggTime = TMath::Abs(grif1->GetTime()-grif2->GetTime());
			fH1["gammaGammaTiming"]->Fill(ggTime);
			fH2["gammaGammaHP"]->Fill(grif1->GetArrayNumber(), grif2->GetArrayNumber());

			if(ggTime < ggHigh) {
				fH2["gammaGamma"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				fH2[Form("gammaGamma%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				if(coincBeta) {
					fH2["gammaGammaBeta"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
					fH2[Form("gammaGammaBeta%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				}
			} else if(bgLow < ggTime && ggTime < bgHigh) {
				fH2["gammaGammaBG"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				fH2[Form("gammaGammaBG%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				if(coincBeta) {
					fH2["gammaGammaBetaBG"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
					fH2[Form("gammaGammaBetaBG%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				}
			}
		}
		//event mixing, we use the last event as second griffin
		for(auto g2 = 0; g2 < fLastGrif.GetMultiplicity(); ++g2) {
			if(g1 == g2) continue;
			auto grif2 = fLastGrif.GetGriffinHit(g2);
			double angle = grif1->GetPosition().Angle(grif2->GetPosition())*180./TMath::Pi();
			if(angle < 0.0001) continue;
			auto angleIndex = fAngleMap.lower_bound(angle-0.0005);
			double ggTime = TMath::Abs(grif1->GetTime()-grif2->GetTime());
			fH2["gammaGammaHPMixed"]->Fill(grif1->GetArrayNumber(), grif2->GetArrayNumber());

			fH2["gammaGammaMixed"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
			fH2[Form("gammaGammaMixed%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
			if(coincBeta) {
				fH2["gammaGammaBetaMixed"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				fH2[Form("gammaGammaBetaMixed%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
			}
		}
	}
	//with addback
	for(auto g1 = 0; g1 < fGrif->GetAddbackMultiplicity(); ++g1) {
		auto grif1 = fGrif->GetAddbackHit(g1);
		//check for coincident betas
		bool coincBeta = false;
		for(auto s = 0; s < fScep->GetMultiplicity(); ++s) {
			auto scep = fScep->GetSceptarHit(s);
			if(!coincBeta && gbLow <= grif1->GetTime()-scep->GetTime() && grif1->GetTime()-scep->GetTime() <= gbHigh) coincBeta = true;
			fH1["betaAddbackTiming"]->Fill(scep->GetTime()-grif1->GetTime());
			fH2["betaAddbackHP"]->Fill(scep->GetDetector(), grif1->GetArrayNumber());
		}
		fH1["addbackEnergy"]->Fill(grif1->GetEnergy());
		if(coincBeta) fH1["addbackEnergyBeta"]->Fill(grif1->GetEnergy());
		for(auto g2 = 0; g2 < fGrif->GetAddbackMultiplicity(); ++g2) {
			if(g1 == g2) continue;
			auto grif2 = fGrif->GetAddbackHit(g2);
			double angle = grif1->GetPosition().Angle(grif2->GetPosition())*180./TMath::Pi();
			if(angle < 0.0001) continue;
			auto angleIndex = fAngleMapAddback.lower_bound(angle-0.0005);
			double ggTime = TMath::Abs(grif1->GetTime()-grif2->GetTime());
			fH1["addbackAddbackTiming"]->Fill(ggTime);
			fH2["addbackAddbackHP"]->Fill(grif1->GetArrayNumber(), grif2->GetArrayNumber());

			if(ggTime < ggHigh) {
				fH2["addbackAddback"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				fH2[Form("addbackAddback%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				if(coincBeta) {
					fH2["addbackAddbackBeta"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
					fH2[Form("addbackAddbackBeta%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				}
			} else if(bgLow < ggTime && ggTime < bgHigh) {
				fH2["addbackAddbackBG"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				fH2[Form("addbackAddbackBG%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				if(coincBeta) {
					fH2["addbackAddbackBetaBG"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
					fH2[Form("addbackAddbackBetaBG%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				}
			}
		}
		//event mixing, we use the last event as second griffin
		for(auto g2 = 0; g2 < fLastGrif.GetAddbackMultiplicity(); ++g2) {
			if(g1 == g2) continue;
			auto grif2 = fLastGrif.GetAddbackHit(g2);
			double angle = grif1->GetPosition().Angle(grif2->GetPosition())*180./TMath::Pi();
			if(angle < 0.0001) continue;
			auto angleIndex = fAngleMapAddback.lower_bound(angle-0.0005);
			double ggTime = TMath::Abs(grif1->GetTime()-grif2->GetTime());
			fH2["addbackAddbackHPMixed"]->Fill(grif1->GetArrayNumber(), grif2->GetArrayNumber());

			fH2["addbackAddbackMixed"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
			fH2[Form("addbackAddbackMixed%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
			if(coincBeta) {
				fH2["addbackAddbackBetaMixed"]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
				fH2[Form("addbackAddbackBetaMixed%d", angleIndex->second)]->Fill(grif1->GetEnergy(), grif2->GetEnergy());
			}
		}
	}

	//update "last" event
	fLastGrif = *fGrif;
	fLastScep = *fScep;
}
