#include "TDecayFit.h"

ClassImp(TDecay)
ClassImp(TDecayChain)
ClassImp(TDecayFit)

/*TDecay::TDecay(Double_t tlow, Double_t thigh) : fParent(0), fDaughter(0), fDecayFunc(0), fGeneration(1), fDetectionEfficiency(1.0) {
   fFirstParent = this;
   fDecayFunc = new TF1("decayfunc",this,&TDecay::ActivityFunc,0,10,2,"TDecay","ActivityFunc");
   fDecayFunc->SetParameters(0.0,0.0);
   fDecayFunc->SetParNames("Intensity","DecayRate");
   fTotalDecayFunc = new TF1(*fDecayFunc);
   SetTotalDecayParameters();
   SetRange(tlow,thigh);
}*/

TDecay::TDecay(TDecay* parent, Double_t tlow, Double_t thigh) : fParent(0), fDaughter(0), fDecayFunc(0), fDetectionEfficiency(1.0){
   if(parent){
      fParent = parent;
      fParent->SetDaughterDecay(this);
      //See if the decay chain makes sense.
      TDecay* curParent = parent;
      UInt_t gencounter = 1;
      while(curParent){
         ++gencounter;
         fFirstParent = curParent;  
         curParent =curParent->GetParentDecay();
      }

      fGeneration = gencounter;
   }
   else{
      fFirstParent = this;
      fGeneration = 1;
   }
   fDecayFunc = new TF1(Form("decayfunc_gen%d",fGeneration),this,&TDecay::ActivityFunc,0,10,2,"TDecay","ActivityFunc");
   fDecayFunc->SetParameters(fFirstParent->GetIntensity(),0.0);
   fDecayFunc->SetParNames("Intensity","DecayRate");

   fTotalDecayFunc = new TF1(Form("totaldecayfunc_gen%d",fGeneration),this,&TDecay::ActivityFunc,0,10,fGeneration+1,"TDecay","ActivityFunc");
   SetTotalDecayParameters();
   if(fFirstParent != this)
      FixIntensity(0);

   SetRange(tlow,thigh);

}

TDecay::TDecay(UInt_t generation, TDecay* parent, Double_t tlow, Double_t thigh) : fParent(0), fDaughter(0), fDecayFunc(0), fDetectionEfficiency(1.0){
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

   fDecayFunc = new TF1("tmpname",this,&TDecay::ActivityFunc,0,10,2,"TDecay","ActivityFunc");
   fDecayFunc->SetParameters(fFirstParent->GetIntensity(),0.0);
   fDecayFunc->SetParNames("Intensity","DecayRate");

   fTotalDecayFunc = new TF1("tmpname",this,&TDecay::ActivityFunc,0,10,fGeneration+1,"TDecay","ActivityFunc");
   SetTotalDecayParameters();
   if(fFirstParent != this)
      FixIntensity(0);
   
   SetName("");
   SetRange(tlow,thigh);
}

TDecay::~TDecay() {
   if(fDecayFunc) delete fDecayFunc;
   if(fTotalDecayFunc) delete fTotalDecayFunc;
}

void TDecay::SetName(const char * name){
   TNamed::SetName(name);
   fDecayFunc->SetName(Form("%s_df_gen%d",name,fGeneration));
   fTotalDecayFunc->SetName(Form("%s_tdf_gen%d",name,fGeneration));
}

void TDecay::SetTotalDecayParameters() {
   //Sets the total fit function to know about the other parmaters in the decay chain.
   Double_t low_limit,high_limit;
   
   //We need to include the fact that we have parents and use that TF1 to perform the fit.
   fTotalDecayFunc->SetParameter(0,fFirstParent->GetIntensity());
   fTotalDecayFunc->SetParName(0,"Intensity1");
   fTotalDecayFunc->SetParNames("Intensity","DecayRate1");
   fFirstParent->GetDecayFunc()->GetParLimits(0,low_limit,high_limit);
   fTotalDecayFunc->SetParLimits(0,low_limit,high_limit);
   //Now we need to get the parameters for each of the parents
   Int_t parCounter = 1;
   TDecay* curDecay = fFirstParent;
   while(curDecay){
      fTotalDecayFunc->SetParameter(parCounter,curDecay->GetDecayRate());
      fTotalDecayFunc->SetParName(parCounter,Form("DecayRate%d",parCounter));
      curDecay->GetDecayFunc()->GetParLimits(1,low_limit,high_limit);
      fTotalDecayFunc->SetParLimits(parCounter,low_limit,high_limit);
      ++parCounter;
      curDecay = curDecay->GetDaughterDecay();
   }
}

