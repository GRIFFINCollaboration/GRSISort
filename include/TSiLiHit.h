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

    Short_t  GetSegment()      {  return segment; }
    Double_t GetEnergy()       {  return energy;  }
    Long_t   GetTimeStamp()    {  return ts;    }
    Int_t    GetTime()         {  return time;    }
    Int_t    GetCharge()       {  return charge;  }
    Double_t GetTimeCFD()      {  return cfd;     }
    Double_t GetTimeLED()      {  return led;     }
    
    Double_t    GetTimeFit()   {  return time_fit; }
    Double_t    GetSig2Noise() {  return sig2noise; }    
    Int_t GetRing()            {  return 9-(segment/12);  }
    Int_t GetSector()          {  return segment%12;     }
    Int_t GetPreamp()          { return ((GetSector()/3)*2)+(((GetSector()%3)+GetRing())%2); }
    
    void SetSegment(Short_t seg)       { segment = seg; }
    using TGRSIDetectorHit::SetPosition; //This is here to fix warnings. Will leave when lean-ness occurs
    void SetPosition(TVector3 &vec)    { fposition = vec; }
    void SetVariables(TFragment &frag) { charge = frag.GetCharge();
                                         energy = frag.GetEnergy();
                                         ts     = frag.GetTimeStamp();
                                         time   = frag.GetZCross();
                                         cfd    = frag.GetCfd();
                                         led    = frag.GetLed();
					 TPulseAnalyzer pulse(frag,4);
					 if(pulse.IsSet()){
						time_fit = pulse.fit_newT0();
						sig2noise= pulse.get_sig2noise();
					 }
					}
//     Double_t fit_time(TFragment &);
    
  private:
    //TVector3 position;  //held in base.
    Short_t  segment;
    Double_t energy;
    Double_t cfd;
    Double_t led;
    Int_t    charge;
    ULong_t  ts;
    Double_t time;
    Double_t    time_fit;
    Double_t    sig2noise;

  
  ClassDef(TSiLiHit,4);

};



#endif
