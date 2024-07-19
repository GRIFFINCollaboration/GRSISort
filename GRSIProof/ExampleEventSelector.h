//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Oct 25 13:18:27 2016 by ROOT version 5.34/24
// from TTree FragmentTree/FragmentTree
// found on file: fragment07844_000.root
//////////////////////////////////////////////////////////

#ifndef ExampleEventSelector_h
#define ExampleEventSelector_h

#include "TChain.h"
#include "TFile.h"

#include "TH1.h"
#include "TH2.h"
#include "THnSparse.h"

// Header file for the classes stored in the TTree if any.
#include "TGRSISelector.h"
#include "TGriffin.h"
#include "TSceptar.h"
#include "TGriffinBgo.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class ExampleEventSelector : public TGRSISelector {   // Must be same name as .C and .h

public:
   TGriffin*    fGrif;   // Pointers to spot that events will be
   TSceptar*    fScep;
   TGriffinBgo* fGriffinBgo;

   explicit ExampleEventSelector(TTree* /*tree*/ = 0) : TGRSISelector(), fGrif(nullptr), fScep(nullptr), fGriffinBgo(nullptr)
   {
      SetOutputPrefix("ExampleEvent");   // Changes prefix of output file
   }
   // These functions are expected to exist
   virtual ~ExampleEventSelector() = default;
   virtual Int_t Version() const { return 2; }
   void          CreateHistograms();
   void          FillHistograms();
   void          InitializeBranches(TTree* tree);

   ClassDef(ExampleEventSelector, 2);   // Makes ROOT happier
};

#endif

#ifdef ExampleEventSelector_cxx
void ExampleEventSelector::InitializeBranches(TTree* tree)
{
   if(!tree) return;
   if(tree->SetBranchAddress("TGriffin", &fGrif) == TTree::kMissingBranch) {
      fGrif = new TGriffin;
   }
   if(tree->SetBranchAddress("TSceptar", &fScep) == TTree::kMissingBranch) {
      fScep = new TSceptar;
   }
   if(tree->SetBranchAddress("TGriffinBgo", &fGriffinBgo) == TTree::kMissingBranch) {
      fGriffinBgo = new TGriffinBgo;
   }
}

#endif   // #ifdef ExampleEventSelector_cxx
