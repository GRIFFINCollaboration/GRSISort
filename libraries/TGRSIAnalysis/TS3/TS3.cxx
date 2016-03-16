#include "TS3.h"
#include <cmath>
#include "TMath.h"

/// \cond CLASSIMP
ClassImp(TS3)
/// \endcond

int    TS3::fRingNumber;
int    TS3::fSectorNumber;

double TS3::fOffsetPhi;
double TS3::fOuterDiameter;
double TS3::fInnerDiameter;
double TS3::fTargetDistance;

Int_t TS3::fFrontBackTime;   
double TS3::fFrontBackEnergy;  

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

void TS3::AddFragment(TVirtualFragment* frag, MNEMONIC* mnemonic) {
	///This function just stores the fragments in vectors, separated by detector type (front/back strip).
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
	///This function takes the fragments that were stored in the successive AddFragment calls and builds hits out of them
  
	
//old method
// for(size_t i = 0; i < fS3_RingFragment.size(); ++i) {
//     for(size_t j = 0; j < fS3_SectorFragment.size(); ++j) {
//       //mnemonic->arrayposition
//       //if(sdata->GetRing_Detector(i) == sdata->GetSector_Detector(j))     {
//         //Set the base data     
//         TS3Hit dethit(*fS3_RingFragment[i]);
// 	dethit.SetVariables(*fS3_RingFragment[i]);	
// 	
//         dethit.SetRingNumber(*fS3_RingFragment[i]);
//         dethit.SetSectorNumber(*fS3_SectorFragment[j]);
// 		
//         fS3Hits.push_back(dethit);
//      // }
//     }
//   }	
	
// We are going to want energies sereral times and TVirtualFragment calibrates on every call
// So build a quick vector to save on repeat calulations
std::vector<double> EneR,EneS;
for(size_t i = 0; i < fS3_RingFragment.size(); ++i) EneR.push_back(fS3_RingFragment[i]->GetEnergy());
for(size_t j = 0; j < fS3_SectorFragment.size(); ++j)  EneS.push_back(fS3_SectorFragment[j]->GetEnergy());
	
		
	//new	
	///Loop over two vectors and build energy+time matching hits
	for(size_t i = 0; i < fS3_RingFragment.size(); ++i) {
	for(size_t j = 0; j < fS3_SectorFragment.size(); ++j) {      
	
			if(abs(fS3_RingFragment[i]->GetCfd()-fS3_SectorFragment[j]->GetCfd()) < fFrontBackTime){ //first check time
				if(EneR[i]*fFrontBackEnergy<EneS[j]&&
					EneS[j]*fFrontBackEnergy<EneR[i]){  //if time is good check energy
						
					//Now we have accepted a good event, build it
					TS3Hit dethit(*fS3_RingFragment[i]); // Ring defines all data sector just gives position
					dethit.SetVariables(*fS3_RingFragment[i]);	
					dethit.SetRingNumber(*fS3_RingFragment[i]);
					dethit.SetSectorNumber(*fS3_SectorFragment[j]);
					fS3Hits.push_back(dethit);
					
					//Now we have "used" these parts of hits the are removes
					//This is a relatively cheap operation because fragment vectors are pointers.
					fS3_SectorFragment.erase(fS3_SectorFragment.begin() + j);
					EneS.erase(EneS.begin() + j);
					fS3_RingFragment.erase(fS3_RingFragment.begin() + i);
					EneR.erase(EneR.begin() + i);
					j=fS3_SectorFragment.size();
					i--;
				}
			}
		}
	}
	
	///If we have parts of hit left here they are possibly a shared strip hit not easy singles
	if(fS3_SectorFragment.size()>1||fS3_RingFragment.size()>1){
		
	//Shared Ring loop
	for(size_t i = 0; i < fS3_RingFragment.size(); ++i) {
		for(size_t j = 0; j < fS3_SectorFragment.size(); ++j) {   
			for(size_t k = j+1; k < fS3_SectorFragment.size(); ++k) {  
		
				if(abs(fS3_RingFragment[i]->GetCfd()-fS3_SectorFragment[j]->GetCfd()) < fFrontBackTime
					&& abs(fS3_RingFragment[i]->GetCfd()-fS3_SectorFragment[k]->GetCfd()) < fFrontBackTime){ //first check time
					if(EneR[i]*fFrontBackEnergy<(EneS[j]+EneS[k])&&
						(EneS[j]+EneS[k])*fFrontBackEnergy<EneR[i]){  //if time is good check energy
							
						//Now we have accepted a shared ring event, build them
						TS3Hit dethit(*fS3_SectorFragment[j]); // Now we have to use the sector data
						dethit.SetVariables(*fS3_SectorFragment[j]);	
						dethit.SetRingNumber(*fS3_RingFragment[i]);
						dethit.SetSectorNumber(*fS3_SectorFragment[j]);
						fS3Hits.push_back(dethit);
					
						TS3Hit dethitB(*fS3_SectorFragment[k]); // Now we have to use the sector data
						dethitB.SetVariables(*fS3_SectorFragment[k]);	
						dethitB.SetRingNumber(*fS3_RingFragment[i]);
						dethitB.SetSectorNumber(*fS3_SectorFragment[k]);
						fS3Hits.push_back(dethitB);
								
						//Now we have "used" these parts of hits the are removes
						//This is a relatively cheap operation because fragment vectors are pointers.
						fS3_SectorFragment.erase(fS3_SectorFragment.begin() + k);//Make sure to delete the later element first
						EneS.erase(EneS.begin() + k);
						fS3_SectorFragment.erase(fS3_SectorFragment.begin() + j);
						EneS.erase(EneS.begin() + j);
						fS3_RingFragment.erase(fS3_RingFragment.begin() + i);
						EneR.erase(EneR.begin() + i);
						k=fS3_SectorFragment.size();
						j=fS3_SectorFragment.size();
						i--;
					}
				}
			}
		}
	} //End Shared Ring loop
	
	//IF we STILL have events left	 
	//Shared Sector loop
	for(size_t i = 0; i < fS3_SectorFragment.size(); ++i) {
		for(size_t j = 0; j < fS3_RingFragment.size(); ++j) {   
			for(size_t k = j+1; k < fS3_RingFragment.size(); ++k) {  
		
				if(abs(fS3_SectorFragment[i]->GetCfd()-fS3_RingFragment[j]->GetCfd()) < fFrontBackTime
					&& abs(fS3_SectorFragment[i]->GetCfd()-fS3_RingFragment[k]->GetCfd()) < fFrontBackTime){ //first check time
					if(EneS[i]*fFrontBackEnergy<(EneR[j]+EneR[k])&&
						(EneR[j]+EneR[k])*fFrontBackEnergy<EneS[i]){  //if time is good check energy
							
						//Now we have accepted a shared sector event, build them
						TS3Hit dethit(*fS3_RingFragment[j]); // Ring defines all data sector just gives position
						dethit.SetVariables(*fS3_RingFragment[j]);	
						dethit.SetRingNumber(*fS3_RingFragment[j]);
						dethit.SetSectorNumber(*fS3_SectorFragment[i]);
						fS3Hits.push_back(dethit);
					
						TS3Hit dethitB(*fS3_RingFragment[k]); // Ring defines all data sector just gives position
						dethitB.SetVariables(*fS3_RingFragment[k]);	
						dethitB.SetRingNumber(*fS3_RingFragment[k]);
						dethitB.SetSectorNumber(*fS3_SectorFragment[i]);
						fS3Hits.push_back(dethitB);
						
						//Now we have "used" these parts of hits the are removes
						fS3_RingFragment.erase(fS3_RingFragment.begin() + k);//Make sure to delete the later element first
						EneR.erase(EneR.begin() + k);
						fS3_RingFragment.erase(fS3_RingFragment.begin() + j);
						EneR.erase(EneR.begin() + j);
						fS3_SectorFragment.erase(fS3_SectorFragment.begin() + i);
						EneS.erase(EneS.begin() + i);
						k=fS3_RingFragment.size();
						j=fS3_RingFragment.size();
						i--;
					}
				}
			}
		}
	} //End Shared Sector loop
  }//End Shared Strip Loop


  //If there are any used hit parts clear 'em
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
  
  fFrontBackTime=75;   
  fFrontBackEnergy=0.9; 
}


