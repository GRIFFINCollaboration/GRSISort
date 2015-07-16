
#include <iostream>
#include "TDescant.h"
#include <TRandom.h>
#include <TMath.h>

////////////////////////////////////////////////////////////
//                    
// TDescant
//
// The TDescant class defines the observables and algorithms used
// when analyzing GRIFFIN data. It includes detector positions,
// etc. 
//
////////////////////////////////////////////////////////////

ClassImp(TDescant)


bool TDescant::fSetWave = false;

TVector3 TDescant::gPosition[71] = { 
   //Sceptar positions from James' Thesis
   TVector3(   0.0,    0.0,    1.0),
   TVector3(  98.6,    0.0,  490.2),
   TVector3(  30.5,   93.8,  490.2),
   TVector3( -79.8,   57.9,  490.2),
   TVector3( -79.8,  -57.9,  490.2),
   TVector3(  30.5,  -93.8,  490.2),
   TVector3( 208.7,    0.0,  454.4),
   TVector3( 140.9,  102.4,  468.7),
   TVector3(  64.5,  198.5,  454.4),
   TVector3( -53.8,  165.6,  468.7),
   TVector3(-168.8,  122.7,  454.4),
   TVector3(-174.2,    0.0,  468.7),
   TVector3(-168.8, -122.7,  454.4),
   TVector3( -53.8, -165.6,  468.7),
   TVector3(  64.5, -198.5,  454.4),
   TVector3( 140.9, -102.4,  468.7),
   TVector3( 313.1,    0.0,  389.9),
   TVector3( 256.9,  105.8,  415.7),
   TVector3( 180.0,  211.6,  415.7),
   TVector3(  96.7,  297.7,  389.9),
   TVector3( -21.2,  277.0,  415.7),
   TVector3(-145.6,  236.6,  415.7),
   TVector3(-253.3,  184.0,  389.9),
   TVector3(-270.0,   65.4,  415.7),
   TVector3(-270.0,  -65.4,  415.7),
   TVector3(-253.3, -184.0,  389.9),
   TVector3(-145.6, -236.6,  415.7),
   TVector3( -21.2, -277.0,  415.7),
   TVector3(  96.7, -297.7,  389.9),
   TVector3( 180.0, -211.6,  415.7),
   TVector3( 256.9, -105.8,  415.7),
   TVector3( 394.3,    0.0,  307.4),
   TVector3( 356.2,  102.4,  335.6),
   TVector3( 291.3,  211.6,  346.9),
   TVector3( 207.4,  307.1,  335.6),
   TVector3( 121.9,  375.0,  307.4),
   TVector3(  12.7,  370.4,  335.6),
   TVector3(-111.3,  342.4,  346.9),
   TVector3(-228.0,  292.2,  335.6),
   TVector3(-319.0,  231.8,  307.4),
   TVector3(-348.3,  126.5,  335.6),
   TVector3(-360.1,    0.0,  346.9),
   TVector3(-348.3, -126.5,  335.6),
   TVector3(-319.0, -231.8,  307.4),
   TVector3(-228.0, -292.2,  335.6),
   TVector3(-111.3, -342.4,  346.9),
   TVector3(  12.7, -370.4,  335.6),
   TVector3( 121.9, -375.0,  307.4),
   TVector3( 207.4, -307.1,  335.6),
   TVector3( 291.3, -211.6,  346.9),
   TVector3( 356.2, -102.4,  335.6),
   TVector3( 424.8,   93.8,  246.5),
   TVector3( 377.5,  198.5,  260.9),
   TVector3( 305.4,  297.7,  260.9),
   TVector3( 220.4,  375.0,  246.5),
   TVector3(  42.1,  433.0,  246.5),
   TVector3( -72.1,  420.4,  260.9),
   TVector3(-188.8,  382.5,  260.9),
   TVector3(-288.6,  325.6,  246.5),
   TVector3(-398.8,  173.8,  246.5),
   TVector3(-422.1,   61.3,  260.9),
   TVector3(-422.1,  -61.3,  260.9),
   TVector3(-398.8, -173.8,  246.5),
   TVector3(-288.6, -325.6,  246.5),
   TVector3(-188.8, -382.5,  260.9),
   TVector3( -72.1, -420.4,  260.9),
   TVector3(  42.1, -433.0,  246.5),
   TVector3( 220.4, -375.0,  246.5),
   TVector3( 305.4, -297.7,  260.9),
   TVector3( 377.5, -198.5,  260.9),
   TVector3( 424.8,  -93.8,  246.5)
};


