#include "TGriffin.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include "TRandom.h"
#include "TMath.h"
#include "TInterpreter.h"
#include "TMnemonic.h"

#include "TGRSIRunInfo.h"

////////////////////////////////////////////////////////////
//                    
// TGriffin
//
// The TGriffin class defines the observables and algorithms used
// when analyzing GRIFFIN data. It includes detector positions,
// add-back methods, etc. 
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TGriffin)
	/// \endcond

	bool DefaultAddback(TGriffinHit& one, TGriffinHit& two) {
		return ((one.GetDetector() == two.GetDetector()) &&
		(std::fabs(one.GetTime() - two.GetTime()) < TGRSIRunInfo::AddBackWindow()));
	}

std::function<bool(TGriffinHit&, TGriffinHit&)> TGriffin::fAddbackCriterion = DefaultAddback;

bool TGriffin::fSetCoreWave = false;
Int_t TGriffin::fDefaultGainType = kLowGain;

//This seems unnecessary, and why 17?;//  they are static members, and need
//  to be defined outside the header
//  17 is to have the detectors go from 1-16 
//  plus we can use position zero 
//  when the detector winds up back in 
//  one of the stands like Alex used in the
//  gps run. pcb.
//Initiallizes the HPGe Clover positions as per the wiki <https://www.triumf.info/wiki/tigwiki/index.php/HPGe_Coordinate_Table>
//                                                                             theta                                 phi                                 theta                                phi                                 theta
TVector3 TGriffin::gCloverPosition[17] = { 
	TVector3(TMath::Sin(TMath::DegToRad()*(0.0))*TMath::Cos(TMath::DegToRad()*(0.0)), TMath::Sin(TMath::DegToRad()*(0.0))*TMath::Sin(TMath::DegToRad()*(0.0)), TMath::Cos(TMath::DegToRad()*(0.0))),
	//Downstream lampshade
	TVector3(TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Cos(TMath::DegToRad()*(67.5)), TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Sin(TMath::DegToRad()*(67.5)), TMath::Cos(TMath::DegToRad()*(45.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Cos(TMath::DegToRad()*(157.5)), TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Sin(TMath::DegToRad()*(157.5)), TMath::Cos(TMath::DegToRad()*(45.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Cos(TMath::DegToRad()*(247.5)), TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Sin(TMath::DegToRad()*(247.5)), TMath::Cos(TMath::DegToRad()*(45.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Cos(TMath::DegToRad()*(337.5)), TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Sin(TMath::DegToRad()*(337.5)), TMath::Cos(TMath::DegToRad()*(45.0))),
	//Corona
	TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(22.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(22.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(67.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(67.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(112.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(112.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(157.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(157.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(202.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(202.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(247.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(247.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(292.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(292.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(337.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(337.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
	//Upstream lampshade
	TVector3(TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Cos(TMath::DegToRad()*(67.5)), TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Sin(TMath::DegToRad()*(67.5)), TMath::Cos(TMath::DegToRad()*(135.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Cos(TMath::DegToRad()*(157.5)), TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Sin(TMath::DegToRad()*(157.5)), TMath::Cos(TMath::DegToRad()*(135.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Cos(TMath::DegToRad()*(247.5)), TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Sin(TMath::DegToRad()*(247.5)), TMath::Cos(TMath::DegToRad()*(135.0))),
	TVector3(TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Cos(TMath::DegToRad()*(337.5)), TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Sin(TMath::DegToRad()*(337.5)), TMath::Cos(TMath::DegToRad()*(135.0)))
};

//Cross Talk stuff
const Double_t TGriffin::gStrongCT[2] = { -0.02674, -0.000977 }; //This is for the 0-1 and 2-3 combination
const Double_t TGriffin::gWeakCT[2]   = { 0.005663, - 0.00028014};
const Double_t TGriffin::gCrossTalkPar[2][4][4] = {
	{ {0.0, gStrongCT[0], gWeakCT[0], gWeakCT[0]}, {gStrongCT[0], 0.0, gWeakCT[0], gWeakCT[0]}, {gWeakCT[0], gWeakCT[0], 0.0, gStrongCT[0]}, {gWeakCT[0], gWeakCT[0], gStrongCT[0], 0.0}},
	{ {0.0, gStrongCT[1], gWeakCT[1], gWeakCT[1]}, {gStrongCT[1], 0.0, gWeakCT[1], gWeakCT[1]}, {gWeakCT[1], gWeakCT[1], 0.0, gStrongCT[1]}, {gWeakCT[1], gWeakCT[1], gStrongCT[1], 0.0}}};

TGriffin::TGriffin() : TGRSIDetector() {
	//Default ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TGriffin::TGriffin(const TGriffin& rhs) : TGRSIDetector() {
	//Copy ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TGriffin::Copy(TObject &rhs) const {
	//Copy function.
	TGRSIDetector::Copy(rhs);

	static_cast<TGriffin&>(rhs).fGriffinLowGainHits        = fGriffinLowGainHits;
	static_cast<TGriffin&>(rhs).fGriffinHighGainHits       = fGriffinHighGainHits;
	static_cast<TGriffin&>(rhs).fAddbackLowGainHits        = fAddbackLowGainHits;
	static_cast<TGriffin&>(rhs).fAddbackHighGainHits       = fAddbackHighGainHits;
	static_cast<TGriffin&>(rhs).fAddbackLowGainFrags       = fAddbackLowGainFrags;
	static_cast<TGriffin&>(rhs).fAddbackHighGainFrags      = fAddbackHighGainFrags;
	static_cast<TGriffin&>(rhs).fSetCoreWave               = fSetCoreWave;
	static_cast<TGriffin&>(rhs).fCycleStart                = fCycleStart;
	static_cast<TGriffin&>(rhs).fGriffinBits               = 0;

}                                       

TGriffin::~TGriffin()	{
	//Default Destructor
}

void TGriffin::Clear(Option_t *opt)	{
	//Clears the mother, and all of the hits
	//  if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
	ClearStatus();
	//  }
	TGRSIDetector::Clear(opt);
	fGriffinLowGainHits.clear();
	fGriffinHighGainHits.clear();
	fAddbackLowGainHits.clear();
	fAddbackHighGainHits.clear();
	fAddbackLowGainFrags.clear();
	fAddbackHighGainFrags.clear();
	fCycleStart = 0;
	//fGriffinBits.Class()->IgnoreTObjectStreamer(kTRUE);
}


void TGriffin::Print(Option_t *opt) const {
	std::cout << "Griffin Contains: " << std::endl;
	std::cout << std::setw(6) << GetLowGainMultiplicity() << " Low gain hits"  << std::endl;
	std::cout << std::setw(6) << GetHighGainMultiplicity() << " High gain hits"  << std::endl;

	if(IsAddbackSet(kLowGain)) 
		std::cout << std::setw(6) << fAddbackLowGainHits.size() << " Low gain addback hits" << std::endl; 
	else
		std::cout << std::setw(6) << " " << " Low Gain Addback not set" << std::endl;

	if(IsAddbackSet(kHighGain))  
		std::cout << std::setw(6) << fAddbackHighGainHits.size() << " High gain addback hits" << std::endl;  
	else
		std::cout << std::setw(6) << " " << " High Gain Addback not set" << std::endl;

	std::cout << std::setw(6) << " " << " Cross-talk Set?  Low gain: " << IsCrossTalkSet(kLowGain) << "   High gain: " << IsCrossTalkSet(kHighGain) << std::endl;
	std::cout << std::setw(6) << fCycleStart  << " cycle start" << std::endl;
}

TGriffin& TGriffin::operator=(const TGriffin& rhs) {
	rhs.Copy(*this);
	return *this;
}

void TGriffin::SetDefaultGainType(const Int_t &gain_type) { 
	if((gain_type == kLowGain) || (gain_type == kHighGain)){
		 fDefaultGainType = gain_type;
	} else {
		std::cout << gain_type << " is not a known gain type. Please use kLowGain or kHighGain" << std::endl;
	}
}

Int_t TGriffin::GetMultiplicity(const Int_t &gain_type) const{
	switch(gain_type){
		case kLowGain:
			return fGriffinLowGainHits.size();
		case kHighGain:
			return fGriffinHighGainHits.size();
	};
	return 0;
}

std::vector<TGriffinHit> *TGriffin::GetHitVector(const Int_t &gain_type) {
	switch(gain_type){
		case kLowGain:
			return &fGriffinLowGainHits;
		case kHighGain:
			return &fGriffinHighGainHits;
	};
	return nullptr;
}

std::vector<TGriffinHit> *TGriffin::GetAddbackVector(const Int_t &gain_type) {
	switch(gain_type){
		case kLowGain:
			return &fAddbackLowGainHits;
		case kHighGain:
			return &fAddbackHighGainHits;
	};
	return nullptr;
}

std::vector<UShort_t> *TGriffin::GetAddbackFragVector(const Int_t &gain_type) {
	switch(gain_type){
		case kLowGain:
			return &fAddbackLowGainFrags;
		case kHighGain:
			return &fAddbackHighGainFrags;
	};
	return nullptr;
}

bool TGriffin::IsAddbackSet(const Int_t &gain_type) const{
	switch(gain_type){
		case kLowGain:
			return TestBitNumber(kIsLowGainAddbackSet);
		case kHighGain:
			return TestBitNumber(kIsHighGainAddbackSet);
	};
	return false;
}

bool TGriffin::IsCrossTalkSet(const Int_t &gain_type) const{
	switch(gain_type){
		case kLowGain:
			return TestBitNumber(kIsLowGainCrossTalkSet);
		case kHighGain:
			return TestBitNumber(kIsHighGainCrossTalkSet);
	};
	return false;
}

void TGriffin::SetAddback(const Int_t &gain_type,const Bool_t flag) const{
	switch(gain_type){
		case kLowGain:
			return SetBitNumber(kIsLowGainAddbackSet,flag);
		case kHighGain:
			return SetBitNumber(kIsHighGainAddbackSet,flag);
	};
}

void TGriffin::SetCrossTalk(const Int_t &gain_type, const Bool_t flag) const{
	switch(gain_type){
		case kLowGain:
			return SetBitNumber(kIsLowGainCrossTalkSet,flag);
		case kHighGain:
			return SetBitNumber(kIsHighGainCrossTalkSet,flag);
	};
}

TGRSIDetectorHit* TGriffin::GetHit(const Int_t& idx) {
	return GetGriffinHit(idx);
}

TGriffinHit* TGriffin::GetGriffinLowGainHit(const int& i) {
	return GetGriffinHit(i,kLowGain);
}

TGriffinHit* TGriffin::GetGriffinHighGainHit(const int& i) {
	return GetGriffinHit(i,kHighGain);
}

TGriffinHit* TGriffin::GetGriffinHit(const int& i, const Int_t &gain_type) {
	try {
		if(!IsCrossTalkSet(gain_type)){
			FixCrossTalk(gain_type);
		}
		return &(GetHitVector(gain_type)->at(i));
	} catch (const std::out_of_range& oor) {
		std::cerr << ClassName() << " Hits are out of range: " << oor.what() << std::endl;
		if(!gInterpreter)
			throw grsi::exit_exception(1);
	}
	return NULL;
}

Int_t TGriffin::GetAddbackLowGainMultiplicity() {
	return GetAddbackMultiplicity(kLowGain);
}

Int_t TGriffin::GetAddbackHighGainMultiplicity() {
	return GetAddbackMultiplicity(kHighGain);
}

Int_t TGriffin::GetAddbackMultiplicity(const Int_t &gain_type)  {
	// Automatically builds the addback hits using the fAddbackCriterion (if the size of the fAddbackHits vector is zero) and return the number of addback hits.
	if(!IsCrossTalkSet(gain_type)){
		//Calculate Cross Talk on each hit
		FixCrossTalk(gain_type);
	} 
	auto hit_vec = GetHitVector(gain_type);
	auto ab_vec = GetAddbackVector(gain_type);
	auto frag_vec = GetAddbackFragVector(gain_type);
	if(hit_vec->size() == 0) {
		return 0;
	}
	//if the addback has been reset, clear the addback hits
	if(!IsAddbackSet(gain_type)) {
		ab_vec->clear();
	}
	if(ab_vec->size() == 0) {
		// use the first griffin hit as starting point for the addback hits
		ab_vec->push_back(hit_vec->at(0));
		frag_vec->push_back(1);

		// loop over remaining griffin hits
		size_t i, j;
		for(i = 1; i < hit_vec->size(); ++i) {
			// check for each existing addback hit if this griffin hit should be added
			for(j = 0; j < ab_vec->size(); ++j) {
				if(fAddbackCriterion(ab_vec->at(j), hit_vec->at(i))) {
					ab_vec->at(j).Add(&(hit_vec->at(i)));
					frag_vec->at(j)++;
					break;
				}
			}
			if(j == ab_vec->size()) {
				ab_vec->push_back(hit_vec->at(i));
				frag_vec->push_back(1);
			}
		}
		SetAddback(gain_type,true);
	}


	return ab_vec->size();
}


TGriffinHit* TGriffin::GetAddbackLowGainHit(const int& i) {
	return GetAddbackHit(i,kLowGain);
}

TGriffinHit* TGriffin::GetAddbackHighGainHit(const int& i) {
	return GetAddbackHit(i,kHighGain);
}

TGriffinHit* TGriffin::GetAddbackHit(const int& i, const Int_t &gain_type) {
	if(i < GetAddbackMultiplicity(gain_type)) {
		return &GetAddbackVector(gain_type)->at(i);
	} else {
		std::cerr << "Addback hits are out of range" << std::endl;
		throw grsi::exit_exception(1);
		return NULL;
	}
}

void TGriffin::AddFragment(std::shared_ptr<const TFragment> frag, TChannel* chan) {
	//Builds the GRIFFIN Hits directly from the TFragment. Basically, loops through the hits for an event and sets observables. 
	//This is done for both GRIFFIN and it's suppressors.
	if(frag == NULL || chan == NULL) {
		return;
	}
	if(chan->GetMnemonic()->OutputSensor() == TMnemonic::kA) {  }  

	switch(chan->GetMnemonic()->SubSystem()){
		case TMnemonic::kG :
			TGriffinHit geHit(*frag);
			switch(chan->GetMnemonic()->OutputSensor()){
				case TMnemonic::kA :
					GetHitVector(kLowGain)->push_back(std::move(geHit));
					break;
				case TMnemonic::kB :
					GetHitVector(kHighGain)->push_back(std::move(geHit));
					break;
			};
			//     case TMnemonic::kS :
			//do supressor stuff in the future
			//      break;
	};
}

TVector3 TGriffin::GetPosition(int DetNbr,int CryNbr, double dist ) {
	//Gets the position vector for a crystal specified by CryNbr within Clover DetNbr at a distance of dist mm away.
	//This is calculated to the most likely interaction point within the crystal.
	if(DetNbr>16)
		return TVector3(0,0,1);

	TVector3 temp_pos(gCloverPosition[DetNbr]);

	//Interaction points may eventually be set externally. May make these members of each crystal, or pass from waveforms.
	Double_t cp = 26.0; //Crystal Center Point  mm.
	Double_t id = 45.0;//45.0;  //Crystal interaction depth mm.
	//Set Theta's of the center of each DETECTOR face
	////Define one Detector position
	TVector3 shift;
	switch(CryNbr) {
		case 0:
			shift.SetXYZ(-cp,cp,id);
			break;
		case 1:
			shift.SetXYZ(cp,cp,id);
			break;
		case 2:
			shift.SetXYZ(cp,-cp,id);
			break;
		case 3:
			shift.SetXYZ(-cp,-cp,id);
			break;
		default:
			shift.SetXYZ(0,0,1);
			break;
	};
	shift.RotateY(temp_pos.Theta());
	shift.RotateZ(temp_pos.Phi());

	temp_pos.SetMag(dist);

	return (temp_pos + shift);

}

void TGriffin::ResetFlags() const{
	fGriffinBits = 0;
}


void TGriffin::ResetLowGainAddback() {
	ResetAddback(kLowGain);
}

void TGriffin::ResetHighGainAddback() {
	ResetAddback(kLowGain);
}

void TGriffin::ResetAddback(const Int_t &gain_type) {
	SetAddback(gain_type, false);
	SetCrossTalk(gain_type,false);
	GetAddbackVector(gain_type)->clear();
	GetAddbackFragVector(gain_type)->clear();
}

UShort_t TGriffin::GetNLowGainAddbackFrags(const size_t &idx) {
	return GetNAddbackFrags(idx,kLowGain);
}

UShort_t TGriffin::GetNHighGainAddbackFrags(const size_t &idx) {
	return GetNAddbackFrags(idx,kHighGain);
}

UShort_t TGriffin::GetNAddbackFrags(const size_t &idx, const Int_t &gain_type) {
	//Get the number of addback "fragments" contributing to the total addback hit
	//with index idx.
	if(idx < GetAddbackFragVector(gain_type)->size())
		return GetAddbackFragVector(gain_type)->at(idx);   
	else
		return 0;
}

void TGriffin::SetBitNumber(enum EGriffinBits bit,Bool_t set) const{
	//Used to set the flags that are stored in TGriffin.
	fGriffinBits.SetBit(bit,set);
/*	if(set)
		fGriffinBits |= bit;
	else
		fGriffinBits &= (~bit);*/
}

Double_t TGriffin::CTCorrectedEnergy(const TGriffinHit* const hit_to_correct, const TGriffinHit* const other_hit, Bool_t time_constraint){
	if(!hit_to_correct || !other_hit){
		printf("One of the hits is invalid in TGriffin::CTCorrectedEnergy\n");
		return 0;
	}

	if(time_constraint){
		//Figure out if this passes the selected window
		if(TMath::Abs(other_hit->GetTime() - hit_to_correct->GetTime()) > TGRSIRunInfo::AddBackWindow()) //placeholder
			return hit_to_correct->GetEnergy();
	}

	if(hit_to_correct->GetDetector() != other_hit->GetDetector() ){
		return hit_to_correct->GetEnergy();
	}
	return hit_to_correct->GetEnergy() - (gCrossTalkPar[0][hit_to_correct->GetCrystal()][other_hit->GetCrystal()] + gCrossTalkPar[1][hit_to_correct->GetCrystal()][other_hit->GetCrystal()]*other_hit->GetNoCTEnergy());

}

void TGriffin::FixLowGainCrossTalk() {
	FixCrossTalk(kLowGain);
}

void TGriffin::FixHighGainCrossTalk() {
	FixCrossTalk(kHighGain);	
}


void TGriffin::FixCrossTalk(const Int_t& gain_type) {
	auto hit_vec = GetHitVector(gain_type);
	if(hit_vec->size() < 2) {
		SetCrossTalk(gain_type,true);
		return;
	}
	for(size_t i=0; i<hit_vec->size(); ++i)
		hit_vec->at(i).ClearEnergy();

	if(TGRSIRunInfo::Get()->IsCorrectingCrossTalk()){
		size_t i, j;
		for(i = 0; i < hit_vec->size(); ++i) {
			for(j = i+1; j < hit_vec->size(); ++j) {
				hit_vec->at(i).SetEnergy(TGriffin::CTCorrectedEnergy(&(hit_vec->at(i)),&(hit_vec->at(j))));
				hit_vec->at(j).SetEnergy(TGriffin::CTCorrectedEnergy(&(hit_vec->at(j)),&(hit_vec->at(i))));
			}
		}
	}
	SetCrossTalk(gain_type,true);
}
