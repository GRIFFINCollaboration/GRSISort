


#include "TRF.h"


ClassImp(TRF)


TRF::TRF(){
	Clear();
}

void TRF::Copy(TObject &rhs) const {

  TDetector::Copy(rhs);
  static_cast<TRF&>(rhs).midastime     = midastime;
  static_cast<TRF&>(rhs).timestamp     = timestamp;
  static_cast<TRF&>(rhs).timesfu    = timesfu;
  return;                                      
}  

TRF::TRF(const TRF& rhs) : TDetector() {
  rhs.Copy(*this);
}

TRF::~TRF() {  }

void TRF::FillData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
	
	TPulseAnalyzer pulse((TFragment&)(*frag));	    
	if(pulse.IsSet()){
		timesfu = pulse.fit_rf(period_ns*0.2);//period givin in half ticks... for reasons
		midastime = frag->MidasTimeStamp;
		timestamp = frag->GetTimeStamp();		
	}
}


void TRF::BuildHits(TDetectorData *ddata,Option_t *opt)	{
	
}

void TRF::Clear(Option_t *opt)	{
   midastime =  0.0;
   timestamp =  0.0;
   timesfu   =  0.0;
}

void TRF::Print(Option_t *opt) const { } 





