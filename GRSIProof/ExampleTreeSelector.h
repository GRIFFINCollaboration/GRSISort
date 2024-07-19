#ifndef ExampleTreeSelector_h
#define ExampleTreeSelector_h

/** \addtogroup Selectors
 * @{
 */

////////////////////////////////////////////////////////////////////////////////
/// \class ExampleTreeSelector
///
/// This selector shows how to create a tree with selected events (beta-tagged
/// with gamma multiplicities of at least three), and selected information
/// (suppressed addback energies, and beta-gamma timing differences).
///
////////////////////////////////////////////////////////////////////////////////

#include "TChain.h"
#include "TFile.h"

#include "TH1.h"
#include "TH2.h"
#include "THnSparse.h"

// Header file for the classes stored in the TTree if any.
#include "TGRSISelector.h"
#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TSceptar.h"
#include "TZeroDegree.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class ExampleTreeSelector : public TGRSISelector {   // Must be same name as .C and .h
public:
   explicit ExampleTreeSelector(TTree* /*tree*/ = 0) : TGRSISelector(), fGrif(nullptr), fScep(nullptr), fZds(nullptr), fGriffinBgo(nullptr)
   {
      SetOutputPrefix("ExampleTree");   // Changes prefix of output file
   }
   // These functions are expected to exist
   virtual ~ExampleTreeSelector() {}
   virtual Int_t Version() const { return 1; }
   void          CreateHistograms();
   void          FillHistograms();
   void          InitializeBranches(TTree* tree);

private:
   // branches of input tree
   TGriffin*    fGrif;         ///< pointer for griffin branch
   TSceptar*    fScep;         ///< pointer for sceptar branch
   TGriffinBgo* fGriffinBgo;   ///< pointer for griffin BGO branch
   TZeroDegree* fZds;          ///< pointer for ZDS branch

   // branches of output trees
   std::vector<double> fSuppressedAddback;   ///< vector of suppressed addback energies
   std::vector<double> fBetaGammaTiming;     ///< vector of beta-gamma timing

   ClassDef(ExampleTreeSelector, 2);   // Makes ROOT happier
};
/*! @} */
#endif

#ifdef ExampleTreeSelector_cxx
void ExampleTreeSelector::InitializeBranches(TTree* tree)
{
   if(!tree) return;
   if(tree->SetBranchAddress("TGriffin", &fGrif) == TTree::kMissingBranch) {
      fGrif = new TGriffin;
   }
   if(tree->SetBranchAddress("TSceptar", &fScep) == TTree::kMissingBranch) {
      fScep = new TSceptar;
   }
   if(tree->SetBranchAddress("TZeroDegree", &fZds) == TTree::kMissingBranch) {
      fZds = new TZeroDegree;
   }
   if(tree->SetBranchAddress("TGriffinBgo", &fGriffinBgo) == TTree::kMissingBranch) {
      fGriffinBgo = new TGriffinBgo;
   }
}
#endif   // #ifdef ExampleTreeSelector_cxx
