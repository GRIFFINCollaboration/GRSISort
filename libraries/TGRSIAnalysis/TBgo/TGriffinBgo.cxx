#include "TGriffinBgo.h"
#include "TGriffinBgoHit.h"

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

void TGriffinBgo::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   // Builds the BGO Hits directly from the TFragment. Basically, loops through the hits for an event and sets
   // observables.
   if(frag == nullptr || chan == nullptr) {
      return;
   }

	TGriffinBgoHit hit(*frag);
	fBgoHits.push_back(std::move(hit));
}

