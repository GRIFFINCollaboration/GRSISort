#include "TTdrTigressBgo.h"

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
// TTdrTigressBgo
//
// The TTdrTigressBgo class is just a differently name TBgo.
// This allows us to distinguish between the BGOs for diiferent
// detector classes.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TTdrTigressBgo)
/// \endcond

TTdrTigressBgo::TTdrTigressBgo() : TBgo()
{
	/// Default ctor.
   TBgo::Clear();
}

TTdrTigressBgo::TTdrTigressBgo(const TTdrTigressBgo& rhs) : TBgo()
{
	/// Copy ctor.
   rhs.Copy(*this);
}

TTdrTigressBgo::~TTdrTigressBgo()
{
   // Default Destructor
}

TTdrTigressBgo& TTdrTigressBgo::operator=(const TTdrTigressBgo& rhs)
{
   rhs.Copy(*this);
   return *this;
}
