#ifndef TS3HIT_H
#define TS3HIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include "TFragment.h"
#include "TChannel.h"
#include "TMath.h"
#include "TGRSIDetectorHit.h" 

class TS3Hit : public TGRSIDetectorHit {
   public:
    TS3Hit();
    TS3Hit(TFragment &);
    virtual ~TS3Hit();
    TS3Hit(const TS3Hit&);

    Double_t GetLed()   const  { return fLed;    }
    Short_t  GetRing()  const  { return fRing;   }
    Short_t  GetSector() const { return fSector; }
		Bool_t	 GetIsDownstream() const { return fIsDownstream; }

  public:
    void Copy(TObject&) const;        //!
    void Print(Option_t* opt="") const;
    void Clear(Option_t* opt="");

    void SetVariables(TFragment &frag) {  fLed    = frag.GetLed(); }
    void SetRingNumber(Short_t rn)     { fRing = rn;   }
    void SetSectorNumber(Short_t sn)   { fSector = sn; }
		void SetIsDownstream(Bool_t dwnstrm) 	{ fIsDownstream = dwnstrm; }
    
    void SetRingNumber(TFragment &frag)     { fRing = GetMnemonicSegment(frag);   }
    void SetSectorNumber(TFragment &frag)   { fSector =GetMnemonicSegment(frag) ; }
    
    Short_t GetMnemonicSegment(TFragment &frag);//could be added to TGRSIDetectorHit base class
	 
		Double_t GetTheta(double offset=0, TVector3 *vec=0) {
			if(vec==0) {
				vec = new TVector3();
				vec->SetXYZ(0,0,1);
			}
			return TMath::Pi() - TMath::Cos(this->GetChannelPosition(offset).Angle(*vec));
		}

  private:
     TVector3 GetChannelPosition(Double_t offset = 0, Double_t dist = 0) const; //!

      
    Bool_t 	 fIsDownstream; // Downstream check
    Double_t fLed;
    Short_t  fRing;   //front
    Short_t  fSector; //back

/// \cond CLASSIMP
  ClassDef(TS3Hit,4);
/// \endcond
};
/*! @} */
#endif
