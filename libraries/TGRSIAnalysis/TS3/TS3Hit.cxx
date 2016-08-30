#include "TS3Hit.h"
#include "TS3.h"

/// \cond CLASSIMP
ClassImp(TS3Hit)
	/// \endcond

	TS3Hit::TS3Hit()	{
		Clear();
	}
TS3Hit::TS3Hit(TFragment &frag)	: TGRSIDetectorHit(frag) {}

TS3Hit::~TS3Hit()	{}

TS3Hit::TS3Hit(const TS3Hit &rhs) : TGRSIDetectorHit() {
	Clear();
	((TS3Hit&)rhs).Copy(*this);
}


void TS3Hit::Copy(TObject &rhs) const {
  TGRSIDetectorHit::Copy(rhs);

  //static_cast<TS3Hit&>(rhs).fLed          = fLed;
  static_cast<TS3Hit&>(rhs).fRing         = fRing;
  static_cast<TS3Hit&>(rhs).fSector       = fSector;
  static_cast<TS3Hit&>(rhs).fIsDownstream = fIsDownstream;
  static_cast<TS3Hit&>(rhs).fTimeFit	  = fTimeFit;
  static_cast<TS3Hit&>(rhs).fSig2Noise	  = fSig2Noise;
  return;
}

void TS3Hit::Clear(Option_t *opt)	{
	TGRSIDetectorHit::Clear(opt);
	fRing           = -1;
	fSector         = -1;
	fIsDownstream		 = false;
}

Short_t TS3Hit::GetMnemonicSegment(TFragment &frag){//could be added to TGRSIDetectorHit base class
	TChannel *channel = TChannel::GetChannel(frag.GetAddress());
	if(!channel){
		Error("SetDetector","No TChannel exists for address %u",GetAddress());
		return 0;
	}
	return channel->GetMnemonic()->segment;
}

void TS3Hit::SetWavefit(TFragment &frag)   {
	TPulseAnalyzer pulse(frag);
	if(pulse.IsSet()){
		fTimeFit   = pulse.fit_newT0();
		fSig2Noise = pulse.get_sig2noise();
	}
}

TVector3 TS3Hit::GetPosition(double offset, double dist) const {
	return TS3::GetPosition(GetRing(),GetSector(),this->GetIsDownstream(),offset);
}

TVector3 TS3Hit::GetPosition(double offset) const {
	return TS3::GetPosition(GetRing(),GetSector(),this->GetIsDownstream(),GetDefaultOffset());
}

TVector3 TS3Hit::GetPosition() const {
	return TS3::GetPosition(GetRing(),GetSector(),this->GetIsDownstream(),GetDefaultOffset());
}

void TS3Hit::Print(Option_t *opt) const	{
	printf("================\n");
	printf("not yet written.\n");
	printf("================\n");
}
