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
#include "GHSym.h"
#include "GCube.h"
#include "TAnalysisOptions.h"

#include <string>

// Fixed size dimensions of array or collections stored in the TTree if any.

class TGRSISelector : public TSelector {
public:
   TTree* fChain; //! pointer to the analyzed TTree or TChain

   // Declaration of leaf types

   // Methods are purposely not virtual so that TGRSISelector has control
   TGRSISelector(TTree* /*tree*/ = nullptr) : fChain(nullptr) { SetOutputPrefix(ClassName()); }
   ~TGRSISelector() override     = default;
   Int_t Version() const override { return 2; }
   void Begin(TTree* tree) override;
   void SlaveBegin(TTree* tree) override;
   void Init(TTree* tree) override;
   Bool_t Notify() override;
   Bool_t Process(Long64_t entry) override;
   Int_t GetEntry(Long64_t entry, Int_t getall = 0) override
   {
      return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0;
   }
   void SetOption(const char* option) override { fOption = option; }
   void SetObject(TObject* obj) override { fObject = obj; }
   // void    SetInputList(TList *input) { fInput = input; }
   TList* GetOutputList() const override { return fOutput; }
   void   SlaveTerminate() override;
   void   Terminate() override;

   virtual void CreateHistograms()              = 0;
   virtual void FillHistograms()                = 0;
   virtual void InitializeBranches(TTree* tree) = 0;
   virtual void EndOfSort() {};
   void SetOutputPrefix(const char* prefix) { fOutputPrefix = prefix; }

protected:
   std::map<std::string, TH1*>        fH1;
   std::map<std::string, TH2*>        fH2;
   std::map<std::string, GHSym*>      fSym;
   std::map<std::string, GCube*>      fCube;
   std::map<std::string, THnSparseF*> fHSparse;

private:
   std::string       fOutputPrefix;
   TAnalysisOptions* fAnalysisOptions{nullptr};

   ClassDefOverride(TGRSISelector, 2);
};

#endif
