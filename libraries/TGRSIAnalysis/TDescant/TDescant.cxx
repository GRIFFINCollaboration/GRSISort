
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
   //Sceptar positions from Evan; Thanks Evan.
   TVector3(0,0,1),
   TVector3(9.86,0.00,49.02),
   TVector3(20.87,0.00,45.44),
   TVector3(31.31,0.00,38.99),
   TVector3(39.43,0.00,30.74),
   TVector3(44.72,0.00,22.36),
   TVector3(14.09,10.24,46.87),
   TVector3(25.69,10.58,41.57),
   TVector3(35.62,10.24,33.56),
   TVector3(42.48,9.38,24.65),
   TVector3(18.00,21.16,41.57),
   TVector3(29.13,21.16,34.69),
   TVector3(37.75,19.85,26.09),
   TVector3(20.74,30.71,33.56),
   TVector3(30.54,29.77,26.09),
   TVector3(22.04,37.50,24.65),
   TVector3(3.05,9.38,49.02),
   TVector3(6.45,19.85,45.44),
   TVector3(9.67,29.77,38.99),
   TVector3(12.19,37.50,30.74),
   TVector3(13.82,42.53,22.36),
   TVector3(-5.38,16.56,46.87),
   TVector3(-2.12,27.70,41.57),
   TVector3(1.27,37.04,33.56),
   TVector3(4.21,43.30,24.65),
   TVector3(-14.56,23.66,41.57),
   TVector3(-11.13,34.24,34.69),
   TVector3(-7.21,42.04,26.09),
   TVector3(-22.80,29.22,33.56),
   TVector3(-18.88,38.25,26.09),
   TVector3(-28.86,32.56,24.65),
   TVector3(-7.98,5.79,49.02),
   TVector3(-16.88,12.27,45.44),
   TVector3(-25.33,18.40,38.99),
   TVector3(-31.90,23.18,30.74),
   TVector3(-36.18,26.29,22.36),
   TVector3(-17.42,0.00,46.87),
   TVector3(-27.00,6.54,41.57),
   TVector3(-34.83,12.65,33.56),
   TVector3(-39.88,17.38,24.65),
   TVector3(-27.00,-6.54,41.57),
   TVector3(-36.01,0.00,34.69),
   TVector3(-42.21,6.13,26.09),
   TVector3(-34.83,-12.65,33.56),
   TVector3(-42.21,-6.13,26.09),
   TVector3(-39.88,-17.38,24.65),
   TVector3(-7.98,-5.79,49.02),
   TVector3(-16.88,-12.27,45.44),
   TVector3(-25.33,-18.40,38.99),
   TVector3(-31.90,-23.18,30.74),
   TVector3(-36.18,-26.29,22.36),
   TVector3(-5.38,-16.56,46.87),
   TVector3(-14.56,-23.66,41.57),
   TVector3(-22.80,-29.22,33.56),
   TVector3(-28.86,-32.56,24.65),
   TVector3(-2.12,-27.70,41.57),
   TVector3(-11.13,-34.24,34.69),
   TVector3(-18.88,-38.25,26.09),
   TVector3(1.27,-37.04,33.56),
   TVector3(-7.21,-42.04,26.09),
   TVector3(4.21,-43.30,24.65),
   TVector3(3.05,-9.38,49.02),
   TVector3(6.45,-19.85,45.44),
   TVector3(9.67,-29.77,38.99),
   TVector3(12.19,-37.50,30.74),
   TVector3(13.82,-42.53,22.36),
   TVector3(14.09,-10.24,46.87),
   TVector3(18.00,-21.16,41.57),
   TVector3(20.74,-30.71,33.56),
   TVector3(22.04,-37.50,24.65),
   TVector3(25.69,-10.58,41.57),
   TVector3(29.13,-21.16,34.69),
   TVector3(30.54,-29.77,26.09),
   TVector3(35.62,-10.24,33.56),
   TVector3(37.75,-19.85,26.09),
   TVector3(42.48,-9.38,24.65)
};


TDescant::TDescant() : descantdata(0)	{
   //Default Constructor
   Class()->IgnoreTObjectStreamer(true);
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
  Class()->IgnoreTObjectStreamer(kTRUE);

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

