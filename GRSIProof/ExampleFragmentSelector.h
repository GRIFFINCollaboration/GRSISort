//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Oct 25 13:18:27 2016 by ROOT version 5.34/24
// from TTree FragmentTree/FragmentTree
// found on file: fragment07844_000.root
//////////////////////////////////////////////////////////

#ifndef ExampleFragmentSelector_h
#define ExampleFragmentSelector_h

#include "TChain.h"
#include "TFile.h"

// Header file for the classes stored in the TTree if any.
#include "TFragment.h"
#include "TGRSISelector.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class ExampleFragmentSelector : public TGRSISelector {

public:
   TFragment* fFragment{nullptr};

   explicit ExampleFragmentSelector(TTree* /*tree*/ = nullptr) : TGRSISelector() { SetOutputPrefix("ExampleFragment"); }
   virtual ~ExampleFragmentSelector() = default;
   virtual Int_t Version() const { return 2; }
   void          CreateHistograms();
   void          FillHistograms();
   void          InitializeBranches(TTree* tree);

   ClassDef(ExampleFragmentSelector, 2);
};

#endif

#ifdef ExampleFragmentSelector_cxx
void ExampleFragmentSelector::InitializeBranches(TTree* tree)
{
   if(!tree) return;
   tree->SetBranchAddress("TFragment", &fFragment);
}

#endif   // #ifdef ExampleFragmentSelector_cxx
