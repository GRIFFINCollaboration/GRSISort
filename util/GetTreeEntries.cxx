#include <iostream>

#include "TFile.h"
#include "TTree.h"

#include "TS3.h"
#include "TGRSIDetector.h"
#include "TTip.h"
#include "TSiLi.h"
#include "TSharc.h"
#include "TTAC.h"
#include "TGriffin.h"
#include "TDescant.h"
#include "TFragment.h"
#include "TMnemonic.h"
#include "TLaBr.h"
#include "TZeroDegree.h"
#include "TSceptar.h"
#include "TPaces.h"
#include "TTigress.h"

////////////////////////////////////////////////////////////////////////////////
/// 
/// This program simply prints the entries of the FragmentTree in the file (if
/// it finds one). 
/// If there is no FragmentTree, but an AnalysisTree, it prints the number of
/// entries in it, plus the number if hits for each detector (though not all are
/// implemented yet).
/// 
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if(argc == 1) {
		std::cout<<"Usage: "<<argv[0]<<" <root file name>"<<std::endl;
		return -1;
	}

	std::cout<<"Running on "<<argc-1<<" file(s)"<<std::endl;

	for(int i = 1; i < argc; ++i) {
		TFile f(argv[i]);
		if(!f.IsOpen()) {
			std::cout<<"Failed to open "<<argv[i]<<std::endl;
			continue;
		}

		TTree* tree;

		tree = dynamic_cast<TTree*>(f.Get("FragmentTree"));
		if(tree != nullptr) {
			std::cout<<argv[i]<<" - FragmentTree: "<<tree->GetEntries()<<std::endl;
			continue;
		}

		tree = dynamic_cast<TTree*>(f.Get("AnalysisTree"));
		if(tree != nullptr) {
			std::cout<<argv[i]<<" - AnalysisTree: "<<tree->GetEntries()<<std::endl;

			//S3

			//Tip

			//SiLi

			//TSharc
			
			//TTAC
			
			bool gotGriffin = (tree->FindBranch("TGriffin") != nullptr);
			TGriffin* grif = nullptr;
			if(gotGriffin)	tree->SetBranchAddress("TGriffin", &grif);
			long numGriffin = 0;

			bool gotDescant = (tree->FindBranch("TDescant") != nullptr);
			TDescant* desc = nullptr;
			if(gotDescant) tree->SetBranchAddress("TDescant", &desc);
			long numDescant = 0;

			bool gotLaBr = (tree->FindBranch("TLaBr") != nullptr);
			TLaBr* laBr = nullptr;
			if(gotLaBr)	tree->SetBranchAddress("TLaBr", &laBr);
			long numLaBr = 0;

			bool gotZeroDegree = (tree->FindBranch("TZeroDegree") != nullptr);
			TZeroDegree* zd = nullptr;
			if(gotZeroDegree)	tree->SetBranchAddress("TZeroDegree", &zd);
			long numZeroDegree = 0;

			bool gotSceptar = (tree->FindBranch("TSceptar") != nullptr);
			TSceptar* scep = nullptr;
			if(gotSceptar) tree->SetBranchAddress("TSceptar", &scep);
			long numSceptar = 0;

			bool gotPaces = (tree->FindBranch("TPaces") != nullptr);
			TPaces* pace = nullptr;
			if(gotPaces) tree->SetBranchAddress("TPaces", &pace);
			long numPaces = 0;

			//Tigress

			for(long e = 0; e < tree->GetEntries(); ++e) {
				tree->GetEntry(e);

				if(gotGriffin)    numGriffin    += grif->GetMultiplicity();
				if(gotDescant)    numDescant    += desc->GetMultiplicity();
				if(gotLaBr)       numLaBr       += laBr->GetMultiplicity();
				if(gotZeroDegree) numZeroDegree += zd->GetMultiplicity();
				if(gotSceptar)    numSceptar    += scep->GetMultiplicity();
				if(gotPaces)      numPaces      += pace->GetMultiplicity();
			}

			if(gotGriffin)    std::cout<<numGriffin<<" GRIFFIN, ";
			if(gotDescant)    std::cout<<numDescant<<" DESCANT, ";
			if(gotLaBr)       std::cout<<numLaBr<<" LABR, ";
			if(gotZeroDegree) std::cout<<numZeroDegree<<" ZERO-DEGREE, ";
			if(gotSceptar)    std::cout<<numSceptar<<" SCEPTAR, ";
			if(gotPaces)      std::cout<<numPaces<<" PACES, ";
			std::cout<<" "<<numGriffin+numDescant+numLaBr+numZeroDegree+numSceptar+numPaces<<" total hits"<<std::endl;

			continue;
		}

		std::cout<<"Failed to find 'FragmentTree' or 'AnalysisTree' in "<<argv[i]<<std::endl;

		f.ls();
	}

	return -1;
}