TDescant::TDescant() : descantdata(0)	{
   //Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   //Class()->AddRule("TDescant descant_hits attributes=NotOwner");
   //Class()->AddRule("TDescant descantdata attributes=NotOwner");
   Clear();
}

TDescant::~TDescant()	{
   //Default Destructor
   if(descantdata) delete descantdata;
}

void TDescant::Copy(TDescant &rhs) const {
  TGRSIDetector::Copy((TGRSIDetector&)rhs);
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif

  ((TDescant&)rhs).descantdata     = 0;

  ((TDescant&)rhs).descant_hits        = descant_hits;
  ((TDescant&)rhs).fSetWave            = fSetWave;
  return;                                      
}                                       

TDescant::TDescant(const TDescant& rhs) {
  ((TDescant&)rhs).Copy(*this);
}

void TDescant::Clear(Option_t *opt)	{
//Clears all of the hits and data
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
      TGRSIDetector::Clear(opt);
      if(descantdata) descantdata->Clear();
   }
	descant_hits.clear();
}

TDescant& TDescant::operator=(const TDescant& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TDescant::Print(Option_t *opt) const	{
  //Prints out TDescant members, currently does little.
  printf("descantdata = 0x%p\n",descantdata);
  if(descantdata) descantdata->Print();
  printf("%lu descant_hits\n",descant_hits.size());
}

TGRSIDetectorHit* TDescant::GetHit(const Int_t idx){
   return GetDescantHit(idx);
}

TDescantHit* TDescant::GetDescantHit(const Int_t idx) {
   if( idx < GetMultiplicity())
      return &(descant_hits.at(idx));
   else
      return 0;

}

void TDescant::PushBackHit(TGRSIDetectorHit *deshit) {
  descant_hits.push_back(*((TDescantHit*)deshit));
  return;
}

void TDescant::FillData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
//Fills the "Data" structure for a specific channel with TFragment frag.
   if(!frag || !channel || !mnemonic)
      return;

   if(!descantdata)   
      descantdata = new TDescantData();

   descantdata->SetDet(frag,channel,mnemonic);
   TDescantData::Set();
}


void TDescant::BuildHits(TGRSIDetectorData *data,Option_t *opt)	{
//Builds the GRIFFIN Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
//This is done for both GRIFFIN and it's suppressors.
   TDescantData *gdata = (TDescantData*)data;
   if(gdata==0)
      gdata = (this->descantdata);

   if(!gdata)
      return;

   Clear("");
   descant_hits.reserve(gdata->GetMultiplicity());
   

   for(int i=0;i<gdata->GetMultiplicity();i++)	{
      TDescantHit dethit;

//      dethit.SetDetectorNumber(gdata->GetDetNumber(i));
   
      dethit.SetAddress(gdata->GetDetAddress(i));
      
//      dethit.SetEnergy(gdata->GetDetEnergy(i));
      dethit.SetCharge(gdata->GetDetCharge(i));

      dethit.SetTime(gdata->GetDetTime(i));
      dethit.SetCfd(gdata->GetDetCFD(i));
      dethit.SetZc(gdata->GetDetZc(i));
      dethit.SetCcShort(gdata->GetDetCcShort(i));
      dethit.SetCcLong(gdata->GetDetCcLong(i));
 
      if(TDescant::SetWave()){
         if(gdata->GetDetWave(i).size() == 0) {
            //printf("Warning, TDescant::SetWave() set, but data waveform size is zero!\n");
         }
         dethit.SetWaveform(gdata->GetDetWave(i));
         if(dethit.GetWaveform().size() > 0) {
            printf("Analyzing waveform, current cfd = %d, psd = %d\n",dethit.GetCfd(),dethit.GetPsd());
            bool analyzed = dethit.AnalyzeWaveform();
            printf("%s analyzed waveform, cfd = %d, psd = %d\n",analyzed ? "successfully":"unsuccessfully",dethit.GetCfd(),dethit.GetPsd());
         }
      }
		
//      dethit.SetPosition(TDescant::GetPosition(gdata->GetDetNumber(i)));
//FIX
      AddHit(&dethit);
 //     descant_hits.push_back(dethit);
     // TDescant::SetHit();
   }
}

