#include "TTdrClover.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include "TRandom.h"
#include "TMath.h"
#include "TInterpreter.h"
#include "TMnemonic.h"

#include "TGRSIOptions.h"

////////////////////////////////////////////////////////////
//
// TTdrClover
//
// The TTdrClover class defines the observables and algorithms used
// when analyzing GRIFFIN data. It includes detector positions,
// add-back methods, etc.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TTdrClover)
/// \endcond

bool DefaultAddback(TTdrCloverHit& one, TTdrCloverHit& two)
{
   return ((one.GetDetector() == two.GetDetector()) &&
           (std::fabs(one.GetTime() - two.GetTime()) < TGRSIOptions::AnalysisOptions()->AddbackWindow()));
}

std::function<bool(TTdrCloverHit&, TTdrCloverHit&)> TTdrClover::fAddbackCriterion = DefaultAddback;

bool DefaultSuppression(TTdrCloverHit& clo, TBgoHit& bgo)
{
   return ((clo.GetDetector() == bgo.GetDetector()) &&
           (std::fabs(clo.GetTime() - bgo.GetCorrectedTime()) < TGRSIOptions::AnalysisOptions()->SuppressionWindow()) &&
			  (bgo.GetEnergy() > TGRSIOptions::AnalysisOptions()->SuppressionEnergy()));
}

std::function<bool(TTdrCloverHit&, TBgoHit&)> TTdrClover::fSuppressionCriterion = DefaultSuppression;

bool  TTdrClover::fSetCoreWave     = false;

