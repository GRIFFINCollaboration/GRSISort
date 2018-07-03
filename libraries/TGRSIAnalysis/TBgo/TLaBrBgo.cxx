#include "TLaBrBgo.h"

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
// TLaBrBgo
//
// The TLaBrBgo class is just a differently name TBgo.
// This allows us to distinguish between the BGOs for diiferent
// detector classes.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TLaBrBgo)
/// \endcond

TLaBrBgo::TLaBrBgo() : TBgo()
{
	/// Default ctor.
   TBgo::Clear();
}

TLaBrBgo::TLaBrBgo(const TLaBrBgo& rhs) : TBgo()
{
	/// Copy ctor.
   rhs.Copy(*this);
}

TLaBrBgo::~TLaBrBgo()
{
   // Default Destructor
}

TLaBrBgo& TLaBrBgo::operator=(const TLaBrBgo& rhs)
{
   rhs.Copy(*this);
   return *this;
}
