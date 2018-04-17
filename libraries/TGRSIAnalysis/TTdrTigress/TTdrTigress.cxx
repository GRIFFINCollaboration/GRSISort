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

std::vector<TTdrTigressHit>* TTdrTigress::GetHitVector()
{
	return &fTdrTigressHits;
}

std::vector<TTdrTigressHit>* TTdrTigress::GetAddbackVector()
{
	return &fAddbackHits;
}

std::vector<UShort_t>* TTdrTigress::GetAddbackFragVector()
{
	return &fAddbackFrags;
}

bool TTdrTigress::IsAddbackSet() const
{
	return TestBitNumber(ETdrTigressBits::kIsAddbackSet);
}

void TTdrTigress::SetAddback(const Bool_t flag) const
{
	return SetBitNumber(ETdrTigressBits::kIsAddbackSet, flag);
}

TGRSIDetectorHit* TTdrTigress::GetHit(const Int_t& idx)
{
	return GetTdrTigressHit(idx);
}

TTdrTigressHit* TTdrTigress::GetTdrTigressHit(const int& i)
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

Int_t TTdrTigress::GetAddbackMultiplicity()
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

TTdrTigressHit* TTdrTigress::GetAddbackHit(const int& i)
{
	if(i < GetAddbackMultiplicity()) {
		return &GetAddbackVector()->at(i);
	}
	std::cerr<<"Addback hits are out of range"<<std::endl;
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
	GetHitVector()->push_back(std::move(geHit));
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
	GetAddbackVector()->clear();
	GetAddbackFragVector()->clear();
}

UShort_t TTdrTigress::GetNAddbackFrags(const size_t& idx)
{
	// Get the number of addback "fragments" contributing to the total addback hit
	// with index idx.
	if(idx < GetAddbackFragVector()->size()) {
		return GetAddbackFragVector()->at(idx);
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
