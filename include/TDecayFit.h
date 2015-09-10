#ifndef TDECAYFIT_H
#define TDECAYFIT_H

#include "TNamed.h"
#include "TF1.h"

class TDecay : public TNamed {
  public:
   TDecay();
   TDecay(UInt_t generation, TDecay* parent);
   ~TDecay();

  public:
   ///// TF1 Helpers /////
   Double_t GetHalfLife() const  { return (fDecayFunc->GetParameter(1) > 0.0) ? std::log(2.0)/fDecayFunc->GetParameter(1) : 0.0; }
   Double_t GetDecayRate() const { return fDecayFunc->GetParameter(1); }
   Double_t GetIntensity() const { return fDecayFunc->GetParameter(0); }
   void SetHalfLife(const Double_t &halflife)  { fDecayFunc->SetParameter(1,std::log(2.0)/halflife); }
   void SetDecayRate(const Double_t &decayrate){ fDecayFunc->SetParameter(1,decayrate); }
   void SetIntensity(const Double_t &intens)   { fDecayFunc->SetParameter(0,intens); }
   void FixHalfLife(const Double_t &halflife)  { fDecayFunc->FixParameter(1,std::log(2)/halflife); }
   void FixHalfLife()                          { fDecayFunc->FixParameter(1,GetHalfLife());}
   void FixDecayRate(const Double_t &decayrate){ fDecayFunc->FixParameter(1,decayrate); }
   void FixDecayRate()                         { fDecayFunc->FixParameter(1,GetDecayRate());}
   void FixIntensity(const Double_t &intensity){ fDecayFunc->FixParameter(0,intensity); }
   void FixIntensity()                         { fDecayFunc->FixParameter(0,GetIntensity());}
   void Draw(Option_t *option = "");
   Double_t Eval(Double_t t);

   void SetDaughterDecay(TDecay *daughter) { fDaughter = daughter; }
   void SetParentDecay(TDecay *parent) { fParent = parent; }

   const TF1 * const GetDecayFunc() { return fDecayFunc; }

   TDecay* const GetParentDecay();
   TDecay* const GetDaughterDecay();
   
   Double_t ActivityFunc(Double_t *dim, Double_t *par);

   void Print(Option_t *option = "") const;

  private:
   UInt_t fGeneration;     //Generation from the primary
   TF1 *fDecayFunc;        //Function describing decay
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
