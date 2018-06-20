#include "TGriffin.h"

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

bool DefaultAddback(const TGRSIDetectorHit& one, const TGRSIDetectorHit& two)
{
   return ((one.GetDetector() == two.GetDetector()) &&
           (std::fabs(one.GetTime() - two.GetTime()) < TGRSIOptions::AnalysisOptions()->AddbackWindow()));
}

std::function<bool(const TGRSIDetectorHit&, const TGRSIDetectorHit&)> TGriffin::fAddbackCriterion = DefaultAddback;

bool DefaultSuppression(const TGRSIDetectorHit& hit, const TBgoHit& bgoHit)
{
	return ((hit.GetDetector() == bgoHit.GetDetector() && hit.GetCrystal() == bgoHit.GetCrystal()) &&
	(std::fabs(hit.GetTime() - bgoHit.GetTime()) < TGRSIOptions::AnalysisOptions()->SuppressionWindow()) &&
	(bgoHit.GetEnergy() > TGRSIOptions::AnalysisOptions()->SuppressionEnergy()));
}

std::function<bool(const TGRSIDetectorHit&, const TBgoHit&)> TGriffin::fSuppressionCriterion = DefaultSuppression;

bool  TGriffin::fSetCoreWave     = false;
TGriffin::EGainBits TGriffin::fDefaultGainType = TGriffin::EGainBits::kLowGain;

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
TVector3 TGriffin::gCloverPosition[17] = {
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

std::map<int, TSpline*> TGriffin::fEnergyResiduals;

TGriffin::TGriffin() : TSuppressed()
{
// Default ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TGriffin::TGriffin(const TGriffin& rhs) : TSuppressed()
{
// Copy ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

void TGriffin::Copy(TObject& rhs) const
{
   // Copy function.
   TSuppressed::Copy(rhs);

   static_cast<TGriffin&>(rhs).fGriffinLowGainHits   = fGriffinLowGainHits;
   static_cast<TGriffin&>(rhs).fGriffinHighGainHits  = fGriffinHighGainHits;
   static_cast<TGriffin&>(rhs).fAddbackLowGainHits   = fAddbackLowGainHits;
   static_cast<TGriffin&>(rhs).fAddbackHighGainHits  = fAddbackHighGainHits;
   static_cast<TGriffin&>(rhs).fAddbackLowGainFrags  = fAddbackLowGainFrags;
   static_cast<TGriffin&>(rhs).fAddbackHighGainFrags = fAddbackHighGainFrags;
   static_cast<TGriffin&>(rhs).fSetCoreWave          = fSetCoreWave;
   static_cast<TGriffin&>(rhs).fCycleStart           = fCycleStart;
   static_cast<TGriffin&>(rhs).fGriffinBits          = 0;
}

TGriffin::~TGriffin()
{
   // Default Destructor
}

void TGriffin::Clear(Option_t* opt)
{
   // Clears the mother, and all of the hits
   ClearStatus();
   TSuppressed::Clear(opt);
   fGriffinLowGainHits.clear();
   fGriffinHighGainHits.clear();
   fAddbackLowGainHits.clear();
   fAddbackHighGainHits.clear();
   fAddbackLowGainFrags.clear();
   fAddbackHighGainFrags.clear();
   fCycleStart = 0;
}

void TGriffin::LoadEnergyResidual(int chan, TSpline* residual)
{
   std::cout<<"Adding: "<<chan<<std::endl;
   fEnergyResiduals[chan] = residual;
}

Double_t TGriffin::GetEnergyNonlinearity(int chan, double energy)
{
   static int counter = 0;
   counter++;
   if(fEnergyResiduals.find(chan) != fEnergyResiduals.end()) {
      return fEnergyResiduals[chan]->Eval(energy);
   }
   return 0.0;
}

void TGriffin::Print(Option_t*) const
{
   std::cout<<"Griffin Contains: "<<std::endl;
   std::cout<<std::setw(6)<<GetLowGainMultiplicity()<<" Low gain hits"<<std::endl;
   std::cout<<std::setw(6)<<GetHighGainMultiplicity()<<" High gain hits"<<std::endl;

   if(IsAddbackSet(EGainBits::kLowGain)) {
      std::cout<<std::setw(6)<<fAddbackLowGainHits.size()<<" Low gain addback hits"<<std::endl;
   } else {
      std::cout<<std::setw(6)<<" "
               <<" Low Gain Addback not set"<<std::endl;
   }

   if(IsAddbackSet(EGainBits::kHighGain)) {
      std::cout<<std::setw(6)<<fAddbackHighGainHits.size()<<" High gain addback hits"<<std::endl;
   } else {
      std::cout<<std::setw(6)<<" "
               <<" High Gain Addback not set"<<std::endl;
   }

   std::cout<<std::setw(6)<<" "
            <<" Cross-talk Set?  Low gain: "<<IsCrossTalkSet(EGainBits::kLowGain)
            <<"   High gain: "<<IsCrossTalkSet(EGainBits::kHighGain)<<std::endl;
   std::cout<<std::setw(6)<<fCycleStart<<" cycle start"<<std::endl;
}

TGriffin& TGriffin::operator=(const TGriffin& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TGriffin::SetDefaultGainType(const EGainBits& gain_type)
{
   if((gain_type == EGainBits::kLowGain) || (gain_type == EGainBits::kHighGain)) {
      fDefaultGainType = gain_type;
   } else {
      std::cout<<static_cast<std::underlying_type<EGainBits>::type>(gain_type)<<" is not a known gain type. Please use kLowGain or kHighGain"<<std::endl;
   }
}

Short_t TGriffin::GetMultiplicity(const EGainBits& gain_type) const
{
   switch(gain_type) {
		case EGainBits::kLowGain: return fGriffinLowGainHits.size();
		case EGainBits::kHighGain: return fGriffinHighGainHits.size();
   };
   return 0;
}

std::vector<TGriffinHit>& TGriffin::GetHitVector(const EGainBits& gain_type)
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return fGriffinLowGainHits;
		case EGainBits::kHighGain: return fGriffinHighGainHits;
   };
}

std::vector<TGriffinHit>& TGriffin::GetAddbackVector(const EGainBits& gain_type)
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return fAddbackLowGainHits;
		case EGainBits::kHighGain: return fAddbackHighGainHits;
   };
}

