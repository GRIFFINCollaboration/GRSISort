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
  fRingNumber=24;
  fSectorNumber=32;

  fOffsetPhi=15*TMath::Pi()/180.; // according to dave.
  fOuterDiameter=70.;
  fInnerDiameter=22.;
  fTargetDistance=21.;
}

TS3::~TS3()  { 
}

void TS3::AddFragment(TFragment* frag, MNEMONIC* mnemonic) {
	///this function takes a fragment and either adds it to an existing hit (if it's a ring for a matching sector or vice versa)
	///or creates a new hit from it
	if(frag == NULL || mnemonic == NULL) {
		return;
	}

	for(size_t i = 0; i < fS3Hits.size(); ++i) {
		if(static_cast<UInt_t>(mnemonic->arrayposition) == fS3Hits[i].GetDetector()) { //same detector
			if(mnemonic->collectedcharge.compare(0,1,"P")==0) { //front  (ring)
				//this means we've already found a sector of this detector
				//so we set the ring number and all other variables from this ring
				fS3Hits[i].SetRingNumber(mnemonic->segment);
				fS3Hits[i].SetVariables(*frag);
				TVector3 tmppos = GetPosition(fS3Hits[i].GetRingNumber(),fS3Hits[i].GetSectorNumber());
				fS3Hits[i].SetPosition(tmppos);
				return; //we've filled the data of the current fragment into the hits so we're done 
			} else { //back (sector)
				//this means we've already found a ring of this detector
				//so we set the sector number from this sector (all other variables are set by the front)
				fS3Hits[i].SetSectorNumber(mnemonic->segment);
				//fS3Hits[i].SetVariables(*frag);
				TVector3 tmppos = GetPosition(fS3Hits[i].GetRingNumber(),fS3Hits[i].GetSectorNumber());
				fS3Hits[i].SetPosition(tmppos);
				return; //we've filled the data of the current fragment into the hits so we're done 
			}
		}
	}
	//if we reach here we haven't found a detector before so we create a new hit
  TS3Hit hit;
  
  if(mnemonic->collectedcharge.compare(0,1,"P")==0) { //front  (ring)
	  //this means we've already found a sector of this detector
	  //so we set the ring number and all other variables from this ring
	  hit.SetRingNumber(mnemonic->segment);
	  hit.SetVariables(*frag);
  } else { //back (sector)
	  //this means we've already found a ring of this detector
	  //so we set the sector number from this sector (all other variables are set by the front)
	  hit.SetSectorNumber(mnemonic->segment);
	  //hit.SetVariables(*frag);
  }

  fS3Hits.push_back(hit);
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

TS3Hit *TS3::GetS3Hit(const int& i) {  
   try {
      return &fS3Hits.at(i);   
   } catch (const std::out_of_range& oor) {
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw exit_exception(1);
   }
   return 0;
}  

void TS3::Print(Option_t *opt) const {
   printf("%s\tnot yet written.\n",__PRETTY_FUNCTION__);
}

void TS3::Clear(Option_t *opt)    {
  fS3Hits.clear();
}