TDecay* const TDecay::GetParentDecay(){
   return fParent;
}

TDecay* const TDecay::GetDaughterDecay(){
   return fDaughter;
}

void TDecay::Draw(Option_t* option){
   SetTotalDecayParameters();
   fTotalDecayFunc->Draw(option);
}

Double_t TDecay::Eval(Double_t t){
   SetTotalDecayParameters();
   return fTotalDecayFunc->Eval(t);
}

Double_t TDecay::EvalPar(const Double_t* x, const Double_t* par){
   fTotalDecayFunc->InitArgs(x,par);

   return fTotalDecayFunc->EvalPar(x,par);
}

Double_t TDecay::ActivityFunc(Double_t *dim, Double_t *par){
   //The general function for a decay chain
   //par[0] is the intensity
   //par[1*i] is the activity
   Double_t result = 1.0;
   UInt_t gencounter = 0;
   TDecay *curDecay = this;
   //Compute the first multiplication
   while(curDecay){
      ++gencounter;
      result *= par[curDecay->GetGeneration()];

      curDecay = curDecay->GetParentDecay();
   }
   if(gencounter != fGeneration){
         printf("We have Problems!\n");
         return 0.0;
   }
   //Multiply by the initial intensity of the intial parent.
   result*=par[0]/par[1];
   //Now we need to deal with the second term
   Double_t sum = 0.0;
   curDecay = this;
   while(curDecay){
      Double_t denom = 1.0;
      TDecay* denomDecay = this;
      while(denomDecay){
         if(denomDecay != curDecay){
            denom*=par[denomDecay->GetGeneration()] - par[curDecay->GetGeneration()]; 
         } 
         denomDecay = denomDecay->GetParentDecay();
      }

      sum+=TMath::Exp(-par[curDecay->GetGeneration()]*dim[0])/denom; 

      curDecay = curDecay->GetParentDecay();
   }
   result*=sum;
   
   return result*GetEfficiency();

}

TFitResultPtr TDecay::Fit(TH1* fithist) {
   Int_t parCounter = 1;
   TDecay* curDecay = fFirstParent;
   SetTotalDecayParameters();
   TFitResultPtr fitres = fithist->Fit(fTotalDecayFunc,"LRSME");
   Double_t chi2 = fitres->Chi2();
   Double_t ndf = fitres->Ndf();

   printf("Chi2/ndf = %lf\n",chi2/ndf);

   //Now copy the fits back to the appropriate nuclei.
   fFirstParent->SetIntensity(fTotalDecayFunc->GetParameter(0)); fFirstParent->SetIntensityError(fTotalDecayFunc->GetParError(0));
   //Now we need to set the parameters for each of the parents
   while(curDecay){
      curDecay->SetDecayRate(fTotalDecayFunc->GetParameter(parCounter)); curDecay->SetDecayRateError(fTotalDecayFunc->GetParError(parCounter));
      curDecay = curDecay->GetDaughterDecay();
      ++parCounter;
   }
   
   return fitres;

}

void TDecay::Fix(){
   FixHalfLife();
   FixIntensity();
}

void TDecay::Release() {
   ReleaseHalfLife();
   ReleaseIntensity();
}

void TDecay::SetRange(Double_t tlow, Double_t thigh){
   fDecayFunc->SetRange(tlow,thigh);
   fTotalDecayFunc->SetRange(tlow,thigh);
}

