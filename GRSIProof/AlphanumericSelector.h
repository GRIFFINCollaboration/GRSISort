//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Oct 25 13:18:27 2016 by ROOT version 5.34/24
// from TTree FragmentTree/FragmentTree
// found on file: fragment07844_000.root
//////////////////////////////////////////////////////////

#ifndef AlphanumericSelector_h
#define AlphanumericSelector_h

#include "TChain.h"
#include "TFile.h"

#include "TH1.h"
#include "TH2.h"
#include "THnSparse.h"

// Header file for the classes stored in the TTree if any.
#include "TGRSISelector.h"
#include "TGriffin.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class AlphanumericSelector : public TGRSISelector {   // Must be same name as .C and .h

public:
   TGriffin* fGrif;   // Pointers to spot that events will be

   explicit AlphanumericSelector(TTree* /*tree*/ = 0) : TGRSISelector(), fGrif(nullptr)
   {
      SetOutputPrefix("Alphanumeric");   // Changes prefix of output file
   }
   // These functions are expected to exist
   virtual ~AlphanumericSelector() = default;
   virtual Int_t Version() const { return 2; }
   void          CreateHistograms();
   void          FillHistograms();
   void          InitializeBranches(TTree* tree);

   ClassDef(AlphanumericSelector, 2);   // Makes ROOT happier
};

#endif

#ifdef AlphanumericSelector_cxx
void AlphanumericSelector::InitializeBranches(TTree* tree)
{
   if(!tree) return;
   if(tree->SetBranchAddress("TGriffin", &fGrif) == TTree::kMissingBranch) {
      fGrif = new TGriffin;
   }
}

#endif   // #ifdef AlphanumericSelector_cxx