std::vector<UShort_t>& TGriffin::GetAddbackFragVector(const EGainBits& gain_type)
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return fAddbackLowGainFrags;
		case EGainBits::kHighGain: return fAddbackHighGainFrags;
   };
}

bool TGriffin::IsAddbackSet(const EGainBits& gain_type) const
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return TestBitNumber(EGriffinBits::kIsLowGainAddbackSet);
		case EGainBits::kHighGain: return TestBitNumber(EGriffinBits::kIsHighGainAddbackSet);
   };
   return false;
}

bool TGriffin::IsCrossTalkSet(const EGainBits& gain_type) const
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return TestBitNumber(EGriffinBits::kIsLowGainCrossTalkSet);
		case EGainBits::kHighGain: return TestBitNumber(EGriffinBits::kIsHighGainCrossTalkSet);
   };
   return false;
}

void TGriffin::SetAddback(const EGainBits& gain_type, const Bool_t flag) const
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return SetBitNumber(EGriffinBits::kIsLowGainAddbackSet, flag);
		case EGainBits::kHighGain: return SetBitNumber(EGriffinBits::kIsHighGainAddbackSet, flag);
   };
}

void TGriffin::SetCrossTalk(const EGainBits& gain_type, const Bool_t flag) const
{
   switch(gain_type) {
		case EGainBits::kLowGain: return SetBitNumber(EGriffinBits::kIsLowGainCrossTalkSet, flag);
		case EGainBits::kHighGain: return SetBitNumber(EGriffinBits::kIsHighGainCrossTalkSet, flag);
   };
}

TGRSIDetectorHit* TGriffin::GetHit(const Int_t& idx)
{
   return GetGriffinHit(idx);
}

TGriffinHit* TGriffin::GetGriffinLowGainHit(const int& i)
{
   return GetGriffinHit(i, EGainBits::kLowGain);
}

TGriffinHit* TGriffin::GetGriffinHighGainHit(const int& i)
{
   return GetGriffinHit(i, EGainBits::kHighGain);
}

