#include "TS3.h"
#include <cmath>
#include "TMath.h"

#include "TGRSIRunInfo.h"

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
  static_cast<TS3&>(rhs).fS3RingHits    	= fS3RingHits;
  static_cast<TS3&>(rhs).fS3SectorHits    = fS3SectorHits;
  return;                                      
}  

void TS3::AddFragment(TFragment* frag, TChannel* chan) {
	///This function creates TS3Hits for each fragment and stores them in separate front and back vectors
	if(frag == NULL || chan == NULL) {
		return;
	}

   TS3Hit dethit(*frag);

	bool IsDownstream = false;	
   //preferably channel would be pushed to the ctor as well and all of this would be done in the ctor
   if(chan->GetMnemonic()->CollectedCharge() == MNEMONIC::kN){
		dethit.SetRingNumber(*frag);
		dethit.SetSectorNumber(0);
		if(chan->GetMnemonic()->ArrayPosition() == 0 || chan->GetMnemonic()->ArrayPosition() == 2)
			IsDownstream = true;
		else if(chan->GetMnemonic()->ArrayPosition() == 1)
			IsDownstream = false;
		else
			IsDownstream = true; // In case of incorrect value, we assume downstream
		
      dethit.SetIsDownstream(IsDownstream);
		if(TGRSIRunInfo::IsWaveformFitting())	// Only fit waveforms for rings
			dethit.SetWavefit(*frag);

		fS3RingHits.push_back(std::move(dethit));
   }
   else {
		dethit.SetRingNumber(0);
		dethit.SetSectorNumber(*frag);
		if(chan->GetMnemonic()->ArrayPosition() == 0 || chan->GetMnemonic()->ArrayPosition() == 2)
			IsDownstream = true;
		else if(chan->GetMnemonic()->ArrayPosition() == 1)
			IsDownstream = false;
		else
			IsDownstream = true; // In case of incorrect value, we assume downstream
		
      dethit.SetIsDownstream(IsDownstream);
		if(TGRSIRunInfo::IsWaveformFitting())	// Only fit waveforms for rings
			dethit.SetWavefit(*frag);
		
      fS3SectorHits.push_back(std::move(dethit));
	}	
/*
	if(chan->GetMnemonic()->collectedcharge.compare(0,1,"N")==0) { // ring	
			dethit.SetRingNumber(*frag);
			dethit.SetSectorNumber(0);
			if(chan->GetMnemonic()->arrayposition == 0 || chan->GetMnemonic()->arrayposition == 2)
				IsDownstream = true;
			else if(chan->GetMnemonic()->arrayposition == 1)
				IsDownstream = false;
			else
				IsDownstream = true; // In case of incorrect value, we assume downstream
			dethit.SetIsDownstream(IsDownstream);
			if(TGRSIRunInfo::IsWaveformFitting())	// Only fit waveforms for rings
				dethit.SetWavefit(*frag);
			fS3RingHits.push_back(dethit);
	} else {
			dethit.SetRingNumber(0);
			dethit.SetSectorNumber(*frag);
			if(chan->GetMnemonic()->arrayposition == 0 || chan->GetMnemonic()->arrayposition == 2)
				IsDownstream = true;
			else if(chan->GetMnemonic()->arrayposition == 1)
				IsDownstream = false;
			else
				IsDownstream = true; // In case of incorrect value, we assume downstream
			dethit.SetIsDownstream(IsDownstream);
			if(TGRSIRunInfo::IsWaveformFitting())	// Only fit waveforms for rings
				dethit.SetWavefit(*frag);
			fS3SectorHits.push_back(dethit);
	}	*/
}

void TS3::SetBitNumber(enum ES3Bits bit,Bool_t set){
  //Used to set the flags that are stored in TTigress.
  if(set)
    fS3Bits |= bit;
  else
    fS3Bits &= (~bit);
}

Int_t TS3::GetPixelMultiplicity(){
	// Creates a vector of TS3Hits based on front/back coincidences
	// Returns the size of the resultant vector

	BuildPixels();
	
	return fS3Hits.size();

}

