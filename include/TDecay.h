// Author: Ryan Dunlop    09/15
#ifndef TDECAY_H
#define TDECAY_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <string>

#include "TNamed.h"
#include "TMath.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TGraph.h"
#include "TF1.h"
#include "TH1.h"
#include "TVirtualFitter.h"

class TVirtualDecay;
class TSingleDecay;
class TDecay;
class TDecayChain;

class TDecayFit : public TF1 {
public:
   TDecayFit() : fDecay(nullptr) {}
   TDecayFit(const char* name, const char* formula, Double_t xmin = 0, Double_t xmax = 1)
      : TF1(name, formula, xmin, xmax), fDecay(nullptr)
   {
      DefaultGraphs();
   }
   TDecayFit(const char* name, Double_t xmin, Double_t xmax, Int_t npar) : TF1(name, xmin, xmax, npar), fDecay(nullptr)
   {
      DefaultGraphs();
   }
   TDecayFit(const char* name, const ROOT::Math::ParamFunctor& f, Double_t xmin = 0, Double_t xmax = 1, Int_t npar = 0)
      : TF1(name, f, xmin, xmax, npar), fDecay(nullptr)
   {
      DefaultGraphs();
   }
#if !defined(__CINT__) && !defined(__CLING__)
   TDecayFit(const char* name, Double_t (*fcn)(Double_t*, Double_t*), Double_t xmin = 0, Double_t xmax = 1,
             Int_t npar = 0)
      : TF1(name, fcn, xmin, xmax, npar), fDecay(nullptr)
   {
      DefaultGraphs();
   }
   TDecayFit(const char* name, Double_t (*fcn)(const Double_t*, const Double_t*), Double_t xmin = 0, Double_t xmax = 1,
             Int_t npar = 0)
      : TF1(name, fcn, xmin, xmax, npar), fDecay(nullptr)
   {
      DefaultGraphs();
   }
#endif

   template <class PtrObj, typename MemFn>
   TDecayFit(const char* name, const PtrObj& p, MemFn memFn, Double_t xmin, Double_t xmax, Int_t npar,
             const char* className = nullptr, const char* methodName = nullptr)
      : TF1(name, p, memFn, xmin, xmax, npar, className, methodName), fDecay(nullptr)
   {
      DefaultGraphs();
   }

   template <typename Func>
   TDecayFit(const char* name, Func f, Double_t xmin, Double_t xmax, Int_t npar, const char* className = nullptr)
      : TF1(name, f, xmin, xmax, npar, className), fDecay(nullptr)
   {
      DefaultGraphs();
   }
   ~TDecayFit() override = default;

   void           SetDecay(TVirtualDecay* decay);
   TVirtualDecay* GetDecay() const;
   void           DrawComponents() const;   // *MENU*

   void          Print(Option_t* opt = "") const override;
   void          UpdateResiduals(TH1* hist);
   void          DrawResiduals();   // *MENU*
   TGraph*       GetResiduals() { return &fResiduals; }
   TFitResultPtr Fit(TH1* hist, Option_t* opt = "");

private:
   void DefaultGraphs();

   TVirtualDecay* fDecay{nullptr};       // VirtualDecay that made this fit
   TGraph         fResiduals;   // Last histogram fit by this function

   /// \cond CLASSIMP
   ClassDefOverride(TDecayFit, 1);   // Extends TF1 for nuclear decays
   /// \endcond
};

class TVirtualDecay : public TNamed {
public:
   TVirtualDecay() {}
   ~TVirtualDecay() override = default;

   virtual void DrawComponents(Option_t* opt = "", Bool_t color_flag = true);
   void         Print(Option_t* opt = "") const override = 0;

private:
   virtual const TDecayFit* GetFitFunction() = 0;

   /// \cond CLASSIMP
   ClassDefOverride(TVirtualDecay, 1)   // Abstract Class for TDecayFit
   /// \endcond
};

