#include "TGRSIDetector.h"
#include "TGRSIDetectorHit.h"
#include <TClass.h>

/// \cond CLASSIMP
ClassImp(TGRSIDetector)
/// \endcond

TGRSIDetector::TGRSIDetector()
   : TDetector()
{
// Default constructor.
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetector::TGRSIDetector(const TGRSIDetector& rhs) : TDetector(rhs)
{
   // Default Copy constructor.
   // Class()->IgnoreTObjectStreamer(kTRUE);
   rhs.Copy(*this);
}

TGRSIDetector::~TGRSIDetector()
{
   // Default Destructor.
}

void TGRSIDetector::Copy(TObject& rhs) const
{
   // if(!rhs.InheritsFrom("TGRSIDetector"))
   //   return;
   TObject::Copy(rhs);
}

void TGRSIDetector::Print(Option_t*) const
{
   // Default print statement for TGRSIDetector. Currently does
   // nothing
}

void TGRSIDetector::Clear(Option_t* opt)
{
   // Default clear statement for TGRSIDetector.
   TDetector::Clear(opt);
}