TGriffinHit* TGriffin::GetGriffinHit(const int& i, const EGainBits& gain_type)
{
   try {
      if(!IsCrossTalkSet(gain_type)) {
         FixCrossTalk(gain_type);
      }
      return &(GetHitVector(gain_type).at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr<<ClassName()<<" Hits are out of range: "<<oor.what()<<std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return nullptr;
}

Short_t TGriffin::GetAddbackLowGainMultiplicity()
{
   return GetAddbackMultiplicity(EGainBits::kLowGain);
}

Short_t TGriffin::GetAddbackHighGainMultiplicity()
{
   return GetAddbackMultiplicity(EGainBits::kHighGain);
}

Short_t TGriffin::GetAddbackMultiplicity(const EGainBits& gain_type)
{
   // Automatically builds the addback hits using the fAddbackCriterion (if the size of the fAddbackHits vector is zero)
   // and return the number of addback hits.
   if(!IsCrossTalkSet(gain_type)) {
      // Calculate Cross Talk on each hit
      FixCrossTalk(gain_type);
   }
   auto& hit_vec  = GetHitVector(gain_type);
   auto& ab_vec   = GetAddbackVector(gain_type);
   auto& frag_vec = GetAddbackFragVector(gain_type);
   if(hit_vec.empty()) {
      return 0;
   }
   // if the addback has been reset, clear the addback hits
   if(!IsAddbackSet(gain_type)) {
      ab_vec.clear();
      frag_vec.clear();
   }
   if(ab_vec.empty()) {
		CreateAddback(hit_vec, ab_vec, frag_vec);
      SetAddback(gain_type, true);
   }

   return ab_vec.size();
}

TGriffinHit* TGriffin::GetAddbackLowGainHit(const int& i)
{
   return GetAddbackHit(i, EGainBits::kLowGain);
}

TGriffinHit* TGriffin::GetAddbackHighGainHit(const int& i)
{
   return GetAddbackHit(i, EGainBits::kHighGain);
}

TGriffinHit* TGriffin::GetAddbackHit(const int& i, const EGainBits& gain_type)
{
   if(i < GetAddbackMultiplicity(gain_type)) {
      return &(GetAddbackVector(gain_type)[i]);
   }
   std::cerr<<"Addback hits are out of range"<<std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

void TGriffin::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   // Builds the GRIFFIN Hits directly from the TFragment. Basically, loops through the hits for an event and sets
   // observables.
   // This is done for both GRIFFIN and it's suppressors.
   if(frag == nullptr || chan == nullptr) {
      return;
   }
   if(chan->GetMnemonic()->OutputSensor() == TMnemonic::EMnemonic::kA) {
   }

   switch(chan->GetMnemonic()->SubSystem()) {
		case TMnemonic::EMnemonic::kG:
			{
				TGriffinHit geHit(*frag);
				switch(chan->GetMnemonic()->OutputSensor()) {
					case TMnemonic::EMnemonic::kA:
						GetHitVector(EGainBits::kLowGain).push_back(std::move(geHit));
						break;
					case TMnemonic::EMnemonic::kB:
						GetHitVector(EGainBits::kHighGain).push_back(std::move(geHit));
						break;
					default:
						break;
				};
			}
			break;
			//     case TMnemonic::EMnemonic::kS :
			// do supressor stuff in the future
			//      break;
		default:
			break;
	};
}

TVector3 TGriffin::GetPosition(int DetNbr, int CryNbr, double dist)
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

void TGriffin::ResetFlags() const
{
   fGriffinBits = 0;
}

void TGriffin::ResetLowGainAddback()
{
   ResetAddback(EGainBits::kLowGain);
}

void TGriffin::ResetHighGainAddback()
{
   ResetAddback(EGainBits::kLowGain);
}

void TGriffin::ResetAddback(const EGainBits& gain_type)
{
   SetAddback(gain_type, false);
   SetCrossTalk(gain_type, false);
   GetAddbackVector(gain_type).clear();
   GetAddbackFragVector(gain_type).clear();
}

UShort_t TGriffin::GetNLowGainAddbackFrags(const size_t& idx)
{
   return GetNAddbackFrags(idx, EGainBits::kLowGain);
}

UShort_t TGriffin::GetNHighGainAddbackFrags(const size_t& idx)
{
   return GetNAddbackFrags(idx, EGainBits::kHighGain);
}

UShort_t TGriffin::GetNAddbackFrags(const size_t& idx, const EGainBits& gain_type)
{
   // Get the number of addback "fragments" contributing to the total addback hit
   // with index idx.
   if(idx < GetAddbackFragVector(gain_type).size()) {
      return GetAddbackFragVector(gain_type)[idx];
   }
   return 0;
}

void TGriffin::SetBitNumber(enum EGriffinBits bit, Bool_t set) const
{
   // Used to set the flags that are stored in TGriffin.
   fGriffinBits.SetBit(bit, set);
   /*	if(set)
         fGriffinBits |= bit;
      else
         fGriffinBits &= (~bit);*/
}

Double_t TGriffin::CTCorrectedEnergy(const TGriffinHit* const hit_to_correct, const TGriffinHit* const other_hit,
                                     Bool_t time_constraint)
{
   if((hit_to_correct == nullptr) || (other_hit == nullptr)) {
      printf("One of the hits is invalid in TGriffin::CTCorrectedEnergy\n");
      return 0;
   }

   if(time_constraint) {
      // Figure out if this passes the selected window
      if(TMath::Abs(other_hit->GetTime() - hit_to_correct->GetTime()) >
         TGRSIOptions::AnalysisOptions()->AddbackWindow()) { // placeholder
         return hit_to_correct->GetEnergy();
      }
   }

   if(hit_to_correct->GetDetector() != other_hit->GetDetector()) {
      return hit_to_correct->GetEnergy();
   }
   static bool been_warned[256] = {false};
   double      fixed_energy     = hit_to_correct->GetEnergy();
   try {
      fixed_energy -=
         hit_to_correct->GetChannel()->GetCTCoeff().at(other_hit->GetCrystal()) * other_hit->GetNoCTEnergy();
   } catch(const std::out_of_range& oor) {
      if(!been_warned[16 * hit_to_correct->GetDetector() + 4 * hit_to_correct->GetCrystal() +
                      other_hit->GetCrystal()]) {
         been_warned[16 * hit_to_correct->GetDetector() + 4 * hit_to_correct->GetCrystal() + other_hit->GetCrystal()] =
            true;
         std::cout<<DRED<<"Missing CT correction for Det: "<<hit_to_correct->GetDetector()
                  <<" Crystals: "<<hit_to_correct->GetCrystal()<<" "<<other_hit->GetCrystal()<<std::endl;
      }
      return hit_to_correct->GetEnergy();
   }

   return fixed_energy;

   // return hit_to_correct->GetEnergy() - (gCrossTalkPar[0][hit_to_correct->GetCrystal()][other_hit->GetCrystal()] +
   // gCrossTalkPar[1][hit_to_correct->GetCrystal()][other_hit->GetCrystal()]*other_hit->GetNoCTEnergy());
}

void TGriffin::FixLowGainCrossTalk()
{
   FixCrossTalk(EGainBits::kLowGain);
}

void TGriffin::FixHighGainCrossTalk()
{
   FixCrossTalk(EGainBits::kHighGain);
}

void TGriffin::FixCrossTalk(const EGainBits& gain_type)
{
   auto& hit_vec = GetHitVector(gain_type);
   if(hit_vec.size() < 2) {
      SetCrossTalk(gain_type, true);
      return;
   }
   for(auto& hit : hit_vec) {
      hit.ClearEnergy();
   }

   if(TGRSIOptions::AnalysisOptions()->IsCorrectingCrossTalk()) {
		for(auto& one : hit_vec) {
			for(auto& two : hit_vec) {
				one.SetEnergy(TGriffin::CTCorrectedEnergy(&one, &two));
				two.SetEnergy(TGriffin::CTCorrectedEnergy(&two, &one));
			}
		}
   }
   SetCrossTalk(gain_type, true);
}

const char* TGriffin::GetColorFromNumber(Int_t number)
{
   switch(number) {
   case(0): return "B";
   case(1): return "G";
   case(2): return "R";
   case(3): return "W";
   };
   return "X";
}

TGriffinHit* TGriffin::GetSuppressedLowGainHit(const int& i)
{
	return GetSuppressedHit(i, EGainBits::kLowGain);
}

TGriffinHit* TGriffin::GetSuppressedHighGainHit(const int& i)
{
	return GetSuppressedHit(i, EGainBits::kHighGain);
}

Short_t TGriffin::GetSuppressedLowGainMultiplicity(const TBgo* bgo)
{
	return GetSuppressedMultiplicity(bgo, EGainBits::kLowGain);
}

Short_t TGriffin::GetSuppressedHighGainMultiplicity(const TBgo* bgo)
{
	return GetSuppressedMultiplicity(bgo, EGainBits::kHighGain);
}

bool TGriffin::IsSuppressed(const EGainBits& gain_type) const
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return TestBitNumber(EGriffinBits::kIsLowGainSuppressed);
		case EGainBits::kHighGain: return TestBitNumber(EGriffinBits::kIsHighGainSuppressed);
   };
	return false;
}