class TSingleDecay : public TVirtualDecay {
   friend class TDecayChain;
   friend class TDecayFit;
   friend class TDecay;
   //  friend class TDecay;

public:
   TSingleDecay()
      : fDetectionEfficiency(1.0), fDecayFunc(nullptr), fTotalDecayFunc(nullptr), fParent(nullptr), fDaughter(nullptr),
        fFirstParent(nullptr), fChainId(-1)
   {
   }
   TSingleDecay(UInt_t generation, TSingleDecay* parent, Double_t tlow = 0, Double_t thigh = 10);
   explicit TSingleDecay(TSingleDecay* parent, Double_t tlow = 0, Double_t thigh = 10);
   ~TSingleDecay() override;

   ///// TF1 Helpers ////
   UInt_t   GetGeneration() const { return fGeneration; }
   Double_t GetHalfLife() const
   {
      return (fDecayFunc->GetParameter(1) > 0.) ? std::log(2.0) / fDecayFunc->GetParameter(1) : 0.0;
   }
   Double_t GetDecayRate() const { return fDecayFunc->GetParameter(1); }
   Double_t GetIntensity() const { return fDecayFunc->GetParameter(0); }
   Double_t GetEfficiency() const { return fDetectionEfficiency; }
   Double_t GetHalfLifeError() const
   {
      return (GetDecayRate() > 0.) ? GetHalfLife() * GetDecayRateError() / GetDecayRate() : 0.0;
   }
   Double_t GetDecayRateError() const { return fDecayFunc->GetParError(1); }
   Double_t GetIntensityError() const { return fDecayFunc->GetParError(0); }
   void     SetHalfLife(const Double_t& halflife)
   {
      fDecayFunc->SetParameter(1, std::log(2.0) / halflife);
      UpdateDecays();
   }
   void SetDecayRate(const Double_t& decayrate)
   {
      fDecayFunc->SetParameter(1, decayrate);
      UpdateDecays();
   }
   void SetIntensity(const Double_t& intens)
   {
      fDecayFunc->SetParameter(0, intens);
      UpdateDecays();
   }
   void SetEfficiency(const Double_t& eff) { fDetectionEfficiency = eff; }
   void FixHalfLife(const Double_t& halflife)
   {
      fDecayFunc->FixParameter(1, std::log(2) / halflife);
      UpdateDecays();
   }
   void FixHalfLife()
   {
      fDecayFunc->FixParameter(1, GetHalfLife());
      UpdateDecays();
   }
   void FixDecayRate(const Double_t& decayrate)
   {
      fDecayFunc->FixParameter(1, decayrate);
      UpdateDecays();
   }
   void FixDecayRate()
   {
      fDecayFunc->FixParameter(0, GetDecayRate());
      UpdateDecays();
   }
   void          FixIntensity(const Double_t& intensity) { fDecayFunc->FixParameter(0, intensity); }
   void          FixIntensity() { fDecayFunc->FixParameter(0, GetIntensity()); }
   void          SetHalfLifeLimits(const Double_t& low, const Double_t& high);
   void          SetIntensityLimits(const Double_t& low, const Double_t& high);
   void          SetDecayRateLimits(const Double_t& low, const Double_t& high);
   void          GetHalfLifeLimits(Double_t& low, Double_t& high) const;
   void          GetIntensityLimits(Double_t& low, Double_t& high) const;
   void          GetDecayRateLimits(Double_t& low, Double_t& high) const;
   void          ReleaseHalfLife() { fDecayFunc->ReleaseParameter(1); }
   void          ReleaseDecayRate() { fDecayFunc->ReleaseParameter(1); }
   void          ReleaseIntensity() { fDecayFunc->ReleaseParameter(0); }
   void          Draw(Option_t* option = "") override;
   Double_t      Eval(Double_t t);
   Double_t      EvalPar(const Double_t* x, const Double_t* par = nullptr);
   TFitResultPtr Fit(TH1* fithist, Option_t* opt = "");
   void          Fix();
   void          Release();
   void          SetRange(Double_t tlow, Double_t thigh);
   void          SetName(const char* name) override;
   void          SetLineColor(Color_t color) { fTotalDecayFunc->SetLineColor(color); }
   Color_t       GetLineColor() const { return fTotalDecayFunc->GetLineColor(); }
   void          SetMinimum(Double_t min)
   {
      fTotalDecayFunc->SetMinimum(min);
      fDecayFunc->SetMinimum(min);
   }
   void SetMaximum(Double_t max)
   {
      fTotalDecayFunc->SetMaximum(max);
      fDecayFunc->SetMaximum(max);
   }

private:
   void SetDecayRateError(Double_t err) { fDecayFunc->SetParError(1, err); }
   void SetIntensityError(Double_t err) { fDecayFunc->SetParError(0, err); }

