#include "TS3.h"
#include <cmath>
#include "TMath.h"

ClassImp(TS3)

int    TS3::fRingNumber;
int    TS3::fSectorNumber;

double TS3::fOffsetPhi;
double TS3::fOuterDiameter;
double TS3::fInnerDiameter;
double TS3::fTargetDistance;

TS3::TS3() {
   Clear();	
}

TS3::~TS3()  { 
}


TS3& TS3::operator=(const TS3& rhs) {
   rhs.Copy(*this);
   return *this;
}

TS3::TS3(const TS3& rhs) : TGRSIDetector() {
  rhs.Copy(*this);
}

void TS3::Copy(TObject &rhs) const {
  TGRSIDetector::Copy(rhs);
  static_cast<TS3&>(rhs).fS3Hits    = fS3Hits;
  return;                                      
}  

void TS3::AddFragment(TFragment* frag, MNEMONIC* mnemonic) {
	///this function takes a fragment and either adds it to an existing hit (if it's a ring for a matching sector or vice versa)
	///or creates a new hit from it
	if(frag == NULL || mnemonic == NULL) {
		return;
	}

	if(mnemonic->collectedcharge.compare(0,1,"P")==0) { //front  (ring)	
			fS3_RingFragment.push_back(frag);
	}else{
			fS3_SectorFragment.push_back(frag);
	}	
}


void TS3::BuildHits()  {
  
  for(size_t i = 0; i < fS3_RingFragment.size(); ++i) {
    for(size_t i = 0; i < fS3_SectorFragment.size(); ++i) {

	    //mnemonic->arrayposition
      //if(sdata->GetRing_Detector(i) == sdata->GetSector_Detector(j))     {

        //Set the base data     
        TS3Hit dethit(*fS3_RingFragment[i]);
	dethit.SetVariables(*fS3_RingFragment[i]);	
	
        dethit.SetRingNumber(*fS3_RingFragment[i]);
        dethit.SetSectorNumber(*fS3_SectorFragment[i]);
		
        fS3Hits.push_back(dethit);
     // }
    }
  }
  
  
  fS3_RingFragment.clear();
  fS3_SectorFragment.clear();
}
















TVector3 TS3::GetPosition(int ring, int sector)  {
  TVector3 position;
  double ring_width=(fOuterDiameter-fInnerDiameter)*0.5/fRingNumber; // 24 rings   radial width!
  double inner_radius=fInnerDiameter/2.0;
  double correctedsector = 6+sector; //moe is currently checking.
  double phi     =  2.*TMath::Pi()/fSectorNumber * (correctedsector + 0.5);   //the phi angle....
  double radius =  inner_radius + ring_width * (ring + 0.5) ;
  position.SetMagThetaPhi(sqrt(radius*radius + fTargetDistance*fTargetDistance),atan((radius/fTargetDistance)),phi+fOffsetPhi);

  return position;
}

TGRSIDetectorHit* TS3::GetHit(const int& idx){
   return GetS3Hit(idx);
}

TS3Hit *TS3::GetS3Hit(const int& i) {  
   try {
      return &fS3Hits.at(i);   
   } catch (const std::out_of_range& oor) {
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return 0;
}  

void TS3::PushBackHit(TGRSIDetectorHit *deshit) {
  fS3Hits.push_back(*((TS3Hit*)deshit));
  return;
}


void TS3::Print(Option_t *opt) const {
   printf("%s\tnot yet written.\n",__PRETTY_FUNCTION__);
}

void TS3::Clear(Option_t *opt) {
  TGRSIDetector::Clear(opt);
  fS3Hits.clear();
  fS3_RingFragment.clear();
  fS3_SectorFragment.clear();
  fRingNumber=24;
  fSectorNumber=32;
  fOffsetPhi=15*TMath::Pi()/180.; // according to dave.
  fOuterDiameter=70.;
  fInnerDiameter=22.;
  fTargetDistance=21.;
}


