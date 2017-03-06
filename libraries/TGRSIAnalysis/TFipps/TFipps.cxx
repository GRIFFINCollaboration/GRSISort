#include "TFipps.h"

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
// TFipps
//
// The TFipps class defines the observables and algorithms used
// when analyzing FIPPS data. It includes detector positions,
// add-back methods, etc. 
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TFipps)
/// \endcond

bool DefaultAddback(TFippsHit& one, TFippsHit& two) {
	return ((one.GetDetector() == two.GetDetector()) &&
	(std::fabs(one.GetTime() - two.GetTime()) < TGRSIRunInfo::AddBackWindow()));
}

std::function<bool(TFippsHit&, TFippsHit&)> TFipps::fAddbackCriterion = DefaultAddback;

//This seems unnecessary, and why 17?;//  they are static members, and need
//  to be defined outside the header
//  17 is to have the detectors go from 1-16 
//  plus we can use position zero 
//  when the detector winds up back in 
//  one of the stands like Alex used in the
//  gps run. pcb.
//Initiallizes the HPGe Clover positions as per the wiki <https://www.triumf.info/wiki/tigwiki/index.php/HPGe_Coordinate_Table>
//                                                                             theta                                 phi                                 theta                                phi                                 theta
TVector3 TFipps::gCloverPosition[17] = { 
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
const Double_t TFipps::gStrongCT[2] = { -0.02674, -0.000977 }; //This is for the 0-1 and 2-3 combination
const Double_t TFipps::gWeakCT[2]   = { 0.005663, - 0.00028014};
const Double_t TFipps::gCrossTalkPar[2][4][4] = {
	{ {0.0, gStrongCT[0], gWeakCT[0], gWeakCT[0]}, {gStrongCT[0], 0.0, gWeakCT[0], gWeakCT[0]}, {gWeakCT[0], gWeakCT[0], 0.0, gStrongCT[0]}, {gWeakCT[0], gWeakCT[0], gStrongCT[0], 0.0}},
	{ {0.0, gStrongCT[1], gWeakCT[1], gWeakCT[1]}, {gStrongCT[1], 0.0, gWeakCT[1], gWeakCT[1]}, {gWeakCT[1], gWeakCT[1], 0.0, gStrongCT[1]}, {gWeakCT[1], gWeakCT[1], gStrongCT[1], 0.0}}};

TFipps::TFipps() : TGRSIDetector() {
	//Default ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TFipps::TFipps(const TFipps& rhs) : TGRSIDetector() {
	//Copy ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TFipps::Copy(TObject &rhs) const {
	//Copy function.
	TGRSIDetector::Copy(rhs);

	static_cast<TFipps&>(rhs).fFippsHits        = fFippsHits;
	static_cast<TFipps&>(rhs).fAddbackHits        = fAddbackHits;
	static_cast<TFipps&>(rhs).fAddbackFrags       = fAddbackFrags;
	static_cast<TFipps&>(rhs).fFippsBits               = 0;
}                                       

TFipps::~TFipps()	{
	//Default Destructor
}

void TFipps::Clear(Option_t *opt)	{
	//Clears the mother, and all of the hits
	//  if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
	ClearStatus();
	//  }
	TGRSIDetector::Clear(opt);
	fFippsHits.clear();
	fAddbackHits.clear();
	fAddbackFrags.clear();
	//fFippsBits.Class()->IgnoreTObjectStreamer(kTRUE);
}


void TFipps::Print(Option_t *opt) const {
	std::cout << "Fipps Contains: " << std::endl;
	std::cout << std::setw(6) << GetMultiplicity() << " hits"  << std::endl;

	if(IsAddbackSet()) 
		std::cout << std::setw(6) << fAddbackHits.size() << " addback hits" << std::endl; 
	else
		std::cout << std::setw(6) << " " << " Addback not set" << std::endl;

	std::cout << std::setw(6) << " " << " Cross-talk Set?  " << IsCrossTalkSet() << std::endl;
}

TFipps& TFipps::operator=(const TFipps& rhs) {
	rhs.Copy(*this);
	return *this;
}

Int_t TFipps::GetMultiplicity() const{
	return fFippsHits.size();
}

std::vector<TFippsHit> *TFipps::GetHitVector() {
	return &fFippsHits;
}

std::vector<TFippsHit> *TFipps::GetAddbackVector() {
	return &fAddbackHits;
}

std::vector<UShort_t> *TFipps::GetAddbackFragVector() {
	return &fAddbackFrags;
}

bool TFipps::IsAddbackSet() const{
	return TestBitNumber(kIsAddbackSet);
}

bool TFipps::IsCrossTalkSet() const{
	return TestBitNumber(kIsCrossTalkSet);
}

void TFipps::SetAddback(const Bool_t flag) const{
	return SetBitNumber(kIsAddbackSet,flag);
}

void TFipps::SetCrossTalk(const Bool_t flag) const{
	return SetBitNumber(kIsCrossTalkSet,flag);
}

TGRSIDetectorHit* TFipps::GetHit(const Int_t& idx) {
	return GetFippsHit(idx);
}

TFippsHit* TFipps::GetFippsHit(const int& i) {
	try {
		if(!IsCrossTalkSet()){
			FixCrossTalk();
		}
		return &(GetHitVector()->at(i));
	} catch (const std::out_of_range& oor) {
		std::cerr << ClassName() << " Hits are out of range: " << oor.what() << std::endl;
		if(!gInterpreter)
			throw grsi::exit_exception(1);
	}
	return nullptr;
}

Int_t TFipps::GetAddbackMultiplicity()  {
	// Automatically builds the addback hits using the fAddbackCriterion (if the size of the fAddbackHits vector is zero) and return the number of addback hits.
	if(!IsCrossTalkSet()){
		//Calculate Cross Talk on each hit
		FixCrossTalk();
	} 
	auto hit_vec = GetHitVector();
	auto ab_vec = GetAddbackVector();
	auto frag_vec = GetAddbackFragVector();
	if(hit_vec->size() == 0) {
		return 0;
	}
	//if the addback has been reset, clear the addback hits
	if(!IsAddbackSet()) {
		ab_vec->clear();
	}
	if(ab_vec->size() == 0) {
		// use the first fipps hit as starting point for the addback hits
		ab_vec->push_back(hit_vec->at(0));
		frag_vec->push_back(1);

		// loop over remaining fipps hits
		size_t i, j;
		for(i = 1; i < hit_vec->size(); ++i) {
			// check for each existing addback hit if this fipps hit should be added
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
		SetAddback(true);
	}


	return ab_vec->size();
}


TFippsHit* TFipps::GetAddbackHit(const int& i) {
	if(i < GetAddbackMultiplicity()) {
		return &GetAddbackVector()->at(i);
	} else {
		std::cerr << "Addback hits are out of range" << std::endl;
		throw grsi::exit_exception(1);
		return nullptr;
	}
}

void TFipps::AddFragment(std::shared_ptr<const TFragment> frag, TChannel* chan) {
	//Builds the FIPPS Hits directly from the TFragment. Basically, loops through the hits for an event and sets observables. 
	//This is done for both FIPPS and it's suppressors.
	if(frag == nullptr || chan == nullptr) {
		return;
	}

	switch(chan->GetMnemonic()->SubSystem()) {
		case TMnemonic::kG :
			TFippsHit geHit(*frag);
			GetHitVector()->push_back(std::move(geHit));
	};
}

TVector3 TFipps::GetPosition(int DetNbr,int CryNbr, double dist ) {
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

void TFipps::ResetFlags() const{
	fFippsBits = 0;
}

void TFipps::ResetAddback() {
	SetAddback(false);
	SetCrossTalk(false);
	GetAddbackVector()->clear();
	GetAddbackFragVector()->clear();
}

UShort_t TFipps::GetNAddbackFrags(const size_t &idx) {
	//Get the number of addback "fragments" contributing to the total addback hit
	//with index idx.
	if(idx < GetAddbackFragVector()->size())
		return GetAddbackFragVector()->at(idx);   
	else
		return 0;
}

void TFipps::SetBitNumber(enum EFippsBits bit,Bool_t set) const{
	//Used to set the flags that are stored in TFipps.
	fFippsBits.SetBit(bit,set);
	/*	if(set)
		fFippsBits |= bit;
		else
		fFippsBits &= (~bit);*/
}

Double_t TFipps::CTCorrectedEnergy(const TFippsHit* const hit_to_correct, const TFippsHit* const other_hit, Bool_t time_constraint){
	if(!hit_to_correct || !other_hit){
		printf("One of the hits is invalid in TFipps::CTCorrectedEnergy\n");
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

void TFipps::FixCrossTalk() {
	auto hit_vec = GetHitVector();
	if(hit_vec->size() < 2) {
		SetCrossTalk(true);
		return;
	}
	for(size_t i=0; i<hit_vec->size(); ++i)
		hit_vec->at(i).ClearEnergy();

	if(TGRSIRunInfo::Get()->IsCorrectingCrossTalk()){
		size_t i, j;
		for(i = 0; i < hit_vec->size(); ++i) {
			for(j = i+1; j < hit_vec->size(); ++j) {
				hit_vec->at(i).SetEnergy(TFipps::CTCorrectedEnergy(&(hit_vec->at(i)),&(hit_vec->at(j))));
				hit_vec->at(j).SetEnergy(TFipps::CTCorrectedEnergy(&(hit_vec->at(j)),&(hit_vec->at(i))));
			}
		}
	}
	SetCrossTalk(true);
}
