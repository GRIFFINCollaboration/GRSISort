#include "TDecay.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "GCanvas.h"
#include "TLMFitter.h"

/// \cond CLASSIMP
ClassImp(TSingleDecay)
ClassImp(TDecayChain)
ClassImp(TDecayFit)
ClassImp(TDecay)
ClassImp(TVirtualDecay)
/// \endcond

UInt_t TSingleDecay::fCounter  = 0;
UInt_t TDecayChain::fChainCounter = 0;

TDecayFit::~TDecayFit() = default;

void TDecayFit::DrawComponents() const
{
   /// This draws the individual components on the current canvas
   fDecay->DrawComponents("same");
}

void TDecayFit::SetDecay(TVirtualDecay* decay)
{
   /// This tells the TDecayFit which TVirtualDecay it belongs to
   fDecay = decay;
}

void TDecayFit::Print(Option_t* opt) const
{
   /// This prints the parameters of the fit (decay rate, intensities, etc...)
   TF1::Print(opt);
   printf("fDecay = %p\n", static_cast<void*>(fDecay));
}

TVirtualDecay* TDecayFit::GetDecay() const
{
   return fDecay;
}

TFitResultPtr TDecayFit::Fit(TH1* hist, Option_t* opt)
{
   if(!hist) {
      return 0;
   }
   TFitResultPtr tmpres = hist->Fit(this, opt);
   UpdateResiduals(hist);
   // Might be able to copy the style over to the new clone for the residuals.
   // Will take a look at this later
   // fDecay->Update();
   // this->DrawClone("same");
   this->Draw("same");
   return tmpres;
}

void TDecayFit::Streamer(TBuffer& R__b)
{
   /// Stream an object of class TDecayFit.
   if(R__b.IsReading()) {
      R__b.ReadClassBuffer(TDecayFit::Class(), this);
   } else {
      R__b.WriteClassBuffer(TDecayFit::Class(), this);
   }
}

void TDecayFit::DefaultGraphs()
{
   fResiduals.SetMarkerStyle(20); // Filled circle
   fResiduals.SetMarkerSize(0.6);
   fResiduals.SetTitle("Residuals");
}

void TDecayFit::UpdateResiduals(TH1* hist)
{
   const Size_t  oldmarker_size  = fResiduals.GetMarkerSize();
   const Color_t oldmarker_color = fResiduals.GetMarkerColor();
   const Style_t oldmarker_style = fResiduals.GetMarkerStyle();

   // Clear the data points from the old TGraph
   for(int i = 0; i < fResiduals.GetN(); ++i) {
      fResiduals.RemovePoint(i);
   }

   Double_t xlow, xhigh;
   GetRange(xlow, xhigh);
   Int_t    nbins = hist->GetXaxis()->GetNbins();
   Double_t res;
   Double_t bin;

   for(int i = 0; i < nbins; ++i) {
      if((hist->GetBinCenter(i) <= xlow) || (hist->GetBinCenter(i) >= xhigh)) {
         continue;
      }
      // This might not be correct for Poisson statistics.
      res = (hist->GetBinContent(i) - this->Eval(hist->GetBinCenter(i))) /
            hist->GetBinError(i); /// GetHist()->GetBinError(i));// + this->GetParameter("Height") + 10.;
      bin = hist->GetBinCenter(i);
      fResiduals.SetPoint(i, bin, res);
   }

   fResiduals.SetMarkerSize(oldmarker_size);
   fResiduals.SetMarkerColor(oldmarker_color);
   fResiduals.SetMarkerStyle(oldmarker_style);
}

void TDecayFit::DrawResiduals()
{
   if(fResiduals.GetN()) {
      new GCanvas;
      fResiduals.Draw("AP");
   } else {
      printf("Residuals not set yet\n");
   }
}

void TVirtualDecay::DrawComponents(Option_t* opt, Bool_t)
{
   printf("Draw components has not been set in %s \n", ClassName());
   Draw(Form("same%s", opt));
}

void TVirtualDecay::Streamer(TBuffer& R__b)
{
   /// Stream an object of class TVirtualDecay.
   if(R__b.IsReading()) {
      R__b.ReadClassBuffer(TVirtualDecay::Class(), this);
   } else {
      R__b.WriteClassBuffer(TVirtualDecay::Class(), this);
   }
}

