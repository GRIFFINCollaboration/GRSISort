#include "TTriFoil.h"

/// \cond CLASSIMP
ClassImp(TTriFoil)
/// \endcond

TTriFoil::TTriFoil()	{
	Clear();
}

TTriFoil::~TTriFoil() {
}

void TTriFoil::Clear(Option_t *opt)	{
	//Clears all of the hits and data
   TDetector::Clear(opt);
   fTfWave.clear();
   fTimestamp =0;
   fBeam = false;
   fTBeam = 0;
}

void TTriFoil::Copy(TObject &rhs) const {
   TDetector::Copy(rhs);
   static_cast<TTriFoil&>(rhs).fTfWave    = fTfWave;
   static_cast<TTriFoil&>(rhs).fTimestamp = fTimestamp;
   static_cast<TTriFoil&>(rhs).fBeam      = fBeam;
   static_cast<TTriFoil&>(rhs).fTBeam     = fTBeam;
	return;                                      
}                                       

TTriFoil::TTriFoil(const TTriFoil& rhs) : TDetector() {
  Class()->IgnoreTObjectStreamer(kTRUE);
  rhs.Copy(*this);
}

void TTriFoil::AddFragment(TFragment* frag, MNEMONIC* mnemonic) {
	if(frag == NULL || mnemonic == NULL) {
		return;
	}
	
	if(frag->GetWavebufferSize() != 0) {
		fTfWave = frag->GetWavebuffer();
	}
	//fTimestamp = frag->GetTimeStamp();
	int max = 0;
	int imax =0;
	for(size_t i=0;i<fTfWave.size();i++){
		if(fTfWave[i]>max){
			max = fTfWave[i];
			imax = i;
		}
	}
	if(max>2000) {
		fBeam = true;
		fTBeam = imax;
	}
}

void TTriFoil::Print(Option_t *opt) const { } 





