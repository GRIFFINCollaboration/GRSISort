#include "TRuntimeObjects.h"

#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TSceptar.h"
#include "TZeroDegree.h"
#include "TDescant.h"

const Double_t ps          = 1E-3;
const Double_t ns          = 1.;
const Double_t ts_units    = 10*ns;
const Double_t us          = 1E3;
const Double_t ms          = 1E6;
const Double_t s           = 1E9;

const Double_t keV         = 1.;
const Double_t MeV         = 1E3;

const Double_t ggTLow      = -200*ns;  
const Double_t ggTHigh     =  200*ns; 
const Double_t gbgoTLow    = -250*ns;  
const Double_t gbgoTHigh   =  250*ns; 
const Double_t gbTLow      = -150*ns;  
const Double_t gbTHigh     =  300*ns; 
const Double_t gzTLow      =  -50*ns;  
const Double_t gzTHigh     =  50*ns; 

const Double_t bELow       = 50*keV;
const Double_t zELow       = 200*keV;
const ULong64_t kCycleLength = 34.5*s;

bool PromptCoincidence(TGriffinHit* hit1, TGriffinHit* hit2){
   return (((hit2->GetTime()*ns - hit1->GetTime()*ns) >= ggTLow) && ((hit2->GetTime()*ns - hit1->GetTime()*ns) <= ggTHigh)); 
}

bool PromptCoincidence(TBgoHit* bgo_hit, TGriffinHit* grif_hit){
   return (((grif_hit->GetTime()*ns - bgo_hit->GetTime()*ns) >= gbgoTLow) && ((grif_hit->GetTime()*ns - bgo_hit->GetTime()*ns) <= gbgoTHigh)); 
}

bool PromptCoincidence(TSceptarHit* sc_hit, TGriffinHit* grif_hit){
   return (((grif_hit->GetTime()*ns - sc_hit->GetTime()*ns) >= gbTLow) && ((grif_hit->GetTime()*ns - sc_hit->GetTime()*ns) <= gbTHigh) && (sc_hit->GetEnergy() > bELow)); 
}

bool PromptCoincidence(TZeroDegreeHit* sc_hit, TGriffinHit* grif_hit){
   return (((grif_hit->GetTime()*ns - sc_hit->GetTime()*ns) >= gzTLow) && ((grif_hit->GetTime()*ns - sc_hit->GetTime()*ns) <= gzTHigh) && (sc_hit->GetEnergy() > zELow) && (sc_hit->GetDetector() == 1)); 
}

