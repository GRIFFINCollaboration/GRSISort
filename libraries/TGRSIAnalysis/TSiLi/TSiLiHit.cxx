#include "TSiLi.h"
#include "TSiLiHit.h"

/// \cond CLASSIMP
ClassImp(TSiLiHit)
/// \endcond

TSiLiHit::TSiLiHit()  {  }

TSiLiHit::~TSiLiHit()  {  }


void TSiLiHit::Clear(Option_t *opt)  {
   TGRSIDetectorHit::Clear(opt);
	fLed       = -1;
	fSegment   = -1;
	fRing      = -1;
	fSector    = -1;
	fPreamp    = -1; 
	fTimeFit   = -1;
	fSig2Noise = -1;
}

void TSiLiHit::SetWavefit(TFragment &frag)   { 
	TPulseAnalyzer pulse(frag,4);	    
	if(pulse.IsSet()){
		fTimeFit = pulse.fit_newT0();
		fSig2Noise = pulse.get_sig2noise();
	}
}


void TSiLiHit::Print(Option_t *opt) const {
	printf("===============\n");
	printf("not yet written\n");
	printf("===============\n");
}
