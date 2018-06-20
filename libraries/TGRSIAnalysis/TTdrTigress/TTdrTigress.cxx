#include "TTdrTigress.h"

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
// TTdrTigress
//
// The TTdrTigress class defines the observables and algorithms used
// when analyzing GRIFFIN data. It includes detector positions,
// add-back methods, etc.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TTdrTigress)
/// \endcond

bool DefaultAddback(TTdrTigressHit& one, TTdrTigressHit& two)
{
   return ((one.GetDetector() == two.GetDetector()) &&
           (std::fabs(one.GetTime() - two.GetTime()) < TGRSIOptions::AnalysisOptions()->AddbackWindow()));
}

std::function<bool(TTdrTigressHit&, TTdrTigressHit&)> TTdrTigress::fAddbackCriterion = DefaultAddback;

bool DefaultSuppression(TTdrTigressHit& clo, TBgoHit& bgo)
{
	// the tigress detector is the 4th detector after the three clovers
   return ((4 == bgo.GetDetector() && clo.GetCrystal() == bgo.GetCrystal()) &&
           (std::fabs(clo.GetTime() - bgo.GetCorrectedTime()) < TGRSIOptions::AnalysisOptions()->SuppressionWindow()) &&
			  (bgo.GetEnergy() > TGRSIOptions::AnalysisOptions()->SuppressionEnergy()));
}

std::function<bool(TTdrTigressHit&, TBgoHit&)> TTdrTigress::fSuppressionCriterion = DefaultSuppression;

bool  TTdrTigress::fSetCoreWave     = false;

