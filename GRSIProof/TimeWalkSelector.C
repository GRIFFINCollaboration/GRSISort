#define TimeWalkSelector_cxx
// The class definition in TimeWalkSelector.h has been generated automatically
#include "TimeWalkSelector.h"

void TimeWalkSelector::CreateHistograms()
{
   // Define Histograms
   fH2["time_eng_walk"]   = new TH2F("time_eng_walk", "time_eng_walk", 120, -20, 100, 4000, 0, 4000);
   fH2["time_eng"]        = new TH2F("time_eng", "time_eng", 400, -200, 200, 4000, 0, 4000);
   fH2["time_eng_nogate"] = new TH2F("time_eng_nogate", "time_eng_nogate", 400, -200, 200, 4000, 0, 4000);

   fH2["time_engcfd"]     = new TH2F("time_engcfd", "time_eng including CFD correction", 4000, -2000, 2000, 4000, 0, 4000);
   fH2["time_engcfdnopu"] = new TH2F("time_engcfdnopu", "time_eng including CFD correction", 4000, -2000, 2000, 4000, 0, 4000);

   fH2["time_eng_walk_bg"] = new TH2F("time_eng_walk_bg", "time_eng_walk_bg", 120, -20, 100, 4000, 0, 4000);
   fH2["time_eng_bg"]      = new TH2F("time_eng_bg", "time_eng_bg", 120, -20, 100, 4000, 0, 4000);

   fH2["time_eng_walk_bgnoPU"] = new TH2F("time_eng_walk_bgnoPU", "time_eng_walk_bgnoPU", 120, -20, 100, 4000, 0, 4000);
   fH2["time_eng_bgnoPU"]      = new TH2F("time_eng_bgnoPU", "time_eng_bgnoPU", 120, -20, 100, 4000, 0, 4000);

   fH2["time_eng_walk_noPU"] = new TH2F("time_eng_walk_noPU", "time_eng_walk_noPU", 120, -20, 100, 4000, 0, 4000);
   fH2["time_eng_noPU"]      = new TH2F("time_eng_noPU", "time_eng_noPU", 120, -20, 100, 4000, 0, 4000);
   fH2["eng_time_noPU"]      = new TH2F("eng_time_noPU", "eng_time_noPU", 8000, 0, 8000, 120, -20, 100);

   fH2["time_eng_walk_high"] = new TH2F("time_eng_walk_high", "time_eng_walk_high", 120, -20, 100, 4000, 0, 4000);
   fH2["time_eng_high"]      = new TH2F("time_eng_high", "time_eng_high", 120, -20, 100, 4000, 0, 4000);

   fH2["time_eng_walk2"] = new TH2F("eng_time_walk", "eng_time_walk", 4000, 0, 4000, 120, -20, 100);
   fH2["time_eng2"]      = new TH2F("eng_teng", "eng_time", 4000, 0, 4000, 120, -20, 100);

   fH2["time_eng_walk_no0"] = new TH2F("time_eng_walk_no0", "time_eng_walk_no0", 120, -20, 100, 4000, 0, 4000);
   fH2["time_eng_no0"]      = new TH2F("time_eng_no0", "time_eng_no0", 120, -20, 100, 4000, 0, 4000);

   fH2["time_eng_walk_beta"] = new TH2F("time_eng_walk_beta", "time_eng_walk_beta", 120, -20, 100, 4000, 0, 4000);
   fH2["time_eng_beta"]      = new TH2F("time_eng_beta", "time_eng_beta", 120, -20, 100, 4000, 0, 4000);

   fH3["time_eng_chan"]  = new TH3F("time_engd_chan", "time_engd_chan", 120, -20, 100, 2000, 0, 2000, 64, 0, 64);
   fH3["time_eng_chan2"] = new TH3F("time_engd_chan2", "time_engd_chan2", 120, -20, 100, 2000, 0, 2000, 64, 0, 64);
   fH3["time_eng_det"]   = new TH3F("time_engd_det", "time_engd_det", 120, -20, 100, 2000, 0, 2000, 17, 0, 17);
   fH3["time_eng_det2"]  = new TH3F("time_engd_det2", "time_engd_det2", 120, -20, 100, 2000, 0, 2000, 17, 0, 17);

   fH2["kValueChan"]           = new TH2F("kValueChan", "kValueChan", 800, 0, 800, 65, 0, 65);
   fH2["kValueTDiff"]          = new TH2F("kValueTDiff", "kValueTDiff", 400, -200, 200, 800, 0, 800);
   fH2["kValueTDiff_samechan"] = new TH2F("kValueTDiff_samechan", "kValueTDiff_samechan", 400, -200, 200, 800, 0, 800);
   fH2["kValueTDiff_nogate"]   = new TH2F("kValueTDiff_nogate", "kValueTDiff_nogate", 400, -200, 200, 800, 0, 800);

   // Send histograms to Output list to be added and written.
   for(auto it : fH1) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fH2) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fH3) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fHSparse) {
      GetOutputList()->Add(it.second);
   }
}

