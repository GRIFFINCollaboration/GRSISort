#ifndef TS3HIT_H
#define TS3HIT_H

#include <cstdio>
#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h" 

class TS3Hit : public TGRSIDetectorHit {
  public:
    TS3Hit();
    ~TS3Hit();

    Double_t GetEnergy()       {  return energy;  }
    Int_t    GetCharge()       {  return charge;  }
    Long_t   GetTime()         {  return time;    }
    Short_t  GetDetector()     { return detectornumber;  }
    Short_t  GetRingNumber()   { return ring;   }
    Short_t  GetSectorNumber() { return sector; }
    Double_t GetCFD()          { return cfd;    }

  public:
    void Print(Option_t *opt="") const;
    void Clear(Option_t *opt="");

    void SetRingNumber(Short_t rn)     { ring = rn;   }
    void SetSectorNumber(Short_t sn)   { sector = sn; }
    void SetDetectorNumber(Short_t dn) { detectornumber = dn; }
    void SetPosition(TVector3 &vec)    { position = vec; }
    void SetVariables(TFragment &frag) { energy = frag.GetEnergy();
                                         cfd    = frag.GetCfd();
                                         charge = frag.GetCharge();
                                         time   = frag.GetTimeStamp(); }
 

  private:
    //TVector3 position;  held in base.
    Short_t  ring;   //front
    Short_t  sector; //back
    Short_t  detectornumber;
    Double_t energy;
    Double_t cfd;
    Int_t    charge;
    Long_t  time;

  ClassDef(TS3Hit,2);

};

#endif
