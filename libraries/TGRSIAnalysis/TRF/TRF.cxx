#include "TRF.h"

/// \cond CLASSIMP
ClassImp(TRF)
/// \endcond

Double_t    TRF::fPeriod;

TRF::TRF() {
	Clear();
}

void TRF::Copy(TObject &rhs) const {
  TDetector::Copy(rhs);
  static_cast<TRF&>(rhs).fMidasTime     = fMidasTime;
  static_cast<TRF&>(rhs).fTimeStamp     = fTimeStamp;
  static_cast<TRF&>(rhs).fTime    = fTime;
  return;                                      
}  

TRF::TRF(const TRF& rhs) : TDetector() {
  rhs.Copy(*this);
}

TRF::~TRF() {
}

void TRF::AddFragment(std::shared_ptr<const TFragment> frag, TChannel* chan) {
	TPulseAnalyzer pulse(*frag);	    
	if(pulse.IsSet()){
		fTime = pulse.fit_rf(fPeriod*0.2);//period taken in half ticks... for reasons
		fMidasTime = frag->GetMidasTimeStamp();
		fTimeStamp = frag->GetTimeStamp();		
	}
}

void TRF::Clear(Option_t *opt) {
	fMidasTime =  0.0;
	fTimeStamp =  0.0;
	fTime   =  0.0;
	
	fPeriod = 84.409;
}

void TRF::Print(Option_t *opt) const {
	printf("time = %f\n",fTime);
	printf("timestamp = %ld\n",fTimeStamp);
	printf("midastime = %ld\n",fMidasTime);
} 

