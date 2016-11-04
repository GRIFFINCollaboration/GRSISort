#include "TTigressHit.h"

#include "TClass.h"

#include "TTigress.h"

/// \cond CLASSIMP
ClassImp(TTigressHit)
/// \endcond

TTigressHit::TTigressHit() {	
  Clear();
}

TTigressHit::~TTigressHit() {	}

TTigressHit::TTigressHit(const TTigressHit& rhs) : TGRSIDetectorHit() {	
  rhs.Copy(*this);
}

TTigressHit::TTigressHit(const TFragment &frag) : TGRSIDetectorHit(frag) {

}

void TTigressHit::CopyFragment(const TFragment &frag) {
  //TGRSIDetectorHit::Copy((TGRSIDetectorHit&)frag,false);
  frag.Copy(*this);
}


void TTigressHit::Clear(Option_t *opt) {
  TGRSIDetectorHit::Clear(opt);
  //fFirstSegment = 0;
  //fFirstSegmentCharge = 0.0;
  fTimeFit   = 0.0;
  fSig2Noise = 0.0;

  fBgoFired = false;
  fSegments.clear();
  //fLastHit.SetXYZ(0,0,0);
}

void TTigressHit::Copy(TObject &rhs) const {
  TGRSIDetectorHit::Copy(rhs);
  static_cast<TTigressHit&>(rhs).fTimeFit              = fTimeFit;
  static_cast<TTigressHit&>(rhs).fSegments             = fSegments;
  static_cast<TTigressHit&>(rhs).fBgoFired             = fBgoFired;

  //static_cast<TTigressHit&>(rhs).fCrystal              = fCrystal;
  //static_cast<TTigressHit&>(rhs).fFirstSegment         = fFirstSegment;
  //static_cast<TTigressHit&>(rhs).fFirstSegmentCharge   = fFirstSegmentCharge;
  //fLastHit.Copy(static_cast<TTigressHit&>(rhs).fLastHit);
}

TVector3 TTigressHit::GetPosition(Double_t dist) const {
  return TTigress::GetPosition(*this,dist);  
}

TVector3 TTigressHit::GetPosition() const {
  return GetPosition(GetDefaultDistance());  
}

TVector3 TTigressHit::GetLastPosition(Double_t dist) const {
  const TGRSIDetectorHit &seg = GetSegmentHit(GetNSegments()-1);
  return TTigress::GetPosition(seg.GetDetector(),seg.GetCrystal(),seg.GetSegment(),dist);  
}

TVector3 TTigressHit::GetLastPosition() const {
  return GetPosition(GetDefaultDistance());  
}

void TTigressHit::Print(Option_t *opt) const	{
  TString sopt(opt);
  printf("==== TigressHit @ 0x%p\n ====",(void*)this);
  printf("\t%s\n",GetName());
  printf("\tCharge: %i\n",GetCharge());
  printf("\tEnergy: %.2f\n",GetEnergy());
  printf("\tTime:   %.2f\n",GetTime());
  printf("\tBGO Fired: %s\n",BGOFired() ? "true" : "false");
  std::cout <<"\tTime:   "<< GetTimeStamp() <<"\n";
  printf("\thit contains %i segments.\n",GetNSegments());
  //printf("\tintial segment: %i\n",GetInitialHit());
  if(sopt.Contains("all")) {
     printf("Name           Charge\n");
    for(int x=0;x<GetNSegments();x++) {
      printf("\t\t%s  |   %i\n",GetSegmentHit(x).GetName(),GetSegmentHit(x).GetCharge());
    }
    GetPosition().Print();
  }
  printf("============================\n");
}


bool TTigressHit::Compare(TTigressHit lhs, TTigressHit rhs) {
  if (lhs.GetDetector() == rhs.GetDetector()) {
    return(lhs.GetCrystal() < rhs.GetCrystal());
  } else {
    return (lhs.GetDetector() < rhs.GetDetector()); 
  }
}


bool TTigressHit::CompareEnergy(TTigressHit lhs, TTigressHit rhs) {
  return(lhs.GetEnergy()) > rhs.GetEnergy();
}


//void TTigressHit::CheckFirstHit(int charge,int segment) {
//  if(std::fabs(charge) > fFirstSegmentCharge) {
//    fFirstSegment = segment;
//  }
//  return;				
//}

void TTigressHit::SumHit(TTigressHit *hit) {
  //if(this == hit) {
  //  fLastPos = std::make_tuple(GetDetector(),GetCrystal(),GetInitialHit());
  //  return;
  //}
  if(this!=hit)
    this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
  for(int x =0;x<hit->GetNSegments();x++) {
    this->AddSegment((hit->fSegments[x]));
  }
  if(hit->BGOFired()) {
    SetBGOFired(true);
  }
  //this->fLastHit = hit->GetPosition();
  //this->fLastPos = std::make_tuple(hit->GetDetector(),hit->GetCrystal(),hit->GetInitialHit());
}


//int TTigressHit::GetCrystal() const {
//  TChannel *chan = GetChannel();
//  if(!chan)
//    return -1;
//  return chan->GetCrystalNumber();
//}

void TTigressHit::SetWavefit(const TFragment &frag)   { 
  TPulseAnalyzer pulse(frag);	    
  if(pulse.IsSet()){
    fTimeFit   = pulse.fit_newT0();
    fSig2Noise = pulse.get_sig2noise();
  }
}

void TTigressHit::SetWavefit()   { 
  TPulseAnalyzer pulse(*GetWaveform(),0,GetName());	    
  if(pulse.IsSet()){
    fTimeFit   = pulse.fit_newT0();
    fSig2Noise = pulse.get_sig2noise();
  }
}
