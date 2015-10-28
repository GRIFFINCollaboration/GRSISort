#include <iostream>
#include "TS3.h"
#include <TRandom.h>
#include <TMath.h>
#include <TClass.h>
#include <TGRSIRunInfo.h>

ClassImp(TS3)

int    TS3::ring_number;
int    TS3::sector_number;

double TS3::offset_phi;
double TS3::outer_diameter;
double TS3::inner_diameter;
double TS3::target_distance;



TS3::TS3():s3data(0)  {
// #if MAJOR_ROOT_VERSION < 6
//    Class()->IgnoreTObjectStreamer(kTRUE);
// #endif
   Clear();	
}

TS3::~TS3()  { 
  if(s3data) delete s3data;   
}

void TS3::Copy(TObject &rhs) const {
  TGRSIDetector::Copy(rhs);
// #if MAJOR_ROOT_VERSION < 6
//    Class()->IgnoreTObjectStreamer(kTRUE);
// #endif

  static_cast<TS3&>(rhs).s3data     = 0;
  static_cast<TS3&>(rhs).s3_hits    = s3_hits;
  return;                                      
}  

TS3::TS3(const TS3& rhs) : TGRSIDetector() {
  rhs.Copy(*this);
}


void TS3::Clear(Option_t *opt)    {
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
      TGRSIDetector::Clear(opt);
      if(s3data) s3data->Clear();
   }

  s3_hits.clear();
  
  ring_number=24;
  sector_number=32;

  offset_phi=15*TMath::Pi()/180.; // according to dave.
  outer_diameter=70.;
  inner_diameter=22.;
  target_distance=21.;
}

TS3& TS3::operator=(const TS3& rhs) {
   rhs.Copy(*this);
   return *this;
}


void TS3::Print(Option_t *opt) const {
  printf("s3data = 0x%p\n", (void*) s3data);
  if(s3data) s3data->Print();
  printf("%lu s3_hits\n",s3_hits.size());
}


TGRSIDetectorHit* TS3::GetHit(const Int_t& idx){
   return GetS3Hit(idx);
}

TS3Hit *TS3::GetS3Hit(const int& i) {  
   try{
      return &s3_hits.at(i);   
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return 0;
}  

void TS3::PushBackHit(TGRSIDetectorHit *deshit) {
  s3_hits.push_back(*((TS3Hit*)deshit));
  return;
}


void TS3::FillData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
   if(!frag || !channel || !mnemonic)
      return;

   if(!s3data)   
      s3data = new TS3Data();

  if(mnemonic->collectedcharge.compare(0,1,"P")==0) { //front  (ring)
    s3data->SetRing(frag,channel,mnemonic);
  } else {
    s3data->SetSector(frag,channel,mnemonic);
  }
  TS3Data::Set();
}



void TS3::BuildHits(TDetectorData *data,Option_t *opt)  {
  TS3Data *sdata = (TS3Data*)data;
  if(sdata==0)
     sdata = this->s3data;
  if(!sdata)
     return;

   Clear("");
  
   //s3_hits.reserve(sdata->GetMultiplicity());
  
  for(int i=0;i<sdata->GetRingMultiplicity();i++)     { 
    for(int j=0;j<sdata->GetSectorMultiplicity();j++)    { 
	   
      if(sdata->GetRing_Detector(i) == sdata->GetSector_Detector(j))     {
	      
	//Set the base data     
        TFragment tmpfrag = sdata->GetRing_Fragment(i);
        TS3Hit dethit(tmpfrag);
	//TS3Hit dethit;
        //dethit.CopyFragment(tmpfrag);
	
        //set the detector unique data	
        dethit.SetRingNumber(sdata->GetRing_Number(i));
        dethit.SetSectorNumber(sdata->GetSector_Number(j));
        dethit.SetDetector(sdata->GetRing_Detector(i));
        TVector3 tmppos = GetPosition(dethit.GetRingNumber(),dethit.GetSectorNumber());
        dethit.SetPosition(tmppos);
	
        //used to set the basic here, now just set the basic that isnt standard (led in this case)
        dethit.SetVariables(tmpfrag);
		
        s3_hits.push_back(dethit);
      }
    }
  }
  
}

TVector3 TS3::GetPosition(int ring, int sector)  {

  TVector3 position;

  double ring_width=(outer_diameter-inner_diameter)*0.5/ring_number; // 24 rings   radial width!
  double inner_radius=inner_diameter/2.0;

  
  double correctedsector = 6+sector; //moe is currently checking.

  double phi     =  2.*TMath::Pi()/sector_number * (correctedsector + 0.5);   //the phi angle....
  double radius =  inner_radius + ring_width * (ring + 0.5) ;
  
  position.SetMagThetaPhi(sqrt(radius*radius + target_distance*target_distance),atan((radius/target_distance)),phi+offset_phi);


  return position;


}

