#include <iostream>
#include "TTip.h"
#include <TRandom.h>
#include <TMath.h>
#include <TClass.h>

////////////////////////////////////////////////////////////
//                    
// TTip
//
// The TTip class defines the observables and algorithms used
// when analyzing TIP data. It includes detector positions,
// etc. 
//
////////////////////////////////////////////////////////////

ClassImp(TTip)

TTip::TTip() : tipdata(0) {   
}

TTip::~TTip() {
   //Default Destructor
   if(tipdata) delete tipdata;
}

TTip::TTip(const TTip& rhs) : TGRSIDetector() {
  Class()->IgnoreTObjectStreamer(kTRUE);
  ((TTip&)rhs).Copy(*this);
}

void TTip::Copy(TObject &rhs) const {
  TGRSIDetector::Copy(rhs);

  static_cast<TTip&>(rhs).tipdata   = 0;
  static_cast<TTip&>(rhs).tip_hits             = tip_hits;
  return;                                      
}                                       

void TTip::Clear(Option_t *opt) {
//Clears all of the hits and data
   if(tipdata) tipdata->Clear();

   tip_hits.clear();
}

TTip& TTip::operator=(const TTip& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TTip::FillData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
//Fills the "Data" structure for a specific channel with TFragment frag.
   if(!frag || !channel || !mnemonic)
      return;

   if(!tipdata)   
      tipdata = new TTipData();

   //tipdata->SetDet(frag,channel,mnemonic);
   TTipData::Set();
}

void TTip::BuildHits(TDetectorData *data,Option_t *opt)	{
//Builds the TIP Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
   TTipData *gdata = (TTipData*)data;
   if(gdata==0)
      gdata = (this->tipdata);

   if(!gdata)
      return;

   tip_hits.clear();
   
   for(size_t i=0;i<gdata->GetMultiplicity();i++) {
      TTipHit dethit;

      //dethit.SetAddress(gdata->GetDetAddress(i));
      
      //dethit.SetCharge(gdata->GetDetCharge(i));

      //dethit.SetTime(gdata->GetDetTime(i));
      //dethit.SetCfd(gdata->GetDetCFD(i));

      //dethit.SetPID(gdata->GetPID(i));

      tip_hits.push_back(dethit);
   }
}

void TTip::Print(Option_t *opt) const {
  //Prints out TSceptar members, currently does little.
  if(tipdata) tipdata->Print();
  printf("%lu tip_hits\n",tip_hits.size());
}

TGRSIDetectorHit* TTip::GetHit(const Int_t& idx) {
   return GetTipHit(idx);
}

TTipHit* TTip::GetTipHit(const int& i) {
   try{
      return &tip_hits.at(i);   
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw exit_exception(1);
   }
   return 0;
}

void TTip::PushBackHit(TGRSIDetectorHit *tiphit) {
  tip_hits.push_back(*(static_cast<TTipHit*>(tiphit)));
  return;
}
