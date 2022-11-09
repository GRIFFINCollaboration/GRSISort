//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Oct 25 13:18:27 2016 by ROOT version 5.34/24
// from TTree FragmentTree/FragmentTree
// found on file: fragment07844_000.root
//////////////////////////////////////////////////////////

#ifndef FastTimingHistSelector_h
#define FastTimingHistSelector_h

#include "TChain.h"
#include "TFile.h"

#include "TH1.h"
#include "TH2.h"
#include "THnSparse.h"

// Header file for the classes stored in the TTree if any.
#include "GValue.h"
#include "TGRSISelector.h"
#include "TGriffin.h"
#include "TSceptar.h"
#include "TGriffinBgo.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class FastTimingHistSelector : public TGRSISelector { //Must be same name as .C and .h

public :
	// branches
	double fLaBrEnergy[2];
	uint8_t fLaBrId[2];
	double fTac;
	uint8_t fTacId;
	std::vector<double>* fGeEnergies;
	Long64_t fEntry;

	// settings
	double fDrainingEnergy;
	double fDrainingLow;
	double fDrainingHigh;
	double fFeedingEnergy;
	double fFeedingLow;
	double fFeedingHigh;
	double fGeEnergy;
	double fGeLow;
	double fGeHigh;

   FastTimingHistSelector(TTree * /*tree*/ =0) : TGRSISelector(), fGeEnergies(nullptr) {
		SetOutputPrefix("FastTimingHist");
	}
	//These functions are expected to exist
	virtual ~FastTimingHistSelector() { }
	virtual Int_t   Version() const { return 2; }
	void CreateHistograms();
	void FillHistograms();
	void InitializeBranches(TTree *tree);

	ClassDef(FastTimingHistSelector,1); //Makes ROOT happier
};

#endif

#ifdef FastTimingHistSelector_cxx
void FastTimingHistSelector::InitializeBranches(TTree* tree)
{
	if(!tree) return;
	if(tree->SetBranchAddress("germanium", &fGeEnergies) == TTree::kMissingBranch ||
			tree->SetBranchAddress("firstEnergy", &fLaBrEnergy[0]) == TTree::kMissingBranch ||
			tree->SetBranchAddress("firstLaBr", &fLaBrId[0]) == TTree::kMissingBranch ||
			tree->SetBranchAddress("secondEnergy", &fLaBrEnergy[1]) == TTree::kMissingBranch ||
			tree->SetBranchAddress("secondLaBr", &fLaBrId[1]) == TTree::kMissingBranch ||
			tree->SetBranchAddress("tac", &fTac) == TTree::kMissingBranch ||
			tree->SetBranchAddress("tacId", &fTacId) == TTree::kMissingBranch ||
			tree->SetBranchAddress("entry", &fEntry) == TTree::kMissingBranch) {
		throw std::runtime_error("Failed to find all branches necessary for fast timing histograms!");
	}
}

#endif // #ifdef FastTimingHistSelector_cxx
