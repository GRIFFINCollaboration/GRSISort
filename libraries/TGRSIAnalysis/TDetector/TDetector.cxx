

#include "TDetector.h"
#include <TClass.h>

/// \cond CLASSIMP
ClassImp(TDetector)
/// \endcond

TDetector::TDetector()
   : TObject()
{
   /// Default constructor.
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TDetector::TDetector(const TDetector& rhs) : TObject()
{
   /// Default Copy constructor.
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

TDetector::~TDetector()
{
   /// Default Destructor.
}

void TDetector::Copy(TObject& rhs) const
{
   // if(!rhs.InheritsFrom("TDetector"))
   //   return;
   TObject::Copy(rhs);
	static_cast<TDetector&>(rhs).fHits = fHits;
}

void TDetector::Print(Option_t*) const
{
   /// Default print statement for TDetector. Currently does
   /// nothing
}

void TDetector::ClearTransients()
{
	for(auto hit : fHits) {
		hit->ClearTransients();
	}
}

TDetectorHit* TDetector::GetHit(const int& i) const
{
	try {
		return fHits.at(i);
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" is out of range: "<<oor.what()<<std::endl;
		throw grsi::exit_exception(1);
	}
	return nullptr;
}
