//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

#ifndef BasicPACESEventSelector_h
#define BasicPACESEventSelector_h

#include "TChain.h"
#include "TFile.h"

#include "TH1.h"
#include "TH2.h"
#include "THnSparse.h"
#include "TGraph.h"

// Header file for the classes stored in the TTree if any.
#include "TPaces.h"
#include "TGRSISelector.h"
#include "TGRSIRunInfo.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class BasicPACESEventSelector : public TGRSISelector {

 public :
   TPaces      * fPaces;

   //PACES numbering system begins at #1 for experiments circa October 2017 (after run 10097) and #0 preceding this time.
   int runRef = 10097;
   //Array of residuals to correct PACES non-linearities.
   //x = measured energy (keV), y = energy residual (correction, keV), dp = number of data points
   int dp = 10;
   double x[5][10]={{50.,100.,200.,300.,400.,500.,600.,700.,800.,900.},
		    {50.,100.,200.,300.,400.,500.,600.,700.,800.,900.},
		    {50.,100.,200.,300.,400.,500.,600.,700.,800.,900.},
		    {50.,100.,200.,300.,400.,500.,600.,700.,800.,900.},
		    {50.,100.,200.,300.,400.,500.,600.,700.,800.,900.}
		    };
   double y[5][10]={{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},
		    {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},
		    {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},
		    {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},
		    {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}
		    };

   BasicPACESEventSelector(TTree * /*tree*/ =0) : TGRSISelector(), fPaces(0) {
      SetOutputPrefix("BasicPACESEvent");
   }
   virtual ~BasicPACESEventSelector() { }
   virtual Int_t   Version() const { return 2; }
   void CreateHistograms();
   void FillHistograms();
   void InitializeBranches(TTree *tree);

   ClassDef(BasicPACESEventSelector,2);
};

#endif

#ifdef BasicPACESEventSelector_cxx
void BasicPACESEventSelector::InitializeBranches(TTree* tree) {
   if (!tree) return;
   tree->SetBranchAddress("TPaces", &fPaces);

}

#endif // #ifdef BasicPACESEventSelector_cxx