/*
bool GriffinCloverSuppression(const TGRSIDetectorHit& hit, const TBgoHit& bgoHit){
	return ((hit.GetDetector() == bgoHit.GetDetector() &&
	(std::fabs(hit.GetTime() - bgoHit.GetTime()) < TGRSIOptions::AnalysisOptions()->SuppressionWindow()) &&
	(bgoHit.GetEnergy() > TGRSIOptions::AnalysisOptions()->SuppressionEnergy()));
}
*/
//std::function<bool(const TGRSIDetectorHit&, const TBgoHit&)> TGriffin::fSuppressionCriterion = DefaultGriffinSuppression;
extern "C" void MakeAnalysisHistograms(TRuntimeObjects& obj)
{
   auto grif      = obj.GetDetector<TGriffin>();
   auto grif_bgo  = obj.GetDetector<TGriffinBgo>().get();
   auto scep      = obj.GetDetector<TSceptar>();
   auto zds       = obj.GetDetector<TZeroDegree>();
   auto descant   = obj.GetDetector<TDescant>();
   
   static int kRunNumber = 0;
   if(kRunNumber == 0){
      kRunNumber = TGRSIRunInfo::RunNumber();
   }

   static ULong64_t less_background = 0.5*s;
   
   if(kRunNumber >= 12012){
      less_background = 0.0*s;
   }
   else{
      less_background = 0.5*s;
   }


//   static TPPG* ppg = TPPG::Get();
 //  static const ULong64_t kCycleLength = ppg->OdbCycleLength();//in us

   if(grif != nullptr) {
      for(auto g1 = 0; g1 < grif->GetMultiplicity(); ++g1){
         obj.FillHistogram("GRIFFIN","gE_Cyc",5000,0,5000, grif->GetGriffinHit(g1)->GetEnergy(),kCycleLength*20/s,0,kCycleLength/s,(((ULong64_t)(grif->GetHit(g1)->GetTimeStamp()*ts_units))%(kCycleLength-less_background)+less_background)/s);
         if(scep != nullptr){
            bool beta_found = false;
            for(auto b1 = 0; b1 < scep->GetMultiplicity(); ++b1){
               obj.FillHistogram("TimeDiffs","gbT",2000,-2000,2000, grif->GetHit(g1)->GetTime()*ns - scep->GetHit(b1)->GetTime()*ns,20,1,21,scep->GetHit(b1)->GetDetector());
               if(PromptCoincidence(scep->GetSceptarHit(b1),grif->GetGriffinHit(g1)) && !beta_found ){
                  obj.FillHistogram("GRIFFIN","gEB",8000,0,8000,grif->GetHit(g1)->GetEnergy());
                  beta_found = true;
               }
            }
         }
         if(zds != nullptr){
            bool beta_found = false;
            for(auto z1 = 0; z1 < zds->GetMultiplicity(); ++z1){
               obj.FillHistogram("TimeDiffs","gzT",2000,-2000,2000, grif->GetHit(g1)->GetTime()*ns - zds->GetHit(z1)->GetTime()*ns,2,1,3,zds->GetHit(z1)->GetDetector());
               if(PromptCoincidence(zds->GetZeroDegreeHit(z1),grif->GetGriffinHit(g1)) && !beta_found ){
                  obj.FillHistogram("GRIFFIN","gEZ",8000,0,8000,grif->GetHit(g1)->GetEnergy());
                  beta_found = true;
               }
            }
         }
         for(auto g2 = g1+1; g2 < grif->GetMultiplicity(); ++g2){
            obj.FillHistogram("TimeDiffs","ggT",2000,-2000,2000, grif->GetHit(g2)->GetTime()*ns - grif->GetHit(g1)->GetTime()*ns);
            if(PromptCoincidence(grif->GetGriffinHit(g1),grif->GetGriffinHit(g2))){
               obj.FillHistogram("GRIFFIN","ggE",5000,0,5000,grif->GetHit(g1)->GetEnergy(),5000,0,5000,grif->GetHit(g2)->GetEnergy());
               obj.FillHistogram("GRIFFIN","ggE",5000,0,5000,grif->GetHit(g2)->GetEnergy(),5000,0,5000,grif->GetHit(g1)->GetEnergy());
               obj.FillHistogram("HitPatterns","gg_HP",65,0,65,grif->GetGriffinHit(g1)->GetChannel()->GetNumber(),65,0,65,grif->GetGriffinHit(g2)->GetChannel()->GetNumber());
            }
         }
         if(grif_bgo){
            for(auto bgo1 = 0; bgo1 < grif_bgo->GetMultiplicity(); ++bgo1){
               obj.FillHistogram("TimeDiffs","gbgoT",2000,-2000,2000,grif_bgo->GetHit(bgo1)->GetTime()*ns-grif->GetHit(g1)->GetTime()*ns,64*5,1,64*5+1,grif_bgo->GetHit(bgo1)->GetArrayNumber());
               obj.FillHistogram("HitPatterns","ggbgo_HP",65,0,65,grif->GetGriffinHit(g1)->GetArrayNumber(),64*5,1,64*5+1,grif_bgo->GetBgoHit(bgo1)->GetArrayNumber());

               obj.FillHistogram("HitPatterns","gbgoDet",17,0,17,grif->GetGriffinHit(g1)->GetDetector(),17,0,17,grif_bgo->GetHit(bgo1)->GetDetector());
               if((grif_bgo->GetHit(bgo1)->GetEnergy() > 50) && PromptCoincidence(grif_bgo->GetBgoHit(bgo1),grif->GetGriffinHit(g1))){
                  obj.FillHistogram("HitPatterns","gbgoDet_ETCut",17,0,17,grif->GetGriffinHit(g1)->GetDetector(),17,0,17,grif_bgo->GetHit(bgo1)->GetDetector());
               }
               if(grif->GetHit(g1)->GetDetector() == grif_bgo->GetHit(bgo1)->GetDetector()){
                  obj.FillHistogram("GRIFFIN","bgoEgE",2000,0,2000,grif->GetHit(g1)->GetEnergy(),2000,0,2000,grif_bgo->GetHit(bgo1)->GetEnergy());
                  obj.FillHistogram("GRIFFIN","bgoTgE",2000,0,2000,grif->GetHit(g1)->GetEnergy(),2000,-1000,1000,grif_bgo->GetHit(bgo1)->GetTime()*ns - grif->GetHit(g1)->GetTime()*ns);
                  if(std::fabs(grif_bgo->GetHit(bgo1)->GetTime()*ns - grif->GetHit(g1)->GetTime()*ns) < 300*ns){
                     obj.FillHistogram("GRIFFIN","bgoEgE_TCut",2000,0,2000,grif->GetHit(g1)->GetEnergy(),2000,0,2000,grif_bgo->GetHit(bgo1)->GetEnergy());
                  }
                  if(grif_bgo->GetHit(bgo1)->GetEnergy()*keV > 50*keV){ 
                     obj.FillHistogram("GRIFFIN","bgoTgE_ECut",2000,0,2000,grif->GetHit(g1)->GetEnergy(),2000,-1000,1000,grif_bgo->GetHit(bgo1)->GetTime()*ns - grif->GetHit(g1)->GetTime()*ns);
                  }
               }
            } 
         }    
      }
      for(auto a1 = 0; a1 < grif->GetAddbackMultiplicity(); ++a1){
		   obj.FillHistogram("GRIFFIN","aE", 5000, 0, 5000, grif->GetAddbackHit(a1)->GetEnergy());
		   obj.FillHistogram("GRIFFIN","aE_Cyc", 5000, 0, 5000, grif->GetAddbackHit(a1)->GetEnergy(),kCycleLength*20/s,0,kCycleLength/s,(((ULong64_t)(grif->GetHit(a1)->GetTimeStamp()*ts_units))%(kCycleLength-less_background)+less_background)/s);
         if(zds != nullptr){
            bool beta_found = false;
            for(auto z1 = 0; z1 < zds->GetMultiplicity(); ++z1){
               if(PromptCoincidence(zds->GetZeroDegreeHit(z1),grif->GetAddbackHit(a1)) && !beta_found ){
                  obj.FillHistogram("GRIFFIN","aEZ_Cyc",8000,0,8000,grif->GetAddbackHit(a1)->GetEnergy(),kCycleLength*20/s,0,kCycleLength/s,(((ULong64_t)(grif->GetHit(a1)->GetTimeStamp()*ts_units))%(kCycleLength-less_background)+less_background)/s);
                  beta_found = true;
               }
            }
         }
      }
      for(auto as1 = 0; as1 < grif->GetSuppressedAddbackMultiplicity(grif_bgo); ++as1){
         obj.FillHistogram("GRIFFIN","aES",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy());
         bool zds_or_scep_found = false;
         if(scep != nullptr){
            bool beta_found = false;
            for(auto b1 = 0; b1 < scep->GetMultiplicity(); ++b1){
               if(PromptCoincidence(scep->GetSceptarHit(b1),grif->GetSuppressedAddbackHit(as1)) && !beta_found ){
                  obj.FillHistogram("GRIFFIN","aESB_Cyc",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy(),kCycleLength*20/s,0,kCycleLength/s,(((ULong64_t)(grif->GetSuppressedAddbackHit(as1)->GetTimeStamp()*ts_units))%(kCycleLength-less_background)+less_background)/s);
                  beta_found = true;

                  if(!zds_or_scep_found){
                     obj.FillHistogram("GRIFFIN","aESbeta_Cyc",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy(),kCycleLength*20/s,0,kCycleLength/s,(((ULong64_t)(grif->GetSuppressedAddbackHit(as1)->GetTimeStamp()*ts_units))%(kCycleLength-less_background)+less_background)/s);
                     zds_or_scep_found = true;
                  }
               }
            }
         }
         if(zds != nullptr){
            bool beta_found = false;
            for(auto z1 = 0; z1 < zds->GetMultiplicity(); ++z1){
               if(PromptCoincidence(zds->GetZeroDegreeHit(z1),grif->GetSuppressedAddbackHit(as1)) && !beta_found ){
                  obj.FillHistogram("GRIFFIN","aESZ_Cyc",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy(),kCycleLength*20/s,0,kCycleLength/s,(((ULong64_t)(grif->GetSuppressedAddbackHit(as1)->GetTimeStamp()*ts_units))%(kCycleLength-less_background)+less_background)/s);
                  beta_found = true;
                  if(!zds_or_scep_found){
                     obj.FillHistogram("GRIFFIN","aESbeta_Cyc",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy(),kCycleLength*20/s,0,kCycleLength/s,(((ULong64_t)(grif->GetSuppressedAddbackHit(as1)->GetTimeStamp()*ts_units))%(kCycleLength-less_background)+less_background)/s);
                     zds_or_scep_found = true;
                  }
               }
            }
         }
         for(auto as2 = as1+1; as2 < grif->GetSuppressedAddbackMultiplicity(grif_bgo); ++as2){
               obj.FillHistogram("GRIFFIN","aaES",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy());
               obj.FillHistogram("GRIFFIN","aaES",8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy());
         
            bool combined_beta_found = false;
            if(scep != nullptr){
               bool beta_found = false;
               for(auto b1 = 0; b1 < scep->GetMultiplicity(); ++b1){
                  if((PromptCoincidence(scep->GetSceptarHit(b1),grif->GetSuppressedAddbackHit(as1)) || PromptCoincidence(scep->GetSceptarHit(b1),grif->GetSuppressedAddbackHit(as2)))&& !beta_found){
                     obj.FillHistogram("GRIFFIN","aaESb",8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy());
                     obj.FillHistogram("GRIFFIN","aaESb",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy());
                     beta_found = true;

                     if(!combined_beta_found){
                        obj.FillHistogram("GRIFFIN","aaESbeta",8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy());
                        obj.FillHistogram("GRIFFIN","aaESbeta",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy());
                        combined_beta_found = true;

                     }

                  }
               }
            }
            if(zds != nullptr){
               bool beta_found = false;
               for(auto z1 = 0; z1 < zds->GetMultiplicity(); ++z1){
                  if((PromptCoincidence(zds->GetZeroDegreeHit(z1),grif->GetSuppressedAddbackHit(as1)) || PromptCoincidence(zds->GetZeroDegreeHit(z1),grif->GetSuppressedAddbackHit(as2)))&& !beta_found){
                     obj.FillHistogram("GRIFFIN","aaESz",8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy());
                     obj.FillHistogram("GRIFFIN","aaESz",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy());
                     beta_found = true;
                     if(!combined_beta_found){
                        obj.FillHistogram("GRIFFIN","aaESbeta",8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy());
                        obj.FillHistogram("GRIFFIN","aaESbeta",8000,0,8000,grif->GetSuppressedAddbackHit(as1)->GetEnergy(),8000,0,8000,grif->GetSuppressedAddbackHit(as2)->GetEnergy());
                        combined_beta_found = true;
                     }
                  }
               }
            }
         }
      }

      for(auto sg1 = 0; sg1 < grif->GetSuppressedMultiplicity(grif_bgo); ++sg1){
         obj.FillHistogram("HitPatterns","sMultgMult",100,0,100,grif->GetMultiplicity(),100,0,100,grif->GetSuppressedMultiplicity(grif_bgo));
         obj.FillHistogram("GRIFFIN","gES", 8000, 0, 8000, grif->GetSuppressedHit(sg1)->GetEnergy(),65,0,65,grif->GetSuppressedHit(sg1)->GetArrayNumber());
         for(auto sg2 = sg1+1; sg2 < grif->GetSuppressedMultiplicity(grif_bgo); ++sg2){
            obj.FillHistogram("GRIFFIN","ggES", 8000, 0, 8000, grif->GetSuppressedHit(sg1)->GetEnergy(),8000,0,8000,grif->GetSuppressedHit(sg2)->GetEnergy());
            obj.FillHistogram("GRIFFIN","ggES", 8000, 0, 8000, grif->GetSuppressedHit(sg2)->GetEnergy(),8000,0,8000,grif->GetSuppressedHit(sg1)->GetEnergy());
         }
   
         if(scep != nullptr){
            bool beta_found = false;
            for(auto b1 = 0; b1 < scep->GetMultiplicity(); ++b1){
               if(PromptCoincidence(scep->GetSceptarHit(b1),grif->GetSuppressedHit(sg1)) && !beta_found ){
                  obj.FillHistogram("GRIFFIN","gEBS",8000,0,8000,grif->GetSuppressedHit(sg1)->GetEnergy());
                  beta_found = true;
               }
            }
         }
         if(grif_bgo){
            for(auto bgo1 = 0; bgo1 < grif_bgo->GetMultiplicity(); ++bgo1){
               obj.FillHistogram("HitPatterns","ggbgoS_HP",65,0,65,grif->GetSuppressedHit(sg1)->GetArrayNumber(),64*5,1,64*5+1,grif_bgo->GetBgoHit(bgo1)->GetArrayNumber());
            } 
         }               
      }
	}
   if(grif_bgo != nullptr){
      for(auto bgo1 = 0; bgo1 < grif_bgo->GetMultiplicity(); ++bgo1){
         obj.FillHistogram("BGO","bgoE",3000,0,3000,grif_bgo->GetHit(bgo1)->GetEnergy(),64*5,1,64*5+1,grif_bgo->GetBgoHit(bgo1)->GetArrayNumber());
         obj.FillHistogram("HitPatterns", "bgoHP", 20*16+1,0,20*16+1,grif_bgo->GetHit(bgo1)->GetArrayNumber());
      } 
   }
   if(zds != nullptr){
      for(auto z1 = 0; z1 < zds->GetMultiplicity(); ++z1){
         obj.FillHistogram("ZDS","ZE",2500,0,65000,zds->GetHit(z1)->GetEnergy(),2,1,3,zds->GetHit(z1)->GetDetector());
         obj.FillHistogram("ZDS","z_Cyc",1000,0,10000,zds->GetHit(z1)->GetEnergy(), kCycleLength*20/s,0,kCycleLength/s,(((ULong64_t)(zds->GetHit(z1)->GetTimeStamp()*ts_units))%(kCycleLength-less_background)+less_background)/s);
      }
   }
   if(scep != nullptr){
      for(auto b1 = 0; b1 < scep->GetMultiplicity(); ++b1){
         obj.FillHistogram("SCEPTAR","bE",10000,0,6e7,scep->GetHit(b1)->GetEnergy(),20,1,21,scep->GetHit(b1)->GetDetector());
         obj.FillHistogram("HitPatterns", "bHP", 21,0,21,scep->GetHit(b1)->GetDetector());
         obj.FillHistogram("SCEPTAR","b_Cyc", kCycleLength*20/s,0,kCycleLength/s,(((ULong64_t)(scep->GetHit(b1)->GetTimeStamp()*ts_units))%(kCycleLength-less_background)+less_background)/s);
      }
   }
   if(descant != nullptr){
      for(auto d1 = 0; d1 < descant->GetMultiplicity(); ++d1){
         obj.FillHistogram("HitPatterns", "dHP", 10,0,10,descant->GetHit(d1)->GetDetector());
      }
   }
   if(grif != nullptr){
      for(auto g1 = 0; g1 < grif->GetLowGainMultiplicity(); ++g1){
         obj.FillHistogram("HitPatterns", "gHP_lg", 65,0,65,grif->GetGriffinLowGainHit(g1)->GetArrayNumber());
		   obj.FillHistogram("GRIFFIN","gE_lg", 5000, 0, 5000, grif->GetGriffinLowGainHit(g1)->GetEnergy(),65,0,65,grif->GetGriffinLowGainHit(g1)->GetArrayNumber());

      }
      for(auto g1 = 0; g1 < grif->GetHighGainMultiplicity(); ++g1){
         obj.FillHistogram("HitPatterns", "gHP_hg", 65,0,65,grif->GetGriffinHighGainHit(g1)->GetArrayNumber());
		   obj.FillHistogram("GRIFFIN","gE_hg", 5000, 0, 5000, grif->GetGriffinHighGainHit(g1)->GetEnergy(),65,0,65,grif->GetGriffinHighGainHit(g1)->GetArrayNumber());
      }
   }

}