TSingleDecay::TSingleDecay(TSingleDecay* parent, Double_t tlow, Double_t thigh)
   : fDetectionEfficiency(1.0), fDecayFunc(nullptr), fTotalDecayFunc(nullptr), fParent(nullptr), fDaughter(nullptr),
     fFirstParent(nullptr), fChainId(-1)
{
   if(parent) {
      fParent = parent;
      fParent->SetDaughterDecay(this);
      // See if the decay chain makes sense.
      TSingleDecay* curParent  = parent;
      UInt_t        gencounter = 1;
      while(curParent) {
         ++gencounter;
         fFirstParent = curParent;
         curParent    = curParent->GetParentDecay();
      }

      fGeneration = gencounter;
   } else {
      fFirstParent = this;
      fGeneration  = 1;
   }
   // This will potentially leak with ROOT IO, shouldnt be a big deal. Might come back to this later
   fDecayFunc = new TDecayFit(Form("decayfunc_gen%d", fGeneration), this, &TSingleDecay::ActivityFunc, 0, 10, 2,
                              "TSingleDecay", "ActivityFunc");
   fDecayFunc->SetDecay(this);
   fDecayFunc->SetParameters(fFirstParent->GetIntensity(), 0.0);
   fDecayFunc->SetParNames("Intensity", "DecayRate");

   fTotalDecayFunc = new TDecayFit(Form("totaldecayfunc_gen%d", fGeneration), this, &TSingleDecay::ActivityFunc, 0, 10,
                                   fGeneration + 1, "TSingleDecay", "ActivityFunc");
   fTotalDecayFunc->SetDecay(this);
   SetTotalDecayParameters();
   if(fFirstParent != this) {
      FixIntensity(0);
   }

   SetRange(tlow, thigh);
   fUnId = fCounter;
   fCounter++;

   SetName("Decay");
}

TSingleDecay::TSingleDecay(UInt_t generation, TSingleDecay* parent, Double_t tlow, Double_t thigh)
   : fDetectionEfficiency(1.0), fDecayFunc(nullptr), fTotalDecayFunc(nullptr), fParent(nullptr), fDaughter(nullptr),
     fFirstParent(nullptr), fChainId(-1)
{
   if(parent) {
      fParent = parent;
      fParent->SetDaughterDecay(this);
      // See if the decay chain makes sense.
      TSingleDecay* curParent  = parent;
      UInt_t        gencounter = 2;
      for(UInt_t i = 0; i < generation; ++i) {
         if(curParent->GetParentDecay()) {
            curParent = parent->GetParentDecay();
            ++gencounter;
         } else {
            fFirstParent = curParent;
         }
      }
      if(gencounter != generation) {
         printf("Generation numbers do not make sense\n");
      }
   }
   if(generation == 1) {
      fFirstParent = this;
   }

   fGeneration = generation;

   fDecayFunc = new TDecayFit("tmpname", this, &TSingleDecay::ActivityFunc, 0, 10, 2, "TSingleDecay", "ActivityFunc");
   fDecayFunc->SetDecay(this);
   fDecayFunc->SetParameters(fFirstParent->GetIntensity(), 0.0);
   fDecayFunc->SetParNames("Intensity", "DecayRate");

   fTotalDecayFunc = new TDecayFit("tmpname", this, &TSingleDecay::ActivityFunc, 0, 10, fGeneration + 1, "TSingleDecay",
                                   "ActivityFunc");
   fTotalDecayFunc->SetDecay(this);
   SetTotalDecayParameters();
   if(fFirstParent != this) {
      FixIntensity(0);
   }

   SetName("");
   SetRange(tlow, thigh);
   fUnId = fCounter;
   fCounter++;

   SetName("Decay");
}

TSingleDecay::~TSingleDecay()
{
   //  if(fDecayFunc) delete fDecayFunc;
   //  if(fTotalDecayFunc) delete fTotalDecayFunc;

   fDecayFunc      = nullptr;
   fTotalDecayFunc = nullptr;
}

void TSingleDecay::SetName(const char* name)
{
   TNamed::SetName(name);
   fDecayFunc->SetName(Form("%s_df_gen%d", name, fGeneration));
   fTotalDecayFunc->SetName(Form("%s_tdf_gen%d", name, fGeneration));
}

