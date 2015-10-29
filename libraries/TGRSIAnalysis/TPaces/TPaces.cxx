#include <iostream>
#include "TPaces.h"
#include <TRandom.h>
#include <TMath.h>

////////////////////////////////////////////////////////////
//                    
// TPaces
//
// The TPaces class defines the observables and algorithms used
// when analyzing GRIFFIN data.
//
////////////////////////////////////////////////////////////

ClassImp(TPaces)

bool TPaces::fSetCoreWave = false;

TPaces::TPaces() : TGRSIDetector() {
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TPaces::TPaces(const TPaces& rhs) : TGRSIDetector() {
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TPaces::Copy(TObject &rhs) const {
  TGRSIDetector::Copy(rhs);

  static_cast<TPaces&>(rhs).fPacesHits          = fPacesHits;
  static_cast<TPaces&>(rhs).fSetCoreWave        = fSetCoreWave;
}                                       

TPaces::~TPaces()	{
   //Default Destructor
}

void TPaces::Clear(Option_t *opt)	{
   //Clears the mother, all of the hits
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
     TGRSIDetector::Clear(opt);
   }
	fPacesHits.clear();
}


void TPaces::Print(Option_t *opt) const {
  //Prints out TPaces members, currently does nothing.
  printf("%lu fPacesHits\n",fPacesHits.size());
  return;
}

TPaces& TPaces::operator=(const TPaces& rhs) {
	rhs.Copy(*this);
	return *this;
}

void TPaces::PushBackHit(TGRSIDetectorHit *pHit){
   fPacesHits.push_back(*(static_cast<TPacesHit*>(pHit)));
}

TGRSIDetectorHit* TPaces::GetHit(const Int_t& idx) {
   return GetPacesHit(idx);
}

TPacesHit* TPaces::GetPacesHit(const int& i) {
   try {
      return &fPacesHits.at(i);   
   } catch(const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw exit_exception(1);
   }
   return NULL;
}

void TPaces::AddFragment(TFragment* frag, MNEMONIC* mnemonic) {
  //Builds the PACES Hits directly from the TFragment. Basically, loops through the data for an event and sets observables. 
  //This is done for both PACES and it's suppressors.
	if(frag == NULL || mnemonic == NULL) {
		return;
	}

   Clear("");

	for(size_t i = 0; i < frag->Charge.size(); ++i) {
	  TPacesHit hit;
	  hit.SetAddress(frag->ChannelAddress);
	  hit.SetTime(frag->GetTimeStamp());
	  hit.SetCfd(frag->GetCfd(i));
	  hit.SetCharge(frag->GetCharge(i));
	  
	  AddHit(&hit);
	}
}

TVector3 TPaces::GetPosition(int DetNbr) {
   //Gets the position vector for a crystal specified by DetNbr
   //Does not currently contain any positons.
   return TVector3(0,0,1);
}

