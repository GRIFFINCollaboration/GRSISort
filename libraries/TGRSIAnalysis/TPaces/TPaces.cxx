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

TPaces::TPaces() : TGRSIDetector(),pacesdata(0) {
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TPaces::TPaces(const TPaces& rhs) : TGRSIDetector() {
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
  ((TPaces&)rhs).Copy(*this);
}

void TPaces::Copy(TObject &rhs) const {
  TGRSIDetector::Copy(rhs);

  static_cast<TPaces&>(rhs).pacesdata     = 0;

  static_cast<TPaces&>(rhs).paces_hits          = paces_hits;
  static_cast<TPaces&>(rhs).fSetCoreWave        = fSetCoreWave;
  return;                                      
}                                       

TPaces::~TPaces()	{
   //Default Destructor
   if(pacesdata) delete pacesdata;
}

void TPaces::Clear(Option_t *opt)	{
   //Clears the mother, all of the hits and any stored data
   //if(!strcmp(opt,"all") {
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
     TGRSIDetector::Clear(opt);
     if(pacesdata) pacesdata->Clear();
   }
	paces_hits.clear();
}


void TPaces::Print(Option_t *opt) const {
  //Prints out TPaces members, currently does nothing.
  printf("pacesdata = 0x%p\n",(void*) pacesdata);
  if(pacesdata) pacesdata->Print();
  printf("%lu paces_hits\n",paces_hits.size());
  return;
}

TPaces& TPaces::operator=(const TPaces& rhs) {
     ((TPaces&)rhs).Copy(*this);
     return *this;
}

void TPaces::FillData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
//Fills the "Data" structure for a specific channel with TFragment frag.
   if(!frag || !channel || !mnemonic)
      return;

   if(!pacesdata)   
      pacesdata = new TPacesData();

   pacesdata->SetCore(frag,channel,mnemonic);
   TPacesData::Set();
}

void TPaces::PushBackHit(TGRSIDetectorHit *phit){
   paces_hits.push_back(*((TPacesHit*)phit));
}

TGRSIDetectorHit* TPaces::GetHit(const Int_t& idx) {
   return GetPacesHit(idx);
}

TPacesHit* TPaces::GetPacesHit(const int& i) {
   try{
      return &paces_hits.at(i);   
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw exit_exception(1);
   }
   return 0;
}

void TPaces::BuildHits(TDetectorData *data,Option_t *opt)	{
//Builds the GRIFFIN Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
//This is done for both GRIFFIN and it's suppressors.
   TPacesData *pdata = (TPacesData*)data;
   if(pdata==0)
      pdata = (this->pacesdata);

   if(!pdata)
      return;
 
   Clear("");
   paces_hits.reserve(pdata->GetMultiplicity());

   for(size_t i=0;i<pdata->GetMultiplicity();i++)	{
      TPacesHit corehit;

      corehit.SetAddress(pdata->GetCoreAddress(i));
      corehit.SetTimeStamp(pdata->GetCoreTime(i));
      corehit.SetCfd(pdata->GetCoreCFD(i));
      corehit.SetCharge(pdata->GetCoreCharge(i));

 /*     if(TPaces::SetCoreWave()){
         corehit.SetWaveform(pdata->GetCoreWave(i));
      }
 */

      //temp_hits.push_back(corehit);  
      AddHit(&corehit);
   }
}

void TPaces::BuildHits(TFragment* frag, MNEMONIC* mnemonic) {
//Builds the PACES Hits directly from the TFragment. Basically, loops through the data for an event and sets observables. 
//This is done for both PACES and it's suppressors.
	if(!frag || !mnemonic)
      return;

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