void TSingleDecay::SetTotalDecayParameters()
{
   /// Sets the total fit function to know about the other parmaters in the decay chain.
   Double_t low_limit, high_limit;
   // We need to include the fact that we have parents and use that TF1 to perform the fit.
   fTotalDecayFunc->SetParameter(0, fFirstParent->GetIntensity());
   fTotalDecayFunc->SetParNames("Intensity", "DecayRate1");
   fFirstParent->GetDecayFunc()->GetParLimits(0, low_limit, high_limit);
   fTotalDecayFunc->SetParLimits(0, low_limit, high_limit);
   // Now we need to get the parameters for each of the parents
   Int_t         parCounter = 1;
   TSingleDecay* curDecay   = fFirstParent;
   while(curDecay) {
      fTotalDecayFunc->SetParameter(parCounter, curDecay->GetDecayRate());
      fTotalDecayFunc->SetParName(parCounter, Form("DecayRate%d", parCounter));
      curDecay->GetDecayFunc()->GetParLimits(1, low_limit, high_limit);
      if(low_limit != 0 && high_limit != 0) {
         fTotalDecayFunc->SetParLimits(parCounter, low_limit, high_limit);
      }
      ++parCounter;
      curDecay = curDecay->GetDaughterDecay();
   }
   UpdateDecays();
}

void TSingleDecay::UpdateDecays()
{
   /// Updates the other decays in the chain to know that they have potential updates.
   Double_t low_limit, high_limit;
   // The current (this) decay we are on is the one that is assumed to be the most recently changed.
   // We will first update it's total decay function

   // Now update the halflives of all the total decay functions.
   TSingleDecay* curDecay = fFirstParent;
   while(curDecay) {
      GetDecayFunc()->GetParLimits(0, low_limit, high_limit);
      curDecay->fTotalDecayFunc->SetParameter(0, GetIntensity());
      curDecay->fTotalDecayFunc->SetParLimits(0, low_limit, high_limit);
      // curDecay->fDecayFunc->SetParameter(0,GetIntensity());
      // curDecay->fDecayFunc->SetParLimits(0,low_limit,high_limit);
      curDecay->fDecayFunc->SetParameter(0, GetIntensity());
      curDecay->fDecayFunc->SetParLimits(0, low_limit, high_limit);
      fDecayFunc->GetParLimits(0, low_limit, high_limit);
      curDecay->fTotalDecayFunc->SetParLimits(0, low_limit, high_limit);

      curDecay->fTotalDecayFunc->SetParameter(0, GetIntensity());

      curDecay->fTotalDecayFunc->SetParameter(GetGeneration(), GetDecayRate());
      fDecayFunc->GetParLimits(1, low_limit, high_limit);
      curDecay->fTotalDecayFunc->SetParLimits(GetGeneration(), low_limit, high_limit);
      curDecay = curDecay->GetDaughterDecay();
   }
}

void TSingleDecay::SetHalfLifeLimits(const Double_t& low, const Double_t& high)
{
   if(low == 0) {
      fDecayFunc->SetParLimits(1, std::log(2) / high, 1e30);
   }

   fDecayFunc->SetParLimits(1, std::log(2) / high, std::log(2) / low);
   // Tell this info to the rest of the decays
   UpdateDecays();
}

void TSingleDecay::SetIntensityLimits(const Double_t& low, const Double_t& high)
{
   fFirstParent->fDecayFunc->SetParLimits(0, low, high);
   UpdateDecays();
}

void TSingleDecay::SetDecayRateLimits(const Double_t& low, const Double_t& high)
{
   fDecayFunc->SetParLimits(1, low, high);
   UpdateDecays();
}

void TSingleDecay::GetHalfLifeLimits(Double_t& low, Double_t& high) const
{
   fDecayFunc->GetParLimits(1, high, low); // This gets the decay rates, not the half-life.
   if(low == 0) {
      low = 0.000000000001;
   }
   if(high == 0) {
      high = 0.000000000001;
   }
   low  = std::log(2) / low;
   high = std::log(2) / high;
}

void TSingleDecay::GetIntensityLimits(Double_t& low, Double_t& high) const
{
   fFirstParent->fDecayFunc->GetParLimits(0, low, high);
}

