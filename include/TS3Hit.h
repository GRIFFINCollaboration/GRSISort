#ifndef TS3HIT_H
#define TS3HIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include "TVirtualFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h" 

class TS3Hit : public TGRSIDetectorHit {
public:
	TS3Hit();
	TS3Hit(TVirtualFragment &);
	virtual ~TS3Hit();
	TS3Hit(const TS3Hit&);
	
	Short_t  GetRing()  const  { return fRing;   }
	Short_t  GetSector() const { return fSector; }
	
public:
	void Copy(TObject&) const;        //!
	void Print(Option_t* opt="") const;
	void Clear(Option_t* opt="");
	
	void SetVariables(TVirtualFragment &frag) {}
	void SetRingNumber(Short_t rn)     { fRing = rn;   }
	void SetSectorNumber(Short_t sn)   { fSector = sn; }
   
	void SetRingNumber(TVirtualFragment &frag)     { fRing = GetMnemonicSegment(frag);   }
	void SetSectorNumber(TVirtualFragment &frag)   { fSector =GetMnemonicSegment(frag) ; }
   
	Short_t GetMnemonicSegment(TVirtualFragment &frag);//could be added to TGRSIDetectorHit base class
	
private:
	TVector3 GetChannelPosition(Double_t dist = 0) const; //!
	
	Short_t  fRing;   //front
	Short_t  fSector; //back
	
/// \cond CLASSIMP
  ClassDef(TS3Hit,4);
/// \endcond
};
/*! @} */
#endif
