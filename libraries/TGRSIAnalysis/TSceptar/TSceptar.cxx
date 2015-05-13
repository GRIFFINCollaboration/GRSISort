
#include <iostream>
#include "TSceptar.h"
#include <TRandom.h>
#include <TMath.h>

////////////////////////////////////////////////////////////
//                    
// TSceptar
//
// The TSceptar class defines the observables and algorithms used
// when analyzing GRIFFIN data. It includes detector positions,
// etc. 
//
////////////////////////////////////////////////////////////

ClassImp(TSceptar)


//bool TSceptar::fSetWave = false;

TVector3 TSceptar::gPaddlePosition[21] = { 
   //Sceptar positions from Evan; Thanks Evan.
   TVector3(0,0,1),
   TVector3(14.3025, 4.6472, 22.8096),  
   TVector3(0, 15.0386, 22.8096), 
   TVector3(-14.3025, 4.6472, 22.8096),  
   TVector3(-8.8395, -12.1665, 22.8096),  
   TVector3(8.8395, -12.1665, 22.8096),     
   TVector3(19.7051, 6.4026, 6.2123),  
   TVector3(0, 20.7192, 6.2123),  
   TVector3(-19.7051, 6.4026, 6.2123),  
   TVector3(-12.1784, -16.7622, 6.2123),  
   TVector3(12.1784, -16.7622, 6.2123),     
   TVector3(19.7051, 6.4026, -6.2123),  
   TVector3(0, 20.7192, -6.2123),  
   TVector3(-19.7051, 6.4026, -6.2123),  
   TVector3(-12.1784, -16.7622, -6.2123),  
   TVector3(12.1784, -16.7622, -6.2123),
   TVector3(14.3025, 4.6472, -22.8096),  
   TVector3(0, 15.0386, -22.8096),  
   TVector3(-14.3025, 4.6472, -22.8096),  
   TVector3(-8.8395, -12.1665, -22.8096),  
   TVector3(8.8395, -12.1665, -22.8096)
};


TSceptar::TSceptar() : sceptardata(0)	{
   //Default Constructor
   //Class()->IgnoreTObjectStreamer(true);
   //Class()->AddRule("TSceptar sceptar_hits attributes=NotOwner");
   //Class()->AddRule("TSceptar sceptardata attributes=NotOwner");
   Clear();
}

TSceptar::~TSceptar()	{
   //Default Destructor
   if(sceptardata) delete sceptardata;
}

TSceptar::TSceptar(const TSceptar& rhs) {
  ((TSceptar&)rhs).Copy(*this);
}

void TSceptar::Clear(Option_t *opt)	{
//Clears all of the hits and data
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
      TGRSIDetector::Clear(opt);
      if(sceptardata) sceptardata->Clear();
   //   ClearStatus();
   }
	sceptar_hits.clear();
}

void TSceptar::Copy(TSceptar &rhs) const {
  TGRSIDetector::Copy((TGRSIDetector&)rhs);

  ((TSceptar&)rhs).sceptardata     = 0;

  ((TSceptar&)rhs).sceptar_hits        = sceptar_hits;
  return;                                      
}                                       

TSceptar& TSceptar::operator=(const TSceptar& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TSceptar::Print(Option_t *opt) const	{
  //Prints out TSceptar members, currently does nothing.
  printf("sceptardata = 0x%p\n",sceptardata);
  if(sceptardata) sceptardata->Print();
  printf("%lu sceptar_hits\n",sceptar_hits.size());
  return;
}


void TSceptar::FillData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
//Fills the "Data" structure for a specific channel with TFragment frag.
   if(!frag || !channel || !mnemonic)
      return;

   if(!sceptardata)   
      sceptardata = new TSceptarData();

   sceptardata->SetDet(frag,channel,mnemonic);
   TSceptarData::Set();
}

void TSceptar::PushBackHit(TGRSIDetectorHit *schit) {
  sceptar_hits.push_back(*((TSceptarHit*)schit));
  return;
}

void TSceptar::BuildHits(TGRSIDetectorData *data,Option_t *opt)	{
//Builds the SCEPTAR Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
//This is done for both GRIFFIN and it's suppressors.
   TSceptarData *gdata = (TSceptarData*)data;
   if(gdata==0)
      gdata = (this->sceptardata);

   if(!gdata)
      return;

   Clear("");
   sceptar_hits.reserve(gdata->GetMultiplicity());
  // TSceptar::SetBeta(false);
   
   for(int i=0;i<gdata->GetMultiplicity();i++)	{
      TSceptarHit dethit;

      dethit.SetAddress(gdata->GetDetAddress(i));
      
 //     dethit.SetEnergy(gdata->GetDetEnergy(i));
      dethit.SetCharge(gdata->GetDetCharge(i));

      dethit.SetTime(gdata->GetDetTime(i));
      dethit.SetCfd(gdata->GetDetCFD(i));

//      if(TSceptar::SetWave()){
//         dethit.SetWaveform(gdata->GetDetWave(i));
//      }
		
//      dethit.SetDetector(gdata->GetDetNumber(i));
   
   //   dethit.SetPosition(TSceptar::GetPosition(gdata->GetDetNumber(i)));
   
      AddHit(&dethit);
//     TSceptar::SetBeta();
   }
}