void TSingleDecay::GetDecayRateLimits(Double_t& low, Double_t& high) const
{
   fDecayFunc->GetParLimits(1, low, high);
}

TSingleDecay* TSingleDecay::GetParentDecay()
{
   return fParent;
}

TSingleDecay* TSingleDecay::GetDaughterDecay()
{
   return fDaughter;
}

void TSingleDecay::Draw(Option_t* option)
{
   SetTotalDecayParameters();
   fTotalDecayFunc->Draw(option);
}

Double_t TSingleDecay::Eval(Double_t t)
{
   /// Evaluates the activity at a given time, t
   SetTotalDecayParameters();
   return fTotalDecayFunc->Eval(t);
}

Double_t TSingleDecay::EvalPar(const Double_t* x, const Double_t* par)
{
   /// Evaluates the activity at a given time t using parameters par.
   fTotalDecayFunc->InitArgs(x, par);
   return fTotalDecayFunc->EvalPar(x, par);
}

Double_t TSingleDecay::ActivityFunc(Double_t* dim, Double_t* par)
{
   /// The general function for a decay chain
   /// par[0] is the intensity
   /// par[1*i] is the activity
   Double_t result     = 1.0;
   UInt_t   gencounter = 0;
   Double_t tlow, thigh;
   fTotalDecayFunc->GetRange(tlow, thigh);
   Double_t      t        = dim[0] - tlow;
   TSingleDecay* curDecay = this;
   // Compute the first multiplication
   while(curDecay) {
      ++gencounter;
      // par[Generation] gets the decay rate for that decay since the parameters are stored
      // as [intensity, decayrate1,decayrate2,...]
      result *= par[curDecay->GetGeneration()];

      curDecay = curDecay->GetParentDecay();
   }
   if(gencounter != fGeneration) {
      printf("We have Problems!\n");
      return 0.0;
   }
   // Multiply by the initial intensity of the initial parent.
   result *= par[0] / par[1];
   // Now we need to deal with the second term
   Double_t sum = 0.0;
   curDecay     = this;
   while(curDecay) {
      Double_t      denom      = 1.0;
      TSingleDecay* denomDecay = this;
      while(denomDecay) {
         if(denomDecay != curDecay) {
            // This term has problems if two or more rates are very similar. Will have to put a taylor expansion in here
            // if this problem comes up. I believe the solution to this also depends on the number of nuclei in the
            // chain
            // that have similar rates. I think either of these issues is fairly rare, so I'll fix it when it comes up.
            denom *= par[denomDecay->GetGeneration()] - par[curDecay->GetGeneration()];
         }
         denomDecay = denomDecay->GetParentDecay();
      }

      sum += TMath::Exp(-par[curDecay->GetGeneration()] * t) / denom;

      curDecay = curDecay->GetParentDecay();
   }
   result *= sum;

   return result * GetEfficiency();
}

TFitResultPtr TSingleDecay::Fit(TH1* fithist, Option_t* opt)
{
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Combination");
   TVirtualFitter::SetPrecision(1.0e-10);
   TVirtualFitter::SetMaxIterations(10000);
   Int_t         parCounter = 1;
   TSingleDecay* curDecay   = fFirstParent;
   SetTotalDecayParameters();
   //   TFitResultPtr fitres = fithist->Fit(fTotalDecayFunc,Form("%sWLRS",opt));
   TFitResultPtr fitres = fTotalDecayFunc->Fit(fithist, Form("%sWLRS", opt));
   Double_t      chi2   = fitres->Chi2();
   Double_t      ndf    = fitres->Ndf(); // This ndf needs to be changed by a weighted poisson.

   printf("Chi2/ndf = %lf\n", chi2 / ndf);

   // Now copy the fits back to the appropriate nuclei.
   fFirstParent->SetIntensity(fTotalDecayFunc->GetParameter(0));
   fFirstParent->SetIntensityError(fTotalDecayFunc->GetParError(0));
   // Now we need to set the parameters for each of the parents
   while(curDecay) {
      curDecay->SetDecayRate(fTotalDecayFunc->GetParameter(parCounter));
      curDecay->SetDecayRateError(fTotalDecayFunc->GetParError(parCounter));
      curDecay = curDecay->GetDaughterDecay();
      ++parCounter;
   }

   return fitres;
}

