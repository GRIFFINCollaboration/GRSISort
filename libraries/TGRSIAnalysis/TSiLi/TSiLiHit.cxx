#include "TSiLi.h"
#include "TSiLiHit.h"

/// \cond CLASSIMP
ClassImp(TSiLiHit)
/// \endcond

TSiLiHit::TSiLiHit()  {    Clear(); }

TSiLiHit::TSiLiHit(TFragment &frag)	: TGRSIDetectorHit(frag) {
  
fFitCharge=frag.GetCharge();
//   if(TGRSIRunInfo::IsWaveformFitting()) // commented out as TGRSIRunInfo seems to be broken
	  SetWavefit(frag);
    
}

TSiLiHit::~TSiLiHit()  {  }

TSiLiHit::TSiLiHit(const TSiLiHit &rhs) : TGRSIDetectorHit() {
   Clear();
   ((TSiLiHit&)rhs).Copy(*this);
}

void TSiLiHit::Copy(TObject &rhs,int suppress) const {
   TGRSIDetectorHit::Copy(rhs);

	static_cast<TSiLiHit&>(rhs).fTimeFit = fTimeFit;
	static_cast<TSiLiHit&>(rhs).fSig2Noise = fSig2Noise;
	static_cast<TSiLiHit&>(rhs).fFitCharge = fFitCharge;
	static_cast<TSiLiHit&>(rhs).fFitBase = fFitBase;
	if(suppress==0){
		static_cast<TSiLiHit&>(rhs).fAddBackSegments = fAddBackSegments;
		static_cast<TSiLiHit&>(rhs).fAddBackEnergy = fAddBackEnergy;
	}
	
   return;
}


void TSiLiHit::Clear(Option_t *opt)  {
   TGRSIDetectorHit::Clear(opt);
	//fSegment   = -1;
	fTimeFit   = -1;
	fFitCharge = -1;
	fFitBase   = -1;
	fSig2Noise = -1;
	
	fAddBackSegments.clear();
	fAddBackEnergy.clear();
}

void TSiLiHit::SetWavefit(TFragment &frag){ 
	TPulseAnalyzer pulse(frag,TSiLi::sili_noise_fac);	    
	if(pulse.IsSet()){
		//THESE VALUES SHOULD BE GOT FROM THE TCHANNEL AND INCLUDED IN THE CAL FOR EACH CHAN
		pulse.GetSiliShape(4616.18,20.90);
		

		fTimeFit = pulse.Get_wpar_T0();
		fFitBase = pulse.Get_wpar_baselinefin();
		fFitCharge= pulse.Get_wpar_amplitude();

		//printf("A0:\t%2.2f, B:\t%2.2f\n",pulse.Get_wpar_amplitude(),pulse.Get_wpar_baselinefin());

		fSig2Noise = pulse.get_sig2noise();
	}
}

TVector3 TSiLiHit::GetPosition(double dist) const {
	return TSiLi::GetPosition(GetRing(),GetSector());
}

TVector3 TSiLiHit::GetPosition() const {
	return GetPosition(GetDefaultDistance());
}

void TSiLiHit::Print(Option_t *opt) const {
	printf("===============\n");
	printf("not yet written\n");
	printf("===============\n");
}


void TSiLiHit::SumHit(TSiLiHit *hit) {
  if(this == hit) {
    return;
  }
  
  if(fAddBackSegments.size()==0){
	hit->Copy(*this,1);//suppresses waveform copying
	fAddBackSegments.clear();
	fAddBackEnergy.clear();
	this->SetEnergy(0);
	SetBit(kIsEnergySet,true);
  }
  
  this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
  fAddBackSegments.push_back(hit->GetSegment());
  fAddBackEnergy.push_back(hit->GetEnergy());
}


Int_t TSiLiHit::GetRing() const { return TSiLi::GetRing(GetSegment()); }
Int_t TSiLiHit::GetSector() const { return TSiLi::GetSector(GetSegment()); }
Int_t TSiLiHit::GetPreamp() const { return TSiLi::GetPreamp(GetSegment()); }


double TSiLiHit::GetWaveformEnergy() const{
	if(float(fFitCharge)==GetCharge()) {
		return GetEnergy();
	}	
	TChannel* chan = GetChannel();
	if(chan == NULL) {
		Error("GetEnergy","No TChannel exists for address 0x%08x",GetAddress());
		return 0.;
	}
	return chan->CalibrateENG(fFitCharge);
}
		
		

