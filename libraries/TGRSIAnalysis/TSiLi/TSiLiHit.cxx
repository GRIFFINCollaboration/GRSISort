#include "TSiLi.h"
#include "TSiLiHit.h"

/// \cond CLASSIMP
ClassImp(TSiLiHit)
/// \endcond

TSiLiHit::TSiLiHit()  {    Clear(); }

TSiLiHit::TSiLiHit(TFragment &frag)	: TGRSIDetectorHit(frag) {
  
  if(TGRSIRunInfo::IsWaveformFitting())
	  SetWavefit(frag);
    
}

TSiLiHit::~TSiLiHit()  {  }

TSiLiHit::TSiLiHit(const TSiLiHit &rhs) : TGRSIDetectorHit() {
   Clear();
   ((TSiLiHit&)rhs).Copy(*this);
}

void TSiLiHit::Copy(TObject &rhs) const {
   TGRSIDetectorHit::Copy(rhs);

	//static_cast<TSiLiHit&>(rhs).fSegment = fSegment;
	static_cast<TSiLiHit&>(rhs).fTimeFit = fTimeFit;
	static_cast<TSiLiHit&>(rhs).fSig2Noise = fSig2Noise;

   return;
}


/*void TSiLiHit::SetSegment(TFragment &frag){
	TChannel *channel = TChannel::GetChannel(frag.ChannelAddress);
      if(!channel)  return;
      
      char seg[5]; 
      strncpy(seg,channel->GetChannelName()+7,3);
      fSegment=strtol(seg, NULL, 16);
}*/



void TSiLiHit::Clear(Option_t *opt)  {
   TGRSIDetectorHit::Clear(opt);
	//fSegment   = -1;
	fTimeFit   = -1;
	fSig2Noise = -1;
}

void TSiLiHit::SetWavefit(TFragment &frag)   { 
	TPulseAnalyzer pulse(frag,4);	    
	if(pulse.IsSet()){
		fTimeFit = pulse.fit_newT0();
		fSig2Noise = pulse.get_sig2noise();
	}
}

TVector3 TSiLiHit::GetPosition(double dist) const {
	return TSiLi::GetPosition(GetSegment());
}

TVector3 TSiLiHit::GetPosition() const {
	return GetPosition(GetDefaultDistance());
}

void TSiLiHit::Print(Option_t *opt) const {
	printf("===============\n");
	printf("not yet written\n");
	printf("===============\n");
}
