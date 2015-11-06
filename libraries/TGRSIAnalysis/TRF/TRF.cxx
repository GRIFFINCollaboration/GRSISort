#include "TRF.h"

/// \cond CLASSIMP
ClassImp(TRF)
/// \endcond

TRF::TRF() : fFitter(NULL) {
	Clear();
}

TRF::~TRF() {
	if(fFitter != NULL) { 
		delete fFitter;
	}
}

void TRF::AddFragment(TFragment* frag, MNEMONIC* mnemonic) {
	if(fFitter == NULL)
		fFitter = new TRFFitter();
	fFitter->FindPhase((TFragment&)(*frag));
   if(!fFitter->IsSet()) 
      return;
   fPhase     = fFitter->GetPhase(); 
   fMidasTime = fFitter->GetMidasTime();
   fTimeStamp = fFitter->GetTimeStamp();
   fTime      = fFitter->GetTime();
}

void TRF::Clear(Option_t *opt) {
	if(fFitter)
		fFitter->Clear();
	
   fPhase     = -1.0; 
   fMidasTime =  0.0;
   fTimeStamp =  0.0;
   fTime      =  0.0;
}

void TRF::Print(Option_t *opt) const {
	printf("phase = %f\n",fPhase);
	printf("time = %f\n",fTime);
	printf("timestamp = %ld\n",fTimeStamp);
	printf("midastime = %ld\n",fMidasTime);
} 