void TGriffin::ResetLowGainSuppressed()
{
	ResetSuppressed(EGainBits::kLowGain);
}

void TGriffin::ResetHighGainSuppressed()
{
	ResetSuppressed(EGainBits::kHighGain);
}

Short_t TGriffin::GetSuppressedAddbackLowGainMultiplicity(const TBgo* bgo)
{
	return GetSuppressedAddbackMultiplicity(bgo, EGainBits::kLowGain);
}

Short_t TGriffin::GetSuppressedAddbackHighGainMultiplicity(const TBgo* bgo)
{
	return GetSuppressedAddbackMultiplicity(bgo, EGainBits::kHighGain);
}

TGriffinHit* TGriffin::GetSuppressedAddbackLowGainHit(const int& i)
{
	return GetSuppressedAddbackHit(i, EGainBits::kLowGain);
}

TGriffinHit* TGriffin::GetSuppressedAddbackHighGainHit(const int& i)
{
	return GetSuppressedAddbackHit(i, EGainBits::kHighGain);
}

bool TGriffin::IsSuppressedAddbackSet(const EGainBits& gain_type) const
{
   switch(gain_type) {
		case EGainBits::kLowGain: return TestBitNumber(EGriffinBits::kIsLowGainSuppressedAddbackSet);
		case EGainBits::kHighGain: return TestBitNumber(EGriffinBits::kIsHighGainSuppressedAddbackSet);
   };
	return false;
}

