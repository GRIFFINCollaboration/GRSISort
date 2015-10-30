#include "TSiLi.h"
#include "TSiLiHit.h"

ClassImp(TSiLiHit)

TSiLiHit::TSiLiHit()  {
   Clear();
}

TSiLiHit::TSiLiHit(TFragment &frag)	: TGRSIDetectorHit(frag) {}

TSiLiHit::~TSiLiHit()  {  }

TSiLiHit::TSiLiHit(const TSiLiHit &rhs) : TGRSIDetectorHit() {
   Clear();
   ((TSiLiHit&)rhs).Copy(*this);
}

void TSiLiHit::Copy(TObject &rhs) const {
   TGRSIDetectorHit::Copy(rhs);

	static_cast<TSiLiHit&>(rhs).led = led;
	static_cast<TSiLiHit&>(rhs).segment = segment;
	static_cast<TSiLiHit&>(rhs).time_fit = time_fit;
	static_cast<TSiLiHit&>(rhs).sig2noise = sig2noise;

   return;
}


void TSiLiHit::Clear(Option_t *opt)  {
   TGRSIDetectorHit::Clear(opt);
  led     = -1;
  segment = -1;
  time_fit =-1;
  sig2noise=-1;
}

void TSiLiHit::SetWavefit(TFragment &frag)   { 
	TPulseAnalyzer pulse(frag,4);	    
	if(pulse.IsSet()){
		time_fit = pulse.fit_newT0();
		sig2noise= pulse.get_sig2noise();
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