   void UpdateDecays();

   void SetChainId(Int_t id) { fChainId = id; }

public:
   void  SetDaughterDecay(TSingleDecay* daughter) { fDaughter = daughter; }
   void  SetParentDecay(TSingleDecay* parent) { fParent = parent; }
   void  SetTotalDecayParameters();
   void  SetDecayId(Int_t Id) { fUnId = Id; }
   Int_t GetDecayId() const { return fUnId; }
   Int_t GetChainId() const { return fChainId; }

   const TDecayFit* GetDecayFunc() const { return fDecayFunc; }
   const TDecayFit* GetTotalDecayFunc()
   {
      SetTotalDecayParameters();
      return fTotalDecayFunc;
   }

   TSingleDecay* GetParentDecay();
   TSingleDecay* GetDaughterDecay();

   Double_t ActivityFunc(Double_t* dim, Double_t* par);

   void Print(Option_t* option = "") const override;

private:
   const TDecayFit* GetFitFunction() override
   {
      SetTotalDecayParameters();
      return fTotalDecayFunc;
   }

   UInt_t        fGeneration{0};             // Generation from the primary
   Double_t      fDetectionEfficiency{0.};   // The probability that this decay can be detected
   TDecayFit*    fDecayFunc{nullptr};        // Function describing decay
   TDecayFit*    fTotalDecayFunc{nullptr};   // Function used to access other fits
   TSingleDecay* fParent{nullptr};           // Parent Decay
   TSingleDecay* fDaughter{nullptr};         // Daughter Decay
   TSingleDecay* fFirstParent{nullptr};      // FirstParent in the decay
   Int_t         fUnId{0};                   // The Unique ID of the Decay
   static UInt_t fCounter;                   // Helps set unique Id's
   Int_t         fChainId{0};                // The chain that the single decay belongs to

   /// \cond CLASSIMP
   ClassDefOverride(TSingleDecay, 1)   // Class containing Single Decay information
   /// \endcond
};

class TDecayChain : public TVirtualDecay {
public:
   TDecayChain();
   explicit TDecayChain(UInt_t generations);
   ~TDecayChain() override;

   TSingleDecay* GetDecay(UInt_t generation);
   Double_t      Eval(Double_t t) const;
   void          Draw(Option_t* opt = "") override;
   Int_t         Size() const { return static_cast<Int_t>(fDecayChain.size()); }

   void Print(Option_t* option = "") const override;

   void             SetChainParameters();
   void             SetRange(Double_t xlow, Double_t xhigh);
   const TDecayFit* GetChainFunc()
   {
      SetChainParameters();
      return fChainFunc;
   }
   void          DrawComponents(Option_t* opt = "", Bool_t color_flag = true) override;
   TFitResultPtr Fit(TH1* fithist, Option_t* opt = "");
   Double_t      EvalPar(const Double_t* x, const Double_t* par = nullptr);

   Int_t GetChainId() const { return fChainId; }

private:
   void             AddToChain(TSingleDecay* decay);
   Double_t         ChainActivityFunc(Double_t* dim, Double_t* par);
   static UInt_t    fChainCounter;
   const TDecayFit* GetFitFunction() override
   {
      SetChainParameters();
      return fChainFunc;
   }

