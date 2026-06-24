#include "CrossTalkHelper.hh"

bool pileup_reject = false;

const double gg_time_low  = -200.;
const double gg_time_high = 300.;

// default k-value this might have to be adjusted for each experiment!!!
const Short_t defaultKValue = 379;

bool PromptCoincidence(TGriffinHit* one, TGriffinHit* two)
{
   return ((two->GetTime() - one->GetTime()) >= gg_time_low) && ((two->GetTime() - one->GetTime()) <= gg_time_high);
}

void CrossTalkHelper::CreateHistograms(unsigned int slot)
{
   for(int det_num = 1; det_num <= 16; ++det_num) {
      for(int crys1 = 0; crys1 < 4; ++crys1) {
         for(int crys2 = crys1 + 1; crys2 < 4; ++crys2) {
            std::string name_str  = Form("det_%d_%d_%d", det_num, crys1, crys2);
            const char* hist_name = name_str.c_str();
            std::cout << "Creating histogram: " << hist_name;
            fH2[slot][name_str] = new TH2I(hist_name, Form("Detector #%d, crystal %d vs. crystal %d", det_num, crys2, crys1), 1500, 0, 1500, 1500, 0, 1500);
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
   std::array<Int_t, 17> detMultiplicity = {0};
   for(auto gr1 = 0; gr1 < grif.GetSuppressedMultiplicity(&grifBgo); ++gr1) {
      ++(detMultiplicity[grif.GetSuppressedHit(gr1)->GetDetector()]);
   }

   for(auto gr1 = 0; gr1 < grif.GetSuppressedMultiplicity(&grifBgo); ++gr1) {
      auto* grif1 = grif.GetSuppressedHit(gr1);
      if(pileup_reject && (grif1->GetKValue() != defaultKValue)) { continue; }   // This pileup number might have to change for other expmnts
      fH2[slot].at("gE_chan")->Fill(grif1->GetArrayNumber(), grif1->GetEnergy());
      fH1[slot].at("gE")->Fill(grif1->GetEnergy());
      fH1[slot].at("gEnoCT")->Fill(grif1->GetNoCTEnergy());
      for(auto gr2 = gr1 + 1; gr2 < grif.GetSuppressedMultiplicity(&grifBgo); ++gr2) {
         auto* grif2 = grif.GetSuppressedHit(gr2);
         if(pileup_reject && grif2->GetKValue() != defaultKValue) { continue; }   // This pileup number might have to change for other expmnts
         if((detMultiplicity[grif1->GetDetector()] == 2) && grif.AddbackCriterion(grif1, grif2)) {
            TGriffinHit* lowCrystalHit  = grif1;
            TGriffinHit* highCrystalHit = grif2;
            if(grif1->GetCrystal() > grif2->GetCrystal()) {
               lowCrystalHit  = grif2;
               highCrystalHit = grif1;
            }
            if(lowCrystalHit->GetCrystal() != highCrystalHit->GetCrystal()) {
               fH2[slot].at(Form("det_%d_%d_%d", lowCrystalHit->GetDetector(), lowCrystalHit->GetCrystal(), highCrystalHit->GetCrystal()))->Fill(lowCrystalHit->GetNoCTEnergy(), highCrystalHit->GetNoCTEnergy());
            }
         }
      }
   }

   for(auto gr1 = 0; gr1 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++gr1) {
      auto* grif1 = grif.GetSuppressedAddbackHit(gr1);
      if(pileup_reject && (grif1->GetKValue() != defaultKValue)) { continue; }   // This pileup number might have to change for other expmnts
      fH1[slot].at("aE")->Fill(grif1->GetEnergy());
      fH1[slot].at("aMult")->Fill(grif.GetNSuppressedAddbackFrags(gr1));
   }
}
