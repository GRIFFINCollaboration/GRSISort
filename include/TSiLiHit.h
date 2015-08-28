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

    Short_t  GetSegment() {  return segment;  }
    Double_t GetEnergy()  {  return energy;  }
    Double_t GetTime()    {  return time;    }
    Int_t    GetCharge()  {  return charge;  }
    
    void SetSegment(Short_t seg)       { segment = seg; }
    void SetPosition(TVector3 &vec)    { position = vec; }
    void SetVariables(TFragment &frag) { charge = frag.GetCharge();
                                         energy = frag.GetEnergy();
                                         time   = frag.GetTimeStamp();
                                         cfd    = frag.GetCfd(); }

  private:
    //TVector3 position;  //held in base.
    Short_t  segment;
    Double_t energy;
    Double_t cfd;
    Int_t    charge;
    Long_t   time;

  
  ClassDef(TSiLiHit,2);

};



#endif
