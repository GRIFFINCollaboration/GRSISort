#include "TSiLi.h"
#include "TSiLiHit.h"

/// \cond CLASSIMP
ClassImp(TSiLiHit)
/// \endcond

TSiLiHit::TSiLiHit()
{
   Clear();
}

TSiLiHit::TSiLiHit(const TFragment& frag) : TGRSIDetectorHit(frag)
{

   fFitCharge = frag.GetCharge();
   //   if(TGRSIRunInfo::IsWaveformFitting()) // commented out as TGRSIRunInfo seems to be broken
   SetWavefit(frag);
}

TSiLiHit::~TSiLiHit() = default;

TSiLiHit::TSiLiHit(const TSiLiHit& rhs) : TGRSIDetectorHit(rhs)
{
   Clear();
   (const_cast<TSiLiHit&>(rhs)).Copy(*this);
}

void TSiLiHit::Copy(TObject& rhs, bool suppress) const
{
   TGRSIDetectorHit::Copy(rhs);

   static_cast<TSiLiHit&>(rhs).fTimeFit     = fTimeFit;
   static_cast<TSiLiHit&>(rhs).fSig2Noise   = fSig2Noise;
   static_cast<TSiLiHit&>(rhs).fSmirnov     = fSmirnov;
   static_cast<TSiLiHit&>(rhs).fFitCharge   = fFitCharge;
   static_cast<TSiLiHit&>(rhs).fFitBase     = fFitBase;
   static_cast<TSiLiHit&>(rhs).fSiLiHitBits = 0;
   if(!suppress) {
      static_cast<TSiLiHit&>(rhs).fAddBackSegments = fAddBackSegments;
      static_cast<TSiLiHit&>(rhs).fAddBackEnergy   = fAddBackEnergy;
   }

   return;
}

void TSiLiHit::Clear(Option_t* opt)
{
   TGRSIDetectorHit::Clear(opt);
   // fSegment   = -1;
   fTimeFit   = -1;
   fFitCharge = -1;
   fFitBase   = -1;
   fSig2Noise = -1;
   fSmirnov   = -1;

   fAddBackSegments.clear();
   fAddBackEnergy.clear();
   fSiLiHitBits.Clear();
   ClearTransients();
}

void TSiLiHit::SetWavefit(const TFragment& frag)
{
   TPulseAnalyzer* pulse = FitFrag(frag, TSiLi::FitSiLiShape, GetChannel());
   if(pulse != nullptr) {
      fTimeFit   = pulse->Get_wpar_T0();
      fFitBase   = pulse->Get_wpar_baselinefin();
      fFitCharge = pulse->Get_wpar_amplitude();
      fSig2Noise = pulse->get_sig2noise();
      fSmirnov   = pulse->GetsiliSmirnov();
      delete pulse;
   }
}

// Broken up for external analysis script use
TPulseAnalyzer* TSiLiHit::FitFrag(const TFragment& frag, int ShapeFit, int segment)
{
   return FitFrag(frag, ShapeFit, GetSiLiHitChannel(segment));
}

TPulseAnalyzer* TSiLiHit::FitFrag(const TFragment& frag, int ShapeFit, TChannel* channel)
{
   auto* pulse = new TPulseAnalyzer(frag, TSiLi::sili_noise_fac);
   if(FitPulseAnalyzer(pulse, ShapeFit, channel) != 0) {
      return pulse;
   }
   delete pulse;
   return nullptr;
}

TChannel* TSiLiHit::GetSiLiHitChannel(int segment)
{
	std::stringstream ss;
	ss<<"SPI00XN"<< std::uppercase << std::uppercase << std::setfill('0') << std::setw(2)<< std::hex << segment;
//	std::cout<<std::endl<<ss.str().c_str();
	return TChannel::FindChannelByName(ss.str().c_str());
}

int TSiLiHit::FitPulseAnalyzer(TPulseAnalyzer* pulse, int ShapeFit, int segment)
{
   return FitPulseAnalyzer(pulse, ShapeFit, GetSiLiHitChannel(segment));
}

