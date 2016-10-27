#ifndef TSILIHIT_H
#define TSILIHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <utility>

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h"
#include "TPulseAnalyzer.h"

class TSiLiHit : public TGRSIDetectorHit {
	public:
		TSiLiHit();
		TSiLiHit(const TFragment &);	
		virtual ~TSiLiHit();
		TSiLiHit(const TSiLiHit&);
		
		void Copy(TObject&) const;        //!
		void Clear(Option_t *opt="");
		void Print(Option_t *opt="") const;

		Double_t GetSig2Noise()const { return fSig2Noise;}    
		Int_t GetRing()        const {  return 9-(GetSegment()/12); }
		Int_t GetSector()      const {  return GetSegment()%12; }
		Int_t GetPreamp()      const {  return  ((GetSector()/3)*2)+(((GetSector()%3)+GetRing())%2); }
		Double_t GetTimeFit()   { return fTimeFit;  }

		void SetWavefit(const TFragment&);
		TVector3 GetPosition(Double_t dist) const; //!  
		TVector3 GetPosition() const; //!  

	private:
      Double_t GetDefaultDistance() const { return 0.0; }
      
		Double_t    fTimeFit;
		Double_t    fSig2Noise;

/// \cond CLASSIMP
		ClassDef(TSiLiHit,7);
/// \endcond
};
/*! @} */
#endif