void TS3::BuildPixels(){
	// Constructs the front/back coincidences to create pixels based on energy and time differences
	// Energy and time differences can be changed using the SetFrontBackEnergy and SetFrontBackTime functions
	// Shared rings and sectors can be constructed, by default they are not. 
	// To enable shared hits, use SetMultiHit function


	if(fS3RingHits.size()==0 || fS3SectorHits.size()==0)
		return;
  //if the pixels have been reset, clear the pixel hits
  if((fS3Bits & kPixelsSet) == 0x0)
    fS3Hits.clear();
  if(fS3Hits.size() == 0) {
		

		// We are going to want energies sereral times and TFragment calibrates on every call
		// So build a quick vector to save on repeat calulations
		std::vector<double> EneR,EneS;
		std::vector<bool> UsedRing, UsedSector;
		for(size_t i = 0; i < fS3RingHits.size(); ++i){
			EneR.push_back(fS3RingHits[i].GetEnergy());
			UsedRing.push_back(false);
		}
		for(size_t j = 0; j < fS3SectorHits.size(); ++j){
			EneS.push_back(fS3SectorHits[j].GetEnergy());
			UsedSector.push_back(false);
		}
	

		//new	
		///Loop over two vectors and build energy+time matching hits
		for(size_t i = 0; i < fS3RingHits.size(); ++i) {
			for(size_t j = 0; j < fS3SectorHits.size(); ++j) {      
			

				if(abs(fS3RingHits[i].GetCfd()-fS3SectorHits[j].GetCfd()) < fFrontBackTime){ // check time
					if(EneR[i]*fFrontBackEnergy<EneS[j]&&
						EneS[j]*fFrontBackEnergy<EneR[i]){  //if time is good check energy

						//Now we have accepted a good event, build it
						TS3Hit dethit = fS3RingHits[i]; // Ring defines all data sector just gives position
						dethit.SetSectorNumber(fS3SectorHits[j].GetSector());
						if(TGRSIRunInfo::IsWaveformFitting()){
							dethit.SetTimeFit(fS3RingHits[i].GetFitTime());
							dethit.SetSig2Noise(fS3RingHits[i].GetSignalToNoise());
						}
						fS3Hits.push_back(dethit);

						UsedRing[i]=true;
						UsedSector[j]=true;
					}
				}
			}
		}
	
		if((fS3Bits & kMultHit) == 0x1){
		
			int ringcount = 0;
			int sectorcount = 0;
			for(unsigned int i=0;i<UsedRing.size();i++)
				if(!UsedRing.at(i))
					ringcount++;

			for(unsigned int i=0;i<UsedSector.size();i++)
				if(!UsedSector.at(i))
					sectorcount++;

		
			///If we have parts of hit left here they are possibly a shared strip hit not easy singles
			if(ringcount>1||sectorcount>1){
		
				//Shared Ring loop
				for(size_t i = 0; i < fS3RingHits.size(); ++i) {
					if(UsedRing.at(i))
						continue;
					for(size_t j = 0; j < fS3SectorHits.size(); ++j) { 
						if(UsedSector.at(j))
							continue;  
						for(size_t k = j+1; k < fS3SectorHits.size(); ++k) {  
							if(UsedSector.at(k))
								continue;  
		
							if(abs(fS3RingHits[i].GetCfd()-fS3SectorHits[j].GetCfd()) < fFrontBackTime
								&& abs(fS3RingHits[i].GetCfd()-fS3SectorHits[k].GetCfd()) < fFrontBackTime){ // check time
								if(EneR[i]*fFrontBackEnergy<(EneS[j]+EneS[k])&&
									(EneS[j]+EneS[k])*fFrontBackEnergy<EneR[i]){  //if time is good check energy

									//Now we have accepted a good event, build it
									TS3Hit dethit = fS3SectorHits[j]; // Sector now defines all data ring just gives position
									dethit.SetRingNumber(fS3RingHits[i].GetRing());
									if(TGRSIRunInfo::IsWaveformFitting()){
										dethit.SetTimeFit(fS3SectorHits[j].GetFitTime());
										dethit.SetSig2Noise(fS3SectorHits[j].GetSignalToNoise());
									}
									fS3Hits.push_back(dethit);

									//Now we have accepted a good event, build it
									TS3Hit dethitB = fS3SectorHits[k]; // Sector now defines all data ring just gives position
									dethitB.SetRingNumber(fS3RingHits[i].GetRing());
									if(TGRSIRunInfo::IsWaveformFitting()){
										dethitB.SetTimeFit(fS3SectorHits[k].GetFitTime());
										dethitB.SetSig2Noise(fS3SectorHits[k].GetSignalToNoise());
									}
									fS3Hits.push_back(dethitB);

									UsedRing[i]=true;
									UsedSector[j]=true;
									UsedSector[k]=true;
								}
							}
						}
					}
				} //End Shared Ring loop
			} 


			ringcount = 0;
			sectorcount = 0;
			for(unsigned int i=0;i<UsedRing.size();i++)
				if(!UsedRing.at(i))
					ringcount++;

			for(unsigned int i=0;i<UsedSector.size();i++)
				if(!UsedSector.at(i))
					sectorcount++;

			if(ringcount>1||sectorcount>1){

				//Shared Sector loop
				for(size_t i = 0; i < fS3SectorHits.size(); ++i) {
					if(UsedSector.at(i))
						continue;
					for(size_t j = 0; j < fS3RingHits.size(); ++j) {
						if(UsedRing.at(j))
							continue;   
						for(size_t k = j+1; k < fS3RingHits.size(); ++k) {  
						if(UsedRing.at(k))
							continue;
		
							if(abs(fS3SectorHits[i].GetCfd()-fS3RingHits[j].GetCfd()) < fFrontBackTime
								&& abs(fS3SectorHits[i].GetCfd()-fS3RingHits[k].GetCfd()) < fFrontBackTime){ //first check time
								if(EneS[i]*fFrontBackEnergy<(EneR[j]+EneR[k])&&
									(EneR[j]+EneR[k])*fFrontBackEnergy<EneS[i]){  //if time is good check energy
							
									//Now we have accepted a good event, build it
									TS3Hit dethit = fS3RingHits[j]; // Ring defines all data sector just gives position
									dethit.SetSectorNumber(fS3SectorHits[i].GetSector());
									if(TGRSIRunInfo::IsWaveformFitting()){
										dethit.SetTimeFit(fS3RingHits[j].GetFitTime());
										dethit.SetSig2Noise(fS3RingHits[j].GetSignalToNoise());
									}
									fS3Hits.push_back(dethit);

									//Now we have accepted a good event, build it
									TS3Hit dethitB = fS3RingHits[k]; // Ring defines all data sector just gives position
									dethitB.SetSectorNumber(fS3SectorHits[i].GetSector());
									if(TGRSIRunInfo::IsWaveformFitting()){
										dethitB.SetTimeFit(fS3RingHits[k].GetFitTime());
										dethitB.SetSig2Noise(fS3RingHits[k].GetSignalToNoise());
									}
									fS3Hits.push_back(dethitB);

									UsedRing[i]=true;
									UsedSector[j]=true;
									UsedSector[k]=true;

								}
							}
						}
					}
				} //End Shared Sector loop

			}

		}

		SetBitNumber(kPixelsSet, true);
	}

}

