#include "TGriffinBgo.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include "TRandom.h"
#include "TMath.h"
#include "TInterpreter.h"
#include "TMnemonic.h"

#include "TGRSIOptions.h"

////////////////////////////////////////////////////////////
//
// TGriffinBgo
//
// The TGriffinBgo class is just a differently name TBgo.
// This allows us to distinguish between the BGOs for diiferent
// detector classes.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TGriffinBgo)
/// \endcond

TGriffinBgo::TGriffinBgo() : TBgo()
{
	/// Default ctor.
   TBgo::Clear();
}

TGriffinBgo::TGriffinBgo(const TGriffinBgo& rhs) : TBgo()
{
	/// Copy ctor.
   rhs.Copy(*this);
}

TGriffinBgo::~TGriffinBgo()
{
   // Default Destructor
}

TGriffinBgo& TGriffinBgo::operator=(const TGriffinBgo& rhs)
{
   rhs.Copy(*this);
   return *this;
}