int TSiLiHit::FitPulseAnalyzer(TPulseAnalyzer* pulse, int ShapeFit, TChannel* channel)
{
  	if(!pulse)return 0;
	if(pulse->IsSet()){
		double Decay=0,Rise=0,Base=0;
		
		if(channel){
			if(channel->UseWaveParam()){
				Rise=channel->GetWaveRise();
				Decay=channel->GetWaveDecay();
				Base=channel->GetWaveBaseLine();
			}
		}
// 		std::cout<<std::endl<<Decay<<" "<<Rise<<" "<<Base;

		if(!Decay)Decay=TSiLi::sili_default_decay;
		if(!Rise)Rise=TSiLi::sili_default_rise;
		if(!Base)Base=TSiLi::sili_default_baseline;
		
		bool goodfit=false;
		if(ShapeFit<2)goodfit=pulse->GetSiliShape(Decay,Rise);
		if(ShapeFit==1&&!goodfit)ShapeFit++;//So currently it does a TF1 fit if initial fit fails, this might be a bad idea
		if(ShapeFit==2)goodfit=pulse->GetSiliShapeTF1(Decay,Rise,Base);
		if(ShapeFit==3)goodfit=pulse->GetSiliShapeTF1(Decay,Rise,Base,TSiLi::BaseFreq);
		if(goodfit)return 1+ShapeFit;
	}
	return 0;
}

TVector3 TSiLiHit::GetPosition(Double_t, bool smear) const
{
   return TSiLi::GetPosition(GetRing(), GetSector(), smear);
}

TVector3 TSiLiHit::GetPosition(bool smear) const
{
   return GetPosition(GetDefaultDistance(), smear);
}

void TSiLiHit::Print(Option_t*) const
{
   printf("===============\n");
   printf("not yet written\n");
   printf("===============\n");
}

void TSiLiHit::SumHit(TSiLiHit* hit)
{
   if(this == hit) {
      return;
   }

   if(fAddBackSegments.empty()) {
      hit->Copy(*this, true); // suppresses copying of addback
      fAddBackSegments.clear();
      fAddBackEnergy.clear();
      SetEnergy(0);
      SetHitBit(EBitFlag::kIsEnergySet, true);
   }

   SetEnergy(GetEnergy() + hit->GetEnergy());
   fAddBackSegments.push_back(hit->GetSegment());
   fAddBackEnergy.push_back(hit->GetEnergy());
}

Int_t TSiLiHit::GetRing() const
{
   return TSiLi::GetRing(GetSegment());
}
Int_t TSiLiHit::GetSector() const
{
   return TSiLi::GetSector(GetSegment());
}
Int_t TSiLiHit::GetPreamp() const
{
   return TSiLi::GetPreamp(GetSegment());
}
bool TSiLiHit::MagnetShadow() const
{
   return TSiLi::MagnetShadow(GetSegment());
}


double TSiLiHit::GetFitEnergy() const
{
   if(fSiLiHitBits.TestBit(ESiLiHitBits::kUseFitCharge)) {
      return TGRSIDetectorHit::GetEnergy();
   }
   TChannel* chan = GetChannel();
   if(chan == nullptr) {
      return fFitCharge;
   }
   return chan->CalibrateENG(fFitCharge, 0);
}

double TSiLiHit::GetEnergy(Option_t*) const
{
   if(TestHitBit(EBitFlag::kIsEnergySet) || !fSiLiHitBits.TestBit(ESiLiHitBits::kUseFitCharge)) {
      return TGRSIDetectorHit::GetEnergy(); // If not fitting waveforms, be normal.
   }
   TChannel* chan = GetChannel();
   if(chan == nullptr) {
      return SetEnergy(fFitCharge);
   }

   return SetEnergy(chan->CalibrateENG(fFitCharge, 0)); // this will use the integration value
} // in the TChannel if it exists.
