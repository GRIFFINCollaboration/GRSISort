

#include "TS3.h"

#include <TMath.h>
#include <cmath>

ClassImp(TS3)


int    TS3::ring_number;
int    TS3::sector_number;

double TS3::offset_phi;
double TS3::outer_diameter;
double TS3::inner_diameter;
double TS3::target_distance;




TS3::TS3():data(0)  {
  ring_number=24;
  sector_number=32;

  offset_phi=15*TMath::Pi()/180.; // according to dave.
  outer_diameter=70.;
  inner_diameter=22.;
  target_distance=21.;
}

TS3::~TS3()  { 
  if(data) delete data;   
}


void TS3::FillData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
  if(!data)
     data = new TS3Data();
  if(mnemonic->collectedcharge.compare(0,1,"P")==0) { //front  (ring)
    data->SetRing(frag,channel,mnemonic);
  } else {
    data->SetSector(frag,channel,mnemonic);
  }
  //TS3Data::Set();
}



void TS3::BuildHits(TGRSIDetectorData *data,Option_t *opt)  {
  TS3Data *sdata = (TS3Data*)data;
  if(sdata==0)
     sdata = this->data;
  if(!sdata)
     return;

  TS3Hit hit;
  
  for(int i=0;i<sdata->GetRingMultiplicity();i++)     { 
    for(int j=0;j<sdata->GetSectorMultiplicity();j++)    { 
      if(sdata->GetRing_Detector(i) == sdata->GetSector_Detector(j))     {
        hit.SetRingNumber(sdata->GetRing_Number(i));
        hit.SetSectorNumber(sdata->GetSector_Number(j));
        hit.SetDetectorNumber(sdata->GetRing_Detector(i));
        TFragment tmpfrag = sdata->GetRing_Fragment(i);
        
        hit.SetVariables(tmpfrag);
        TVector3 tmppos = GetPosition(hit.GetRingNumber(),hit.GetSectorNumber());
        hit.SetPosition(tmppos);
        s3_hits.push_back(hit);

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

void TS3::Print(Option_t *opt) const {
   printf("%s\tnot yet written.\n",__PRETTY_FUNCTION__);
}

void TS3::Clear(Option_t *opt)    {
  if(data) data->Clear();
  s3_hits.clear();

}