void TDecay::Print(Option_t *option) const{
   printf(" Intensity: %lf +/- %lf c/s\n", GetIntensity(), GetIntensityError());
   printf("  HalfLife: %lf +/- %lf s\n", GetHalfLife(), GetHalfLifeError());
   printf("Efficiency: %lf\n",GetEfficiency());
   printf("My Address: %p\n",this);
   if(fParent)
      printf("Parent Address: %p\n", fParent);
   if(fDaughter)
      printf("Daughter Address: %p\n", fDaughter);
   printf("First Parent: %p\n", fFirstParent);

}


TDecayChain::TDecayChain(){
   
}

TDecayChain::TDecayChain(UInt_t generations){
   fDecayChain.clear();
   TDecay* parent = new TDecay();
   fDecayChain.push_back(parent);
   for(UInt_t i = 1; i < generations; i++){
      TDecay* curDecay = new TDecay(parent);
      fDecayChain.push_back(curDecay);
      parent = curDecay;
   }

   fChainFunc = new TF1("tmpname",this,&TDecayChain::ChainActivityFunc,0,10,fDecayChain.size()+1,"TDecayChain","ChainActivityFunc");
   SetChainParameters();
}

TDecayChain::~TDecayChain() {
//dtor
   
   //Might have to think about ownership if we allow external decays to be added
   for(int i =0; i<fDecayChain.size(); ++i) delete fDecayChain.at(i);

}

void TDecayChain::SetChainParameters(){
   for(int i=0;i<fDecayChain.size();++i){
      fDecayChain.at(i)->SetTotalDecayParameters();
   }
   Double_t low_limit,high_limit;
   //We need to include the fact that we have parents and use that TF1 to perform the fit.
   for(int i=0; i<fChainFunc->GetNpar(); ++i){
      fChainFunc->SetParameter(i,fDecayChain.back()->GetTotalDecayFunc()->GetParameter(i));
      fChainFunc->SetParError(i,fDecayChain.back()->GetTotalDecayFunc()->GetParError(i));
      fChainFunc->SetParName(i,fDecayChain.back()->GetTotalDecayFunc()->GetParName(i));
      fDecayChain.back()->GetTotalDecayFunc()->GetParLimits(i,low_limit,high_limit);
      fChainFunc->SetParLimits(i,low_limit,high_limit);
      fDecayChain.back()->GetTotalDecayFunc()->Print();

   }
}

Double_t TDecayChain::ChainActivityFunc(Double_t *dim, Double_t *par){
   //This fits the total activity caused by the entire chain.
   Double_t result = 0.0;
   for(int i=0;i<fDecayChain.size();++i){
      result += GetDecay(i)->EvalPar(dim,par);
   }
   
   return result;
}

Double_t TDecayChain::Eval(Double_t t) const{
   return fChainFunc->Eval(t);
}

void TDecayChain::Draw(Option_t *opt) {
   SetChainParameters();
   fChainFunc->Draw(opt);
}

void TDecayChain::DrawComponents(Option_t *opt, Bool_t color_flag) {
   SetChainParameters();
   fDecayChain.at(0)->SetMinimum(0);
   fDecayChain.at(0)->SetLineColor(1);
   fDecayChain.at(0)->Draw(opt);
   for(int i=1; i< fDecayChain.size(); ++i){
      if(color_flag){
         fDecayChain.at(i)->SetLineColor(i+3);
      }
      fDecayChain.at(i)->Draw(Form("same%s",opt));
    //  fDecayChain.at(i)->SetLineColor(orig_color);
   }

}

void TDecayChain::AddToChain(TDecay* decay){
   if(decay)
      fDecayChain.push_back(decay);
}

TDecay* TDecayChain::GetDecay(UInt_t generation){
   if(generation < fDecayChain.size()){
      return fDecayChain.at(generation);
   }

   return 0;   
}

void TDecayChain::Print(Option_t *option) const {
   printf("Number of Decays in Chain: %d\n",fDecayChain.size());

}


TDecayFit::TDecayFit(){
   //This is the default Ctor. It creates one decaying species
   

}

TDecayFit::~TDecayFit(){

}
