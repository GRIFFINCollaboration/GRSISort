#ifndef TSILIHIT_H
#define TSILIHIT_H

#include <cstdio>
#include <utility>

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h"
#include "TWaveformAnalyzer.h"

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
    Double_t    GetTimeFit()      {  return time_fit;    }
    Int_t    GetCharge()       {  return charge;  }
    Double_t GetTimeCFD()      {  return cfd;     }
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
					 time_fit = fit_time(frag);}
    Double_t fit_time(TFragment &);
    
  private:
    //TVector3 position;  //held in base.
    Short_t  segment;
    Double_t energy;
    Double_t cfd;
    Int_t    charge;
    ULong_t  ts;
    Double_t time;
    Double_t    time_fit;

  
  ClassDef(TSiLiHit,3);

};



#endif
