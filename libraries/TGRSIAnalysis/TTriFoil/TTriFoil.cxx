#include "TTriFoil.h"

/// \cond CLASSIMP
ClassImp(TTriFoil)
/// \endcond

TTriFoil::TTriFoil()
{
	Clear();
}

TTriFoil::~TTriFoil() = default;

void TTriFoil::Clear(Option_t* opt)
{
	// Clears all of the hits and data
	TDetector::Clear(opt);
	fTfWave.clear();
	fTimestamp = 0;
	fBeam      = false;
	fTBeam.clear();
}

void TTriFoil::Copy(TObject& rhs) const
{
	TDetector::Copy(rhs);
	static_cast<TTriFoil&>(rhs).fTfWave    = fTfWave;
	static_cast<TTriFoil&>(rhs).fTimestamp = fTimestamp;
	static_cast<TTriFoil&>(rhs).fBeam      = fBeam;
	static_cast<TTriFoil&>(rhs).fTBeam     = fTBeam;
}

TTriFoil::TTriFoil(const TTriFoil& rhs) : TDetector()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TTriFoil::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	if(frag == nullptr || chan == nullptr) {
		return;
	}

	if(!(frag->GetWaveform()->empty())) {
		fTfWave = *(frag->GetWaveform());
	}
	fTimestamp = frag->GetTimeStamp();
	int max    = 0;
	int imax   = 0;
	/*for(size_t i=0;i<fTfWave.size();i++){
	  if(fTfWave[i]>max){
	  max = fTfWave[i];
	  imax = i;
	  }
	  }
	  if(max>1500) {
	  fBeam = true;
	  fTBeam = imax;
	  }*/

	fTBeam.clear();

	for(size_t i = 0; i < fTfWave.size(); i++) {
		if(fTfWave[i] > 1500 && fTfWave[i] > max) {
			max  = fTfWave[i];
			imax = i;
		}
		if(max != 0 && imax != 0 && (i - imax) > 15) {
			fTBeam.push_back(imax);
			max  = 0;
			imax = 0;
		}
	}
}

void TTriFoil::Print(Option_t*) const
{
}
