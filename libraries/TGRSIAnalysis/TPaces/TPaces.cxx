#include <iostream>
#include "TPaces.h"
#include <TRandom.h>
#include <TMath.h>

////////////////////////////////////////////////////////////
//
// TPaces
//
// The TPaces class defines the observables and algorithms used
// when analyzing GRIFFIN data.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TPaces)
/// \endcond

bool TPaces::fSetCoreWave = false;

TPaces::TPaces() : TGRSIDetector()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TPaces::TPaces(const TPaces& rhs) : TGRSIDetector()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TPaces::Copy(TObject& rhs) const
{
	TGRSIDetector::Copy(rhs);

	static_cast<TPaces&>(rhs).fPacesHits   = fPacesHits;
	static_cast<TPaces&>(rhs).fSetCoreWave = fSetCoreWave;
}

TPaces::~TPaces()
{
	// Default Destructor
}

void TPaces::Clear(Option_t* opt)
{
	// Clears the mother, all of the hits
	if(TString(opt).Contains("all", TString::ECaseCompare::kIgnoreCase)) {
		TGRSIDetector::Clear(opt);
	}
	fPacesHits.clear();
}

void TPaces::Print(Option_t*) const
{
	// Prints out TPaces members, currently does nothing.
	printf("%lu fPacesHits\n", fPacesHits.size());
}

TPaces& TPaces::operator=(const TPaces& rhs)
{
	rhs.Copy(*this);
	return *this;
}

void TPaces::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
	TPacesHit hit(*frag);
	fPacesHits.push_back(std::move(hit));
}

TGRSIDetectorHit* TPaces::GetHit(const Int_t& idx)
{
	return GetPacesHit(idx);
}

TPacesHit* TPaces::GetPacesHit(const int& i)
{
	try {
		return &fPacesHits.at(i);
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" is out of range: "<<oor.what()<<std::endl;
		throw grsi::exit_exception(1);
	}
	return nullptr;
}

TVector3 TPaces::GetPosition(int DetNbr)
{
	// Gets the position vector for a crystal specified by DetNbr
	// Angles measured 12 December 2017 (Garnsworthy/Bowry)
	double PACESTHETA[5] = {120.1780, 120.8275, 119.7421, 120.2992, 120.1934};
	double PACESPHI[5] = {21.0000, 94.0000, 166.0000, 237.0000, 313.0000};

	TVector3 pacesPosition [5] = {
		TVector3(TMath::Sin(TMath::DegToRad()*PACESTHETA[0]) *TMath::Cos(TMath::DegToRad()*PACESPHI [0] ), TMath::Sin(TMath::DegToRad()*PACESTHETA[0]) *TMath::Sin(TMath::DegToRad()*PACESPHI [0] ), 
		TMath::Cos(TMath::DegToRad()*PACESTHETA[0])), 
		
		TVector3(TMath::Sin(TMath::DegToRad()*PACESTHETA[1]) *TMath::Cos(TMath::DegToRad()*PACESPHI [1] ), TMath::Sin(TMath::DegToRad()*PACESTHETA[1]) *TMath::Sin(TMath::DegToRad()*PACESPHI [1] ), 
		TMath::Cos(TMath::DegToRad()*PACESTHETA[1])), 
		
		TVector3(TMath::Sin(TMath::DegToRad()*PACESTHETA[2]) *TMath::Cos(TMath::DegToRad()*PACESPHI [2] ), TMath::Sin(TMath::DegToRad()*PACESTHETA[2]) *TMath::Sin(TMath::DegToRad()*PACESPHI [2] ), 
		TMath::Cos(TMath::DegToRad()*PACESTHETA[2])), 
		
		TVector3(TMath::Sin(TMath::DegToRad()*PACESTHETA[3]) *TMath::Cos(TMath::DegToRad()*PACESPHI [3] ), TMath::Sin(TMath::DegToRad()*PACESTHETA[3]) *TMath::Sin(TMath::DegToRad()*PACESPHI [3] ), 
		TMath::Cos(TMath::DegToRad()*PACESTHETA[3])), 
		
		TVector3(TMath::Sin(TMath::DegToRad()*PACESTHETA[4]) *TMath::Cos(TMath::DegToRad()*PACESPHI [4] ), TMath::Sin(TMath::DegToRad()*PACESTHETA[4]) *TMath::Sin(TMath::DegToRad()*PACESPHI [4] ), 
		TMath::Cos(TMath::DegToRad()*PACESTHETA[4])) 	
	};
	return pacesPosition[DetNbr];
}
