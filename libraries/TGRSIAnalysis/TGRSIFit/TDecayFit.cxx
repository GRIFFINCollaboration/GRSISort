#include "TDecayFit.h"

ClassImp(TDecay)
ClassImp(TDecayChain)
ClassImp(TDecayFit)

TDecay::TDecay() : fParent(0), fDaughter(0), fDecayFunc(0), fGeneration(1) {
   fDecayFunc = new TF1("decayfunc",this,&TDecay::ActivityFunc,0,10,2,"TDecay","ActivityFunc");
   fDecayFunc->SetParameters(0.0,0.0);
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
            printf("Gencounter = %d\n",gencounter);
         }
         else{
            fFirstParent = curParent;  
         }
      }
      if(gencounter != generation)
         printf("Generation numbers do not make sense\n");
   }
   fGeneration = generation;

   fDecayFunc = new TF1("decayfunc",this,&TDecay::ActivityFunc,0,10,2,"TDecay","ActivityFunc");
   fDecayFunc->SetParameters(0.0,0.0);
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
   Double_t result = par[1];
   UInt_t gencounter = 1;
   TDecay *curparent = GetParentDecay();
   //Compute the first multiplication
   while(curparent){
      ++gencounter;
      result*= curparent->GetDecayRate();
      curparent = curparent->GetParentDecay();
   }
   if(gencounter != fGeneration){
         printf("We have Problems!\n");
         return 0.0;
   }
   
   return result;

   /*
   for(Int_t n=0; n<nChain;n++){
      //Calculate this equation for the nth nucleus.
      Double_t firstterm = 1.0;
      //Compute the first multiplication
      for(Int_t j=0; j<n-1; j++){
         firstterm*=par[1+3*j];
      }
      Double_t secondterm = 0.0;
      for(Int_t i=0; i<n; i++){
         Double_t sum = 0.0;
         for(Int_t j=i; j<n; j++){
            Double_t denom = 1.0;
            for(Int_t p=i;p<n;p++){
               if(p!=j){ denom*=par[1+3*p]-par[1+3*j]; } 
            }
            sum+=par[0+3*i]/par[1+3*i]*TMath::Exp(-par[1+3*j]*dim[0])/denom; 
          }
          secondterm += sum;
      }
      par[2+3*n] = par[1+3*n]*firstterm*secondterm;
      totalActivity += par[2+3*n];
   }*/
}

void TDecay::Print(Option_t *option) const{
   printf("Intensity: %lf c/s\n", GetIntensity());
   printf(" HalfLife: %lf s\n", GetHalfLife());
   if(fParent)
      printf("Parent Address: %p\n", fParent);
   if(fDaughter)
      printf("Daughter Address: %p\n", fDaughter);

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