void TSingleDecay::Fix()
{
   FixHalfLife();
   FixIntensity();
}

void TSingleDecay::Release()
{
   ReleaseHalfLife();
   ReleaseIntensity();
}

void TSingleDecay::SetRange(Double_t tlow, Double_t thigh)
{
   fDecayFunc->SetRange(tlow, thigh);
   fTotalDecayFunc->SetRange(tlow, thigh);
}

void TSingleDecay::Print(Option_t*) const
{
   //  printf("      Name: %s\n",GetName());
   printf("  Decay Id: %d\n", GetDecayId());
   printf(" Intensity: %lf +/- %lf c/s\n", GetIntensity(), GetIntensityError());
   printf("  HalfLife: %lf +/- %lf s\n", GetHalfLife(), GetHalfLifeError());
   printf("Efficiency: %lf\n", GetEfficiency());
   printf("My Address: %p\n", static_cast<const void*>(this));
   if(fParent) {
      printf("Parent Address: %p\n", static_cast<void*>(fParent));
   }
   if(fDaughter) {
      printf("Daughter Address: %p\n", static_cast<void*>(fDaughter));
   }
   printf("First Parent: %p\n", static_cast<void*>(fFirstParent));
}

TDecayChain::TDecayChain() : fChainFunc(nullptr)
{
}

TDecayChain::TDecayChain(UInt_t generations) : fChainFunc(nullptr)
{
   if(!generations) {
      generations = 1;
   }
   fDecayChain.clear();
   auto* parent = new TSingleDecay(nullptr);
   parent->SetChainId(fChainCounter);
   fDecayChain.push_back(parent);
   for(UInt_t i = 1; i < generations; i++) {
      auto* curDecay = new TSingleDecay(parent);
      curDecay->SetChainId(fChainCounter);
      fDecayChain.push_back(curDecay);
      parent = curDecay;
   }

   fChainFunc = new TDecayFit("tmpname", this, &TDecayChain::ChainActivityFunc, 0, 10, fDecayChain.size() + 1,
                              "TDecayChain", "ChainActivityFunc");
   fChainFunc->SetDecay(this);
   SetChainParameters();
   fChainId = fChainCounter;
   ++fChainCounter;
}

TDecayChain::~TDecayChain()
{
   // dtor

   // Might have to think about ownership if we allow external decays to be added
   //  for(size_t i =0; i<fDecayChain.size(); ++i) delete fDecayChain.at(i);
}

void TDecayChain::SetChainParameters()
{
   for(auto& i : fDecayChain) {
      i->SetTotalDecayParameters();
   }
   Double_t low_limit, high_limit;
   // We need to include the fact that we have parents and use that TF1 to perform the fit.
   for(int i = 0; i < fChainFunc->GetNpar(); ++i) {
      fChainFunc->SetParameter(i, fDecayChain.back()->GetTotalDecayFunc()->GetParameter(i));
      fChainFunc->SetParError(i, fDecayChain.back()->GetTotalDecayFunc()->GetParError(i));
      fChainFunc->SetParName(i, fDecayChain.back()->GetTotalDecayFunc()->GetParName(i));
      fDecayChain.back()->GetTotalDecayFunc()->GetParLimits(i, low_limit, high_limit);
      fChainFunc->SetParLimits(i, low_limit, high_limit);
   }
}

Double_t TDecayChain::ChainActivityFunc(Double_t* dim, Double_t* par)
{
   /// This fits the total activity caused by the entire chain.
   Double_t result = 0.0;
   for(size_t i = 0; i < fDecayChain.size(); ++i) {
      result += GetDecay(i)->EvalPar(dim, par);
   }

   return result;
}

Double_t TDecayChain::Eval(Double_t t) const
{
   return fChainFunc->Eval(t);
}

void TDecayChain::Draw(Option_t* opt)
{
   SetChainParameters();
   fChainFunc->Draw(opt);
}

