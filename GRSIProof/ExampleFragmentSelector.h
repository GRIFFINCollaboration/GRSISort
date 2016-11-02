//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Oct 25 13:18:27 2016 by ROOT version 5.34/24
// from TTree FragmentTree/FragmentTree
// found on file: fragment07844_000.root
//////////////////////////////////////////////////////////

#ifndef ExampleFragmentSelector_h
#define ExampleFragmentSelector_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

// Header file for the classes stored in the TTree if any.
#include "/home/rdunlop/GRSISort/include/TFragment.h"
#include "/home/rdunlop/GRSISort/include/TFragmentSelector.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class ExampleFragmentSelector : public TFragmentSelector {

 public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leaf types
   //TFragment       *TFragment;
   TFragment *fFragment;

   ExampleFragmentSelector(TTree * /*tree*/ =0) : fChain(0),fFragment(0) { }
   virtual ~ExampleFragmentSelector() { }
 //  virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
 //  virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
 //  virtual void    SetOption(const char *option) { fOption = option; }
 //  virtual void    SetObject(TObject *obj) { fObject = obj; }
 //  virtual void    SetInputList(TList *input) { fInput = input; }
 //  virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   ClassDef(ExampleFragmentSelector,2);
};

#endif

#ifdef ExampleFragmentSelector_cxx
void ExampleFragmentSelector::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).
   // Set branch addresses and branch pointers
   std::cout << "Init " << std::endl;
   if (!tree) return;
   
   fChain = tree;
   //fChain->SetMakeClass(1);
   
   fChain->SetBranchAddress("TFragment", &fFragment);
}


Bool_t ExampleFragmentSelector::Notify()
{

   std::cout << "Notified " << std::endl;
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef ExampleFragmentSelector_cxx