TVector3 TS3::GetPosition(int ring, int sector, bool downstream, double offset)  {
  TVector3 position;
	double dist = 0;
	if(offset == 0)
		dist = fTargetDistance;
	else
		dist = offset;
  double ring_width=(fOuterDiameter-fInnerDiameter)*0.5/fRingNumber; // 24 rings   radial width!
  double inner_radius=fInnerDiameter/2.0;
  double correctedsector = 6+sector; //moe is currently checking.
  double phi     =  2.*TMath::Pi()/fSectorNumber * (correctedsector + 0.5);   //the phi angle....
  double radius =  inner_radius + ring_width * (ring + 0.5) ;
	if(downstream)
 		position.SetMagThetaPhi(sqrt(radius*radius + dist*dist),atan((radius/dist)),phi+fOffsetPhi);
	else
 		position.SetMagThetaPhi(sqrt(radius*radius + dist*dist),TMath::Pi() - atan((radius/dist)),phi+fOffsetPhi);

  return position;
}

TGRSIDetectorHit* TS3::GetHit(const int& idx){
   return GetS3Hit(idx);
}

TS3Hit *TS3::GetS3Hit(const int& i) {  
  if(i < GetPixelMultiplicity()) {
    return &fS3Hits.at(i);
  } else {
    std::cerr << "S3 pixel hits are out of range" << std::endl;
    throw grsi::exit_exception(1);
    return NULL;
  }
}  

/*void TS3::PushBackHit(TGRSIDetectorHit *deshit) {
  fS3Hits.push_back(*((TS3Hit*)deshit));
  return;
}
*/

void TS3::Print(Option_t *opt) const {
   printf("%s\tnot yet written.\n",__PRETTY_FUNCTION__);
}

void TS3::Clear(Option_t *opt) {
  TGRSIDetector::Clear(opt);
  fS3Hits.clear();
	fS3RingHits.clear();
	fS3SectorHits.clear();
  fS3_RingFragment.clear();
  fS3_SectorFragment.clear();
  fRingNumber=24;
  fSectorNumber=32;
  fOffsetPhi=15*TMath::Pi()/180.; // according to dave.
  fOuterDiameter=70.;
  fInnerDiameter=22.;
  fTargetDistance=31.;
  
  fFrontBackTime=75;   
  fFrontBackEnergy=0.9; 
	SetPixels(false);
	SetMultiHit(false);
}