void TDecayChain::DrawComponents(Option_t* opt, Bool_t color_flag)
{
   SetChainParameters();
   fDecayChain.at(0)->SetMinimum(0);
   fDecayChain.at(0)->SetLineColor(1);
   fDecayChain.at(0)->Draw("Same");
   for(size_t i = 1; i < fDecayChain.size(); ++i) {
      if(color_flag) {
         fDecayChain.at(i)->SetLineColor(fDecayChain.at(i)->fUnId);
      }
      fDecayChain.at(i)->Draw(Form("same%s", opt));
      //  fDecayChain.at(i)->SetLineColor(orig_color);
   }
}

void TDecayChain::AddToChain(TSingleDecay* decay)
{
   if(decay) {
      fDecayChain.push_back(decay);
   }
}

TSingleDecay* TDecayChain::GetDecay(UInt_t generation)
{
   if(generation < fDecayChain.size()) {
      return fDecayChain.at(generation);
   }
   SetChainParameters();

   return nullptr;
}

void TDecayChain::Print(Option_t*) const
{
   printf("Number of Decays in Chain: %lu\n", fDecayChain.size());
   printf("Chain Id %d\n", fDecayChain.at(0)->GetChainId());
   for(auto i : fDecayChain) {
      printf("decay ptr: %p\n", static_cast<void*>(i));
      i->Print();
      printf("\n");
   }
}

TFitResultPtr TDecayChain::Fit(TH1* fithist, Option_t* opt)
{
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Combination");
   TVirtualFitter::SetPrecision(1.0e-10);
   TVirtualFitter::SetMaxIterations(10000);
   Int_t         parCounter = 1;
   TSingleDecay* curDecay   = fDecayChain.at(0);
   SetChainParameters();
   //  TFitResultPtr fitres = fithist->Fit(fChainFunc,Form("%sWLRS",opt));
   TFitResultPtr fitres = fChainFunc->Fit(fithist, Form("%sWLRS", opt));
   Double_t      chi2   = fitres->Chi2();
   Double_t      ndf    = fitres->Ndf();

   printf("Chi2/ndf = %lf\n", chi2 / ndf);

   // Now copy the fits back to the appropriate nuclei.
   curDecay->SetIntensity(fChainFunc->GetParameter(0));
   curDecay->SetIntensityError(fChainFunc->GetParError(0));
   // Now we need to set the parameters for each of the parents
   while(curDecay) {
      curDecay->SetDecayRate(fChainFunc->GetParameter(parCounter));
      curDecay->SetDecayRateError(fChainFunc->GetParError(parCounter));
      curDecay = curDecay->GetDaughterDecay();
      ++parCounter;
      curDecay->UpdateDecays();
   }

   return fitres;
}

Double_t TDecayChain::EvalPar(const Double_t* x, const Double_t* par)
{
   fChainFunc->InitArgs(x, par);
   SetChainParameters();

   return fChainFunc->EvalPar(x, par);
}

void TDecayChain::SetRange(Double_t xlow, Double_t xhigh)
{
   fChainFunc->SetRange(xlow, xhigh);
   for(auto& i : fDecayChain) {
      i->SetRange(xlow, xhigh);
   }
}

TDecay::TDecay(std::vector<TDecayChain*> chainlist) : fFitFunc(nullptr)
{
   fChainList = chainlist;

   fFitFunc = new TDecayFit("tmpfit", this, &TDecay::DecayFit, 0, 10, 1, "TDecay", "DecayFit");
   fFitFunc->SetDecay(this);
   RemakeMap();
   SetParameters();
}

TDecay::~TDecay()
{
   //  if(fFitFunc) delete fFitFunc;
}

TDecayChain* TDecay::GetChain(UInt_t idx)
{
   if(idx < fChainList.size()) {
      return fChainList.at(idx);
   }

   SetParameters();

   return nullptr;
}

Double_t TDecay::DecayFit(Double_t* dim, Double_t* par)
{
   /// This fits the total activity caused by the entire chain.
   Double_t result = 0.0;
   // Start the fit with flat background;
   result = par[0];
   // Parameters might be linked, so I have to sort this out here.
   // We Must build parameter arrays for each fit.
   for(auto& i : fChainList) {
      Int_t par_counter     = 0;
      auto* tmppar          = new Double_t[i->Size() + 1];
      tmppar[par_counter++] = par[fFitFunc->GetParNumber(Form("Intensity_ChainId%d", i->GetChainId()))];
      for(int j = 0; j < i->Size(); ++j) {
         tmppar[par_counter++] = par[fFitFunc->GetParNumber(Form("DecayRate_DecayId%d", i->GetDecay(j)->GetDecayId()))];
      }
      result += i->EvalPar(dim, tmppar);
      delete[] tmppar;
   }

   return result;
}

