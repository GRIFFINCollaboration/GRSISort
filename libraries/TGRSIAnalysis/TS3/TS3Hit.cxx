#include "TS3Hit.h"
#include "TS3.h"

/// \cond CLASSIMP
ClassImp(TS3Hit)
/// \endcond

TS3Hit::TS3Hit()
{
   Clear();
}

TS3Hit::TS3Hit(const TFragment& frag) : TGRSIDetectorHit(frag)
{
	
   if(GetChannel()->GetMnemonic()->System() == TMnemonic::kSiLiS3){
	SetIsDownstream(false);   
   }else{
	if(GetArrayPosition()<2)
		SetIsDownstream(false);  
	else
		SetIsDownstream(true);  
   }
}


TS3Hit::~TS3Hit() = default;

TS3Hit::TS3Hit(const TS3Hit& rhs) : TGRSIDetectorHit(rhs)
{
   Clear();
   rhs.Copy(*this);
}

void TS3Hit::Copy(TObject& rhs) const
{
   TGRSIDetectorHit::Copy(rhs);

   // static_cast<TS3Hit&>(rhs).fLed          = fLed;
   static_cast<TS3Hit&>(rhs).fRing         = fRing;
   static_cast<TS3Hit&>(rhs).fSector       = fSector;
   static_cast<TS3Hit&>(rhs).fIsDownstream = fIsDownstream;
   static_cast<TS3Hit&>(rhs).fTimeFit      = fTimeFit;
   static_cast<TS3Hit&>(rhs).fSig2Noise    = fSig2Noise;
}

void TS3Hit::Clear(Option_t* opt)
{
   TGRSIDetectorHit::Clear(opt);
   fRing         = -1;
   fSector       = -1;
   fIsDownstream = false;
}

void TS3Hit::SetWavefit(const TFragment& frag)
{
   TPulseAnalyzer pulse(frag);
   if(pulse.IsSet()) {
      fTimeFit   = pulse.fit_newT0();
      fSig2Noise = pulse.get_sig2noise();
   }
}


Bool_t TS3Hit::SectorsDownstream() const
{
	if(GetChannel()->GetMnemonic()->System() == TMnemonic::kSiLiS3){
		if(GetArrayPosition() == 0) return true;
		return false;
	}
	if(GetArrayPosition()%2)return true;
	return false;
}


TVector3 TS3Hit::GetPosition(Double_t phioffset, Double_t dist, bool smear) const
{
   return TS3::GetPosition(GetRing(), GetSector(), phioffset, dist, SectorsDownstream(), smear);
}

TVector3 TS3Hit::GetPosition(Double_t phioffset, bool smear) const
{
   return TS3::GetPosition(GetRing(), GetSector(), phioffset, GetDefaultDistance(), SectorsDownstream(), smear);
}

TVector3 TS3Hit::GetPosition(bool smear) const
{
   return TS3::GetPosition(GetRing(), GetSector(), GetDefaultPhiOffset(), GetDefaultDistance(), SectorsDownstream(),smear);
}

void TS3Hit::Print(Option_t*) const
{
   printf("================\n");
   printf("not yet written.\n");
   printf("================\n");
}

Double_t TS3Hit::GetDefaultPhiOffset() const
{
   double deg = -90;
   if(GetChannel()->GetMnemonic()->System() == TMnemonic::kSiLiS3) {
      deg = -22.5;
      if(GetChannel()->GetMnemonic()->ArrayPosition() == 2) {
         deg += 90;
      }
   }
   return deg * TMath::Pi() / 180.;
}

Double_t TS3Hit::GetDefaultDistance() const
{ // relative to target (SPICE target not at Z=0)
   double z = 0;
   std::string str = GetDistanceStr();
   if(GetChannel()->GetMnemonic()->System() == TMnemonic::kSiLiS3) {
      if(str.find('D') < str.size()) {
         z = 22.5;
      } else if(str.find('E') < str.size()) {
         z = 28.35;
      } else {
         z = 42.1;
      }
   } else {
      if(str.find('D') < str.size()) {
         z = 20;
      } else if(str.find('E') < str.size()) {
         z = 32;
      } else {
         z = 40;
      }

      if(GetArrayPosition()<2) {
         z = -z;
      }
   }

   return z;
}
