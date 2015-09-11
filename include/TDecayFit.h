#ifndef TDECAYFIT_H
#define TDECAYFIT_H

#include "TNamed.h"
#include "TF1.h"
#include "TMath.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TH1.h"

class TDecay : public TNamed {
  public:
   TDecay();
   TDecay(UInt_t generation, TDecay* parent);
   ~TDecay();

  public:
   ///// TF1 Helpers ////
   UInt_t GetGeneration() const  { return fGeneration; }
   Double_t GetHalfLife() const  { return (fDecayFunc->GetParameter(1) > 0.0) ? std::log(2.0)/fDecayFunc->GetParameter(1) : 0.0; }
   Double_t GetDecayRate() const { return fDecayFunc->GetParameter(1); }
   Double_t GetIntensity() const { return fDecayFunc->GetParameter(0); }
   Double_t GetEfficiency() const { return fDetectionEfficiency; }
   Double_t GetHalfLifeError() const  { return GetDecayRate() ? GetHalfLife()*GetDecayRateError()/GetDecayRate() : 0.0;}
   Double_t GetDecayRateError() const { return fDecayFunc->GetParError(1); }
   Double_t GetIntensityError() const { return fDecayFunc->GetParError(0); }
   void SetHalfLife(const Double_t &halflife)  { fDecayFunc->SetParameter(1,std::log(2.0)/halflife); }
   void SetDecayRate(const Double_t &decayrate){ fDecayFunc->SetParameter(1,decayrate); }
   void SetIntensity(const Double_t &intens)   { fDecayFunc->SetParameter(0,intens); }
   void SetEfficiency(const Double_t &eff)     { fDetectionEfficiency = eff; }
   void FixHalfLife(const Double_t &halflife)  { fDecayFunc->FixParameter(1,std::log(2)/halflife); }
   void FixHalfLife()                          { fDecayFunc->FixParameter(1,GetHalfLife());}
   void FixDecayRate(const Double_t &decayrate){ fDecayFunc->FixParameter(1,decayrate); }
   void FixDecayRate()                         { fDecayFunc->FixParameter(0,GetDecayRate());}
   void FixIntensity(const Double_t &intensity){ fDecayFunc->FixParameter(0,intensity); }
   void FixIntensity()                         { fDecayFunc->FixParameter(0,GetIntensity());}
   void ReleaseHalfLife()                      { fDecayFunc->ReleaseParameter(1);}
   void ReleaseDecayRate()                     { fDecayFunc->ReleaseParameter(1);}
   void ReleaseIntensity()                     { fDecayFunc->ReleaseParameter(0);}
   void Draw(Option_t *option = "");
   Double_t Eval(Double_t t);
   TFitResultPtr Fit(TH1* fithist);
   void Fix();
   void Release();

  private:
   void SetDecayRateError(Double_t err) { fDecayFunc->SetParError(1,err); }
   void SetIntensityError(Double_t err) { fDecayFunc->SetParError(0,err); }
   void SetTotalDecayParameters();

  public:
   void SetDaughterDecay(TDecay *daughter) { fDaughter = daughter; }
   void SetParentDecay(TDecay *parent) { fParent = parent; }

   const TF1 * const GetDecayFunc() const { return fDecayFunc; }
   const TF1 * const GetTotalDecayFunc() { SetTotalDecayParameters(); return fTotalDecayFunc; }

   TDecay* const GetParentDecay();
   TDecay* const GetDaughterDecay();
   
   Double_t ActivityFunc(Double_t *dim, Double_t *par);

   void Print(Option_t *option = "") const;

   

  private:
   UInt_t fGeneration;     //Generation from the primary
   Double_t fDetectionEfficiency; //The probability that this decay can be detected
   TF1 *fDecayFunc;        //Function describing decay
   TF1 *fTotalDecayFunc;   //Function used to access other fits
   TDecay *fParent;        //Parent Decay
   TDecay *fDaughter;      //Daughter Decay
   TDecay *fFirstParent;   //FirstParent in the decay

 //  static Double_t ExpDecay(Double_t *dim, Double_t par);

   ClassDef(TDecay,1) //Class containing Decay information
};

class TDecayChain : public TObject {
  public:
   TDecayChain();
   ~TDecayChain();

  private:
   std::vector<TDecay*> fDecayChain; 

   ClassDef(TDecayChain,1) //Class representing a decay chain
};

class TDecayFit : public TObject {
  public:
   TDecayFit();
   ~TDecayFit();

   //TDecay * GetDecay(int idx);

  private:

   ClassDef(TDecayFit,1) //Class for fitting decay curves
};


#endif
