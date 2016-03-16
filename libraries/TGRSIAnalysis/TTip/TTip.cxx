#include "TTip.h"

#include <iostream>

#include "TRandom.h"
#include "TMath.h"
#include "TClass.h"

#include "TGRSIRunInfo.h"

/// \cond CLASSIMP
ClassImp(TTip)
/// \endcond

TTip::TTip() {   
}

TTip::~TTip() {
   //Default Destructor
}

TTip::TTip(const TTip& rhs) : TGRSIDetector() {
  Class()->IgnoreTObjectStreamer(kTRUE);
  rhs.Copy(*this);
}

void TTip::Copy(TObject &rhs) const {
  TGRSIDetector::Copy(rhs);
  static_cast<TTip&>(rhs).fTipHits             = fTipHits;
}                                       

void TTip::Clear(Option_t *opt) {
  ///Clears all of the hits
   fTipHits.clear();
}

TTip& TTip::operator=(const TTip& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TTip::AddFragment(TVirtualFragment* frag, MNEMONIC* mnemonic) {
	if(frag == NULL || mnemonic == NULL) {
		return;
	}

  TTipHit dethit(*frag);
  TChannel chan = TChannel::GetChannel(dethit.GetAddress());
  dethit.SetUpNumbering(chan);

  if(TGRSIRunInfo::IsWaveformFitting() && !dethit.IsCsI())
		dethit.SetWavefit(*frag);
  else if(TGRSIRunInfo::IsWaveformFitting() && dethit.IsCsI()) 	   
		dethit.SetPID(*frag);

  fTipHits.push_back(dethit);
}

void TTip::Print(Option_t *opt) const {
  ///Prints out TTip members, currently only prints the multiplicity.
  printf("%lu fTipHits\n",fTipHits.size());
}

TGRSIDetectorHit* TTip::GetHit(const Int_t& idx) {
   return GetTipHit(idx);
}

TTipHit* TTip::GetTipHit(const int& i) {
   try {
      return &fTipHits.at(i);   
   } catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return 0;
}

void TTip::PushBackHit(TGRSIDetectorHit *tiphit) {
  fTipHits.push_back(*(static_cast<TTipHit*>(tiphit)));
  return;
}