TFitResultPtr TDecay::Fit(TH1* fithist, Option_t* opt)
{
   // Use the option "G" to use Geoff's Levenberg-Marquardt algorithm to fit.
   TString opt1 = opt;
   opt1.ToUpper();

   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Combination");
   TVirtualFitter::SetPrecision(1.0e-10);
   TVirtualFitter::SetMaxIterations(10000);
   if(!fithist->GetSumw2N()) {
      fithist->Sumw2();
   }
   // Int_t parCounter = 1;
   SetParameters();

   //   TFitResultPtr fitres = fithist->Fit(fFitFunc,Form("%sWLRS0",opt));
   TFitResultPtr fitres;
   if(opt1.Contains("G")) {
      // Fit using LMFitter..Doesn't do residuals yet
      TLMFitter fitter;
      fitter.Fit(fithist, fFitFunc);
   } else {
      fitres        = fFitFunc->Fit(fithist, Form("%sRIS", opt));
      Double_t chi2 = fitres->Chi2();
      Double_t ndf  = fitres->Ndf();
      printf("Chi2/ndf = %lf\n", chi2 / ndf);
   }

   // Now Tell the decays about the results
   for(auto& i : fChainList) {
      Int_t par_num = fFitFunc->GetParNumber(Form("Intensity_ChainId%d", i->GetChainId()));
      i->GetDecay(0)->SetIntensity(fFitFunc->GetParameter(par_num));
      i->GetDecay(0)->SetIntensityError(fFitFunc->GetParError(par_num));
      for(int j = 0; j < i->Size(); ++j) {
         par_num = fFitFunc->GetParNumber(Form("DecayRate_DecayId%d", i->GetDecay(j)->GetDecayId()));
         i->GetDecay(j)->SetDecayRate(fFitFunc->GetParameter(par_num));
         i->GetDecay(j)->SetDecayRateError(fFitFunc->GetParError(par_num));
      }
      i->GetDecay(0)->UpdateDecays();
      i->SetChainParameters();
   }

   return fitres;
}

void TDecay::Draw(Option_t* opt)
{
   SetParameters();
   fFitFunc->Draw(opt);
}

void TDecay::SetParameters()
{
   RemakeMap();
   // Find the number of unique chains and decays.
   Int_t unq_chains = fChainList.size();
   Int_t unq_decays = fDecayMap.size();

   Double_t xlow, xhigh;
   fFitFunc->GetRange(xlow, xhigh);

   Double_t tmpbg = fFitFunc->GetParameter(0);
   Double_t tmpbglow, tmpbghigh;
   fFitFunc->GetParLimits(0, tmpbglow, tmpbghigh);
   Double_t tmpbgerr = fFitFunc->GetParError(0);
   if(fFitFunc) {
      delete fFitFunc;
      fFitFunc = nullptr;
   }
   fFitFunc =
      new TDecayFit("tmpfit", this, &TDecay::DecayFit, xlow, xhigh, unq_chains + unq_decays + 1, "TDecay", "DecayFit");
   fFitFunc->SetDecay(this);
   fFitFunc->SetParName(0, "Background");
   fFitFunc->SetParameter(0, tmpbg);
   fFitFunc->SetParError(0, tmpbgerr);
   fFitFunc->SetParLimits(0, tmpbglow, tmpbghigh);

   Int_t    par_counter = 1;
   Double_t low, high;
   for(auto& i : fChainList) {
      fFitFunc->SetParName(par_counter, Form("Intensity_ChainId%d", i->GetDecay(0)->GetChainId()));
      i->SetChainParameters();
      i->GetDecay(0)->GetIntensityLimits(low, high);
      fFitFunc->SetParLimits(par_counter, low, high);
      fFitFunc->SetParameter(par_counter++, i->GetDecay(0)->GetIntensity());
   }
   for(auto& it : fDecayMap) {
      fFitFunc->SetParName(par_counter, Form("DecayRate_DecayId%d", it.first));
      it.second.at(0)->GetDecayRateLimits(low, high);
      fFitFunc->SetParLimits(par_counter, low, high);
      fFitFunc->SetParameter(par_counter++, it.second.at(0)->GetDecayRate());
   }

   //  fFitFunc->Print();
}

