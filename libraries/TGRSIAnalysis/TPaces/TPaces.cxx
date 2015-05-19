
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

long TPaces::fCycleStart  = 0;
long TPaces::fLastPPG     = 0;

TPaces::TPaces() : TGRSIDetector(),pacesdata(0) {
   Clear();
}

TPaces::TPaces(const TPaces& rhs) {
  ((TPaces&)rhs).Copy(*this);
}

void TPaces::Copy(TPaces &rhs) const {
  TGRSIDetector::Copy((TGRSIDetector&)rhs);

  ((TPaces&)rhs).pacesdata     = 0;

  ((TPaces&)rhs).paces_hits          = paces_hits;
  ((TPaces&)rhs).fSetCoreWave        = fSetCoreWave;
  ((TPaces&)rhs).fPacesBits          = fPacesBits;
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
     ClearStatus();
   }
	paces_hits.clear();
}


void TPaces::Print(Option_t *opt) const {
  //Prints out TPaces members, currently does nothing.
  printf("pacesdata = 0x%p\n",pacesdata);
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

TGRSIDetectorHit* TPaces::GetHit(const Int_t idx) {
   return GetPacesHit(idx);
}

TPacesHit* TPaces::GetPacesHit(const int i) {
   if(i < GetMultiplicity())
      return &paces_hits.at(i);   
   else
      return 0;
}

void TPaces::BuildHits(TGRSIDetectorData *data,Option_t *opt)	{
//Builds the GRIFFIN Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
//This is done for both GRIFFIN and it's suppressors.
   TPacesData *pdata = (TPacesData*)data;
   if(pdata==0)
      pdata = (this->pacesdata);

   if(!pdata)
      return;
 
   Clear("");
   paces_hits.reserve(pdata->GetMultiplicity());

   for(int i=0;i<pdata->GetMultiplicity();i++)	{
      TPacesHit corehit;

      corehit.SetAddress(pdata->GetCoreAddress(i));
      corehit.SetTime(pdata->GetCoreTime(i));
      corehit.SetCfd(pdata->GetCoreCFD(i));
      corehit.SetCharge(pdata->GetCoreCharge(i));

      if(TPaces::SetCoreWave()){
         corehit.SetWaveform(pdata->GetCoreWave(i));
      }
 
      corehit.SetPPG(pdata->GetPPG(i));

      if((pdata->GetPPG(i) == 0xd000 && pdata->GetPPG(i) != fLastPPG) || fCycleStart == 0.) { //this is a background event
         fCycleStart = corehit.GetTime();
      }
      fLastPPG = pdata->GetPPG(i);
      //fCycleStartTime = fCycleStart;

      //temp_hits.push_back(corehit);  
      AddHit(&corehit);
   }
}

TVector3 TPaces::GetPosition(int DetNbr) {
   //Gets the position vector for a crystal specified by DetNbr
   //Does not currently contain any positons.
   return TVector3(0,0,1);
}
