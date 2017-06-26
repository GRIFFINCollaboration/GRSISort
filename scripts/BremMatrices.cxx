//g++ BremMatrices.cxx -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries -lAnalysisTreeBuilder -lGriffin -lSceptar -lDescant -lPaces -lGRSIDetector -lTGRSIFit -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lGRSIFormat -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml `grsi-config --cflags --libs` `root-config --cflags --libs`  -lTreePlayer -lGROOT -lX11 -lXpm -lSpectrum
#include <iostream>
#include <iomanip>
#include <utility>
#include <vector>
#include <cstdio>
#include <sys/stat.h>

#include "TROOT.h"
#include "TTree.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "TChain.h"
#include "TFile.h"
#include "TList.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TCanvas.h"
#include "TStopwatch.h"
#include "TMath.h"
#include "TGRSIRunInfo.h"
#include "TGRSISortInfo.h"
#include "Globals.h"
#include "TVectorD.h"
#include "TPPG.h"

#ifndef __CINT__ 
#include "TGriffin.h"
#include "TSceptar.h"
#endif

//This code is an example of how to write an analysis script to analyse an analysis tree
//The compiled script works like this
//
//  1. Starts in the main function by finding the analysis tree, setting up input and output files
//  2. Calls the BremMatrices function
//  3. BremMatrices creates 1D and 2D histograms and adds them to a list
//  4. Some loops over event "packets" decides which histograms should be filled and fills them.
//  5. The list of now filled histograms is returned to the main function
//  6. The list is written (meaning all of the histograms are written) to the output root file
//  7. Papers are published, theses are granted, high-fives are made
//
/////////////////////////////////////////////////////////////////////////////////////////

//This function gets run if running interpretively
//Not recommended for the analysis scripts
#ifdef __CINT__ 
void BremMatrices() {
	if(!AnalysisTree) {
		printf("No analysis tree found!\n");
		return;
	}
	//coinc window = 0-20, bg window 40-60, 6000 bins from 0. to 6000. (default is 4000)
	TList *list = BremMatrices(AnalysisTree, TPPG, TGRSIRunInfo, 0.);

	TFile *outfile = new TFile("output.root","recreate");
	list->Write();
}
#endif