Double_t TDecay::ComponentFunc(Double_t* dim, Double_t* par)
{
   /// Function for drawing summed components.
   Double_t result = 0;
   /// This function takes 1 parameter, the decay Id.
   Int_t id = (Int_t)(par[0]);
   auto  it = fDecayMap.find(id);

   for(auto& i : it->second) {
      result += i->Eval(dim[0]);
   }
   return result;
}

void TDecay::DrawComponents(Option_t*, Bool_t)
{
   /// Loop over all of the ids and draw them seperately on the pad
   Double_t low, high;
   fFitFunc->GetRange(low, high);

   TF1* tmp_comp = new TF1("tmpname", this, &TDecay::ComponentFunc, low, high, 1, "TDecay", "ComponentFunc");
   for(auto& it : fDecayMap) {
      tmp_comp->SetName(Form("Component_%d", it.first));
      tmp_comp->SetParameter(0, it.first);
      if(it.first == kWhite) {
         tmp_comp->SetLineColor(kOrange);
      } else {
         tmp_comp->SetLineColor(it.first);
      }
      tmp_comp->DrawClone("same");
   }
   delete tmp_comp;
   DrawBackground();
}

void TDecay::DrawBackground(Option_t*)
{
   Double_t low, high;
   fFitFunc->GetRange(low, high);
   auto* bg = new TF1("bg", "pol0", low, high);
   bg->SetParameter(0, GetBackground());
   bg->SetLineColor(kMagenta);
   bg->DrawClone("same");
   delete bg;
}

void TDecay::SetHalfLife(Int_t Id, Double_t halflife)
{
   auto it = fDecayMap.find(Id);
   if(it == fDecayMap.end()) {
      printf("Could not find Id = : %d\n", Id);
      return;
   }
   for(auto& i : it->second) {
      i->SetHalfLife(halflife);
      i->SetTotalDecayParameters();
   }
}

void TDecay::SetHalfLifeLimits(Int_t Id, Double_t low, Double_t high)
{
   auto it = fDecayMap.find(Id);
   if(it == fDecayMap.end()) {
      printf("Could not find Id = : %d\n", Id);
      return;
   }
   for(auto& i : it->second) {
      i->SetHalfLifeLimits(low, high);
      i->SetTotalDecayParameters();
   }
}

void TDecay::SetDecayRateLimits(Int_t Id, Double_t low, Double_t high)
{
   auto it = fDecayMap.find(Id);
   if(it == fDecayMap.end()) {
      printf("Could not find Id = : %d\n", Id);
      return;
   }
   for(auto& i : it->second) {
      i->SetDecayRateLimits(low, high);
      i->SetTotalDecayParameters();
   }
}

void TDecay::Print(Option_t*) const
{
   printf("Background: %lf +/- %lf\n\n", GetBackground(), GetBackgroundError());
   for(const auto& it : fDecayMap) {
      printf("ID: %u Name: %s\n", it.first, it.second.at(0)->GetName());
      it.second.at(0)->Print();
      printf("\n");
   }
}

void TDecay::PrintMap() const
{
   for(const auto& it : fDecayMap) {
      printf("ID: %u\n", it.first);
      for(auto i : it.second) {
         i->Print();
      }
      printf("\n");
   }
}

void TDecay::SetRange(Double_t xlow, Double_t xhigh)
{
   fFitFunc->SetRange(xlow, xhigh);
   for(auto& i : fChainList) {
      i->SetRange(xlow, xhigh);
   }
}

void TDecay::RemakeMap()
{
   fDecayMap.clear();
   for(auto& i : fChainList) {
      for(int j = 0; j < i->Size(); ++j) {
         UInt_t id = i->GetDecay(j)->GetDecayId();
         if(!fDecayMap.count(id)) {
            fDecayMap.insert(std::make_pair(id, std::vector<TSingleDecay*>()));
         }
         fDecayMap.find(id)->second.push_back(i->GetDecay(j));
      }
   }
}