// This seems unnecessary, and why 17?;//  they are static members, and need
//  to be defined outside the header
//  17 is to have the detectors go from 1-16
//  plus we can use position zero
//  when the detector winds up back in
//  one of the stands like Alex used in the
//  gps run. pcb.
// Initiallizes the HPGe Clover positions as per the wiki
// <https://www.triumf.info/wiki/tigwiki/index.php/HPGe_Coordinate_Table>
//                                                                             theta                                 phi
//                                                                             theta                                phi
//                                                                             theta
TVector3 TTdrClover::gCloverPosition[17] = {
   TVector3(TMath::Sin(TMath::DegToRad() * (0.0)) * TMath::Cos(TMath::DegToRad() * (0.0)),
            TMath::Sin(TMath::DegToRad() * (0.0)) * TMath::Sin(TMath::DegToRad() * (0.0)),
            TMath::Cos(TMath::DegToRad() * (0.0))),
   // Downstream lampshade
   TVector3(TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Cos(TMath::DegToRad() * (67.5)),
            TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Sin(TMath::DegToRad() * (67.5)),
            TMath::Cos(TMath::DegToRad() * (45.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Cos(TMath::DegToRad() * (157.5)),
            TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Sin(TMath::DegToRad() * (157.5)),
            TMath::Cos(TMath::DegToRad() * (45.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Cos(TMath::DegToRad() * (247.5)),
            TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Sin(TMath::DegToRad() * (247.5)),
            TMath::Cos(TMath::DegToRad() * (45.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Cos(TMath::DegToRad() * (337.5)),
            TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Sin(TMath::DegToRad() * (337.5)),
            TMath::Cos(TMath::DegToRad() * (45.0))),
   // Corona
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (22.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (22.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (67.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (67.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (112.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (112.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (157.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (157.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (202.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (202.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (247.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (247.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (292.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (292.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (337.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (337.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   // Upstream lampshade
   TVector3(TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Cos(TMath::DegToRad() * (67.5)),
            TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Sin(TMath::DegToRad() * (67.5)),
            TMath::Cos(TMath::DegToRad() * (135.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Cos(TMath::DegToRad() * (157.5)),
            TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Sin(TMath::DegToRad() * (157.5)),
            TMath::Cos(TMath::DegToRad() * (135.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Cos(TMath::DegToRad() * (247.5)),
            TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Sin(TMath::DegToRad() * (247.5)),
            TMath::Cos(TMath::DegToRad() * (135.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Cos(TMath::DegToRad() * (337.5)),
            TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Sin(TMath::DegToRad() * (337.5)),
            TMath::Cos(TMath::DegToRad() * (135.0)))};

std::map<int, TSpline*> TTdrClover::fEnergyResiduals;

TTdrClover::TTdrClover() : TGRSIDetector()
{
// Default ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TTdrClover::TTdrClover(const TTdrClover& rhs) : TGRSIDetector()
{
// Copy ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

void TTdrClover::Copy(TObject& rhs) const
{
   // Copy function.
   TGRSIDetector::Copy(rhs);

   static_cast<TTdrClover&>(rhs).fTdrCloverHits   = fTdrCloverHits;
   static_cast<TTdrClover&>(rhs).fAddbackHits   = fAddbackHits;
   static_cast<TTdrClover&>(rhs).fAddbackFrags  = fAddbackFrags;
   static_cast<TTdrClover&>(rhs).fSuppressedHits   = fSuppressedHits;
   static_cast<TTdrClover&>(rhs).fSuppressedAddbackHits   = fSuppressedAddbackHits;
   static_cast<TTdrClover&>(rhs).fSuppressedAddbackFrags  = fSuppressedAddbackFrags;
   static_cast<TTdrClover&>(rhs).fSetCoreWave          = fSetCoreWave;
   static_cast<TTdrClover&>(rhs).fCycleStart           = fCycleStart;
   static_cast<TTdrClover&>(rhs).fTdrCloverBits          = 0;
}

TTdrClover::~TTdrClover()
{
   // Default Destructor
}

void TTdrClover::Clear(Option_t* opt)
{
   // Clears the mother, and all of the hits
   ClearStatus();
   TGRSIDetector::Clear(opt);
   fTdrCloverHits.clear();
   fAddbackHits.clear();
   fAddbackFrags.clear();
   fSuppressedHits.clear();
   fSuppressedAddbackHits.clear();
   fSuppressedAddbackFrags.clear();
   fCycleStart = 0;
}

void TTdrClover::Print(Option_t*) const
{
   std::cout<<"TdrClover Contains: "<<std::endl;
   std::cout<<std::setw(6)<<GetMultiplicity()<<" hits"<<std::endl;

   if(IsAddbackSet()) {
      std::cout<<std::setw(6)<<fAddbackHits.size()<<" addback hits"<<std::endl;
   } else {
      std::cout<<std::setw(6)<<" "<<" Addback not set"<<std::endl;
   }

   if(IsSuppressedSet()) {
      std::cout<<std::setw(6)<<fSuppressedHits.size()<<" suppressed hits"<<std::endl;
   } else {
      std::cout<<std::setw(6)<<" "<<" suppressed not set"<<std::endl;
   }

   if(IsSuppressedAddbackSet()) {
      std::cout<<std::setw(6)<<fSuppressedAddbackHits.size()<<" suppressed addback hits"<<std::endl;
   } else {
      std::cout<<std::setw(6)<<" "<<" suppressed Addback not set"<<std::endl;
   }

   std::cout<<std::setw(6)<<fCycleStart<<" cycle start"<<std::endl;
}

TTdrClover& TTdrClover::operator=(const TTdrClover& rhs)
{
   rhs.Copy(*this);
   return *this;
}

Short_t TTdrClover::GetMultiplicity() const
{
	return fTdrCloverHits.size();
}

std::vector<TTdrCloverHit>* TTdrClover::GetHitVector()
{
	return &fTdrCloverHits;
}

std::vector<TTdrCloverHit>* TTdrClover::GetAddbackVector()
{
	return &fAddbackHits;
}

std::vector<UShort_t>* TTdrClover::GetAddbackFragVector()
{
	return &fAddbackFrags;
}

bool TTdrClover::IsAddbackSet() const
{
	return TestBitNumber(ETdrCloverBits::kIsAddbackSet);
}

void TTdrClover::SetAddback(const Bool_t flag) const
{
	return SetBitNumber(ETdrCloverBits::kIsAddbackSet, flag);
}

std::vector<TTdrCloverHit>* TTdrClover::GetSuppressedVector()
{
	return &fSuppressedHits;
}

bool TTdrClover::IsSuppressedSet() const
{
	return TestBitNumber(ETdrCloverBits::kIsSuppressedSet);
}

void TTdrClover::SetSuppressed(const Bool_t flag) const
{
	return SetBitNumber(ETdrCloverBits::kIsSuppressedSet, flag);
}

std::vector<TTdrCloverHit>* TTdrClover::GetSuppressedAddbackVector()
{
	return &fSuppressedAddbackHits;
}

std::vector<UShort_t>* TTdrClover::GetSuppressedAddbackFragVector()
{
	return &fSuppressedAddbackFrags;
}

bool TTdrClover::IsSuppressedAddbackSet() const
{
	return TestBitNumber(ETdrCloverBits::kIsSupprAddbSet);
}

void TTdrClover::SetSuppressedAddback(const Bool_t flag) const
{
	return SetBitNumber(ETdrCloverBits::kIsSupprAddbSet, flag);
}

TGRSIDetectorHit* TTdrClover::GetHit(const Int_t& idx)
{
	return GetTdrCloverHit(idx);
}

TTdrCloverHit* TTdrClover::GetTdrCloverHit(const int& i)
{
	try {
		return &(GetHitVector()->at(i));
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" Hits are out of range: "<<oor.what()<<std::endl;
		if(!gInterpreter) {
			throw grsi::exit_exception(1);
		}
	}
	return nullptr;
}

Int_t TTdrClover::GetAddbackMultiplicity()
{
	// Automatically builds the addback hits using the fAddbackCriterion (if the size of the fAddbackHits vector is zero)
	// and return the number of addback hits.
	auto hit_vec  = GetHitVector();
	auto ab_vec   = GetAddbackVector();
	auto frag_vec = GetAddbackFragVector();
	if(hit_vec->empty()) {
		return 0;
	}
	// if the addback has been reset, clear the addback hits
	if(!IsAddbackSet()) {
		ab_vec->clear();
		frag_vec->clear();
	}
	if(ab_vec->empty()) {
		// use the first griffin hit as starting point for the addback hits
		ab_vec->push_back(hit_vec->at(0));
		frag_vec->push_back(1);

		// loop over remaining griffin hits
		size_t i, j;
		for(i = 1; i < hit_vec->size(); ++i) {
			// check for each existing addback hit if this griffin hit should be added
			for(j = 0; j < ab_vec->size(); ++j) {
				if(fAddbackCriterion(ab_vec->at(j), hit_vec->at(i))) {
					ab_vec->at(j).Add(&(hit_vec->at(i))); // copy constructor does not copy the bit field, so we set it.
					ab_vec->at(j).SetHitBit(TGRSIDetectorHit::EBitFlag::kIsEnergySet); // this must be set for summed hits.
					ab_vec->at(j).SetHitBit(TGRSIDetectorHit::EBitFlag::kIsTimeSet);   // this must be set for summed hits. pcb.
					(frag_vec->at(j))++;
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

TTdrCloverHit* TTdrClover::GetAddbackHit(const int& i)
{
	if(i < GetAddbackMultiplicity()) {
		return &GetAddbackVector()->at(i);
	}
	std::cerr<<"Addback hits are out of range"<<std::endl;
	throw grsi::exit_exception(1);
	return nullptr;
}

Int_t TTdrClover::GetSuppressedMultiplicity(TBgo* bgo)
{
	// Automatically builds the suppressed hits using the fSuppressedCriterion (if the size of the fSuppressedHits vector is zero)
	// and return the number of suppressed hits.
	if(fTdrCloverHits.empty()) {
		return 0;
	}
	// if the addback has been reset, clear the addback hits
	if(!IsSuppressedSet()) {
		fSuppressedHits.clear();
	}
	if(fSuppressedHits.empty()) {
		// loop over unsuppressed hits
		for(auto hit : fTdrCloverHits) {
			bool suppress = false;
			for(auto b : bgo->GetHitVector()) {
				if(fSuppressionCriterion(hit, b)) {
					suppress = true;
					break;
				}
			}
			if(!suppress) fSuppressedHits.push_back(hit);
		}
	}

	SetSuppressed(true);

	return fSuppressedHits.size();
}

TTdrCloverHit* TTdrClover::GetSuppressedHit(TBgo* bgo, const int& i)
{
	if(i < GetSuppressedMultiplicity(bgo)) {
		return &GetSuppressedVector()->at(i);
	}
	std::cerr<<"Suppressed hits are out of range"<<std::endl;
	throw grsi::exit_exception(1);
	return nullptr;
}

Int_t TTdrClover::GetSuppressedAddbackMultiplicity(TBgo* bgo)
{
	// Automatically builds the addback hits using the fAddbackCriterion (if the size of the fSuppressedAddbackHits vector is zero)
	// and return the number of addback hits.
	GetSuppressedMultiplicity(bgo); // this forces the population of the suppressed vector
	auto hit_vec  = GetSuppressedVector();
	auto ab_vec   = GetSuppressedAddbackVector();
	auto frag_vec = GetSuppressedAddbackFragVector();
	if(hit_vec->empty()) {
		return 0;
	}
	// if the addback has been reset, clear the addback hits
	if(!IsSuppressedAddbackSet()) {
		ab_vec->clear();
		frag_vec->clear();
	}
	if(ab_vec->empty()) {
		// use the first griffin hit as starting point for the addback hits
		ab_vec->push_back(hit_vec->at(0));
		frag_vec->push_back(1);

		// loop over remaining griffin hits
		size_t i, j;
		for(i = 1; i < hit_vec->size(); ++i) {
			// check for each existing addback hit if this griffin hit should be added
			for(j = 0; j < ab_vec->size(); ++j) {
				if(fAddbackCriterion(ab_vec->at(j), hit_vec->at(i))) {
					ab_vec->at(j).Add(&(hit_vec->at(i))); // copy constructor does not copy the bit field, so we set it.
					ab_vec->at(j).SetHitBit(TGRSIDetectorHit::EBitFlag::kIsEnergySet); // this must be set for summed hits.
					ab_vec->at(j).SetHitBit(TGRSIDetectorHit::EBitFlag::kIsTimeSet);   // this must be set for summed hits. pcb.
					(frag_vec->at(j))++;
					break;
				}
			}
			if(j == ab_vec->size()) {
				ab_vec->push_back(hit_vec->at(i));
				frag_vec->push_back(1);
			}
		}
		SetSuppressedAddback(true);
	}

	return ab_vec->size();
}

TTdrCloverHit* TTdrClover::GetSuppressedAddbackHit(TBgo* bgo, const int& i)
{
	if(i < GetSuppressedAddbackMultiplicity(bgo)) {
		return &GetSuppressedAddbackVector()->at(i);
	}
	std::cerr<<"Suppressed addback hits are out of range"<<std::endl;
	throw grsi::exit_exception(1);
	return nullptr;
}

void TTdrClover::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	// Builds the TdrClover Hits directly from the TFragment. Basically, loops through the hits for an event and sets
	// observables.
	if(frag == nullptr || chan == nullptr) {
		return;
	}

	TTdrCloverHit geHit(*frag);
	GetHitVector()->push_back(std::move(geHit));
}

TVector3 TTdrClover::GetPosition(int DetNbr, int CryNbr, double dist)
{
	// Gets the position vector for a crystal specified by CryNbr within Clover DetNbr at a distance of dist mm away.
	// This is calculated to the most likely interaction point within the crystal.
	if(DetNbr > 16) {
		return TVector3(0, 0, 1);
	}

	TVector3 temp_pos(gCloverPosition[DetNbr]);

	// Interaction points may eventually be set externally. May make these members of each crystal, or pass from
	// waveforms.
	Double_t cp = 26.0; // Crystal Center Point  mm.
	Double_t id = 45.0; // 45.0;  //Crystal interaction depth mm.
	// Set Theta's of the center of each DETECTOR face
	////Define one Detector position
	TVector3 shift;
	switch(CryNbr) {
		case 0: shift.SetXYZ(-cp, cp, id); break;
		case 1: shift.SetXYZ(cp, cp, id); break;
		case 2: shift.SetXYZ(cp, -cp, id); break;
		case 3: shift.SetXYZ(-cp, -cp, id); break;
		default: shift.SetXYZ(0, 0, 1); break;
	};
	shift.RotateY(temp_pos.Theta());
	shift.RotateZ(temp_pos.Phi());

	temp_pos.SetMag(dist);

	return (temp_pos + shift);
}

void TTdrClover::ResetFlags() const
{
	fTdrCloverBits = 0;
}

void TTdrClover::ResetAddback()
{
	SetAddback(false);
	GetAddbackVector()->clear();
	GetAddbackFragVector()->clear();
}

UShort_t TTdrClover::GetNAddbackFrags(const size_t& idx)
{
	// Get the number of addback "fragments" contributing to the total addback hit
	// with index idx.
	if(idx < GetAddbackFragVector()->size()) {
		return GetAddbackFragVector()->at(idx);
	}
	return 0;
}

void TTdrClover::ResetSuppressed()
{
	SetSuppressed(false);
	GetSuppressedVector()->clear();
}

void TTdrClover::ResetSuppressedAddback()
{
	SetSuppressedAddback(false);
	GetSuppressedAddbackVector()->clear();
	GetSuppressedAddbackFragVector()->clear();
}

UShort_t TTdrClover::GetNSuppressedAddbackFrags(const size_t& idx)
{
	// Get the number of addback "fragments" contributing to the total addback hit
	// with index idx.
	if(idx < GetSuppressedAddbackFragVector()->size()) {
		return GetSuppressedAddbackFragVector()->at(idx);
	}
	return 0;
}

void TTdrClover::SetBitNumber(enum ETdrCloverBits bit, Bool_t set) const
{
	// Used to set the flags that are stored in TTdrClover.
	fTdrCloverBits.SetBit(bit, set);
	/*	if(set)
		fTdrCloverBits |= bit;
		else
		fTdrCloverBits &= (~bit);*/
}

const char* TTdrClover::GetColorFromNumber(Int_t number)
{
	switch(number) {
		case(0): return "B";
		case(1): return "G";
		case(2): return "R";
		case(3): return "W";
	};
	return "X";
}
