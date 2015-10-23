#ifndef TTIPHIT_H
#define TTIPHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
#include "TCrystalHit.h"

#include "TVector3.h"


#include "TGRSIDetectorHit.h"

class TTipHit : public TGRSIDetectorHit {
  public:
    TTipHit();
    virtual ~TTipHit();
    TTipHit(const TTipHit&);

  private:
    Int_t    filter;    // 
    Double_t fPID;       // 

    Double_t fast_amplitude;
    Double_t slow_amplitude;
    Double_t gamma_amplitude;
   
    Double_t fit_time;

  public:
    /////////////////////////    /////////////////////////////////////
    inline void SetFilterPattern(const int &x)   { filter   = x; }   //! 
    inline void SetPID(Double_t x)                { fPID = x;     }   //!

    inline Int_t    GetFiterPatter()           { return filter;   }  //!
    inline Double_t GetPID()                   { return fPID;      }  //!
    TVector3 GetPosition(Double_t dist=0) const   { return TVector3();}

    bool   InFilter(Int_t);                                         //!

  public:
    void Clear(Option_t *opt = "");                        //!
    void Print(Option_t *opt = "") const;                  //!
    virtual void Copy(TObject&) const;                     //!

    ClassDef(TTipHit,1);

};

#endif
