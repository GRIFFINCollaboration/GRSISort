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
#include "TGriffin.h"
#include "TSceptar.h"
#include "TGRSISelector.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class ExampleEventSelector : public TGRSISelector {

 public :
   TGriffin * fGrif;
   TSceptar * fScep;

   ExampleEventSelector(TTree * /*tree*/ =0) : TGRSISelector(), fGrif(0), fScep(0) {
      SetOutputPrefix("ExampleEvent");
   }
   virtual ~ExampleEventSelector() { }
   virtual Int_t   Version() const { return 2; }
   void CreateHistograms();
   void FillHistograms();
   void InitializeBranches(TTree *tree);

   ClassDef(ExampleEventSelector,2);
};

#endif

#ifdef ExampleEventSelector_cxx
void ExampleEventSelector::InitializeBranches(TTree* tree) {
   if (!tree) return;
   tree->SetBranchAddress("TGriffin", &fGrif);
   tree->SetBranchAddress("TSceptar", &fScep);
}

#endif // #ifdef ExampleEventSelector_cxx
