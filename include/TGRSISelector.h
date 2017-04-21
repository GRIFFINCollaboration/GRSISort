//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Oct 25 13:18:27 2016 by ROOT version 5.34/24
// from TTree FragmentTree/FragmentTree
// found on file: fragment07844_000.root
//////////////////////////////////////////////////////////

#ifndef TGRSISelector_h
#define TGRSISelector_h

#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TSelector.h"
#include "TH1.h"
#include "TH2.h"
#include "THnSparse.h"

#include <string>

// Fixed size dimensions of array or collections stored in the TTree if any.

class TGRSISelector : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leaf types

   //Methods are purposely not virtual so that TGRSISelector has control
   TGRSISelector(TTree * /*tree*/ =0) : fChain(0) { SetOutputPrefix(ClassName()); }
   virtual ~TGRSISelector() { }
   virtual Int_t   Version() const { return 2; }
   void    Begin(TTree *tree);
   void    SlaveBegin(TTree *tree);
   void    Init(TTree *tree);
   Bool_t  Notify();
   Bool_t  Process(Long64_t entry);
   Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   void    SetOption(const char *option) { fOption = option; }
   void    SetObject(TObject *obj) { fObject = obj; }
   //void    SetInputList(TList *input) { fInput = input; }
   TList  *GetOutputList() const { return fOutput; }
   void    SlaveTerminate();
   void    Terminate();

   virtual void CreateHistograms()   = 0;
   virtual void FillHistograms()     = 0;
   virtual void InitializeBranches(TTree *tree) = 0;
	virtual void EndOfSort() { }; 
   void SetOutputPrefix(const char* prefix) { fOutputPrefix = prefix; }

 protected:
	std::map<std::string, TH1*> fH1;
	std::map<std::string, TH2*> fH2;
	std::map<std::string, THnSparseF*> fHSparse;

 private:
   std::string fOutputPrefix;

   ClassDef(TGRSISelector,2);
};

#endif
