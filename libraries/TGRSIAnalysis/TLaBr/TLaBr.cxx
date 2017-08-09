#include <iostream>
#include "TLaBr.h"
#include <TRandom.h>
#include <TMath.h>

/// \cond CLASSIMP
ClassImp(TLaBr)
/// \endcond

TVector3 TLaBr::gPosition[9] = {
	// These positions should be updated (they are currently SCEPTAR-ish)
	TVector3(0, 0, 1),
	TVector3(14.3025, 4.6472, 22.8096),
	TVector3(0, 15.0386, 22.8096),
	TVector3(-14.3025, 4.6472, 22.8096),
	TVector3(-8.8395, -12.1665, 22.8096),
	TVector3(8.8395, -12.1665, 22.8096),
	TVector3(19.7051, 6.4026, 6.2123),
	TVector3(0, 20.7192, 6.2123),
	TVector3(-19.7051, 6.4026, 6.2123),
};

TLaBr::TLaBr()
{
	// Default Constructor
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TLaBr::~TLaBr()
{
	// Default Destructor
}

TLaBr::TLaBr(const TLaBr& rhs) : TGRSIDetector()
{
	// Copy Contructor
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TLaBr::Clear(Option_t* opt)
{
	// Clears all of the hits
	// The Option "all" clears the base class.
	if(TString(opt).Contains("all", TString::ECaseCompare::kIgnoreCase)) {
		TGRSIDetector::Clear(opt);
	}
	fLaBrHits.clear();
}

void TLaBr::Copy(TObject& rhs) const
{
	// Copies a TLaBr
	TGRSIDetector::Copy(rhs);

	static_cast<TLaBr&>(rhs).fLaBrHits = fLaBrHits;
}

TLaBr& TLaBr::operator=(const TLaBr& rhs)
{
	rhs.Copy(*this);
	return *this;
}

void TLaBr::Print(Option_t*) const
{
	// Prints out TLaBr Multiplicity, currently does little.
	printf("%lu fLaBrHits\n", fLaBrHits.size());
}

TGRSIDetectorHit* TLaBr::GetHit(const Int_t& idx)
{
	// Gets the TLaBrHit at index idx.
	return GetLaBrHit(idx);
}

TLaBrHit* TLaBr::GetLaBrHit(const int& i)
{
	try {
		return &fLaBrHits.at(i);
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" is out of range: "<<oor.what()<<std::endl;
		throw grsi::exit_exception(1);
	}
	return nullptr;
}

void TLaBr::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
	TLaBrHit laHit(*frag);                 // Building is controlled in the constructor of the hit
	fLaBrHits.push_back(std::move(laHit)); // use std::move for efficienciy since laHit loses scope here anyway
}
