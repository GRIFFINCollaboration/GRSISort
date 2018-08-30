#include <iostream>
#include "TLaBr.h"
#include <TRandom.h>
#include <TMath.h>

#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TLaBr)
/// \endcond

bool DefaultLaBrSuppression(const TGRSIDetectorHit& hit, const TBgoHit& bgoHit)
{
	return ((hit.GetDetector() == bgoHit.GetDetector()) &&
	(std::fabs(hit.GetTime() - bgoHit.GetTime()) < TGRSIOptions::AnalysisOptions()->SuppressionWindow()) &&
	(bgoHit.GetEnergy() > TGRSIOptions::AnalysisOptions()->SuppressionEnergy()));
}

std::function<bool(const TGRSIDetectorHit&, const TBgoHit&)> TLaBr::fSuppressionCriterion = DefaultLaBrSuppression;

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

TLaBr::TLaBr(const TLaBr& rhs) : TSuppressed()
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
		TSuppressed::Clear(opt);
	}
	fLaBrHits.clear();
	fSuppressedHits.clear();
	fLaBrBits = 0;
}

void TLaBr::Copy(TObject& rhs) const
{
	// Copies a TLaBr
	TSuppressed::Copy(rhs);

	static_cast<TLaBr&>(rhs).fLaBrHits       = fLaBrHits;
	static_cast<TLaBr&>(rhs).fSuppressedHits = fSuppressedHits;
   static_cast<TLaBr&>(rhs).fLaBrBits       = 0;
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
		return &(fLaBrHits.at(i));
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" is out of range: "<<oor.what()<<std::endl;
		throw grsi::exit_exception(1);
	}
	return nullptr;
}

bool TLaBr::IsSuppressed() const
{
	return TestBitNumber(ELaBrBits::kIsSuppressed);
}

void TLaBr::SetSuppressed(const bool flag)
{
	return SetBitNumber(ELaBrBits::kIsSuppressed, flag);
}

void TLaBr::ResetSuppressed()
{
   SetSuppressed(false);
   fSuppressedHits.clear();
}

Short_t TLaBr::GetSuppressedMultiplicity(const TBgo* bgo)
{
	/// Automatically builds the suppressed hits using the fSuppressionCriterion and returns the number of suppressed hits
	if(fLaBrHits.empty()) {
		return 0;
	}
   // if the suppressed has been reset, clear the suppressed hits
   if(!IsSuppressed()) {
      fSuppressedHits.clear();
   }
   if(fSuppressedHits.empty()) {
		CreateSuppressed(bgo, fLaBrHits, fSuppressedHits);
      SetSuppressed(true);
   }

   return fSuppressedHits.size();
}

TLaBrHit* TLaBr::GetSuppressedHit(const int& i)
{
	try {
		return &(fSuppressedHits.at(i));
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