void TGriffin::ResetLowGainSuppressedAddback()
{
	ResetSuppressedAddback(EGainBits::kLowGain);
}

void TGriffin::ResetHighGainSuppressedAddback()
{
	ResetSuppressedAddback(EGainBits::kHighGain);
}

UShort_t TGriffin::GetNLowGainSuppressedAddbackFrags(const size_t& idx)
{
	return GetNSuppressedAddbackFrags(idx, EGainBits::kLowGain);
}

UShort_t TGriffin::GetNHighGainSuppressedAddbackFrags(const size_t& idx)
{
	return GetNSuppressedAddbackFrags(idx, EGainBits::kHighGain);
}

std::vector<TGriffinHit>& TGriffin::GetSuppressedVector(const EGainBits& gain_type)
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return fSuppressedLowGainHits;
		case EGainBits::kHighGain: return fSuppressedHighGainHits;
   };
}

std::vector<TGriffinHit>& TGriffin::GetSuppressedAddbackVector(const EGainBits& gain_type)
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return fSuppressedAddbackLowGainHits;
		case EGainBits::kHighGain: return fSuppressedAddbackHighGainHits;
   };
}

std::vector<UShort_t>& TGriffin::GetSuppressedAddbackFragVector(const EGainBits& gain_type)
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return fSuppressedAddbackLowGainFrags;
		case EGainBits::kHighGain: return fSuppressedAddbackHighGainFrags;
   };
}

