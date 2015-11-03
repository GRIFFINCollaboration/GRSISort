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

    Double_t GetLed()          { return fLed;     }
    Short_t  GetRingNumber()   { return fRing;   }
    Short_t  GetSectorNumber() { return fSector; }

  public:
    void Print(Option_t* opt="") const;
    void Clear(Option_t* opt="");

    void SetRingNumber(Short_t rn)     { fRing = rn;   }
    void SetSectorNumber(Short_t sn)   { fSector = sn; }
    void SetVariables(TFragment& frag) { 
			 SetCfd(frag.GetCfd());
			 SetCharge(frag.GetCharge());
			 SetTimeStamp(frag.GetTimeStamp()); 
			 fLed  = frag.GetLed(); 
	 }
	 

  private:
    Double_t fLed;
    Short_t  fRing;   //front
    Short_t  fSector; //back

  ClassDef(TS3Hit,4);
};

#endif
