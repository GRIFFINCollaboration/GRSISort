#include "TSiLi.h"
#include "TSiLiHit.h"

/// \cond CLASSIMP
ClassImp(TSiLiHit)
/// \endcond

TSiLiHit::TSiLiHit()  {    Clear(); }

TSiLiHit::TSiLiHit(TVirtualFragment &frag)	: TGRSIDetectorHit(frag) {
	SetVariables(frag);
}

TSiLiHit::~TSiLiHit()  {  }

TSiLiHit::TSiLiHit(const TSiLiHit &rhs) : TGRSIDetectorHit() {
   Clear();
   ((TSiLiHit&)rhs).Copy(*this);
}

void TSiLiHit::Copy(TObject &rhs) const {
   TGRSIDetectorHit::Copy(rhs);

	static_cast<TSiLiHit&>(rhs).fTimeFit = fTimeFit;
	static_cast<TSiLiHit&>(rhs).fSig2Noise = fSig2Noise;

   return;
}

void TSiLiHit::Clear(Option_t *opt)  {
   TGRSIDetectorHit::Clear(opt);
	fTimeFit   = -1;
	fSig2Noise = -1;
}

void TSiLiHit::SetWavefit(TVirtualFragment &frag)   { 
	TPulseAnalyzer pulse(frag,4);	    
	if(pulse.IsSet()){
		fTimeFit = pulse.fit_newT0();
		fSig2Noise = pulse.get_sig2noise();
	}
}

TVector3 TSiLiHit::GetChannelPosition(double dist) const {
	return TSiLi::GetPosition(GetSegment());
}

void TSiLiHit::Print(Option_t *opt) const {
	printf("===============\n");
	printf("not yet written\n");
	printf("===============\n");
}