bool PromptCoincidence(TGriffinHit* g, TSceptarHit* s)
{
   // Check if hits are less then 300 ns apart.
   return std::fabs(g->GetTime() - s->GetTime()) < 300.;
}

bool PromptCoincidence(TGriffinHit* g1, TGriffinHit* g2)
{
   // Check if hits are less then 500 ns apart.
   return std::fabs(g1->GetTime() - g2->GetTime()) < 500.;
}

void TimeWalkSelector::FillHistograms()
{
   // Loop over all Griffin Hits
   for(auto i = 0; i < fGrif->GetMultiplicity(); ++i) {
      auto grif1 = fGrif->GetGriffinHit(i);
      fH2.at("kValueChan")->Fill(grif1->GetKValue(), grif1->GetArrayNumber());
      // Loop over all sceptar hits
      for(auto j = 0; j < fScep->GetMultiplicity(); ++j) {
         auto scep = fScep->GetSceptarHit(j);
         fH2.at("time_eng_bg")->Fill(grif1->GetTimeStamp() - scep->GetTimeStamp(), grif1->GetEnergy());
         fH2.at("time_eng_walk_bg")->Fill((grif1->GetTime() - scep->GetTime()) / 10., grif1->GetEnergy());
         if(grif1->GetKValue() == 700) {
            fH2.at("time_eng_bgnoPU")->Fill(grif1->GetTimeStamp() - scep->GetTimeStamp(), grif1->GetEnergy());
            fH2.at("time_eng_walk_bgnoPU")->Fill((grif1->GetTime() - scep->GetTime()) / 10., grif1->GetEnergy());
         }
      }
      for(auto j = 0; j < fGrif->GetMultiplicity(); ++j) {
         if(i == j) continue;
         auto   grif2         = fGrif->GetGriffinHit(j);
         long   timediff      = grif2->GetTimeStamp() - grif1->GetTimeStamp();
         double timediff_walk = (grif2->GetTime() - grif1->GetTime()) / 10.;
         fH2.at("time_eng_nogate")->Fill(timediff, grif2->GetEnergy());
         fH2.at("kValueTDiff_nogate")->Fill(timediff, grif2->GetKValue());
         if(grif1->GetEnergy() > 0.) {
            fH2.at("time_eng")->Fill(timediff, grif2->GetEnergy());
            fH2.at("time_eng_walk")->Fill(timediff_walk, grif2->GetEnergy());
            fH3.at("time_eng_chan")->Fill(timediff, grif2->GetEnergy(), grif2->GetArrayNumber());    // was crystal1 + 4*detector2???
            fH3.at("time_eng_chan2")->Fill(timediff, grif2->GetEnergy(), grif1->GetArrayNumber());   // was crystal2 + 4*detector1???
            fH3.at("time_eng_det")->Fill(timediff, grif2->GetEnergy(), grif2->GetDetector());
            fH3.at("time_eng_det2")->Fill(timediff, grif2->GetEnergy(), grif1->GetDetector());
            fH2.at("time_engcfd")->Fill(timediff_walk * 10., grif2->GetEnergy());
            if(grif1->GetKValue() == 700 && grif2->GetKValue() == 700) {
               fH2.at("time_engcfdnopu")->Fill(timediff_walk * 10., grif2->GetEnergy());
               fH2.at("time_eng_noPU")->Fill(timediff, grif2->GetEnergy());
               fH2.at("time_eng_walk_noPU")->Fill(timediff_walk, grif2->GetEnergy());
               fH2.at("eng_time_noPU")->Fill(grif2->GetEnergy(), timediff);
            }
            fH2.at("kValueTDiff")->Fill(timediff, grif2->GetKValue());
            if(grif1->GetAddress() == grif2->GetAddress()) {
               fH2.at("kValueTDiff")->Fill(timediff, grif2->GetKValue());
            }
            if(grif1->GetAddress() != 0 && grif2->GetAddress() != 0) {
               fH2.at("time_eng_no0")->Fill(timediff, grif2->GetEnergy());
               fH2.at("time_eng_walk_no0")->Fill(timediff_walk, grif2->GetEnergy());
            }
            fH2.at("time_eng2")->Fill(grif2->GetEnergy(), timediff);
            fH2.at("time_eng_walk2")->Fill(grif2->GetEnergy(), timediff_walk);
         }
         if(grif1->GetEnergy() < 500.) {
            fH2.at("time_eng_high")->Fill(timediff, grif2->GetEnergy());
            fH2.at("time_eng_walk_high")->Fill(timediff_walk, grif2->GetEnergy());
         }
      }
   }
   for(auto i = 0; i < fScep->GetMultiplicity(); ++i) {
      auto scep1 = fScep->GetSceptarHit(i);
      for(auto j = 0; j < fScep->GetMultiplicity(); ++j) {
         if(i == j) continue;
         auto scep2 = fScep->GetSceptarHit(j);
         fH2.at("time_eng_beta")->Fill(scep2->GetTimeStamp() - scep1->GetTimeStamp(), scep2->GetEnergy());
         fH2.at("time_eng_walk_beta")->Fill((scep2->GetTime() - scep1->GetTime()) / 10., scep2->GetEnergy());
      }
   }
}
