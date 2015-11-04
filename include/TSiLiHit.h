#ifndef TSILIHIT_H
#define TSILIHIT_H

#include <cstdio>
#include <utility>

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h"
#include "TPulseAnalyzer.h"

class TSiLiHit : public TGRSIDetectorHit {
	public:
		TSiLiHit();
		TSiLiHit(TFragment &);	
		virtual ~TSiLiHit();
		TSiLiHit(const TSiLiHit&);
		
		void Copy(TObject&) const;        //!
		void Clear(Option_t *opt="");
		void Print(Option_t *opt="") const;

		Double_t GetLed()      const { return fLed;      }
		Short_t  GetSegment()  const { return fSegment;  }
		Double_t GetSig2Noise()const { return fSig2Noise;}    
		Int_t GetRing()        const {  return 9-(fSegment/12); }
		Int_t GetSector()      const {  return fSegment%12; }
		Int_t GetPreamp()      const {  return  ((GetSector()/3)*2)+(((GetSector()%3)+GetRing())%2); }
		Double_t GetTimeFit()   { return fTimeFit;  }

		void SetSegment(Short_t seg)       { fSegment = seg;	}
		void SetSegment(TFragment &frag);
		void SetVariables(TFragment &frag) { fLed    = frag.GetLed();
							SetSegment(frag); }
		void SetWavefit(TFragment&);

	private:
		TVector3 GetChannelPosition(Double_t dist = 0) const; //!  
      
		Double_t    fLed;
		Short_t  fSegment;
		Double_t    fTimeFit;
		Double_t    fSig2Noise;

		ClassDef(TSiLiHit,8);
};
#endif
