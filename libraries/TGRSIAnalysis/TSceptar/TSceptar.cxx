
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


bool TSceptar::fSetWave = false;

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
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   //Class()->AddRule("TSceptar sceptar_hits attributes=NotOwner");
   //Class()->AddRule("TSceptar sceptardata attributes=NotOwner");
   Clear();
}

TSceptar::~TSceptar()	{
   //Default Destructor
   if(sceptardata) delete sceptardata;
}

TSceptar::TSceptar(const TSceptar& rhs) : TGRSIDetector() {
   //Copy Contructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
  ((TSceptar&)rhs).Copy(*this);
}

void TSceptar::Clear(Option_t *opt)	{
//Clears all of the hits
//The Option "all" clears the base class and data.
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
      TGRSIDetector::Clear(opt);
      if(sceptardata) sceptardata->Clear();
   //   ClearStatus();
   }
	sceptar_hits.clear();
}

void TSceptar::Copy(TSceptar &rhs) const {
   //Copies a TSceptar
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
  //Prints out TSceptar Multiplicity, currently does little.
	printf("sceptardata = 0x%p\n",(void*) sceptardata);
  if(sceptardata) sceptardata->Print();
  printf("%lu sceptar_hits\n",sceptar_hits.size());
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
   //Adds a Hit to the list of TSceptar Hits
   sceptar_hits.push_back(*((TSceptarHit*)schit));
   return;
}

void TSceptar::BuildHits(TDetectorData *data,Option_t *opt)	{
//Builds the SCEPTAR Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
//This is done for both GRIFFIN and it's suppressors.
   TSceptarData *gdata = (TSceptarData*)data;
   if(gdata==0)
      gdata = (this->sceptardata);

   if(!gdata)
      return;

   //Clear("");
   sceptar_hits.reserve(gdata->GetMultiplicity());
   
   for(size_t i=0;i<gdata->GetMultiplicity();i++)	{
      TSceptarHit dethit;

      dethit.SetAddress(gdata->GetDetAddress(i));
      
 //     dethit.SetEnergy(gdata->GetDetEnergy(i));
      dethit.SetCharge(gdata->GetDetCharge(i));

      dethit.SetTimeStamp(gdata->GetDetTime(i));
      dethit.SetCfd(gdata->GetDetCFD(i));

      //UNCOMMENTED FOR NOW
      if(TSceptar::SetWave()){
         if(gdata->GetDetWave(i).size() == 0) {
            printf("Warning, TSceptar::SetWave() set, but data waveform size is zero!\n");
         }
         dethit.SetWaveform(gdata->GetDetWave(i));
         if(dethit.GetWaveform().size() > 0) {
//            printf("Analyzing waveform, current cfd = %d\n",dethit.GetCfd());
            dethit.AnalyzeWaveform();
//            printf("%s analyzed waveform, cfd = %d\n",analyzed ? "successfully":"unsuccessfully",dethit.GetCfd());
         }
      }
		
      //dethit.SetDetector(gdata->GetDetNumber(i));
   
   //   dethit.SetPosition(TSceptar::GetPosition(gdata->GetDetNumber(i)));
   
      AddHit(&dethit);
   }
}

TGRSIDetectorHit* TSceptar::GetHit(const Int_t idx){
   //Gets the TSceptarHit at index idx. 
   if(idx < GetMultiplicity())
      return &(sceptar_hits.at(idx));
   else 
      return 0;
}