TList *BremMatrices(TTree* tree, TPPG* ppg, TGRSIRunInfo* runInfo, long maxEntries = 0, TStopwatch* w = nullptr) {
	if(runInfo == nullptr) {
		return nullptr;
	}

	const Int_t nScDet = 21;
	const Int_t nGrDet = 65;
	Bool_t supp_flag[nScDet][nGrDet] = {
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//0
		{ false, false, false, false, true , false, false, false, false, true , false, false, false, true , true , true , true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//1
		{ false, true , true , true , true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//2
		{ false, false, false, false, false, true , true , true , true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//3
		{ false, false, false, false, false, false, false, false, false, true , true , true , true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//4
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//5
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, true , false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//6
		{ false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, true , false, false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//7
		{ false, false, false, false, false, false, true , true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, false, true , false, false, true , false, false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//8
		{ false, false, false, false, false, false, false, false, false, false, true , true , false, false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, false, true , false, false, true , true , false, false, true , false, false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//9
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//10
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, false, false, false, true , false, false, false, false, false, false, false, false, true , false, false, false },//11
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//12
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, false, true , true , false, false, true , true , false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, true , true , false, false, true , false, false, false, false },//13
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//14
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , true , false, false, false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true  },//15
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , true , true , true , false, false, false, false, false, false, false, false, false, true , false, false },//16
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },//17
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , true , true , true , false, false, false, false, false, false, false, false },//18
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , true , true , true , false, false, false, false },//19
		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , false, false, false, false, true , true  }//20
	};

	Bool_t hit_flags[65] = {false};
	///////////////////////////////////// SETUP ///////////////////////////////////////
	//Histogram paramaters
	Double_t low = 0;
	Double_t high = 10000;
	Double_t nofBins = 10000;

	//Coincidence Parameters
	Double_t ggTlow = 0.;   //Times are in 10's of ns
	Double_t ggThigh = 40.;
	Double_t gbTlow =  -15.;
	Double_t gbThigh = 10.;

	Double_t ggBGlow = 100.;
	Double_t ggBGhigh = 175.;
	Double_t gbBGlow = -160.;
	Double_t gbBGhigh = 0.;
	Double_t ggBGScale = (ggThigh - ggTlow)/(ggBGhigh - ggBGlow);
	Double_t gbBGScale = (gbThigh - gbTlow)/(gbBGhigh - gbBGlow);

	Double_t betaThres = 800.;

	//this is in ms
	Double_t cycleLength = 15000;
	if(ppg) {
		cycleLength = ppg->GetCycleLength()/1e5;
	}

	Double_t bgStart  =  1.5e8;
	Double_t bgEnd    =  3.5e8;
	Double_t onStart  =  3.5e8;
	Double_t onEnd    = 14.0e8;
	Double_t offStart = 14.5e8;
	Double_t offEnd   = 15.5e8;

	if(w == nullptr) {
		w = new TStopwatch;
		w->Start();
	}
	auto* list = new TList;

	//const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

	//We create some spectra and then add it to the list
	//hit patterns
	auto* bIdVsgId = new TH2D("bIdVsgIdbrem","Sceptar Id vs Griffin Id",20,1,21,64,1,65); list->Add(bIdVsgId);

	//gamma single spectra
	auto* gammaSingles = new TH1D("gammaSinglesbrem","#gamma singles;energy[keV]",nofBins, low, high); list->Add(gammaSingles);
	auto* gammaSinglesB = new TH1D("gammaSinglesBbrem","#beta #gamma;energy[keV]",nofBins, low, high); list->Add(gammaSinglesB);
	auto* gammaSinglesBm = new TH1D("gammaSinglesBmbrem","#beta #gamma (multiple counting of #beta's);energy[keV]",nofBins, low, high); list->Add(gammaSinglesBm);
	auto* gammaSinglesBt = new TH1D("gammaSinglesBtbrem","#beta #gamma t-rand-corr; energy[keV]",nofBins, low, high); list->Add(gammaSinglesBt);
	auto* ggTimeDiff = new TH1D("ggTimeDiffbrem", "#gamma-#gamma time difference", 300,0,300); list->Add(ggTimeDiff);
	auto* gbTimeDiff = new TH1D("gbTimeDiffbrem", "#gamma-#beta time difference", 2000,-1000,1000); list->Add(gbTimeDiff); 
	auto* bbTimeDiff = new TH2D("bbTimeDiffbrem", "#beta energy vs. #beta-#beta time difference", 2000,-1000,1000, 1000, 0., 2e6); list->Add(bbTimeDiff); 
	auto* gTimeDiff = new TH2D("gTimeDiffbrem", "channel vs. time difference", 2000,0,2000, 65, 1., 65.); list->Add(gTimeDiff); 
	auto* gtimestamp = new TH1F("gtimestampbrem", "#gamma time stamp", 10000,0,1000); list->Add(gtimestamp);
	auto* btimestamp = new TH1F("btimestampbrem", "#beta time stamp", 10000,0,1000); list->Add(btimestamp);
	auto* gbEnergyvsgTime = new TH2F("gbEnergyvsgTimebrem", "#gamma #beta coincident: #gamma timestamp vs. #gamma energy; Time [s]; Energy [keV]", 1000,0,1000, nofBins, low, high); list->Add(gbEnergyvsgTime);
	auto* gbEnergyvsbTime = new TH2F("gbEnergyvsbTimebrem", "#gamma #beta coincident: #beta timestamp vs. #gamma energy; Time [s]; Energy [keV]", 1000,0,1000, nofBins, low, high); list->Add(gbEnergyvsbTime);
	auto* ggmatrix = new TH2D("ggmatrixbrem","#gamma-#gamma matrix",nofBins, low, high,nofBins, low, high); list->Add(ggmatrix);
	auto* ggmatrixt = new TH2D("ggmatrixtbrem","#gamma-#gamma matrix t-corr",nofBins,low,high,nofBins,low,high); list->Add(ggmatrixt);
	auto* gammaSinglesB_hp = new TH2F("gammaSinglesB_hpbrem", "#gamma-#beta vs. SC channel", nofBins,low,high,20,1,21); list->Add(gammaSinglesB_hp);
	auto* ggbmatrix = new TH2F("ggbmatrixbrem","#gamma-#gamma-#beta matrix", nofBins, low, high, nofBins, low, high); list->Add(ggbmatrix);
	auto* ggbmatrixt = new TH2F("ggbmatrixtbrem","#gamma-#gamma-#beta matrix t-corr", nofBins, low, high, nofBins, low, high); list->Add(ggbmatrixt);
	auto* grifscep_hp = new TH2F("grifscep_hpbrem","Sceptar vs Griffin hit pattern",64,0,64,20,0,20); list->Add(grifscep_hp);
	auto* gbTimevsg = new TH2F("gbTimevsgbrem","#gamma energy vs. #gamma-#beta timing",300,-150,150,nofBins,low,high); list->Add(gbTimevsg); 
	auto* ggbmatrixOn = new TH2F("ggbmatrixOnbrem","#gamma-#gamma-#beta matrix, beam on window", nofBins, low, high, nofBins, low, high); list->Add(ggbmatrixOn);
	auto* ggbmatrixBg = new TH2F("ggbmatrixBgbrem","#gamma-#gamma-#beta matrix, background window", nofBins, low, high, nofBins, low, high); list->Add(ggbmatrixBg);
	auto* ggbmatrixOff = new TH2F("ggbmatrixOffbrem","#gamma-#gamma-#beta matrix, beam off window", nofBins, low, high, nofBins, low, high); list->Add(ggbmatrixOff);

	auto* veto_hp = new TH1D("veto_hp","number of vetoed gamma rays",65,0,65); list->Add(veto_hp);

	TH2F* gammaSinglesCyc;
	TH2F* gammaSinglesBCyc;
	TH2F* gammaSinglesBmCyc;
	TH2F* betaSinglesCyc;

	gammaSinglesCyc = new TH2F("gammaSinglesCycbrem", "Cycle time vs. #gamma energy", cycleLength/10.,0.,cycleLength, nofBins,low,high); list->Add(gammaSinglesCyc);
	gammaSinglesBCyc = new TH2F("gammaSinglesBCycbrem", "Cycle time vs. #beta coinc #gamma energy", cycleLength/10.,0.,cycleLength/1e5, nofBins,low,high); list->Add(gammaSinglesBCyc);
	gammaSinglesBmCyc = new TH2F("gammaSinglesBmCycbrem", "Cycle time vs. #beta coinc #gamma energy (multiple counting of #beta's)", cycleLength/10.,0.,cycleLength, nofBins,low,high); list->Add(gammaSinglesBmCyc);
	betaSinglesCyc = new TH2F("betaSinglesCycbrem", "Cycle number vs. cycle time for #beta's", cycleLength/10.,0.,cycleLength,200,0,200); list->Add(betaSinglesCyc);

	//addback spectra
	auto* gammaAddback = new TH1D("gammaAddbackbrem","#gamma singles;energy[keV]",nofBins, low, high); list->Add(gammaAddback);
	auto* gammaAddbackB = new TH1D("gammaAddbackBbrem","#beta #gamma;energy[keV]",nofBins, low, high); list->Add(gammaAddbackB);
	auto* gammaAddbackBm = new TH1D("gammaAddbackBmbrem","#beta #gamma (multiple counting of #beta's);energy[keV]",nofBins, low, high); list->Add(gammaAddbackBm);
	auto* gammaAddbackBt = new TH1D("gammaAddbackBtbrem","#beta #gamma t-rand-corr; energy[keV]",nofBins, low, high); list->Add(gammaAddbackBt);
	auto* aaTimeDiff = new TH1D("aaTimeDiffbrem", "#gamma-#gamma time difference", 300,0,300); list->Add(aaTimeDiff);
	auto* abTimeDiff = new TH1D("abTimeDiffbrem", "#gamma-#beta time difference", 2000,-1000,1000); list->Add(abTimeDiff); 
	auto* abEnergyvsgTime = new TH2F("abEnergyvsgTimebrem", "#gamma #beta coincident: #gamma timestamp vs. #gamma energy; Time [s]; Energy [keV]", 1000,0,1000, nofBins, low, high); list->Add(abEnergyvsgTime);
	auto* abEnergyvsbTime = new TH2F("abEnergyvsbTimebrem", "#gamma #beta coincident: #beta timestamp vs. #gamma energy; Time [s]; Energy [keV]", 1000,0,1000, nofBins, low, high); list->Add(abEnergyvsbTime);
	auto* aamatrix = new TH2D("aamatrixbrem","#gamma-#gamma matrix",nofBins, low, high,nofBins, low, high); list->Add(aamatrix);
	auto* aamatrixt = new TH2D("aamatrixtbrem","#gamma-#gamma matrix t-corr",nofBins,low,high,nofBins,low,high); list->Add(aamatrixt);
	auto* gammaAddbackB_hp = new TH2F("gammaAddbackB_hpbrem", "#gamma-#beta vs. SC channel", nofBins,low,high,20,1,21); list->Add(gammaAddbackB_hp);
	auto* aabmatrix = new TH2F("aabmatrixbrem","#gamma-#gamma-#beta matrix", nofBins, low, high, nofBins, low, high); list->Add(aabmatrix);
	auto* aabmatrixt = new TH2F("aabmatrixtbrem","#gamma-#gamma-#beta matrix t-corr", nofBins, low, high, nofBins, low, high); list->Add(aabmatrixt);
	auto* abTimevsg = new TH2F("abTimevsgbrem","#gamma energy vs. #gamma-#beta timing",300,-150,150,nofBins,low,high); list->Add(abTimevsg); 
	auto* abTimevsgf = new TH2F("abTimevsgfbrem","#gamma energy vs. #gamma-#beta timing (first #beta only)",300,-150,150,nofBins,low,high); list->Add(abTimevsgf); 
	auto* abTimevsgl = new TH2F("abTimevsglbrem","#gamma energy vs. #gamma-#beta timing (last #beta only)",300,-150,150,nofBins,low,high); list->Add(abTimevsgl); 
	auto* aabmatrixOn = new TH2F("aabmatrixOnbrem","#gamma-#gamma-#beta matrix, beam on window", nofBins, low, high, nofBins, low, high); list->Add(aabmatrixOn);
	auto* aabmatrixBg = new TH2F("aabmatrixBgbrem","#gamma-#gamma-#beta matrix, background window", nofBins, low, high, nofBins, low, high); list->Add(aabmatrixBg);
	auto* aabmatrixOff = new TH2F("aabmatrixOffbrem","#gamma-#gamma-#beta matrix, beam off window", nofBins, low, high, nofBins, low, high); list->Add(aabmatrixOff);

	TH2F* gammaAddbackCyc;
	TH2F* gammaAddbackBCyc;
	TH2F* gammaAddbackBmCyc; 
	gammaAddbackCyc = new TH2F("gammaAddbackCycbrem", "Cycle time vs. #gamma energy", cycleLength/10.,0.,cycleLength, nofBins,low,high); list->Add(gammaAddbackCyc);
	gammaAddbackBCyc = new TH2F("gammaAddbackBCycbrem", "Cycle time vs. #beta coinc #gamma energy", cycleLength/10.,0.,cycleLength, nofBins,low,high); list->Add(gammaAddbackBCyc);
	gammaAddbackBmCyc = new TH2F("gammaAddbackBmCycbrem", "Cycle time vs. #beta coinc #gamma energy (multiple counting of #beta's)", cycleLength/10.,0.,cycleLength, nofBins,low,high); list->Add(gammaAddbackBmCyc);
	list->Sort(); //Sorts the list alphabetically
	if(ppg) {
		list->Add(ppg);
	}

	list->Add(runInfo);

	if(ppg) {
		TGRSIDetectorHit::SetPPGPtr(ppg);
	}

	///////////////////////////////////// PROCESSING /////////////////////////////////////

	//set up branches
	//Each branch can hold multiple hits
	//ie TGriffin grif holds 3 gamma rays on a triples event 
	TGriffin* grif = nullptr;
	TSceptar* scep = nullptr;
	tree->SetBranchAddress("TGriffin", &grif); //We assume we always have a Griffin

	bool gotSceptar;
	if(tree->FindBranch("TSceptar") == nullptr) {   //We check to see if we have a Scepter branch in the analysis tree
		gotSceptar = false;
	} else {
		tree->SetBranchAddress("TSceptar", &scep);
		gotSceptar = true;
	}

	//tree->LoadBaskets(MEM_SIZE);   

	//long entries = tree->GetEntries();
	//long entries = 1e6;

	//These are the indices of the two hits being compared
	int one;
	int two;

	auto* t = new TVectorD(2);
	(*t)[0] = runInfo->RunStart();
	(*t)[1] = runInfo->RunStop();

	list->Add(t);

	//store the last timestamp of each channel
	std::vector<long> lastTimeStamp(65,0);

	std::cout<<std::fixed<<std::setprecision(1); //This just make outputs not look terrible
	//size_t angIndex;
	if(maxEntries == 0 || maxEntries > tree->GetEntries()) {
		maxEntries = tree->GetEntries();
	}
	// maxEntries = 1e5;
	int entry;
	for(entry = 1; entry < maxEntries; ++entry) { //Only loop over the set number of entries
		//I'm starting at entry 1 because of the weird high stamp of 4.
		tree->GetEntry(entry);

		grif->ResetAddback();
		bool beta_hit = false;
		for(one=0; one<grif->GetMultiplicity(); ++one){
			grif->GetGriffinHit(one)->SetIsBremSuppressed(false);
			for(int b=0; b<scep->GetMultiplicity();++b){
				if(scep->GetHit(b)->GetEnergy() < betaThres) { continue;
}
				if((gbTlow <= grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) {
					for(int i=0;i<65;++i){
						if(supp_flag[scep->GetHit(b)->GetDetector()][i]) {
							hit_flags[i] = true;
}
					}
					beta_hit = true;
					grif->GetGriffinHit(one)->MakeBremSuppressed(supp_flag[scep->GetSceptarHit(b)->GetDetector()][grif->GetGriffinHit(one)->GetArrayNumber()]);              
				}
			}
		}
		if(beta_hit){
			for(int i =0;i<65;++i){
				if(hit_flags[i]){
					veto_hp->Fill(i);
				}
				hit_flags[i] = false;
			}
		}

		for(one=0; one<grif->GetAddbackMultiplicity(); ++one){
			grif->GetAddbackHit(one)->SetIsBremSuppressed(false);
			for(int b=0; b<scep->GetMultiplicity();++b){
				if(scep->GetHit(b)->GetEnergy() < betaThres) { continue;
}
				if((gbTlow <= grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) {
					grif->GetAddbackHit(one)->MakeBremSuppressed(supp_flag[scep->GetSceptarHit(b)->GetDetector()][grif->GetAddbackHit(one)->GetArrayNumber()]);              
				}
			}
		}


		//loop over the gammas in the event packet
		//grif is the variable which points to the current TGriffin
		for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
			if(grif->GetGriffinHit(one)->GetIsBremSuppressed()) { continue;
}
			//We want to put every gamma ray in this event into the singles
			gammaSingles->Fill(grif->GetGriffinHit(one)->GetEnergy()); 
			gtimestamp->Fill(grif->GetGriffinHit(one)->GetTime()/100000000.);
			Long_t time = (Long_t)(grif->GetHit(one)->GetTime());
			if(ppg){
				time = time%ppg->GetCycleLength();
				//gammaSinglesCyc->Fill((time - ppg->GetLastStatusTime(time, TPPG::kTapeMove))/1e5, grif->GetGriffinHit(one)->GetEnergy()); 
				gammaSinglesCyc->Fill(time/1e5, grif->GetGriffinHit(one)->GetEnergy()); 
			}
			if(grif->GetGriffinHit(one)->GetArrayNumber() < lastTimeStamp.size()) {
				if(lastTimeStamp.at(grif->GetGriffinHit(one)->GetArrayNumber()) > 0) {
					gTimeDiff->Fill(grif->GetHit(one)->GetTimeStamp() - lastTimeStamp.at(grif->GetGriffinHit(one)->GetArrayNumber()), grif->GetGriffinHit(one)->GetArrayNumber());
				}
				lastTimeStamp.at(grif->GetGriffinHit(one)->GetArrayNumber()) = grif->GetHit(one)->GetTimeStamp();
			}
			//We now want to loop over any other gammas in this packet
			for(two = 0; two < (int) grif->GetMultiplicity(); ++two) {
				if(two == one){ //If we are looking at the same gamma we don't want to call it a coincidence
					continue;
				}
				if(grif->GetGriffinHit(two)->GetIsBremSuppressed()) { continue;
}
				//Check to see if the two gammas are close enough in time
				ggTimeDiff->Fill(TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()));
				if(ggTlow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < ggThigh) { 
					//If they are close enough in time, fill the gamma-gamma matrix. This will be symmetric because we are doing a double loop over gammas
					ggmatrix->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
				}
				if(ggBGlow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < ggBGhigh) { 
					//If they are not close enough in time, fill the time-random gamma-gamma matrix. This will be symmetric because we are doing a double loop over gammas
					ggmatrixt->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
				}
			}
		}


		//Now we make beta gamma coincident matrices
		if(gotSceptar && scep->GetMultiplicity() > 0) {
			bool plotted_flag = false;
			//We do an outside loop on gammas so that we can break on the betas if we see a beta in coincidence (we don't want to bin twice just because we have two betas)
			for(int b = 0; b < scep->GetMultiplicity(); ++b) {
				if(scep->GetHit(b)->GetEnergy() < betaThres) { continue;
}
				btimestamp->Fill(scep->GetHit(b)->GetTime()/1e8);
				if(ppg && !plotted_flag){//Fill on first hit only.
					betaSinglesCyc->Fill(((ULong64_t)(ppg->GetTimeInCycle(scep->GetHit(b)->GetTime()))/1e5),ppg->GetCycleNumber((ULong64_t)(scep->GetHit(b)->GetTime()))); 
					//  betaSinglesCyc->Fill((((ULong64_t)(scep->GetHit(b)->GetTime()))%(ppg->GetCycleLength()))/1e5,(scep->GetHit(b)->GetTime())/(ppg->GetCycleLength())); 
					plotted_flag = true;
				}
				for(int b2 = 0; b2 < scep->GetMultiplicity(); ++b2) {
					if(b == b2) { continue;
}
					bbTimeDiff->Fill(scep->GetHit(b)->GetTime()-scep->GetHit(b2)->GetTime(), scep->GetHit(b)->GetEnergy());
				}
			}

			if(grif->GetMultiplicity() && scep->GetMultiplicity()) {
				for(one = 0; one < (int) grif->GetMultiplicity(); ++one) {
					if(grif->GetGriffinHit(one)->GetIsBremSuppressed()) { continue;
}
					bool found = false;
					for(int b = 0; b < scep->GetMultiplicity(); ++b) {
						if(scep->GetHit(b)->GetEnergy() < betaThres) { continue;
}
						//Be careful about time ordering!!!! betas and gammas are not symmetric out of the DAQ
						//Fill the time diffrence spectra
						gbTimeDiff->Fill(grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime());
						gbTimevsg->Fill(grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime(),grif->GetHit(one)->GetEnergy());
						if((gbTlow <= grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) {
							gbEnergyvsbTime->Fill(scep->GetHit(b)->GetTime(),grif->GetHit(one)->GetEnergy());
						}
						if((gbTlow <= grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) {
							ULong64_t time = (ULong64_t)(grif->GetHit(one)->GetTime());
							if(ppg){
								time = time%ppg->GetCycleLength();
								gammaSinglesBmCyc->Fill(time/1e5, grif->GetGriffinHit(one)->GetEnergy()); 
							}
							//Plots a gamma energy spectrum in coincidence with a beta
							gbEnergyvsgTime->Fill(grif->GetGriffinHit(one)->GetTime()/1e8, grif->GetGriffinHit(one)->GetEnergy());
							gammaSinglesBm->Fill(grif->GetGriffinHit(one)->GetEnergy());
							bIdVsgId->Fill(scep->GetHit(b)->GetDetector(), grif->GetGriffinHit(one)->GetArrayNumber());
							if(!found) {
								gammaSinglesB->Fill(grif->GetGriffinHit(one)->GetEnergy());
								if(ppg) {
									gammaSinglesBCyc->Fill(ppg->GetTimeInCycle((ULong64_t)(grif->GetHit(one)->GetTime()))/1e5, grif->GetGriffinHit(one)->GetEnergy()); 
								}
							}
							gammaSinglesB_hp->Fill(grif->GetGriffinHit(one)->GetEnergy(),scep->GetSceptarHit(b)->GetDetector());
							grifscep_hp->Fill(grif->GetGriffinHit(one)->GetArrayNumber(),scep->GetSceptarHit(b)->GetDetector());
							//gammaSinglesBCyc->Fill((time - ppg->GetLastStatusTime(time, TPPG::kTapeMove))/1e5, grif->GetGriffinHit(one)->GetEnergy()); 
							//Now we want to loop over gamma rays if they are in coincidence.
							if(grif->GetMultiplicity() > 1){
								for(two = 0; two < (int) grif->GetMultiplicity(); ++two) {
									if(two == one){ //If we are looking at the same gamma we don't want to call it a coincidence
										continue;
									}
									if(grif->GetGriffinHit(two)->GetIsBremSuppressed()) { continue;
}

									if(ggTlow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < ggThigh) { 
										//If they are close enough in time, fill the gamma-gamma-beta matrix. This will be symmetric because we are doing a double loop over gammas
										ggbmatrix->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
										if(ppg && ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) > bgStart && ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) < bgEnd) {
											ggbmatrixBg->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
										} else if(ppg && ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) > onStart && ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) < onEnd) {
											ggbmatrixOn->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
										} else if(ppg && ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) > offStart && ppg->GetTimeInCycle(grif->GetGriffinHit(one)->GetTimeStamp()) < offEnd) {
											ggbmatrixOff->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
										}
									}
									if(ggBGlow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < ggBGhigh) { 
										//If they are not close enough in time, fill the gamma-gamma-beta time-random matrix. This will be symmetric because we are doing a double loop over gammas
										ggbmatrixt->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());
									}
								}                   
							}
							found = true;
						}
						if((gbBGlow <= grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbBGhigh)) {
							gammaSinglesBt->Fill(grif->GetHit(one)->GetEnergy());                 
						}
					}
				}
}
		}

		//loop over the addbacks in the event packet
		//grif is the variable which points to the current TGriffin
		for(one = 0; one < (int) grif->GetAddbackMultiplicity(); ++one) {
			if(grif->GetAddbackHit(one)->GetIsBremSuppressed()) { continue;
}
			//We want to put every gamma ray in this event into the singles
			gammaAddback->Fill(grif->GetAddbackHit(one)->GetEnergy()); 
			Long_t time = (Long_t)(grif->GetAddbackHit(one)->GetTime());
			if(ppg){
				time = time%ppg->GetCycleLength();
				//gammaSinglesCyc->Fill((time - ppg->GetLastStatusTime(time, TPPG::kTapeMove))/1e5, grif->GetGriffinHit(one)->GetEnergy()); 
				gammaAddbackCyc->Fill(time/1e5, grif->GetAddbackHit(one)->GetEnergy()); 
				//We now want to loop over any other gammas in this packet
			}
			for(two = 0; two < (int) grif->GetAddbackMultiplicity(); ++two) {
				if(grif->GetAddbackHit(two)->GetIsBremSuppressed()) { continue;
}
				if(two == one){ //If we are looking at the same gamma we don't want to call it a coincidence
					continue;
				}
				if(grif->GetAddbackHit(two)->GetIsBremSuppressed()) { continue;
}
				//Check to see if the two gammas are close enough in time
				//VINZENZ I THINK THIS IS BREAKING THIS FOR SOME REASON.
				aaTimeDiff->Fill(TMath::Abs(grif->GetAddbackHit(two)->GetTime()-grif->GetAddbackHit(one)->GetTime()));
				if(ggTlow <= TMath::Abs(grif->GetAddbackHit(two)->GetTime()-grif->GetAddbackHit(one)->GetTime()) && TMath::Abs(grif->GetAddbackHit(two)->GetTime()-grif->GetAddbackHit(one)->GetTime()) < ggThigh) { 
					//If they are close enough in time, fill the gamma-gamma matrix. This will be symmetric because we are doing a double loop over gammas
					aamatrix->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy());
				}
				if(ggBGlow <= TMath::Abs(grif->GetAddbackHit(two)->GetTime()-grif->GetAddbackHit(one)->GetTime()) && TMath::Abs(grif->GetAddbackHit(two)->GetTime()-grif->GetAddbackHit(one)->GetTime()) < ggBGhigh) { 
					//If they are not close enough in time, fill the time-random gamma-gamma matrix. This will be symmetric because we are doing a double loop over gammas
					aamatrixt->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy());
				}
			}
		}


		//Now we make beta gamma coincident matrices
		if(gotSceptar && scep->GetMultiplicity() > 0) {
			//We do an outside loop on gammas so that we can break on the betas if we see a beta in coincidence (we don't want to bin twice just because we have two betas)
			for(one = 0; one < (int) grif->GetAddbackMultiplicity(); ++one) {
				if(grif->GetAddbackHit(one)->GetIsBremSuppressed()) { continue;
}
				bool found = false;
				for(int b = 0; b < scep->GetMultiplicity(); ++b) {
					if(scep->GetHit(b)->GetEnergy() < betaThres) { continue;
}
					//Be careful about time ordering!!!! betas and gammas are not symmetric out of the DAQ
					//Fill the time diffrence spectra
					abTimeDiff->Fill(grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime());
					abTimevsg->Fill(grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime(),grif->GetAddbackHit(one)->GetEnergy());
					if(b == 0) {
						abTimevsgf->Fill(grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime(),grif->GetAddbackHit(one)->GetEnergy());
					}
					if(b == scep->GetMultiplicity()-1) {
						abTimevsgl->Fill(grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime(),grif->GetAddbackHit(one)->GetEnergy());
					}
					if((gbTlow <= grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) {
						abEnergyvsbTime->Fill(scep->GetHit(b)->GetTime(),grif->GetAddbackHit(one)->GetEnergy());
					}
					if((gbTlow <= grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbThigh)) {
						ULong64_t time = (ULong64_t)(grif->GetAddbackHit(one)->GetTime());
						if(ppg){
							time = time%ppg->GetCycleLength();
							gammaAddbackBmCyc->Fill(time/1e5, grif->GetAddbackHit(one)->GetEnergy()); 
							//Plots a gamma energy spectrum in coincidence with a beta
						}
						abEnergyvsgTime->Fill(grif->GetAddbackHit(one)->GetTime()/1e8, grif->GetAddbackHit(one)->GetEnergy());
						gammaAddbackBm->Fill(grif->GetAddbackHit(one)->GetEnergy());
						if(!found) {
							gammaAddbackB->Fill(grif->GetAddbackHit(one)->GetEnergy());
							if(ppg) {
								gammaAddbackBCyc->Fill(ppg->GetTimeInCycle((ULong64_t)(grif->GetAddbackHit(one)->GetTime()))/1e5, grif->GetAddbackHit(one)->GetEnergy()); 
}
						}
						gammaAddbackB_hp->Fill(grif->GetAddbackHit(one)->GetEnergy(),scep->GetSceptarHit(b)->GetDetector());
						//Now we want to loop over gamma rays if they are in coincidence.
						if(grif->GetAddbackMultiplicity() > 1){
							for(two = 0; two < (int) grif->GetAddbackMultiplicity(); ++two) {
								if(two == one){ //If we are looking at the same gamma we don't want to call it a coincidence
									continue;
								}
								if(grif->GetAddbackHit(two)->GetIsBremSuppressed()) { continue;
}

								if(ggTlow <= TMath::Abs(grif->GetAddbackHit(two)->GetTime()-grif->GetAddbackHit(one)->GetTime()) && TMath::Abs(grif->GetAddbackHit(two)->GetTime()-grif->GetAddbackHit(one)->GetTime()) < ggThigh) { 
									//If they are close enough in time, fill the gamma-gamma-beta matrix. This will be symmetric because we are doing a double loop over gammas
									aabmatrix->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy());
									if(ppg && ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) > bgStart && ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) < bgEnd) {
										aabmatrixBg->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy());
									} else if(ppg && ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) > onStart && ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) < onEnd) {
										aabmatrixOn->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy());
									} else if(ppg && ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) > offStart && ppg->GetTimeInCycle(grif->GetAddbackHit(one)->GetTimeStamp()) < offEnd) {
										aabmatrixOff->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy());
									}
								}
								if(ggBGlow <= TMath::Abs(grif->GetAddbackHit(two)->GetTime()-grif->GetAddbackHit(one)->GetTime()) && TMath::Abs(grif->GetAddbackHit(two)->GetTime()-grif->GetAddbackHit(one)->GetTime()) < ggBGhigh) { 
									//If they are not close enough in time, fill the gamma-gamma-beta time-random matrix. This will be symmetric because we are doing a double loop over gammas
									aabmatrixt->Fill(grif->GetAddbackHit(one)->GetEnergy(), grif->GetAddbackHit(two)->GetEnergy());
								}
							}                   
						}
						found = true;
					}
					if((gbBGlow <= grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime()) && (grif->GetAddbackHit(one)->GetTime()-scep->GetHit(b)->GetTime() <= gbBGhigh)) {
						gammaAddbackBt->Fill(grif->GetAddbackHit(one)->GetEnergy());                 
					}
				}
			}
		}
		if((entry%10000) == 0){
			printf("Completed %d of %ld \r",entry,maxEntries);
		}

	}
	ggmatrixt->Scale(-ggBGScale);
	ggmatrixt->Add(ggmatrix);

	ggbmatrixt->Scale(-ggBGScale);
	ggbmatrixt->Add(ggbmatrix);

	gammaSinglesBt->Scale(-gbBGScale);
	gammaSinglesBt->Add(gammaSinglesB);

	aamatrixt->Scale(-ggBGScale);
	aamatrixt->Add(aamatrix);

	aabmatrixt->Scale(-ggBGScale);
	aabmatrixt->Add(aabmatrix);

	gammaAddbackBt->Scale(-gbBGScale);
	gammaAddbackBt->Add(gammaAddbackB);

	std::cout << "creating histograms done after " << w->RealTime() << " seconds" << std::endl;
	w->Continue();
	return list;

}











