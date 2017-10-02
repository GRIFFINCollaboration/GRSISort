#include "TTACHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TTAC.h"

/// \cond CLASSIMP
ClassImp(TTACHit)
/// \endcond

TTACHit::TTACHit()
{
	// Default Constructor
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTACHit::~TTACHit() = default;

TTACHit::TTACHit(const TTACHit& rhs) : TGRSIDetectorHit()
{
	// Copy Constructor
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
	rhs.Copy(*this);
}

void TTACHit::Copy(TObject& rhs) const
{
	// Copies a TTACHit
	TGRSIDetectorHit::Copy(rhs);
	static_cast<TTACHit&>(rhs).fFilter = fFilter;
}

Double_t TTACHit::GetTempCorrectedCharge(TGraph* correction_graph) const {
	//Applies the kValue ot the charge
	if(!correction_graph){
		std::cout << "Graph for temperture corrections is null" << std::endl;
	}

	return GetCharge()*correction_graph->Eval(GetTime()/1e9);//The graph should be defined in seconds
}

Double_t TTACHit::TempCorrectedCharge(TGraph* correction_graph) const {
	//Returns the raw charge with no kValue applied
	if(!correction_graph){
		std::cout << "Graph for temperture corrections is null" << std::endl;
	}

	return Charge()*correction_graph->Eval(GetTime()/1e9);//The graph should be defined in seconds
}

Double_t TTACHit::GetTempCorrectedEnergy(TGraph* correction_graph) const {
	//This will not overwrite the normal energy, nor will it get stored as the energy.
	if(!correction_graph){
		std::cout << "Graph for temperture corrections is null" << std::endl;
	}

	TChannel* channel = GetChannel();
	if(channel == nullptr) {
		return 0.0;
	}
	if(fKValue > 0) {
		return channel->CalibrateENG(TempCorrectedCharge(correction_graph), (int)fKValue);
	} else if(channel->UseCalFileIntegration()) {
		return channel->CalibrateENG(TempCorrectedCharge(correction_graph), 0);
	}
	return channel->CalibrateENG(TempCorrectedCharge(correction_graph));
}

bool TTACHit::InFilter(Int_t)
{
	// check if the desired filter is in wanted filter;
	// return the answer;
	// currently does nothing
	return true;
}

void TTACHit::Clear(Option_t*)
{
	// Clears the TACHit
	fFilter = 0;
	TGRSIDetectorHit::Clear();
}

void TTACHit::Print(Option_t*) const
{
	// Prints the TACHit. Returns:
	// Detector
	// Energy
	// Time
	printf("TAC Detector: %i\n", GetDetector());
	printf("TAC hit energy: %.2f\n", GetEnergy());
	printf("TAC hit time:   %.lf\n", GetTime());
}
