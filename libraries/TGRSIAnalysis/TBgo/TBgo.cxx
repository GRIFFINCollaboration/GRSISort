#include "TBgo.h"

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
// TBgo
//
// The TBgo class defines the observables and algorithms used
// when analyzing BGO data. It includes detector positions,
// etc.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TBgo)
/// \endcond

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
TVector3 TBgo::gScintPosition[17] = {
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

TBgo::TBgo() : TDetector()
{
	/// Default ctor.
   Clear();
}

TBgo::TBgo(const TBgo& rhs) : TDetector()
{
	/// Copy ctor.
   rhs.Copy(*this);
}

void TBgo::Copy(TObject& rhs) const
{
   // Copy function.
   TDetector::Copy(rhs);
}

TBgo::~TBgo()
{
   // Default Destructor
}

void TBgo::Clear(Option_t* opt)
{
   /// Clears the mother, and all of the hits
   TDetector::Clear(opt);
}

void TBgo::Print(Option_t*) const
{
   std::cout<<"Bgo Contains: "<<std::endl;
   std::cout<<std::setw(6)<<GetMultiplicity()<<" hits"<<std::endl;
}

TBgo& TBgo::operator=(const TBgo& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TBgo::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   // Builds the BGO Hits directly from the TFragment. Basically, loops through the hits for an event and sets
   // observables.
   if(frag == nullptr || chan == nullptr) {
      return;
   }

	TBgoHit* hit = new TBgoHit(*frag);
	fHits.push_back(std::move(hit));
}

TVector3 TBgo::GetPosition(int DetNbr, int CryNbr, double dist)
{
   // Gets the position vector for a crystal specified by CryNbr within Clover DetNbr at a distance of dist mm away.
   // This is calculated to the most likely interaction point within the crystal.
   if(DetNbr > 16) return TVector3(0, 0, 1);

   TVector3 temp_pos(gScintPosition[DetNbr]);

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