//This function gets run if running in compiled mode
#ifndef __CINT__ 
int main(int argc, char **argv) {
	if(argc != 4 && argc != 3 && argc != 2) {
		printf("try again (usage: %s <analysis tree file> <optional: output file> <max entries>).\n",argv[0]);
		return 0;
	}

	//We use a stopwatch so that we can watch progress
	TStopwatch w;
	w.Start();

	auto* file = new TFile(argv[1]);

	if(file == nullptr) {
		printf("Failed to open file '%s'!\n",argv[1]);
		return 1;
	}
	if(!file->IsOpen()) {
		printf("Failed to open file '%s'!\n",argv[1]);
		return 1;
	}
	printf("Sorting file:" DBLUE " %s" RESET_COLOR"\n",file->GetName());

	//Get PPG from File
	TPPG* myPPG = dynamic_cast<TPPG*>(file->Get("TPPG"));
	/*   if(myPPG == nullptr) {
		  printf("Failed to find PPG information in file '%s'!\n",argv[1]);
		  return 1;
		  }
		  */
	//Get run info from File
	TGRSIRunInfo* runInfo = dynamic_cast<TGRSIRunInfo*>(file->Get("TGRSIRunInfo"));
	if(runInfo == nullptr) {
		printf("Failed to find run information in file '%s'!\n",argv[1]);
		return 1;
	}

	TTree* tree = dynamic_cast<TTree*>( file->Get("AnalysisTree"));
	TChannel::ReadCalFromTree(tree);
	if(tree == nullptr) {
		printf("Failed to find analysis tree in file '%s'!\n",argv[1]);
		return 1;
	}
	//Get the TGRSIRunInfo from the analysis Tree.

	TList *list;//We return a list because we fill a bunch of TH1's and shove them into this list.
	TFile * outfile;
	if(argc<3)
	{
		if(!runInfo){
			printf("Could not find run info, please provide output file name\n");
			return 0;
		}
		int runnumber = runInfo->RunNumber();
		int subrunnumber = runInfo->SubRunNumber();
		outfile = new TFile(Form("bmatrix%05d_%03d.root",runnumber,subrunnumber),"recreate");
	}
	else
	{
		outfile = new TFile(argv[2],"recreate");
	}

	std::cout << argv[0] << ": starting Analysis after " << w.RealTime() << " seconds" << std::endl;
	w.Continue();
	if(argc < 4) {
		list = BremMatrices(tree,myPPG,runInfo,0, &w);
	} else {
		int entries = atoi(argv[3]);
		std::cout<<"Limiting processing of analysis tree to "<<entries<<" entries!"<<std::endl;
		list = BremMatrices(tree,myPPG,runInfo, entries, &w);
	}
	if(list == nullptr) {
		std::cout<<"BremMatrices returned TList* nullptr!\n"<<std::endl;
		return 1;
	}

	printf("Writing to File: " DYELLOW "%s" RESET_COLOR"\n",outfile->GetName());
	list->Write();
	//Write the run info into the tree as well if there is run info in the Analysis Tree
	auto *sortinfolist = new TGRSISortList;
	if(runInfo){
		auto *info = new TGRSISortInfo(runInfo);
		sortinfolist->AddSortInfo(info);
		sortinfolist->Write("TGRSISortList",TObject::kSingleKey);
	}

	outfile->Close();

	std::cout << argv[0] << " done after " << w.RealTime() << " seconds" << std::endl << std::endl;


	return 0;
}

#endif
