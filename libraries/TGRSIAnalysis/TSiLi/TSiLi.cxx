#include <iostream>
#include "TSiLi.h"
#include <TRandom.h>
#include <TMath.h>
#include <TClass.h>
#include <TGRSIRunInfo.h>

ClassImp(TSiLi)

TSiLi::TSiLi() : silidata(0)  {
   Clear();	
}

TSiLi::~TSiLi()  {
  if(silidata) delete silidata;   
}

void TSiLi::Copy(TObject &rhs) const {
  TGRSIDetector::Copy(rhs);

  static_cast<TSiLi&>(rhs).silidata     = 0;
  static_cast<TSiLi&>(rhs).sili_hits     = sili_hits;
  return;                                      
}  


TSiLi::TSiLi(const TSiLi& rhs) : TGRSIDetector() {
  rhs.Copy(*this);
}

void TSiLi::Clear(Option_t *opt)  {
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
      TGRSIDetector::Clear(opt);
      if(silidata) silidata->Clear();
   }
   sili_hits.clear();
}

TSiLi& TSiLi::operator=(const TSiLi& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TSiLi::Print(Option_t *opt) const  {  
  printf("silidata = 0x%p\n", (void*) silidata);
  if(silidata) silidata->Print();
  printf("%lu sili_hits\n",sili_hits.size());
}

TGRSIDetectorHit* TSiLi::GetHit(const Int_t& idx){
   return GetSiLiHit(idx);
}

TSiLiHit * TSiLi::GetSiLiHit(const int& i)   {  
   try{
      return &sili_hits.at(i);   
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return 0;
}   

void TSiLi::PushBackHit(TGRSIDetectorHit *deshit) {
  sili_hits.push_back(*((TSiLiHit*)deshit));
  return;
}

void TSiLi::FillData(TFragment *frag,TChannel *channel, MNEMONIC *mnemonic) {
   if(!frag || !channel || !mnemonic)
      return;

   if(!silidata)   
      silidata = new TSiLiData();

  silidata->SetSiLi(frag,channel,mnemonic);
  
  TSiLiData::Set();
}


void TSiLi::BuildHits(TDetectorData *data,Option_t *opt)  {
  TSiLiData *sdata = (TSiLiData*)data;
  if(!sdata)
    sdata = this->silidata;
  if(!sdata)
    return;

   Clear("");
  
   //sili_hits.reserve(sdata->GetMultiplicity());
  for(UInt_t i=0;i<sdata->GetMultiplicity();i++)     { 
	  
      //Set the base data
      TFragment tmpfrag = sdata->GetFragment(i);
      TSiLiHit dethit(tmpfrag);
      //TSiLiHit dethit;
      //dethit.CopyFragment(tmpfrag);

      //used to set the basic here, now just set the basic that isnt standard (led in this case)
      dethit.SetVariables(tmpfrag);
      
      //set the detector unique data
      dethit.SetSegment(sdata->GetSegment(i));
      if(TGRSIRunInfo::IsWaveformFitting()) //do some fits and set that data
     	 dethit.SetWavefit(tmpfrag);
  
      sili_hits.push_back(dethit);
  }

}

TVector3 TSiLi::GetPosition(int seg)  {
  TVector3 position;
  position.SetXYZ(0,0,-1);
  return position;
}
