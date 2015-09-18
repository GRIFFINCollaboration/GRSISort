#include "TDecay.h"

ClassImp(TSingleDecay)
ClassImp(TDecayChain)
ClassImp(TDecayFit)
ClassImp(TDecay)
ClassImp(TVirtualDecay)

/*TDecay::TDecay(Double_t tlow, Double_t thigh) : fParent(0), fDaughter(0), fDecayFunc(0), fGeneration(1), fDetectionEfficiency(1.0) {
   fFirstParent = this;
   fDecayFunc = new TF1("decayfunc",this,&TDecay::ActivityFunc,0,10,2,"TDecay","ActivityFunc");
   fDecayFunc->SetParameters(0.0,0.0);
   fDecayFunc->SetParNames("Intensity","DecayRate");
   fTotalDecayFunc = new TF1(*fDecayFunc);
   SetTotalDecayParameters();
   SetRange(tlow,thigh);
}*/

UInt_t TSingleDecay::fCounter = 0;
UInt_t TDecayChain::fChainCounter = 0;

void TDecayFit::DrawComponents() const { 
   printf("pointer: %p\n",fDecay);
   
   printf("Class: %s\n",fDecay->ClassName());
   fDecay->DrawComponents("same");
 //  fDecay->Draw("same");
//   printf("Is valid: %d\n",fDecay.IsValid());
 //  GetDecay()->Print();
//   GetDecay()->DrawComponents(); 
}

void TDecayFit::SetDecay(TVirtualDecay* decay){ 
   fDecay = decay;
   //fDecayClass = decay->ClassName(); 
 //  decay->Print();
} 

void TDecayFit::Print(Option_t *opt) const {
   TF1::Print(opt);
   printf("fDecay = %p\n",fDecay);
}

TVirtualDecay* TDecayFit::GetDecay() const{
  // return (TVirtualDecay*)(fDecay.GetObject());
   return 0;
}

void TVirtualDecay::DrawComponents(Option_t* opt ,Bool_t color_flag) {
   Draw(opt);
}

