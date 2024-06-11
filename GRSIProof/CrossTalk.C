#define CrossTalk_cxx
// The class definition in CrossTalk.h has been generated automatically
#include "CrossTalk.h"

#include "TH1.h"

bool pileup_reject = true;

const double gg_time_low  = -200.;
const double gg_time_high = 300.;

// Beta gamma histograms
const double gb_time_low  = -250.;
const double gb_time_high = 350.;

// default k-value this might have to be adjusted for each experiment!!!
const Short_t defaultKValue = 379;

bool Addback(TGriffinHit& one, TGriffinHit& two)
{
   return ((one.GetDetector() == two.GetDetector()) && (std::fabs(one.GetTime() - two.GetTime()) < 300.));
}

bool PromptCoincidence(TGriffinHit* one, TGriffinHit* two)
{

   return ((two->GetTime() - one->GetTime()) >= gg_time_low) && ((two->GetTime() - one->GetTime()) <= gg_time_high);
}

void CrossTalk::CreateHistograms()
{
   fH2.clear();
   for(int det_num = 1; det_num <= 16; ++det_num) {
      std::string aedet_str  = Form("aEdet%d", det_num);
      std::string gedet_str  = Form("gEdet%d", det_num);
      std::string ae2det_str = Form("aE2det%d", det_num);
      fH1[aedet_str]         = new TH1D(Form("aEdet%d", det_num), Form("Addback detector %d", det_num), 1500, 0, 1500);
      fH1[gedet_str]         = new TH1D(Form("geEdet%d", det_num), Form("Singles detector %d", det_num), 1500, 0, 1500);
      fH1[ae2det_str]        = new TH1D(Form("aE2det%d", det_num), Form("Addback with 2 hits, detector %d", det_num), 1500, 0, 1500);
      for(int crys_1 = 0; crys_1 < 4; ++crys_1) {
         for(int crys_2 = crys_1 + 1; crys_2 < 4; ++crys_2) {
            std::string name_str  = Form("det_%d_%d_%d", det_num, crys_1, crys_2);
            const char* hist_name = name_str.c_str();
            std::cout<<"Creating histogram: "<<hist_name;
            fH2[name_str] = new TH2I(hist_name, hist_name, 1500, 0, 1500, 1500, 0, 1500);
            std::cout<<" at address: "<<fH2[hist_name]<<std::endl;
         }
      }
   }

   fH1["aMult"]   = new TH1D("aMult", "addback multilpicity", 20, 0, 20);
   fH2["gE_chan"] = new TH2D("gE_chan", "gE_chan", 65, 0, 65, 1500, 0, 1500);
   fH1["aE"]      = new TH1D("aE", "Summed Addback", 1500, 0, 1500);
   fH1["gE"]      = new TH1D("gE", "Summed Singles", 1500, 0, 1500);
   fH1["gEnoCT"]  = new TH1D("gEnoCT", "Singles, no CT correction", 1500, 0, 1500);

   for(auto it : fH1) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fH2) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fHSparse) {
      GetOutputList()->Add(it.second);
   }
}

void CrossTalk::FillHistograms()
{
   // find the multiplicity in each clover over the entire event
   // we do this because we want to force a multiplicity of 2
   Int_t det_multiplicity[17] = {0};
   for(auto gr1 = 0; gr1 < fGrif->GetSuppressedMultiplicity(fGriffinBgo); ++gr1) {
      ++(det_multiplicity[fGrif->GetSuppressedHit(gr1)->GetDetector()]);
   }

   for(auto gr1 = 0; gr1 < fGrif->GetSuppressedMultiplicity(fGriffinBgo); ++gr1) {
      if(pileup_reject && (fGrif->GetSuppressedHit(gr1)->GetKValue() != defaultKValue)) continue;   // This pileup number might have to change for other expmnts
      fH1[Form("gEdet%d", fGrif->GetSuppressedHit(gr1)->GetDetector())]->Fill(fGrif->GetSuppressedHit(gr1)->GetEnergy());
      fH2["gE_chan"]->Fill(fGrif->GetSuppressedHit(gr1)->GetArrayNumber(), fGrif->GetSuppressedHit(gr1)->GetEnergy());
      fH1["gE"]->Fill(fGrif->GetSuppressedHit(gr1)->GetEnergy());
      fH1["gEnoCT"]->Fill(fGrif->GetSuppressedHit(gr1)->GetNoCTEnergy());
      for(auto gr2 = gr1 + 1; gr2 < fGrif->GetSuppressedMultiplicity(fGriffinBgo); ++gr2) {
         if(pileup_reject && fGrif->GetSuppressedHit(gr2)->GetKValue() != defaultKValue) continue;   // This pileup number might have to change for other expmnts
         if((det_multiplicity[fGrif->GetSuppressedHit(gr1)->GetDetector()] == 2) && Addback(*(fGrif->GetSuppressedHit(gr1)), *(fGrif->GetSuppressedHit(gr2)))) {
            TGriffinHit *low_crys_hit, *high_crys_hit;
            if(fGrif->GetSuppressedHit(gr1)->GetCrystal() < fGrif->GetSuppressedHit(gr2)->GetCrystal()) {
               low_crys_hit  = fGrif->GetSuppressedHit(gr1);
               high_crys_hit = fGrif->GetSuppressedHit(gr2);
            } else {
               low_crys_hit  = fGrif->GetSuppressedHit(gr2);
               high_crys_hit = fGrif->GetSuppressedHit(gr1);
            }
            if(low_crys_hit->GetCrystal() != high_crys_hit->GetCrystal()) {
               fH2[Form("det_%d_%d_%d", low_crys_hit->GetDetector(), low_crys_hit->GetCrystal(), high_crys_hit->GetCrystal())]->Fill(low_crys_hit->GetNoCTEnergy(), high_crys_hit->GetNoCTEnergy());
            }
         }
      }
   }

   for(auto gr1 = 0; gr1 < fGrif->GetSuppressedAddbackMultiplicity(fGriffinBgo); ++gr1) {
      if(pileup_reject && (fGrif->GetSuppressedAddbackHit(gr1)->GetKValue() != defaultKValue))
         continue;   // This pileup number might have to change for other expmnts
      fH1["aE"]->Fill(fGrif->GetSuppressedAddbackHit(gr1)->GetEnergy());
      fH1[Form("aEdet%d", fGrif->GetSuppressedAddbackHit(gr1)->GetDetector())]->Fill(fGrif->GetSuppressedAddbackHit(gr1)->GetEnergy());
      fH1["aMult"]->Fill(fGrif->GetNSuppressedAddbackFrags(gr1));
      if(fGrif->GetNSuppressedAddbackFrags(gr1) == 2)
         fH1[Form("aE2det%d", fGrif->GetSuppressedAddbackHit(gr1)->GetDetector())]->Fill(fGrif->GetSuppressedAddbackHit(gr1)->GetEnergy());
   }
}
