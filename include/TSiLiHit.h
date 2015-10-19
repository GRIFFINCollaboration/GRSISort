#ifndef TSILIHIT_H
#define TSILIHIT_H

#include <cstdio>
#include <utility>

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h"


class TSiLiHit : public TGRSIDetectorHit {
  public:
    TSiLiHit();
    ~TSiLiHit();

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;

    Short_t  GetSegment()                       {  return segment; }
    Double_t GetEnergy(Option_t *opt = "")      {  return energy;  }
    ULong_t GetTimeStamp(Option_t *opt = "")const{  return ts;    }
    Double_t GetTime(Option_t *opt = "")        {  return time;    }
    Int_t    GetCharge()                        {  return charge;  }
    Double_t GetTimeCFD()                       {  return cfd;     }
    Int_t GetRing()         {  return segment-(floor((double)segment/12.0)*12);  }
    Int_t GetSector()       {  return 9-floor((double)segment/12.0);     }
    Int_t GetPreamp()       {  Int_t sec=this->GetSector();
                                Int_t ring=this->GetRing();
                                Int_t pre=floor((double)sec/3.0);
                                sec-=pre*3;
                                if(sec==2)ring=9-ring;
                                pre*=2; 
                                if(ring%2!=0)pre++;
                                if(pre>1) return 10-pre;
                                else return 2-pre;
                            }
    
    void SetSegment(Short_t seg)       { segment = seg; }
    using TGRSIDetectorHit::SetPosition; //This is here to fix warnings. Will leave when lean-ness occurs
    void SetPosition(TVector3 &vec)    { fposition = vec; }
    void SetVariables(TFragment &frag) { charge = frag.GetCharge();
                                         energy = frag.GetEnergy();
                                         ts     = frag.GetTimeStamp();
                                         time   = frag.GetZCross();
                                         cfd    = frag.GetCfd(); }

  private:
    //TVector3 position;  //held in base.
    Short_t  segment;
    Double_t energy;
    Double_t cfd;
    Int_t    charge;
    ULong_t  ts;
    Double_t time;

  
  ClassDef(TSiLiHit,2);

};



#endif
