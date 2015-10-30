#ifndef TS3HIT_H
#define TS3HIT_H

#include "Globals.h"

#include <cstdio>
#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h" 

class TS3Hit : public TGRSIDetectorHit {
   public:
    TS3Hit();
    TS3Hit(TFragment &);
    virtual ~TS3Hit();
	 TS3Hit(const TS3Hit&);

    Double_t GetLed()   const  { return led;    }
    Short_t  GetRing()  const  { return ring;   }
    Short_t  GetSector() const { return sector; }

  public:
    void Copy(TObject&) const;        //!
    void Print(Option_t *opt="") const;
    void Clear(Option_t *opt="");

    void SetRingNumber(Short_t rn)     { ring = rn;   }
    void SetSectorNumber(Short_t sn)   { sector = sn; }
    void SetVariables(TFragment &frag) {  led    = frag.GetLed(); }
 

  private:
      TVector3 GetChannelPosition(Double_t dist = 0) const; //!
    
    Double_t    led;
    Short_t  ring;   //front
    Short_t  sector; //back
//    Short_t  detectornumber;
//   Double_t energy;
//    Double_t cfd;
//    Int_t    charge;
//    Long_t   ts;
//    Int_t    time;

  ClassDef(TS3Hit,5);

};

#endif
