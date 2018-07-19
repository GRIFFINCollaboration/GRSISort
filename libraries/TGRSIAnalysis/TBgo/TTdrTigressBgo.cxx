#include "TTdrTigressBgo.h"
#include "TTdrTigressBgoHit.h"

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

void TTdrTigressBgo::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   // Builds the BGO Hits directly from the TFragment. Basically, loops through the hits for an event and sets
   // observables.
   if(frag == nullptr || chan == nullptr) {
      return;
   }

	TTdrTigressBgoHit* hit = new TTdrTigressBgoHit(*frag);
	fBgoHits.push_back(std::move(hit));
}

TTdrTigressBgoHit* TTdrTigressBgo::GetTdrTigressBgoHit(const Int_t& i)
{
	try {
		return static_cast<TTdrTigressBgoHit*>(fBgoHits.at(i));
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" Hits are out of range: "<<oor.what()<<std::endl;
		if(!gInterpreter) {
			throw grsi::exit_exception(1);
		}
	}
	return nullptr;
}