   std::vector<TSingleDecay*> fDecayChain;            // The Decays in the Decay Chain
   TDecayFit*                 fChainFunc{nullptr};    // Function describing the total chain activity
   Int_t                      fChainId{0};

   /// \cond CLASSIMP
   ClassDefOverride(TDecayChain, 1)   // Class representing a decay chain
   /// \endcond
};

////////////////////////////////////////////////////////////////////////
///
/// \class TDecay
///
/// TDecay is a class for fitting halflives during nuclear decay
/// A TDecay consists of multiple TDecayChains, where a TDecayChain
/// is starts at a specific nucleus which has a population before the
/// decay fit takes place. This could be a nucleus with a daughter.
/// One TDecayChain would consist of just the daughter while the
/// the other decay chain would be the parent and daughter.
/// TDecayChains are made up of multiple TSingleDecays which holds
/// the nucleus specific information such as name, id, halflife and
/// intensity. When any of the above classes are fit to a histogram,
/// they use a TDecayFit. The TDecayFit is a a TF1 with extra information
/// such as the class that was used to create the TDecayFit. Furthermore,
/// the function DrawComponents() can be used to draw the activites of the
/// individual nuclei involved in the TDecayFit.
///
////////////////////////////////////////////////////////////////////////

class TDecay : public TVirtualDecay {
public:
   TDecay() : fFitFunc(nullptr) {}
   explicit TDecay(std::vector<TDecayChain*> chainList);
   ~TDecay() override = default;

   void         AddChain(TDecayChain* chain) { fChainList.push_back(chain); }
   Double_t     DecayFit(Double_t* dim, Double_t* par);
   TDecayChain* GetChain(UInt_t idx);

   void SetHalfLife(Int_t Id, Double_t halflife);
   void SetHalfLifeLimits(Int_t Id, Double_t low, Double_t high);
   void SetDecayRateLimits(Int_t Id, Double_t low, Double_t high);
   void FixHalfLife(Int_t Id, Double_t halflife)
   {
      SetHalfLife(Id, halflife);
      SetHalfLifeLimits(Id, halflife, halflife);
   }
   TFitResultPtr Fit(TH1* fithist, Option_t* opt = "");

   void       Print(Option_t* opt = "") const override;
   void       PrintMap() const;
   TDecayFit* GetFitFunc() { return fFitFunc; }
   void       SetBackground(Double_t background) { fFitFunc->SetParameter(0, background); }
   Double_t   GetBackground() const { return fFitFunc->GetParameter(0); }
   Double_t   GetBackgroundError() const { return fFitFunc->GetParError(0); }
   void       SetRange(Double_t xlow, Double_t xhigh);
   void       DrawComponents(Option_t* opt = "", Bool_t color_flag = true) override;
   void       Draw(Option_t* opt = "") override;
   void       DrawBackground(Option_t* opt = "");
   void       FixBackground(const Double_t& background) { fFitFunc->FixParameter(0, background); }
   void       FixBackground() { fFitFunc->FixParameter(0, GetBackground()); }
   void       SetBackgroundLimits(const Double_t& low, const Double_t& high) { fFitFunc->SetParLimits(0, low, high); }
   void       ReleaseBackground() { fFitFunc->ReleaseParameter(0); }

   TGraph* GetResiduals() { return fFitFunc->GetResiduals(); }

private:
   void             RemakeMap();
   void             SetParameters();
   Double_t         ComponentFunc(Double_t* dim, Double_t* par);
   const TDecayFit* GetFitFunction() override { return fFitFunc; }

private:
   std::vector<TDecayChain*>                   fChainList;
   TDecayFit*                                  fFitFunc{nullptr};
   std::map<Int_t, std::vector<TSingleDecay*>> fDecayMap;   //

   /// \cond CLASSIMP
   ClassDefOverride(TDecay, 1)   // Contains all decay chains in a fit
   /// \endcond
};
/*! @} */
#endif