TGriffinHit* TGriffin::GetSuppressedHit(const int& i, const EGainBits& gain_type)
{
   try {
      if(!IsCrossTalkSet(gain_type)) {
         FixCrossTalk(gain_type);
      }
      return &(GetSuppressedVector(gain_type).at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr<<ClassName()<<" Suppressed hits are out of range: "<<oor.what()<<std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return nullptr;
}

Short_t TGriffin::GetSuppressedMultiplicity(const TBgo* bgo, const EGainBits& gain_type)
{
	/// Automatically builds the suppressed hits using the fSuppressionCriterion and returns the number of suppressed hits
   if(!IsCrossTalkSet(gain_type)) {
      // Calculate Cross Talk on each hit
      FixCrossTalk(gain_type);
   }
   auto& hit_vec  = GetHitVector(gain_type);
   auto& sup_vec  = GetSuppressedVector(gain_type);
	if(hit_vec.empty()) {
		return 0;
	}
   // if the suppressed has been reset, clear the suppressed hits
   if(!IsSuppressed(gain_type)) {
      sup_vec.clear();
   }
   if(sup_vec.empty()) {
		CreateSuppressed(bgo, hit_vec, sup_vec);
      SetSuppressed(gain_type, true);
   }

   return sup_vec.size();
}

void TGriffin::SetSuppressed(const EGainBits& gain_type, const Bool_t flag) const
{
   switch(gain_type) {
		case EGainBits::kLowGain:  return SetBitNumber(EGriffinBits::kIsLowGainSuppressed, flag);
		case EGainBits::kHighGain: return SetBitNumber(EGriffinBits::kIsHighGainSuppressed, flag);
   };
}

void TGriffin::ResetSuppressed(const EGainBits& gain_type)
{
   SetSuppressed(gain_type, false);
   //SetCrossTalk(gain_type, false);
   GetSuppressedVector(gain_type).clear();
}

TGriffinHit* TGriffin::GetSuppressedAddbackHit(const int& i, const EGainBits& gain_type)
{
   try {
      if(!IsCrossTalkSet(gain_type)) {
         FixCrossTalk(gain_type);
      }
      return &(GetSuppressedAddbackVector(gain_type).at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr<<ClassName()<<" Suppressed addback hits are out of range: "<<oor.what()<<std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return nullptr;
}

Short_t TGriffin::GetSuppressedAddbackMultiplicity(const TBgo* bgo, const EGainBits& gain_type)
{
   /// Automatically builds the suppressed addback hits using the fAddbackCriterion (if the size of the fAddbackHits vector is zero)
   /// and return the number of suppressed addback hits.
   if(!IsCrossTalkSet(gain_type)) {
      // Calculate Cross Talk on each hit
      FixCrossTalk(gain_type);
   }
   auto& hit_vec  = GetHitVector(gain_type);
   auto& ab_vec   = GetAddbackVector(gain_type);
   auto& frag_vec = GetAddbackFragVector(gain_type);
   if(hit_vec.empty()) {
      return 0;
   }
   // if the addback has been reset, clear the addback hits
   if(!IsSuppressedAddbackSet(gain_type)) {
      ab_vec.clear();
      frag_vec.clear();
   }
   if(ab_vec.empty()) {
		CreateSuppressedAddback(bgo, hit_vec, ab_vec, frag_vec);
      SetSuppressedAddback(gain_type, true);
   }

   return ab_vec.size();
}

void TGriffin::SetSuppressedAddback(const EGainBits& gain_type, const Bool_t flag) const
{
   switch(gain_type) {
		case EGainBits::kLowGain: return SetBitNumber(EGriffinBits::kIsLowGainSuppressedAddbackSet, flag);
		case EGainBits::kHighGain: return SetBitNumber(EGriffinBits::kIsHighGainSuppressedAddbackSet, flag);
   };
}

void TGriffin::ResetSuppressedAddback(const EGainBits& gain_type)
{
   SetSuppressedAddback(gain_type, false);
   //SetCrossTalk(gain_type, false);
   GetSuppressedAddbackVector(gain_type).clear();
   GetSuppressedAddbackFragVector(gain_type).clear();
}

UShort_t TGriffin::GetNSuppressedAddbackFrags(const size_t& idx, const EGainBits& gain_type)
{
	try {
		return GetSuppressedAddbackFragVector(gain_type).at(idx);
	} catch(const std::out_of_range& oor) {
      std::cerr<<ClassName()<<" Suppressed addback frags are out of range: "<<oor.what()<<std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return 0;
}
