#ifndef __TGRSIFIT_H
#define __TGRSIFIT_H

#include "TGRSIFunctions.h"
#include "TObject.h"
#include "TH1.h"
#include "TF1.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TMath.h"
#include "TNamed.h"
#include "TROOT.h"
#include <utility>

using namespace TGRSIFunctions;

class TGRSIFit : public TNamed {
 public:
   virtual ~TGRSIFit(){};

 protected: 
   TGRSIFit();
   TGRSIFit(const TGRSIFit &copy);
  
 public:
   //Every fit object should have to initialize parameters and have a fit method defined.
   virtual Double_t Fit(Option_t *opt = "") = 0;
   virtual Bool_t InitParams() = 0;

 protected:
   void SetFitResult(TFitResultPtr fitresult){ ffitresult = fitresult;} 
   Bool_t IsInitialized() const { return init_flag; }
   void SetInitialized(Bool_t flag = true) {init_flag = flag;}

 private:
   TFitResultPtr ffitresult;//->
   Bool_t init_flag;

 public:  
   virtual void Print(Option_t *opt = "") const;
   virtual void Clear();

   ClassDef(TGRSIFit,0);
};

#endif
