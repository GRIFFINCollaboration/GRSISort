

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
}

void TDetector::Print(Option_t*) const
{
   /// Default print statement for TDetector. Currently does
   /// nothing
}

void TDetector::Clear(Option_t*)
{
   /// Default clear statement for TDetector. Currently does
   /// nothing
}
