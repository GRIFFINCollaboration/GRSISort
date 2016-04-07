#ifndef TSHARCHIT_H
#define TSHARCHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <utility>

#include "TVector3.h" 
#include "TObject.h" 
#include "TRandom.h"
#include "Rtypes.h"
#include "TMath.h"

#include "Globals.h"
#include "TGRSIDetectorHit.h"
#include "TFragment.h"

/////////////////////////////////////////////////////////////////////////
//
//  \class TSharcHit, the leaner method.
//
//  This is the updated sharc-hit storage class, designed to better work with 
//  the GRSISort Inheritied class method.  A lot has changed, but the main function
//  stays the same.  Since two physial wire hits are needed to make one sharc hit,
//  and the "front" side will always have better resolution/timing properties, we 
//  will make the inherited charge/time derive from the front.  So we do not lose the 
//  back information, the class additionally holds a TGRSIDetector for the back and
//  another for the pad if it is present.
//
////////////////////////////////////////////////////////////////////////////

class TSharcHit : public TGRSIDetectorHit {
  public:
    TSharcHit();
    TSharcHit(const TSharcHit&);
    ~TSharcHit();
  
  private:
    //UShort_t   fDetectorNumber;  //
    //UShort_t   fFrontStrip;     //
    //UShort_t   fBackStrip;      //

    TGRSIDetectorHit fBackHit;   //
    TGRSIDetectorHit fPadHit;    //

  public:
    virtual void Copy(TObject &) const;        //!<!
    virtual void Clear(Option_t* = "");        //!<!
    virtual void Print(Option_t* = "")  const; //!<!
    
    //inline UShort_t GetDetector()       const  { return TGRSIDetectorHit::GetDetector();  } //!<!
    UShort_t GetFrontStrip()     const  { return TGRSIDetectorHit::GetSegment();      }  //!<!
    UShort_t GetBackStrip()      const  { return GetBack().GetSegment();       }  //!<!

    //TGRSIDetectorHit* GetFront() const { return this; }
    TGRSIDetectorHit GetBack()  const { return fBackHit; }
    TGRSIDetectorHit GetPad()   const { return fPadHit; }

    inline Double_t GetDeltaE()       { return TGRSIDetectorHit::GetEnergy();  }  //!<!
    inline Double_t GetDeltaT()       { return TGRSIDetectorHit::GetTime();    }  //!<!

    inline Double_t GetDeltaFrontE()  { return TGRSIDetectorHit::GetEnergy(); }  //!<!
    inline Double_t GetDeltaFrontT()  { return TGRSIDetectorHit::GetTime();   }  //!<!
    inline Double_t GetDeltaBackE()   { return GetBack().GetEnergy();  }  //!<!
    inline Double_t GetDeltaBackT()   { return GetBack().GetTime();    }  //!<!

    inline Int_t GetFrontAddress()    { return TGRSIDetectorHit::GetAddress(); }  //!<!
    inline Int_t GetBackAddress()     { return GetBack().GetAddress();  }  //!<!
    inline Int_t GetPadAddress()      { return GetPad().GetAddress();   }  //!<!

    inline Double_t GetPadE()         { return GetPad().GetEnergy();    }  //!<!
    inline Double_t GetPadT()         { return GetPad().GetTime();      }  //!<!

    //std::pair<int,int>  GetPixel()  { return std::make_pair(fFrontStrip,fBackStrip);  }  //!<!

    Float_t    GetFrontCharge()        { return TGRSIDetectorHit::GetCharge();  }  //!<!  //Charge is now stored after integration.
    Float_t    GetBackCharge()         { return GetBack().GetCharge();   }  //!<!  //Charge is now stored after integration.
    Float_t    GetPadCharge()          { return GetPad().GetCharge();    }  //!<!  //Charge is now stored after integration.

    inline Double_t GetEnergy(Option_t* opt = "") { if(GetPadAddress()!=-1) 
                                                      return TGRSIDetectorHit::GetEnergy() + GetPad().GetEnergy(); 
                                                    return TGRSIDetectorHit::GetEnergy(); 
                                                  }
    inline Double_t GetTime(Option_t* opt = "")   { return TGRSIDetectorHit::GetTime(); }
   
    Double_t GetThetaDeg(double Xoff = 0.0, double Yoff = 0.0, double Zoff = 0.0) { return GetTheta(Xoff,Yoff,Zoff)*TMath::RadToDeg(); } ; //!<! 
    Double_t GetTheta(double Xoff = 0.0, double Yoff = 0.0, double Zoff = 0.0); //!<! 
    
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
    //void SetDetectorNumber(const UShort_t& det) { fDetectorNumber = det;   }  //!<!
    //void SetFrontStrip(const UShort_t& strip)   { fFrontStrip    = strip; }  //!<!
    //void SetBackStrip(const UShort_t& strip)    { fBackStrip     = strip; }  //!<!

    void SetFront(const TFragment& frag); //!<!  
    void SetBack (const TFragment& frag); //!<!
    void SetPad  (const TFragment& frag); //!<!
      
    TVector3 GetPosition() const { return GetChannelPosition(); } //!<!

  private:
      TVector3 GetChannelPosition(Double_t dist = 0) const; //!<!
    
/// \cond CLASSIMP
  ClassDef(TSharcHit,6)
/// \endcond
};
/*! @} */
#endif
