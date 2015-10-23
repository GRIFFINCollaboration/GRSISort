#include "TSiLi.h"
#include "TSiLiHit.h"

ClassImp(TSiLiHit)

TSiLiHit::TSiLiHit()  {  }

TSiLiHit::~TSiLiHit()  {  }


void TSiLiHit::Clear(Option_t *opt)  {
   TGRSIDetectorHit::Clear(opt);
  led     = -1;
  segment = -1;
  ring    = -1;
  sector  = -1;
  preamp  = -1; 
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


void TSiLiHit::Print(Option_t *opt) const {
  printf("===============\n");
  printf("not yet written\n");
  printf("===============\n");
}