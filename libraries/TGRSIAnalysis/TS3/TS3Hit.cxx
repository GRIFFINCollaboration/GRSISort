#include "TS3Hit.h"
#include "TS3.h"

/// \cond CLASSIMP
ClassImp(TS3Hit)
/// \endcond

TS3Hit::TS3Hit()	{
	Clear();
}
TS3Hit::TS3Hit(TVirtualFragment &frag)	: TGRSIDetectorHit(frag) {}

TS3Hit::~TS3Hit()	{}

TS3Hit::TS3Hit(const TS3Hit &rhs) : TGRSIDetectorHit() {
   Clear();
   ((TS3Hit&)rhs).Copy(*this);
}


void TS3Hit::Copy(TObject &rhs) const {
   TGRSIDetectorHit::Copy(rhs);

	static_cast<TS3Hit&>(rhs).fRing = fRing;
	static_cast<TS3Hit&>(rhs).fSector = fSector;
   return;
}


void TS3Hit::Clear(Option_t *opt)	{
   TGRSIDetectorHit::Clear(opt);
   fRing           = -1;
   fSector         = -1;
}

Short_t TS3Hit::GetMnemonicSegment(TVirtualFragment &frag){//could be added to TGRSIDetectorHit base class
	MNEMONIC mnemonic;
	TChannel *channel = TChannel::GetChannel(frag.GetChannelAddress());
	if(!channel){
		Error("SetDetector","No TChannel exists for address %u",frag.GetChannelAddress());
		return 0;
	}
	ClearMNEMONIC(&mnemonic);
	ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
	return mnemonic.segment;
}


TVector3 TS3Hit::GetChannelPosition(double dist) const {
	return TS3::GetPosition(GetRing(),GetSector());
}

void TS3Hit::Print(Option_t *opt) const	{
	printf("================\n");
	printf("not yet written.\n");
	printf("================\n");
}