TSingleDecay::TSingleDecay(TSingleDecay* parent, Double_t tlow, Double_t thigh) : fParent(0), fDaughter(0), fDecayFunc(0), fDetectionEfficiency(1.0),fChainId(-1){
   if(parent){
      fParent = parent;
      fParent->SetDaughterDecay(this);
      //See if the decay chain makes sense.
      TSingleDecay* curParent = parent;
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
   //This will potentially leak with ROOT IO, shouldnt be a big deal. Might come back to this later
   fDecayFunc = new TDecayFit(Form("decayfunc_gen%d",fGeneration),this,&TSingleDecay::ActivityFunc,0,10,2,"TSingleDecay","ActivityFunc");
   fDecayFunc->SetDecay(this);
   fDecayFunc->SetParameters(fFirstParent->GetIntensity(),0.0);
   fDecayFunc->SetParNames("Intensity","DecayRate");

   fTotalDecayFunc = new TDecayFit(Form("totaldecayfunc_gen%d",fGeneration),this,&TSingleDecay::ActivityFunc,0,10,fGeneration+1,"TSingleDecay","ActivityFunc");
   fTotalDecayFunc->SetDecay(this);
   SetTotalDecayParameters();
   if(fFirstParent != this)
      FixIntensity(0);

   SetRange(tlow,thigh);
   fUnId = fCounter; 
   fCounter++;

   SetName("Decay");

}

TSingleDecay::TSingleDecay(UInt_t generation, TSingleDecay* parent, Double_t tlow, Double_t thigh) : fParent(0), fDaughter(0), fDecayFunc(0), fDetectionEfficiency(1.0),fChainId(-1){
   if(parent){
      fParent = parent;
      fParent->SetDaughterDecay(this);
      //See if the decay chain makes sense.
      TSingleDecay* curParent = parent;
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

   fDecayFunc = new TDecayFit("tmpname",this,&TSingleDecay::ActivityFunc,0,10,2,"TSingleDecay","ActivityFunc");
   fDecayFunc->SetDecay(this);
   fDecayFunc->SetParameters(fFirstParent->GetIntensity(),0.0);
   fDecayFunc->SetParNames("Intensity","DecayRate");

   fTotalDecayFunc = new TDecayFit("tmpname",this,&TSingleDecay::ActivityFunc,0,10,fGeneration+1,"TSingleDecay","ActivityFunc");
   fTotalDecayFunc->SetDecay(this);
   SetTotalDecayParameters();
   if(fFirstParent != this)
      FixIntensity(0);
   
   SetName("");
   SetRange(tlow,thigh);
   fUnId = fCounter;
   fCounter++;

   SetName("Decay");
}

TSingleDecay::~TSingleDecay() {
   if(fDecayFunc) delete fDecayFunc;
   if(fTotalDecayFunc) delete fTotalDecayFunc;
}

void TSingleDecay::SetName(const char * name){
   TNamed::SetName(name);
   fDecayFunc->SetName(Form("%s_df_gen%d",name,fGeneration));
   fTotalDecayFunc->SetName(Form("%s_tdf_gen%d",name,fGeneration));
}

void TSingleDecay::SetTotalDecayParameters() {
   //Sets the total fit function to know about the other parmaters in the decay chain.
   Double_t low_limit,high_limit;
   //We need to include the fact that we have parents and use that TF1 to perform the fit.
   fTotalDecayFunc->SetParameter(0,fFirstParent->GetIntensity());
   fTotalDecayFunc->SetParNames("Intensity","DecayRate1");
   fFirstParent->GetDecayFunc()->GetParLimits(0,low_limit,high_limit);
   fTotalDecayFunc->SetParLimits(0,low_limit,high_limit);
   //Now we need to get the parameters for each of the parents
   Int_t parCounter = 1;
   TSingleDecay* curDecay = fFirstParent;
   while(curDecay){
      fTotalDecayFunc->SetParameter(parCounter,curDecay->GetDecayRate());
      fTotalDecayFunc->SetParName(parCounter,Form("DecayRate%d",parCounter));
      curDecay->GetDecayFunc()->GetParLimits(1,low_limit,high_limit);
      fTotalDecayFunc->SetParLimits(parCounter,low_limit,high_limit);
      ++parCounter;
      curDecay = curDecay->GetDaughterDecay();
   }
   UpdateDecays();
}

void TSingleDecay::UpdateDecays(){
   //Updates the other decays in the chain to know that they have potential updates.
   Double_t low_limit,high_limit;
   //The current (this) decay we are on is the one that is assumed to be the most recently changed.
   //We will first update it's total decay function

   //Now update the halflives of all the total decay functions.
   TSingleDecay *curDecay = fFirstParent;
   while(curDecay){
      GetDecayFunc()->GetParLimits(0,low_limit,high_limit);
      curDecay->fTotalDecayFunc->SetParameter(0,GetIntensity());
      curDecay->fTotalDecayFunc->SetParLimits(0,low_limit,high_limit);
      curDecay->fDecayFunc->SetParameter(0,GetIntensity());
      curDecay->fDecayFunc->SetParLimits(0,low_limit,high_limit);

      curDecay->fTotalDecayFunc->SetParameter(0,GetIntensity());
      fDecayFunc->GetParLimits(0,low_limit,high_limit);
      curDecay->fTotalDecayFunc->SetParLimits(0,low_limit,high_limit);

      curDecay->fTotalDecayFunc->SetParameter(GetGeneration(),GetDecayRate());
      fDecayFunc->GetParLimits(1,low_limit,high_limit);
      curDecay->fTotalDecayFunc->SetParLimits(GetGeneration(),low_limit,high_limit);
      curDecay = curDecay->GetDaughterDecay();
   }

}

void TSingleDecay::SetHalfLifeLimits(const Double_t &low, const Double_t &high){
   fDecayFunc->SetParLimits(1,std::log(2)/low,std::log(2)/high);
   //Tell this info to the rest of the decays
   UpdateDecays();
}
   
void TSingleDecay::SetIntensityLimits(const Double_t &low, const Double_t &high){
   fFirstParent->fDecayFunc->SetParLimits(0,low,high);
   UpdateDecays();
}

void TSingleDecay::SetDecayRateLimits(const Double_t &low, const Double_t &high){
   fDecayFunc->SetParLimits(1,low,high);
   UpdateDecays();
}

void TSingleDecay::GetHalfLifeLimits(Double_t &low, Double_t &high) const{
   fDecayFunc->GetParLimits(1,low,high);
   low = std::log(2)/low;
   high = std::log(2)/high;
}
   
void TSingleDecay::GetIntensityLimits(Double_t &low, Double_t &high) const{
   fFirstParent->fDecayFunc->GetParLimits(0,low,high);
}

void TSingleDecay::GetDecayRateLimits(Double_t &low, Double_t &high) const{
   fDecayFunc->GetParLimits(1,low,high);
}

TSingleDecay* const TSingleDecay::GetParentDecay(){
   return fParent;
}

TSingleDecay* const TSingleDecay::GetDaughterDecay(){
   return fDaughter;
}

void TSingleDecay::Draw(Option_t* option){
   SetTotalDecayParameters();
   fTotalDecayFunc->Draw(option);
}

Double_t TSingleDecay::Eval(Double_t t){
   SetTotalDecayParameters();
   return fTotalDecayFunc->Eval(t);
}

Double_t TSingleDecay::EvalPar(const Double_t* x, const Double_t* par){
   fTotalDecayFunc->InitArgs(x,par);
   return fTotalDecayFunc->EvalPar(x,par);
}

Double_t TSingleDecay::ActivityFunc(Double_t *dim, Double_t *par){
   //The general function for a decay chain
   //par[0] is the intensity
   //par[1*i] is the activity
   Double_t result = 1.0;
   UInt_t gencounter = 0;
   TSingleDecay *curDecay = this;
   //Compute the first multiplication
   while(curDecay){
      ++gencounter;
      //par[Generation] gets the activity for that decay since the parameters are stored
      //as [intensity, act1,act2,...]
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
      TSingleDecay* denomDecay = this;
      while(denomDecay){
         if(denomDecay != curDecay){
            //This term has problems if two or more rates are very similar. Will have to put a taylor expansion in here
            //if this problem comes up. I believe the solution to this also depends on the number of nuclei in the chain
            //that have similar rates. I think either of these issues is fairly rare, so I'll fix it when it comes up.
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

TFitResultPtr TSingleDecay::Fit(TH1* fithist,Option_t *opt) {
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Combination");
   TVirtualFitter::SetPrecision(1.0e-10);
   TVirtualFitter::SetMaxIterations(10000);
   Int_t parCounter = 1;
   TSingleDecay* curDecay = fFirstParent;
   SetTotalDecayParameters();
   TFitResultPtr fitres = fithist->Fit(fTotalDecayFunc,Form("%sWLRS",opt));
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

void TSingleDecay::Fix(){
   FixHalfLife();
   FixIntensity();
}

void TSingleDecay::Release() {
   ReleaseHalfLife();
   ReleaseIntensity();
}

void TSingleDecay::SetRange(Double_t tlow, Double_t thigh){
   fDecayFunc->SetRange(tlow,thigh);
   fTotalDecayFunc->SetRange(tlow,thigh);
}

void TSingleDecay::Print(Option_t *option) const{
   printf("      Name: %s\n",GetName());
   printf("  Decay Id: %d\n",GetDecayId());
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
   if(!generations)
      generations = 1;
   fDecayChain.clear();
   TSingleDecay* parent = new TSingleDecay();
   parent->SetChainId(fChainCounter);
   fDecayChain.push_back(parent);
   for(UInt_t i = 1; i < generations; i++){
      TSingleDecay* curDecay = new TSingleDecay(parent);
      curDecay->SetChainId(fChainCounter);
      fDecayChain.push_back(curDecay);
      parent = curDecay;
   }

   fChainFunc = new TDecayFit("tmpname",this,&TDecayChain::ChainActivityFunc,0,10,fDecayChain.size()+1,"TDecayChain","ChainActivityFunc");
   fChainFunc->SetDecay(this);
   SetChainParameters();
   fChainId = fChainCounter;
   ++fChainCounter;
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
   fDecayChain.at(0)->Draw();
   for(int i=1; i< fDecayChain.size(); ++i){
      if(color_flag){
         fDecayChain.at(i)->SetLineColor(i+3);
      }
      fDecayChain.at(i)->Draw(Form("same%s",opt));
    //  fDecayChain.at(i)->SetLineColor(orig_color);
   }

}

void TDecayChain::AddToChain(TSingleDecay* decay){
   if(decay)
      fDecayChain.push_back(decay);
}

TSingleDecay* TDecayChain::GetDecay(UInt_t generation){
   if(generation < fDecayChain.size()){
      return fDecayChain.at(generation);
   }
   SetChainParameters();

   return 0;   
}

void TDecayChain::Print(Option_t *option) const {
   printf("Number of Decays in Chain: %lu\n",fDecayChain.size());
   printf("Chain Id %d\n",fDecayChain.at(0)->GetChainId());
   for(int i=0; i<fDecayChain.size();++i){
      fDecayChain.at(i)->Print();
      printf("\n");
   }

}

TFitResultPtr TDecayChain::Fit(TH1* fithist, Option_t* opt) {
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Combination");
   TVirtualFitter::SetPrecision(1.0e-10);
   TVirtualFitter::SetMaxIterations(10000);
   Int_t parCounter = 1;
   TSingleDecay* curDecay = fDecayChain.at(0);
   SetChainParameters();
   TFitResultPtr fitres = fithist->Fit(fChainFunc,Form("%sWLRS",opt));
   Double_t chi2 = fitres->Chi2();
   Double_t ndf = fitres->Ndf();

   printf("Chi2/ndf = %lf\n",chi2/ndf);

   //Now copy the fits back to the appropriate nuclei.
   curDecay->SetIntensity(fChainFunc->GetParameter(0)); curDecay->SetIntensityError(fChainFunc->GetParError(0));
   //Now we need to set the parameters for each of the parents
   while(curDecay){
      curDecay->SetDecayRate(fChainFunc->GetParameter(parCounter)); curDecay->SetDecayRateError(fChainFunc->GetParError(parCounter));
      curDecay = curDecay->GetDaughterDecay();
      ++parCounter;
      curDecay->UpdateDecays();
   }
   
   return fitres;

}

Double_t TDecayChain::EvalPar(const Double_t* x, const Double_t* par){
   fChainFunc->InitArgs(x,par);
   SetChainParameters();

   return fChainFunc->EvalPar(x,par);
}

void TDecayChain::SetRange(Double_t xlow, Double_t xhigh){
   fChainFunc->SetRange(xlow,xhigh);
   for(int i =0; i<fDecayChain.size();++i){
      fDecayChain.at(i)->SetRange(xlow,xhigh);
   }
}

TDecay::TDecay(std::vector<TDecayChain*> chainlist) : fFitFunc(0){
   fChainList = chainlist;

   fFitFunc = new TDecayFit("tmpfit",this,&TDecay::DecayFit,0,10,1,"TDecay","DecayFit");
   fFitFunc->SetDecay(this);
   SetParameters();
}

TDecay::~TDecay(){
   if(fFitFunc)
      delete fFitFunc;
}

TDecayChain* TDecay::GetChain(UInt_t idx){
   if(idx < fChainList.size())
      return fChainList.at(idx);

   SetParameters();
   
   return 0;
}

Double_t TDecay::DecayFit(Double_t *dim, Double_t *par){
   //This fits the total activity caused by the entire chain.
   Double_t result = 0.0;
   //Start the fit with flat background;
   result = par[0];
   //Parameters might be linked, so I have to sort this out here.
   //We Must build parameter arrays for each fit.
   for(int i=0; i<fChainList.size(); ++i){
      Int_t par_counter = 0;
      Double_t *tmppar = new Double_t[fChainList.at(i)->Size()+1];
      tmppar[par_counter++] = par[fFitFunc->GetParNumber(Form("Intensity_ChainId%d",fChainList.at(i)->GetChainId()))];
      for(int j=0; j<fChainList.at(i)->Size();++j){
         tmppar[par_counter++] = par[fFitFunc->GetParNumber(Form("DecayRate_DecayId%d",fChainList.at(i)->GetDecay(j)->GetDecayId()))];
      }
      result += fChainList.at(i)->EvalPar(dim,tmppar);
      delete[] tmppar;
   }

   return result;
}

TFitResultPtr TDecay::Fit(TH1* fithist, Option_t* opt) {
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Combination");
   TVirtualFitter::SetPrecision(1.0e-10);
   TVirtualFitter::SetMaxIterations(10000);
   fithist->Sumw2();
   Int_t parCounter = 1;
   SetParameters();

   TFitResultPtr fitres = fithist->Fit(fFitFunc,Form("%sWLRS",opt));
   Double_t chi2 = fitres->Chi2();
   Double_t ndf = fitres->Ndf();

   printf("Chi2/ndf = %lf\n",chi2/ndf);

   //Now Tell the decays about the results
   for(int i=0; i<fChainList.size(); ++i){
      Int_t par_num = fFitFunc->GetParNumber(Form("Intensity_ChainId%d",fChainList.at(i)->GetChainId() ));
      fChainList.at(i)->GetDecay(0)->SetIntensity(fFitFunc->GetParameter(par_num));
      fChainList.at(i)->GetDecay(0)->SetIntensityError(fFitFunc->GetParError(par_num));
      for(int j=0; j<fChainList.at(i)->Size();++j){
         par_num = fFitFunc->GetParNumber(Form("DecayRate_DecayId%d",fChainList.at(i)->GetDecay(j)->GetDecayId() ));
         fChainList.at(i)->GetDecay(j)->SetDecayRate(fFitFunc->GetParameter(par_num));
         fChainList.at(i)->GetDecay(j)->SetDecayRateError(fFitFunc->GetParError(par_num));
         fChainList.at(i)->GetDecay(j)->UpdateDecays();
      }
      fChainList.at(i)->SetChainParameters();
   }

/*
   //Now copy the fits back to the appropriate nuclei.
   curDecay->SetIntensity(fChainFunc->GetParameter(0)); curDecay->SetIntensityError(fChainFunc->GetParError(0));
   //Now we need to set the parameters for each of the parents
   while(curDecay){
      curDecay->SetDecayRate(fChainFunc->GetParameter(parCounter)); curDecay->SetDecayRateError(fChainFunc->GetParError(parCounter));
      curDecay = curDecay->GetDaughterDecay();
      ++parCounter;
   }
   */
   return fitres;

}

void TDecay::Draw(Option_t *opt) {
   SetParameters();
   fFitFunc->Draw(opt);
}

void TDecay::SetParameters(){
   RemakeMap();
   //Find the number of unique chains and decays.
   Int_t unq_chains = fChainList.size();
   Int_t unq_decays = fDecayMap.size();

   Double_t xlow,xhigh;
   fFitFunc->GetRange(xlow,xhigh);

   Double_t tmpbg = fFitFunc->GetParameter(0);
   Double_t tmpbglow, tmpbghigh;
   fFitFunc->GetParLimits(0,tmpbglow,tmpbghigh);
   Double_t tmpbgerr = fFitFunc->GetParError(0);
   if(fFitFunc){
      delete fFitFunc;
      fFitFunc = 0;
   }
   fFitFunc = new TDecayFit("tmpfit",this,&TDecay::DecayFit,xlow,xhigh,unq_chains +unq_decays+1,"TDecay","DecayFit");
   fFitFunc->SetDecay(this);
   fFitFunc->SetParName(0,"Background");
   fFitFunc->SetParameter(0,tmpbg);
   fFitFunc->SetParError(0,tmpbgerr);
   fFitFunc->SetParLimits(0,tmpbglow,tmpbghigh);

   Int_t par_counter = 1;
   Double_t low,high;
   for(int i =0; i<fChainList.size(); ++i){
      fFitFunc->SetParName(par_counter,Form("Intensity_ChainId%d",fChainList.at(i)->GetDecay(0)->GetChainId()));
      fChainList.at(i)->SetChainParameters();
      fChainList.at(i)->GetDecay(0)->GetIntensityLimits(low,high);
      fFitFunc->SetParLimits(par_counter,low,high);
      fFitFunc->SetParameter(par_counter++, fChainList.at(i)->GetDecay(0)->GetIntensity());
   }
   for(auto it = fDecayMap.begin(); it != fDecayMap.end();++it){
      fFitFunc->SetParName(par_counter,Form("DecayRate_DecayId%d",it->first));
      it->second.at(0)->GetDecayRateLimits(low,high);
      fFitFunc->SetParLimits(par_counter,low,high);
      fFitFunc->SetParameter(par_counter++, it->second.at(0)->GetDecayRate());
   }

 //  fFitFunc->Print();

}

Double_t TDecay::ComponentFunc(Double_t *dim, Double_t *par){
   //Function for drawing summed components.
   Double_t result = 0;
   //This function takes 1 parameter, the decay Id.
   Int_t id = (Int_t)(par[0]);
   auto it = fDecayMap.find(id);
   
   for(int i=0;i<it->second.size();++i){
      result += it->second.at(i)->Eval(dim[0]);
   }  
   return result;
}

void TDecay::DrawComponents(Option_t *opt, Bool_t color_flag) {
   //Loop over all of the ids and draw them seperately on the pad
   Double_t low,high;
   fFitFunc->GetRange(low,high);

   TF1* tmp_comp = new TF1("tmpname",this,&TDecay::ComponentFunc,low,high,1,"TDecay","ComponentFunc");
   for(auto it = fDecayMap.begin(); it != fDecayMap.end(); ++it){
      tmp_comp->SetName(Form("Componenet_%d",it->first));
      tmp_comp->SetParameter(0,it->first);
      tmp_comp->SetLineColor(it->first+1);
      tmp_comp->DrawCopy("same");
   }
   delete tmp_comp;
   DrawBackground();

}

void TDecay::DrawBackground(Option_t *opt){
   Double_t low,high;
   fFitFunc->GetRange(low,high);
   TF1 *bg = new TF1("bg","pol0",low,high);
   bg->SetParameter(0,GetBackground());
   bg->SetLineColor(kMagenta);
   bg->DrawCopy("same");
   delete bg;
}

void TDecay::SetHalfLife(Int_t Id, Double_t halflife){
  auto it = fDecayMap.find(Id);
   if(it == fDecayMap.end()){
      printf("Could not find Id = : %d\n",Id);
      return;
   }
   for(int i=0; i<it->second.size(); ++i){
      it->second.at(i)->SetHalfLife(halflife);
      it->second.at(i)->SetTotalDecayParameters();
   }

}

void TDecay::SetHalfLifeLimits(Int_t Id, Double_t low, Double_t high){
  auto it = fDecayMap.find(Id);
   if(it == fDecayMap.end()){
      printf("Could not find Id = : %d\n",Id);
      return;
   }
   for(int i=0; i<it->second.size(); ++i){
      it->second.at(i)->SetHalfLifeLimits(low,high);
      it->second.at(i)->SetTotalDecayParameters();
   }

}


void TDecay::Print(Option_t *opt) const{
   printf("Background: %lf +/- %lf\n\n", GetBackground(),GetBackgroundError());
   for(auto it = fDecayMap.begin(); it!=fDecayMap.end();++it){
      printf("ID: %u\n",it->first);
      it->second.at(0)->Print();
      printf("\n");
   }

}

void TDecay::PrintMap() const{
   for(auto it = fDecayMap.begin(); it!=fDecayMap.end(); ++it){
      printf("ID: %u\n",it->first);
      for(int i=0; i<it->second.size();++i){
         it->second.at(i)->Print();
      }
      printf("\n");
   }

}

void TDecay::SetRange(Double_t xlow, Double_t xhigh){
   fFitFunc->SetRange(xlow,xhigh);
   for(int i=0; i<fChainList.size(); ++i){
      fChainList.at(i)->SetRange(xlow,xhigh);
   }
}

void TDecay::RemakeMap() {
   fDecayMap.clear();
   for(int i=0; i<fChainList.size(); ++i){
      for(int j=0; j<fChainList.at(i)->Size(); ++j){
         UInt_t id = fChainList.at(i)->GetDecay(j)->GetDecayId();
         if(!fDecayMap.count(id)){
            fDecayMap.insert(std::make_pair(id,std::vector<TSingleDecay*>()));
         }
         fDecayMap.find(id)->second.push_back(fChainList.at(i)->GetDecay(j));

      }
   }
   
}
