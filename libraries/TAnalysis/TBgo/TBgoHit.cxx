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

TBgoHit::TBgoHit(TBgoHit&& rhs) noexcept
{
   rhs.Copy(*this, true);
}

// The first version of this simply called the TDetectorHit constructor with the fragment.
// This was problematic as this meant we're slicing the TFragment object to fit the
// TDetectorHit argument of the base constructor
// Now we simply do the same thing as the TDetectorHit constructor would be doing.
TBgoHit::TBgoHit(const TFragment& frag)
{
   static_cast<const TDetectorHit*>(&frag)->Copy(*this);
   static_cast<const TDetectorHit*>(&frag)->CopyWave(*this);
   ClearTransients();
}

TBgoHit& TBgoHit::operator=(const TBgoHit& rhs)
{
   rhs.Copy(*this, true);

   return *this;
}

TBgoHit& TBgoHit::operator=(TBgoHit&& rhs) noexcept
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
