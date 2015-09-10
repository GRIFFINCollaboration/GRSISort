#include "TDecayFit.h"

ClassImp(TDecay)
ClassImp(TDecayChain)
ClassImp(TDecayFit)

TDecay::TDecay() : fParent(0), fDaughter(0), fDecayFunc(0), fGeneration(1) {
   fFirstParent = this;
   fDecayFunc = new TF1("decayfunc",this,&TDecay::ActivityFunc,0,10,2,"TDecay","ActivityFunc");
   fDecayFunc->SetParameters(0.0,0.0);
   fDecayFunc->SetParNames("Intensity","DecayRate");
}

TDecay::TDecay(UInt_t generation, TDecay* parent) : fParent(0), fDaughter(0), fDecayFunc(0){
   if(parent){
      fParent = parent;
      fParent->SetDaughterDecay(this);
      //See if the decay chain makes sense.
      TDecay* curParent = parent;
      UInt_t gencounter = 2;
      for(int i=0; i<generation; ++i){
         if(curParent->GetParentDecay()){
            curParent = parent->GetParentDecay();
            ++gencounter;
         }
         else{
            fFirstParent = curParent;  
         }
      }
      if(gencounter != generation)
         printf("Generation numbers do not make sense\n");
   }
   if(generation == 1){
      fFirstParent = this;
   }

   fGeneration = generation;

   fDecayFunc = new TF1("decayfunc",this,&TDecay::ActivityFunc,0,10,2,"TDecay","ActivityFunc");
   fDecayFunc->SetParameters(fFirstParent->GetIntensity(),0.0);
   fDecayFunc->SetParNames("Intensity","DecayRate");
}

TDecay::~TDecay() {
   if(fDecayFunc)
      delete fDecayFunc;
}

TDecay* const TDecay::GetParentDecay(){
   return fParent;
}

TDecay* const TDecay::GetDaughterDecay(){
   return fDaughter;
}

void TDecay::Draw(Option_t* option){
   fDecayFunc->Draw(option);
}

Double_t TDecay::Eval(Double_t t){
   return fDecayFunc->Eval(t);
}

Double_t TDecay::ActivityFunc(Double_t *dim, Double_t *par){
   //The general function for a decay chain
   //par[0] is the intensity
   //par[1] is the activity
   Double_t result = 1.0;
   UInt_t gencounter = 0;
   TDecay *curDecay = this;
   //Compute the first multiplication
   while(curDecay){
      ++gencounter;

      if(curDecay == this)
         result *= par[1];
      else
         result *= curDecay->GetDecayRate();

      curDecay = curDecay->GetParentDecay();
   }
   if(gencounter != fGeneration){
         printf("We have Problems!\n");
         return 0.0;
   }
   //Multiply by the initial intensity of the intial parent.

   fFirstParent->SetIntensity(par[0]);
   if(fFirstParent == this)
      result*=fFirstParent->GetIntensity()/par[1];
   else
      result*=fFirstParent->GetIntensity()/fFirstParent->GetDecayRate();

   //Now we need to deal with the second term
   Double_t sum = 0.0;
   curDecay = this;
   while(curDecay){
      Double_t denom = 1.0;
      TDecay* denomDecay = this;
      while(denomDecay){
         if(denomDecay != curDecay){ 
       
            if(denomDecay == this)
               denom*=par[1] - curDecay->GetDecayRate(); 
            else if (curDecay == this)
               denom*=denomDecay->GetDecayRate() - par[1]; 
            else
               denom*=denomDecay->GetDecayRate() - curDecay->GetDecayRate(); 

         } 
         denomDecay = denomDecay->GetParentDecay();
      }

      if(curDecay == this)
         sum+=TMath::Exp(-par[1]*dim[0])/denom; 
      else
         sum+=TMath::Exp(-curDecay->GetDecayRate()*dim[0])/denom; 

      curDecay = curDecay->GetParentDecay();
   }
   result*=sum;
   
   return result;

}

TFitResultPtr TDecay::Fit(TH1* fithist) {
   TFitResultPtr fitres = fithist->Fit(fDecayFunc,"LRSME");
   Double_t chi2 = fitres->Chi2();
   Double_t ndf = fitres->Ndf();

   printf("Chi2/ndf = %lf\n",chi2/ndf);
   
   return fitres;

}

void TDecay::Print(Option_t *option) const{
   printf("Intensity: %lf +/- %lf c/s\n", GetIntensity(), GetIntensityError());
   printf(" HalfLife: %lf +/- %lf s\n", GetHalfLife(), GetHalfLifeError());
   if(fParent)
      printf("Parent Address: %p\n", fParent);
   if(fDaughter)
      printf("Daughter Address: %p\n", fDaughter);
   printf("First Parent: %p\n", fFirstParent);

}


TDecayChain::TDecayChain() {}

TDecayChain::~TDecayChain() {}

/*Double_t ExpDecay(Double_t *dim, Double_t par){
   Double_t result = 0;

   Double_t intensity = par[0];
   Double_t halflife  = par[1];

   return result;

}*/

//TDecay *GetDecay(int idx) {
   //if(fDecayChain

//}

TDecayFit::TDecayFit(){
   //This is the default Ctor. It creates one decaying species
   

}

TDecayFit::~TDecayFit(){

}
