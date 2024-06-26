#include "TBgoHit.h"

#include "TClass.h"

#include "GValue.h"

TBgoHit::TBgoHit()
{
   Clear();
}

TBgoHit::~TBgoHit() = default;

TBgoHit::TBgoHit(const TBgoHit& rhs) : TDetectorHit(rhs)
{
   rhs.Copy(*this, true);
}

TBgoHit::TBgoHit(TBgoHit&& rhs)
{
   rhs.Copy(*this, true);
}

TBgoHit& TBgoHit::operator=(const TBgoHit& rhs)
{
   rhs.Copy(*this, true);

	return *this;
}

TBgoHit& TBgoHit::operator=(TBgoHit&& rhs)
{
   rhs.Copy(*this, true);

	return *this;
}

void TBgoHit::Copy(TObject& rhs, bool copywave) const
{
   // Copy function.
   TDetectorHit::Copy(rhs, copywave);
}

void TBgoHit::Clear(Option_t* opt)
{
   /// Clears the mother, and all of the hits
   TDetectorHit::Clear(opt);
}

void TBgoHit::Print(Option_t*) const
{
   Print(std::cout);
}

void TBgoHit::Print(std::ostream& out) const
{
   TDetectorHit::Print(out);
}
