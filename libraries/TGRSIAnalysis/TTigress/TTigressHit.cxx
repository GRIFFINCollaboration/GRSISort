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

void TTigressHit::Clear(Option_t *opt) {
  TGRSIDetectorHit::Clear(opt);
  //detector = -1;
  fCrystal  = -1;
  fFirstSegment = 0;
  fFirstSegmentCharge = 0.0;
  fTimeFit = 0;

  fSegments.clear();
  fBgos.clear();

  fLastHit.SetXYZ(0,0,0);
}

void TTigressHit::Copy(TObject &rhs) const {
  TGRSIDetectorHit::Copy(rhs);
  static_cast<TTigressHit&>(rhs).fTimeFit              = fTimeFit;
  static_cast<TTigressHit&>(rhs).fSegments             = fSegments;
  static_cast<TTigressHit&>(rhs).fBgos                 = fBgos;
  static_cast<TTigressHit&>(rhs).fCrystal              = fCrystal;
  static_cast<TTigressHit&>(rhs).fFirstSegment         = fFirstSegment;
  static_cast<TTigressHit&>(rhs).fFirstSegmentCharge   = fFirstSegmentCharge;
  fLastHit.Copy(static_cast<TTigressHit&>(rhs).fLastHit);
}


void TTigressHit::Print(Option_t *opt) const	{
  TString sopt(opt);
  printf("==== TigressHit @ 0x%p\n ====",(void*)this);
  printf("\t%s\n",GetName());
  printf("\tCharge: %.2f\n",GetCharge());
  printf("\tEnergy: %.2f\n",GetEnergy());
  printf("\tTime:   %.2f\n",GetTime());
  std::cout <<"\tTime:   "<< GetTimeStamp() <<"\n";
  printf("\thit contains %i segments.\n",GetNSegments());
  printf("\tintial segment: %i\n",GetInitialHit());
  if(sopt.Contains("all")) {
     printf("Name           Charge\n");
    for(int x=0;x<GetNSegments();x++) {
      printf("\t\t%s  |   %.2f\n",GetSegment(x).GetName(),GetSegment(x).GetCharge());
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


void TTigressHit::CheckFirstHit(int charge,int segment) {
  if(std::fabs(charge) > fFirstSegmentCharge) {
    fFirstSegment = segment;
  }
  return;				
}

void TTigressHit::SumHit(TTigressHit *hit) {
  if(this == hit) {
    fLastPos = std::make_tuple(GetDetector(),GetCrystal(),GetInitialHit());
    return;
  }
  this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
  this->fLastHit = hit->GetPosition();
  this->fLastPos = std::make_tuple(hit->GetDetector(),hit->GetCrystal(),hit->GetInitialHit());
}


TVector3 TTigressHit::GetChannelPosition(Double_t dist) const {
  //Returns the Position of the crystal of the current Hit.
  return TTigress::GetPosition(GetDetector(),GetCrystal(),GetInitialHit(),dist);
}

int TTigressHit::GetCrystal() const {
  //if(IsCrystalSet())
  //  return fCrystal;

  TChannel *chan = GetChannel();
  if(!chan)
    return -1;
  return chan->GetCrystalNumber();
  //MNEMONIC mnemonic;
  //ParseMNEMONIC(chan->GetChannelName(),&mnemonic);
  //char color = mnemonic.arraysubposition[0];
  //switch(color) {
  //  case 'B':
  //    return 0;
  //  case 'G':
  //    return 1;
  //  case 'R':
  //    return 2;
  //  case 'W':
  //    return 3;  
  //};
  //return -1;  
}
/*
int TTigressHit::GetCrystal() {
  if(IsCrystalSet())
    return fCrystal;

  TChannel *chan = GetChannel();
  if(!chan)
    return -1;
  MNEMONIC mnemonic;
  ParseMNEMONIC(chan->GetChannelName(),&mnemonic);
  char color = mnemonic.arraysubposition[0];
  return SetCrystal(color);  
}
*/
/*
int TTigressHit::SetCrystal(int crynum) {
   fCrystal = crynum;
   return fCrystal;
}
*/
/*
int TTigressHit::SetCrystal(char color) { 
   switch(color) {
      case 'B':
         fCrystal = 0;
         break;
      case 'G':
         fCrystal = 1;
         break;
      case 'R':
         fCrystal = 2;
         break;
      case 'W':
         fCrystal = 3;  
         break;
   };
   SetFlag(TGRSIDetectorHit::kIsSubDetSet,true);
   return fCrystal;
}
*/

void TTigressHit::SetWavefit(TFragment &frag)   { 
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
