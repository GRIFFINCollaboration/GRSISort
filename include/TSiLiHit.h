#ifndef TSILIHIT_H
#define TSILIHIT_H

#include <cstdio>
#include <utility>

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h"
#include "TPulseAnalyzer.h"

class TSiLiHit : public TGRSIDetectorHit {
  public:
    TSiLiHit();
    ~TSiLiHit();

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;
    
    Double_t GetLed()          {  return led;     }
    Short_t  GetSegment()      {  return segment; }
    Double_t    GetSig2Noise() {  return sig2noise;}    
    Int_t GetRing()            {  return ring;      }
    Int_t GetSector()          {  return sector;   }
    Int_t GetPreamp()          { return  preamp;   }
    Double_t    GetTimeFit()   {  return time_fit; }

    void SetSegment(Short_t seg)       { segment = seg; 
					 ring    = 9-(segment/12);
					 sector  = segment%12;
					 preamp  = ((GetSector()/3)*2)+(((GetSector()%3)+GetRing())%2);
					}
//     using TGRSIDetectorHit::SetPosition; //This is here to fix warnings. Will leave when lean-ness occurs
//     void SetPosition(TVector3 &vec)    { fposition = vec; }
    void SetVariables(TFragment &frag) { SetEnergy(frag.GetEnergy());
                                         SetCfd(frag.GetCfd());
                                         SetCharge(frag.GetCharge());
                                         SetTimeStamp(frag.GetTimeStamp()); 
                                         SetTime(frag.GetZCross());
					 
					 led    = frag.GetLed();
				        }
    void SetWavefit(TFragment&);

//     Double_t fit_time(TFragment &);
    
  private:
    //TVector3 position;  //held in base.
    Double_t    led;
    Short_t  segment;
    Short_t  ring;
    Short_t  sector;
    Short_t  preamp;
    Double_t    time_fit;
    Double_t    sig2noise;
  
  ClassDef(TSiLiHit,5);

};



#endif
