#ifndef TGRSIFIT_H
#define TGRSIFIT_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */
#include <utility>

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
#include "TRef.h"
#include "TString.h"
#include "Globals.h"

class TGRSIFit : public TF1 {
public:
   ~TGRSIFit();

protected:
   TGRSIFit();
   TGRSIFit(const char* name, const char* formula, Double_t xmin = 0, Double_t xmax = 1) : TF1(name, formula, xmin, xmax) { Clear(); }
   TGRSIFit(const char* name, Double_t xmin, Double_t xmax, Int_t npar) : TF1(name, xmin, xmax, npar) { Clear(); }
   TGRSIFit(const char* name, const ROOT::Math::ParamFunctor& func, Double_t xmin = 0, Double_t xmax = 1, Int_t npar = 0) : TF1(name, func, xmin, xmax, npar) { Clear(); }
   template <class PtrObj, typename MemFn>
   TGRSIFit(const char* name, const PtrObj& pointer, MemFn memFn, Double_t xmin, Double_t xmax, Int_t npar, const char* class_name, const char* fcn_name)
      : TF1(name, pointer, memFn, xmin, xmax, npar, class_name, fcn_name)
   {
      Clear();
   }

   TGRSIFit(const TGRSIFit& copy);

   TGRSIFit(TGRSIFit&&) noexcept            = default;
   TGRSIFit& operator=(const TGRSIFit&)     = default;
   TGRSIFit& operator=(TGRSIFit&&) noexcept = default;

public:
   void Copy(TObject& obj) const override;
   // Every fit object should have to initialize parameters and have a fit method defined.
   virtual Bool_t InitParams(TH1*) = 0;
   Bool_t         IsGoodFit() const { return fGoodFitFlag; }
   virtual void   SetHist(TH1* hist)
   {
      fHist = hist;
   }   // fHistogram is a member of TF1. I'm not sure this does anything proper right now
   virtual TH1*       GetHist() const { return dynamic_cast<TH1*>(fHist.GetObject()); }
   static const char* GetDefaultFitType() { return fDefaultFitType.Data(); }
   static void        SetDefaultFitType(const char* fitType) { fDefaultFitType = fitType; }

   // These are only to be called in the Dtor of classes to protect from ROOT's insane garbage collection system
   // They can be called anywhere though as long as new classes are carefully destructed.
   Bool_t        AddToGlobalList(Bool_t yes = kTRUE) override;
   static Bool_t AddToGlobalList(TF1* func, Bool_t yes = kTRUE);

protected:
   Bool_t IsInitialized() const { return fInitFlag; }
   void   SetInitialized(Bool_t flag = true) { fInitFlag = flag; }
   void   GoodFit(Bool_t flag = true) { fGoodFitFlag = flag; }

private:
   Bool_t         fInitFlag{false};
   Bool_t         fGoodFitFlag{false};   // This doesn't do anything yet
   TRef           fHist;
   static TString fDefaultFitType;

public:
   void         Print(Option_t* opt = "") const override;
   void         Clear(Option_t* opt = "") override;
   virtual void ClearParameters(Option_t* opt = "");
   virtual void CopyParameters(TF1* copy) const;

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIFit, 0)   // NOLINT
   /// \endcond
};
/*! @} */
#endif