// This seems unnecessary, and why 17?;//  they are static members, and need
//  to be defined outside the header
//  17 is to have the detectors go from 1-16
//  plus we can use position zero
//  when the detector winds up back in
//  one of the stands like Alex used in the
//  gps run. pcb.
// Initiallizes the HPGe Tigress positions as per the wiki
// <https://www.triumf.info/wiki/tigwiki/index.php/HPGe_Coordinate_Table>
//                                                                             theta                                 phi
//                                                                             theta                                phi
//                                                                             theta
TVector3 TTdrTigress::gTigressPosition[17] = {
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

std::map<int, TSpline*> TTdrTigress::fEnergyResiduals;

TTdrTigress::TTdrTigress() : TGRSIDetector()
{
// Default ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TTdrTigress::TTdrTigress(const TTdrTigress& rhs) : TGRSIDetector()
{
// Copy ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

void TTdrTigress::Copy(TObject& rhs) const
{
   // Copy function.
   TGRSIDetector::Copy(rhs);

   static_cast<TTdrTigress&>(rhs).fTdrTigressHits   = fTdrTigressHits;
   static_cast<TTdrTigress&>(rhs).fAddbackHits   = fAddbackHits;
   static_cast<TTdrTigress&>(rhs).fAddbackFrags  = fAddbackFrags;
   static_cast<TTdrTigress&>(rhs).fSuppressedHits   = fSuppressedHits;
   static_cast<TTdrTigress&>(rhs).fSuppressedAddbackHits   = fSuppressedAddbackHits;
   static_cast<TTdrTigress&>(rhs).fSuppressedAddbackFrags  = fSuppressedAddbackFrags;
   static_cast<TTdrTigress&>(rhs).fSetCoreWave          = fSetCoreWave;
   static_cast<TTdrTigress&>(rhs).fCycleStart           = fCycleStart;
   static_cast<TTdrTigress&>(rhs).fTdrTigressBits          = 0;
}

TTdrTigress::~TTdrTigress()
{
   // Default Destructor
}

void TTdrTigress::Clear(Option_t* opt)
{
   // Clears the mother, and all of the hits
   ClearStatus();
   TGRSIDetector::Clear(opt);
   fTdrTigressHits.clear();
   fAddbackHits.clear();
   fAddbackFrags.clear();
   fSuppressedHits.clear();
   fSuppressedAddbackHits.clear();
   fSuppressedAddbackFrags.clear();
   fCycleStart = 0;
}

void TTdrTigress::Print(Option_t*) const
{
   std::cout<<"TdrTigress Contains: "<<std::endl;
   std::cout<<std::setw(6)<<GetMultiplicity()<<" hits"<<std::endl;

   if(IsAddbackSet()) {
      std::cout<<std::setw(6)<<fAddbackHits.size()<<" addback hits"<<std::endl;
   } else {
      std::cout<<std::setw(6)<<" "
               <<" Addback not set"<<std::endl;
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

TTdrTigress& TTdrTigress::operator=(const TTdrTigress& rhs)
{
   rhs.Copy(*this);
   return *this;
}

Short_t TTdrTigress::GetMultiplicity() const
{
	return fTdrTigressHits.size();
}

std::vector<TTdrTigressHit>& TTdrTigress::GetHitVector()
{
	return fTdrTigressHits;
}

std::vector<TTdrTigressHit>& TTdrTigress::GetAddbackVector()
{
	return fAddbackHits;
}

std::vector<UShort_t>& TTdrTigress::GetAddbackFragVector()
{
	return fAddbackFrags;
}

bool TTdrTigress::IsAddbackSet() const
{
	return TestBitNumber(ETdrTigressBits::kIsAddbackSet);
}

void TTdrTigress::SetAddback(const Bool_t flag) const
{
	return SetBitNumber(ETdrTigressBits::kIsAddbackSet, flag);
}

std::vector<TTdrTigressHit>& TTdrTigress::GetSuppressedVector()
{
	return fSuppressedHits;
}

bool TTdrTigress::IsSuppressedSet() const
{
	return TestBitNumber(ETdrTigressBits::kIsSuppressedSet);
}

void TTdrTigress::SetSuppressed(const Bool_t flag) const
{
	return SetBitNumber(ETdrTigressBits::kIsSuppressedSet, flag);
}

std::vector<TTdrTigressHit>& TTdrTigress::GetSuppressedAddbackVector()
{
	return fSuppressedAddbackHits;
}

std::vector<UShort_t>& TTdrTigress::GetSuppressedAddbackFragVector()
{
	return fSuppressedAddbackFrags;
}

bool TTdrTigress::IsSuppressedAddbackSet() const
{
	return TestBitNumber(ETdrTigressBits::kIsSupprAddbSet);
}

void TTdrTigress::SetSuppressedAddback(const Bool_t flag) const
{
	return SetBitNumber(ETdrTigressBits::kIsSupprAddbSet, flag);
}

TGRSIDetectorHit* TTdrTigress::GetHit(const Int_t& idx)
{
	return GetTdrTigressHit(idx);
}

TTdrTigressHit* TTdrTigress::GetTdrTigressHit(const int& i)
{
	try {
		return &(GetHitVector().at(i));
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" Hits are out of range: "<<oor.what()<<std::endl;
		if(!gInterpreter) {
			throw grsi::exit_exception(1);
		}
	}
	return nullptr;
}

Int_t TTdrTigress::GetAddbackMultiplicity()
{
	// Automatically builds the addback hits using the fAddbackCriterion (if the size of the fAddbackHits vector is zero)
	// and return the number of addback hits.
	auto& hit_vec  = GetHitVector();
	auto& ab_vec   = GetAddbackVector();
	auto& frag_vec = GetAddbackFragVector();
	if(hit_vec.empty()) {
		return 0;
	}
	// if the addback has been reset, clear the addback hits
	if(!IsAddbackSet()) {
		ab_vec.clear();
		frag_vec.clear();
	}
	if(ab_vec.empty()) {
		// loop over hits
		size_t i, j;
		for(i = 0; i < hit_vec.size(); ++i) {
			// check for each existing addback hit if this hit should be added
			for(j = 0; j < ab_vec.size(); ++j) {
				if(fAddbackCriterion(ab_vec.at(j), hit_vec.at(i))) {
					ab_vec.at(j).Add(&(hit_vec.at(i))); // copy constructor does not copy the bit field, so we set it.
					ab_vec.at(j).SetHitBit(TGRSIDetectorHit::EBitFlag::kIsEnergySet); // this must be set for summed hits.
					ab_vec.at(j).SetHitBit(TGRSIDetectorHit::EBitFlag::kIsTimeSet);   // this must be set for summed hits. pcb.
					(frag_vec.at(j))++;
					break;
				}
			}
			// if we haven't found an addback hit this belongs to or there were no addback hits
			// we create a new one
			if(j == ab_vec.size()) {
				ab_vec.push_back(hit_vec.at(i));
				frag_vec.push_back(1);
			}
		}
		SetAddback(true);
	}

	return ab_vec.size();
}

TTdrTigressHit* TTdrTigress::GetAddbackHit(const int& i)
{
	if(i < GetAddbackMultiplicity()) {
		return &GetAddbackVector().at(i);
	}
	std::cerr<<"Addback hits are out of range"<<std::endl;
	throw grsi::exit_exception(1);
	return nullptr;
}

Int_t TTdrTigress::GetSuppressedMultiplicity(TBgo* bgo)
{
	// Automatically builds the suppressed hits using the fSuppressedCriterion (if the size of the fSuppressedHits vector is zero)
	// and return the number of suppressed hits.
	if(fTdrTigressHits.empty()) {
		return 0;
	}
	// if the addback has been reset, clear the addback hits
	if(!IsSuppressedSet()) {
		fSuppressedHits.clear();
	}
	if(fSuppressedHits.empty()) {
		// loop over unsuppressed hits
		for(auto hit : fTdrTigressHits) {
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

TTdrTigressHit* TTdrTigress::GetSuppressedHit(TBgo* bgo, const int& i)
{
	if(i < GetSuppressedMultiplicity(bgo)) {
		return &GetSuppressedVector().at(i);
	}
	std::cerr<<"Suppressed hits are out of range"<<std::endl;
	throw grsi::exit_exception(1);
	return nullptr;
}

Int_t TTdrTigress::GetSuppressedAddbackMultiplicity(TBgo* bgo)
{
	// Automatically builds the addback hits using the fAddbackCriterion (if the size of the fSuppressedAddbackHits vector is zero)
	// and return the number of addback hits.
	auto& hit_vec  = GetHitVector();
	auto& ab_vec   = GetSuppressedAddbackVector();
	auto& frag_vec = GetSuppressedAddbackFragVector();
	if(hit_vec.empty()) {
		return 0;
	}
	// if the addback has been reset, clear the addback hits
	if(!IsSuppressedAddbackSet()) {
		ab_vec.clear();
		frag_vec.clear();
	}
	if(ab_vec.empty()) {
		// loop over raw hits
		size_t j;
		for(auto hit : hit_vec) {
			// check for each hit if it is suppressed
			bool suppress = false;
			for(auto b : bgo->GetHitVector()) {
				if(fSuppressionCriterion(hit, b)) {
					suppress = true;
					break;
				}
			}
			// check for each existing addback hit if this hit should be added
			for(j = 0; j < ab_vec.size(); ++j) {
				if(fAddbackCriterion(ab_vec.at(j), hit)) {
					// if this hit is suppressed we need to suppress the whole addback event
					if(suppress) {
						ab_vec.erase(ab_vec.begin()+j);
						frag_vec.erase(frag_vec.begin()+j);
						break;
					}
					ab_vec.at(j).Add(&hit); // copy constructor does not copy the bit field, so we set it.
					ab_vec.at(j).SetHitBit(TGRSIDetectorHit::EBitFlag::kIsEnergySet); // this must be set for summed hits.
					ab_vec.at(j).SetHitBit(TGRSIDetectorHit::EBitFlag::kIsTimeSet);   // this must be set for summed hits. pcb.
					(frag_vec.at(j))++;
					break;
				}
			}
			// if we haven't found an addback hit this belongs to or there were no addback hits
			// we create a new one unless this hit is suppressed
			// this also covers the case where the last addback hit was suppressed and removed
			// because in this case suppress is true so we won't create a new addback hit
			if(j == ab_vec.size() && !suppress) {
				ab_vec.push_back(hit);
				frag_vec.push_back(1);
			}
		}
		SetSuppressedAddback(true);
	}

	return ab_vec.size();
}

TTdrTigressHit* TTdrTigress::GetSuppressedAddbackHit(TBgo* bgo, const int& i)
{
	if(i < GetSuppressedAddbackMultiplicity(bgo)) {
		return &GetSuppressedAddbackVector().at(i);
	}
	std::cerr<<"Suppressed addback hits are out of range"<<std::endl;
	throw grsi::exit_exception(1);
	return nullptr;
}

void TTdrTigress::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	// Builds the TdrTigress Hits directly from the TFragment. Basically, loops through the hits for an event and sets
	// observables.
	if(frag == nullptr || chan == nullptr) {
		return;
	}

	TTdrTigressHit geHit(*frag);
	GetHitVector().push_back(std::move(geHit));
}

TVector3 TTdrTigress::GetPosition(int DetNbr, int CryNbr, double dist)
{
	// Gets the position vector for a crystal specified by CryNbr within Tigress DetNbr at a distance of dist mm away.
	// This is calculated to the most likely interaction point within the crystal.
	if(DetNbr > 16) {
		return TVector3(0, 0, 1);
	}

	TVector3 temp_pos(gTigressPosition[DetNbr]);

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

void TTdrTigress::ResetFlags() const
{
	fTdrTigressBits = 0;
}

void TTdrTigress::ResetAddback()
{
	SetAddback(false);
	GetAddbackVector().clear();
	GetAddbackFragVector().clear();
}

UShort_t TTdrTigress::GetNAddbackFrags(const size_t& idx)
{
	// Get the number of addback "fragments" contributing to the total addback hit
	// with index idx.
	if(idx < GetAddbackFragVector().size()) {
		return GetAddbackFragVector().at(idx);
	}
	return 0;
}

void TTdrTigress::ResetSuppressed()
{
	SetSuppressed(false);
	GetSuppressedVector().clear();
}

void TTdrTigress::ResetSuppressedAddback()
{
	SetSuppressedAddback(false);
	GetSuppressedAddbackVector().clear();
	GetSuppressedAddbackFragVector().clear();
}

UShort_t TTdrTigress::GetNSuppressedAddbackFrags(const size_t& idx)
{
	// Get the number of addback "fragments" contributing to the total addback hit
	// with index idx.
	if(idx < GetSuppressedAddbackFragVector().size()) {
		return GetSuppressedAddbackFragVector().at(idx);
	}
	return 0;
}

void TTdrTigress::SetBitNumber(enum ETdrTigressBits bit, Bool_t set) const
{
	// Used to set the flags that are stored in TTdrTigress.
	fTdrTigressBits.SetBit(bit, set);
	/*	if(set)
		fTdrTigressBits |= bit;
		else
		fTdrTigressBits &= (~bit);*/
}

const char* TTdrTigress::GetColorFromNumber(Int_t number)
{
	switch(number) {
		case(0): return "B";
		case(1): return "G";
		case(2): return "R";
		case(3): return "W";
	};
	return "X";
}
