#include <iostream>
#include "TTdrSiLi.h"
#include <TRandom.h>
#include <TMath.h>

////////////////////////////////////////////////////////////
//
// TTdrSiLi
//
// The TTdrSiLi class defines the observables and algorithms used
// when analyzing GRIFFIN data.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TTdrSiLi)
/// \endcond

bool TTdrSiLi::fSetCoreWave = false;

TTdrSiLi::TTdrSiLi() : TGRSIDetector()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTdrSiLi::TTdrSiLi(const TTdrSiLi& rhs) : TGRSIDetector()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TTdrSiLi::Copy(TObject& rhs) const
{
	TGRSIDetector::Copy(rhs);

	static_cast<TTdrSiLi&>(rhs).fTdrSiLiHits   = fTdrSiLiHits;
	static_cast<TTdrSiLi&>(rhs).fSetCoreWave = fSetCoreWave;
}

TTdrSiLi::~TTdrSiLi()
{
	// Default Destructor
}

void TTdrSiLi::Clear(Option_t* opt)
{
	// Clears the mother, all of the hits
	if(TString(opt).Contains("all", TString::ECaseCompare::kIgnoreCase)) {
		TGRSIDetector::Clear(opt);
	}
	fTdrSiLiHits.clear();
}

void TTdrSiLi::Print(Option_t*) const
{
	// Prints out TTdrSiLi members, currently does nothing.
	printf("%lu fTdrSiLiHits\n", fTdrSiLiHits.size());
}

TTdrSiLi& TTdrSiLi::operator=(const TTdrSiLi& rhs)
{
	rhs.Copy(*this);
	return *this;
}

void TTdrSiLi::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
	TTdrSiLiHit hit(*frag);
	fTdrSiLiHits.push_back(std::move(hit));
}

TGRSIDetectorHit* TTdrSiLi::GetHit(const Int_t& idx)
{
	return GetTdrSiLiHit(idx);
}

TTdrSiLiHit* TTdrSiLi::GetTdrSiLiHit(const int& i)
{
	try {
		return &fTdrSiLiHits.at(i);
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" is out of range: "<<oor.what()<<std::endl;
		throw grsi::exit_exception(1);
	}
	return nullptr;
}

TVector3 TTdrSiLi::GetPosition(int)
{
	// Gets the position vector for a crystal specified by DetNbr
	// Does not currently contain any positons.
	return TVector3(0, 0, 1);
}
