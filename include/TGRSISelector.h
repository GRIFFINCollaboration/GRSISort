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
#include "TCutG.h"

#include "GHSym.h"
#include "GCube.h"
#include "TAnalysisOptions.h"
#include "TPPG.h"
#include "TRunInfo.h"
#include "TGRSIMap.h"

#include <string>

// Fixed size dimensions of array or collections stored in the TTree if any.

class TGRSISelector : public TSelector {
public:
   // Methods are purposely not virtual so that TGRSISelector has control
   TGRSISelector(TTree* /*tree*/ = nullptr) { SetOutputPrefix(ClassName()); }
   ~TGRSISelector() override = default;
   Int_t  Version() const override { return 2; }
   void   Begin(TTree* tree) override;
   void   SlaveBegin(TTree* tree) override;
   void   Init(TTree* tree) override;
   Bool_t Notify() override;
   Bool_t Process(Long64_t entry) override;
   Int_t  GetEntry(Long64_t entry, Int_t getall = 0) override
   {
      return ((fChain != nullptr) ? fChain->GetTree()->GetEntry(entry, getall) : 0);
   }
   void SetOption(const char* option) override { fOption = option; }
   void SetObject(TObject* obj) override { fObject = obj; }
   // void    SetInputList(TList *input) { fInput = input; }
   TList* GetOutputList() const override { return fOutput; }   ///< this does the same as TSelector::GetOutputList()
   void   SlaveTerminate() override;
   void   Terminate() override;

   virtual void CreateHistograms()              = 0;
   virtual void FillHistograms()                = 0;
   virtual void InitializeBranches(TTree* tree) = 0;
   virtual void EndOfSort(){};
   void         SetOutputPrefix(const char* prefix) { fOutputPrefix = prefix; }
   std::string  GetOutputPrefix() const { return fOutputPrefix; }

protected:
   TGRSIMap<std::string, TH1*>        fH1;                 //!<! map for 1-D histograms
   TGRSIMap<std::string, TH2*>        fH2;                 //!<! map for 2-D histograms
   TGRSIMap<std::string, TH3*>        fH3;                 //!<! map for 3-D histograms
   TGRSIMap<std::string, GHSym*>      fSym;                //!<! map for GRSISort's symmetric 2-D histograms
   TGRSIMap<std::string, GCube*>      fCube;               //!<! map for GRSISort's 3-D histograms
   TGRSIMap<std::string, THnSparseF*> fHSparse;            //!<! map for sparse n-D histograms
   TGRSIMap<std::string, TTree*>      fTree;               //!<! map for trees
   std::map<std::string, TCutG*>      fCuts;               //!<! map of cuts
   TPPG*                              fPpg{nullptr};       //!<! pointer to the PPG
   TRunInfo*                          fRunInfo{nullptr};   //!<! pointer to the run info
   int64_t                            fEntry{0};           //!<! entry number currently being processed

private:
   static constexpr int fSizeLimit = 1073741822;   //!<! 1 GB size limit for objects in ROOT

   TTree* fChain{nullptr};   //!<! pointer to the analyzed TTree or TChain

   void              CheckSizes(const char* usage);   ///< Function to check size of objects in output list
   std::string       fOutputPrefix;                   //!<! pre-fix for output files
   TAnalysisOptions* fAnalysisOptions{nullptr};       //!<! pointer to analysis options
   Int_t             fFirstRunNumber{-1};             //!<! run number of first file
   Int_t             fFirstSubRunNumber{-1};          //!<! sub-run number of first file

   /// \cond CLASSIMP
   ClassDefOverride(TGRSISelector, 3) // NOLINT
   /// \endcond
};

#endif
