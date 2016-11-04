

#include "TDetector.h"
#include <TClass.h>

/// \cond CLASSIMP
ClassImp(TDetector)
/// \endcond

TDetector::TDetector() : TObject(){
	///Default constructor.
	Class()->IgnoreTObjectStreamer(kTRUE);
}

TDetector::TDetector(const TDetector& rhs) : TObject() {
	///Default Copy constructor.
	Class()->IgnoreTObjectStreamer(kTRUE);
	rhs.Copy(*this);
}

TDetector::~TDetector()	{
	///Default Destructor.
}

void TDetector::Copy(TObject &rhs) const {
	//if(!rhs.InheritsFrom("TDetector"))
	//   return;
	TObject::Copy(rhs);
}


void TDetector::Print(Option_t *opt) const {
	/// Default print statement for TDetector. Currently does
	/// nothing
}

void TDetector::Clear(Option_t *opt) {
	/// Default clear statement for TDetector.
	ClearBits();
}


