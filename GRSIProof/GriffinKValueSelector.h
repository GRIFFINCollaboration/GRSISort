//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Oct 25 13:18:27 2016 by ROOT version 5.34/24
// from TTree FragmentTree/FragmentTree
// found on file: fragment07844_000.root
//////////////////////////////////////////////////////////

#ifndef GriffinKValueSelector_h
#define GriffinKValueSelector_h

#include "TChain.h"
#include "TFile.h"

// Header file for the classes stored in the TTree if any.
#include "TFragment.h"
#include "TGRSISelector.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class GriffinKValueSelector : public TGRSISelector {

public:
   TFragment* fFragment{nullptr};

   explicit GriffinKValueSelector(TTree* /*tree*/ = 0) : TGRSISelector() { SetOutputPrefix("GriffinKValue"); }
   virtual ~GriffinKValueSelector() = default;
   virtual Int_t Version() const { return 2; }
   void          CreateHistograms();
   void          FillHistograms();
   void          InitializeBranches(TTree* tree);

   ClassDef(GriffinKValueSelector, 2);
};

#endif

#ifdef GriffinKValueSelector_cxx
void GriffinKValueSelector::InitializeBranches(TTree* tree)
{
   if(!tree) return;
   tree->SetBranchAddress("TFragment", &fFragment);
}

#endif   // #ifdef GriffinKValueSelector_cxx
