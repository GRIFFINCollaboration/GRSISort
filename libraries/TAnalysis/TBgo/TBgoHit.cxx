#include "TBgoHit.h"

#include "TClass.h"

#include "GValue.h"

/// \cond CLASSIMP
ClassImp(TBgoHit)
/// \endcond

TBgoHit::TBgoHit()
{
   Clear();
}

TBgoHit::~TBgoHit() = default;

TBgoHit::TBgoHit(const TBgoHit& rhs) : TDetectorHit()
{
   rhs.Copy(*this);
}
