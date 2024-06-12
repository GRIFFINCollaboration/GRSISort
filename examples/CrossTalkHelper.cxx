#include "CrossTalkHelper.hh"

bool pileup_reject = false;

const double gg_time_low  = -200.;
const double gg_time_high = 300.;

// Beta gamma histograms
const double gb_time_low  = -250.;
const double gb_time_high = 350.;

// default k-value this might have to be adjusted for each experiment!!!
const Short_t defaultKValue = 379;

bool Addback(TGriffinHit* one, TGriffinHit* two)
{
   return ((one->GetDetector() == two->GetDetector()) && (std::fabs(one->GetTime() - two->GetTime()) < 300.));
}

bool PromptCoincidence(TGriffinHit* one, TGriffinHit* two)
{

   return ((two->GetTime() - one->GetTime()) >= gg_time_low) && ((two->GetTime() - one->GetTime()) <= gg_time_high);
}

void CrossTalkHelper::CreateHistograms(unsigned int slot)
{
   for(int det_num = 1; det_num <= 16; ++det_num) {
      std::string aedet_str  = Form("aEdet%d", det_num);
      std::string gedet_str  = Form("gEdet%d", det_num);
      std::string ae2det_str = Form("aE2det%d", det_num);
      fH1[slot][aedet_str]   = new TH1D(Form("aEdet%d", det_num), Form("Addback detector %d", det_num), 1500, 0, 1500);
      fH1[slot][gedet_str]   = new TH1D(Form("geEdet%d", det_num), Form("Singles detector %d", det_num), 1500, 0, 1500);
      fH1[slot][ae2det_str]  = new TH1D(Form("aE2det%d", det_num), Form("Addback with 2 hits, detector %d", det_num), 1500, 0, 1500);
      for(int crys_1 = 0; crys_1 < 4; ++crys_1) {
         for(int crys_2 = crys_1 + 1; crys_2 < 4; ++crys_2) {
            std::string name_str  = Form("det_%d_%d_%d", det_num, crys_1, crys_2);
            const char* hist_name = name_str.c_str();
            std::cout << "Creating histogram: " << hist_name;
            fH2[slot][name_str] = new TH2I(hist_name, hist_name, 1500, 0, 1500, 1500, 0, 1500);
            std::cout << " at address: " << fH2[slot][hist_name] << std::endl;
         }
      }
   }

   fH1[slot]["aMult"]   = new TH1D("aMult", "addback multilpicity", 20, 0, 20);
   fH2[slot]["gE_chan"] = new TH2D("gE_chan", "gE_chan", 65, 0, 65, 1500, 0, 1500);
   fH1[slot]["aE"]      = new TH1D("aE", "Summed Addback", 1500, 0, 1500);
   fH1[slot]["gE"]      = new TH1D("gE", "Summed Singles", 1500, 0, 1500);
   fH1[slot]["gEnoCT"]  = new TH1D("gEnoCT", "Singles, no CT correction", 1500, 0, 1500);
}

void CrossTalkHelper::Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo)
{
   // find the multiplicity in each clover over the entire event
   // we do this because we want to force a multiplicity of 2
   Int_t det_multiplicity[17] = {0};
   for(auto gr1 = 0; gr1 < grif.GetSuppressedMultiplicity(&grifBgo); ++gr1) {
      ++(det_multiplicity[grif.GetSuppressedHit(gr1)->GetDetector()]);
   }

   for(auto gr1 = 0; gr1 < grif.GetSuppressedMultiplicity(&grifBgo); ++gr1) {
      auto grif1 = grif.GetSuppressedHit(gr1);
      if(pileup_reject && (grif1->GetKValue() != defaultKValue)) continue;   // This pileup number might have to change for other expmnts
      fH1[slot].at(Form("gEdet%d", grif1->GetDetector()))->Fill(grif1->GetEnergy());
      fH2[slot].at("gE_chan")->Fill(grif1->GetArrayNumber(), grif1->GetEnergy());
      fH1[slot].at("gE")->Fill(grif1->GetEnergy());
      fH1[slot].at("gEnoCT")->Fill(grif1->GetNoCTEnergy());
      for(auto gr2 = gr1 + 1; gr2 < grif.GetSuppressedMultiplicity(&grifBgo); ++gr2) {
         auto grif2 = grif.GetSuppressedHit(gr2);
         if(pileup_reject && grif2->GetKValue() != defaultKValue) continue;   // This pileup number might have to change for other expmnts
         if((det_multiplicity[grif1->GetDetector()] == 2) && Addback(grif1, grif2)) {
            TGriffinHit *low_crys_hit, *high_crys_hit;
            if(grif1->GetCrystal() < grif2->GetCrystal()) {
               low_crys_hit  = grif1;
               high_crys_hit = grif2;
            } else {
               low_crys_hit  = grif2;
               high_crys_hit = grif1;
            }
            if(low_crys_hit->GetCrystal() != high_crys_hit->GetCrystal()) {
               fH2[slot].at(Form("det_%d_%d_%d", low_crys_hit->GetDetector(), low_crys_hit->GetCrystal(), high_crys_hit->GetCrystal()))->Fill(low_crys_hit->GetNoCTEnergy(), high_crys_hit->GetNoCTEnergy());
            }
         }
      }
   }

   for(auto gr1 = 0; gr1 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++gr1) {
      auto grif1 = grif.GetSuppressedAddbackHit(gr1);
      if(pileup_reject && (grif1->GetKValue() != defaultKValue)) continue;   // This pileup number might have to change for other expmnts
      fH1[slot].at("aE")->Fill(grif1->GetEnergy());
      fH1[slot].at(Form("aEdet%d", grif1->GetDetector()))->Fill(grif1->GetEnergy());
      fH1[slot].at("aMult")->Fill(grif.GetNSuppressedAddbackFrags(gr1));
      if(grif.GetNSuppressedAddbackFrags(gr1) == 2) fH1[slot].at(Form("aE2det%d", grif1->GetDetector()))->Fill(grif1->GetEnergy());
   }
}
