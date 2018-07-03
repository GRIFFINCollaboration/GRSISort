#include "TTdrCloverBgo.h"

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
// TTdrCloverBgo
//
// The TTdrCloverBgo class is just a differently name TBgo.
// This allows us to distinguish between the BGOs for diiferent
// detector classes.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TTdrCloverBgo)
/// \endcond

TTdrCloverBgo::TTdrCloverBgo() : TBgo()
{
	/// Default ctor.
   TBgo::Clear();
}

TTdrCloverBgo::TTdrCloverBgo(const TTdrCloverBgo& rhs) : TBgo()
{
	/// Copy ctor.
   rhs.Copy(*this);
}

TTdrCloverBgo::~TTdrCloverBgo()
{
   // Default Destructor
}

TTdrCloverBgo& TTdrCloverBgo::operator=(const TTdrCloverBgo& rhs)
{
   rhs.Copy(*this);
   return *this;
}
