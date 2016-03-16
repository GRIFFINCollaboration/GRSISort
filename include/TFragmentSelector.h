#ifndef TFragmentSelector_h
#define TFragmentSelector_h

/** \addtogroup Sorting
 *  @{
 */

///////////////////////////////////////////////////////////
///
/// \class TFragmentSelector
///
/// This class has been automatically generated on
/// Fri Jul 24 10:50:00 2015 by ROOT version 5.34/24
/// from TTree FragmentTree/FragmentTree
/// found on file: fragment03771_000.root
///
///////////////////////////////////////////////////////////

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

// Header file for the classes stored in the TTree if any.
#include "TVirtualFragment.h"
#include "TChannel.h" //EDIT: added manually

// Fixed size dimensions of array or collections stored in the TTree if any.

class TFragmentSelector : public TSelector {
public :
   TTree          *fChain;   //!<!pointer to the analyzed TTree or TChain

   // Declaration of leaf types - EDIT: we use just the fragment itself, not it's members individually
	TVirtualFragment       *fragment;

	//EDIT: add run and subrun numbers
   TFragmentSelector(int runNumber = 0, int subRunNumber = 0, TTree * /*tree*/ = 0) : fChain(0),fragment(0) { 
		fRunNumber = runNumber; fSubRunNumber = subRunNumber;
	}
   virtual ~TFragmentSelector() { }
   virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) { fInput = input; }
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

	//EDIT: add run and subrun numbers
private:
	int fRunNumber;    //The current run number
	int fSubRunNumber; //The current subrun number

/// \cond CLASSIMP
   ClassDef(TFragmentSelector,0);
/// \endcond
};
/*! @} */
#endif

#ifdef TFragmentSelector_cxx
/** \addtogroup Sorting
 *  @{
 */

void TFragmentSelector::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetBranchAddress("TVirtualFragment", &fragment);
	//start by reading the calibrations from tree
	TChannel::ReadCalFromTree(tree);
}

Bool_t TFragmentSelector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}
/*! @} */
#endif // #ifdef TFragmentSelector_cxx
