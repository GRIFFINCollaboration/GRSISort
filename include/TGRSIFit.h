#ifndef TGRSIFIT_H
#define TGRSIFIT_H

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
#include "TRef.h"
#include "TString.h"
#include "Globals.h"

using namespace TGRSIFunctions;

class TGRSIFit : public TF1 {
 public:
   virtual ~TGRSIFit();

 protected: 
   TGRSIFit();
   TGRSIFit(const char* name, const char* formula, Double_t xmin = 0, Double_t xmax = 1) : TF1(name,formula,xmin,xmax){this->Clear(); } 
   TGRSIFit(const char* name, Double_t xmin, Double_t xmax, Int_t npar) : TF1(name,xmin,xmax,npar){this->Clear(); }
   TGRSIFit(const char* name, ROOT::Math::ParamFunctor f, Double_t xmin = 0, Double_t xmax = 1, Int_t npar = 0) : TF1(name,f,xmin,xmax,npar){this->Clear(); }

   TGRSIFit(const TGRSIFit &copy);
  
 public:
   virtual void Copy(TObject &copy) const;
   //Every fit object should have to initialize parameters and have a fit method defined.
   virtual Bool_t InitParams(TH1*) = 0;
   Bool_t IsGoodFit() const { return fGoodFitFlag; }
   virtual void SetHist(TH1* hist) { fHist = hist;} //fHistogram is a member of TF1. I'm not sure this does anything proper right now
   virtual TH1* GetHist() const { return static_cast<TH1*>(fHist.GetObject());}
   static const char* GetDefaultFitType(){ return fDefaultFitType.Data(); }
   static void SetDefaultFitType(const char* fitType){ fDefaultFitType = fitType; }

   //These are only to be called in the Dtor of classes to protect from ROOT's insane garbage collection system
   //They can be called anywhere though as long as new classes are carefully destructed. 
   Bool_t AddToGlobalList(Bool_t on = kTRUE);
   static Bool_t AddToGlobalList(TF1* func, Bool_t on = kTRUE);

 protected:
   Bool_t IsInitialized() const { return fInitFlag; }
   void SetInitialized(Bool_t flag = true) { fInitFlag = flag;}
   void GoodFit(Bool_t flag = true) { fGoodFitFlag = flag; }

 private:
   Bool_t fInitFlag;
   Bool_t fGoodFitFlag; //This doesn't do anything yet
   TRef fHist;
   static TString fDefaultFitType;

 public:  
   virtual void Print(Option_t* opt = "") const;
   virtual void Clear(Option_t* opt = "" );
   virtual void ClearParameters(Option_t* opt = "");
   virtual void CopyParameters(TF1* copy) const;

/// \cond CLASSIMP
   ClassDef(TGRSIFit,0);
/// \endcond
};

#endif
