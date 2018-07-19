#include "TTdrCloverBgo.h"
#include "TTdrCloverBgoHit.h"

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

void TTdrCloverBgo::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   // Builds the BGO Hits directly from the TFragment. Basically, loops through the hits for an event and sets
   // observables.
   if(frag == nullptr || chan == nullptr) {
      return;
   }

	TTdrCloverBgoHit* hit = new TTdrCloverBgoHit(*frag);
	fBgoHits.push_back(std::move(hit));
}

TTdrCloverBgoHit* TTdrCloverBgo::GetTdrCloverBgoHit(const Int_t& i)
{
	try {
		return static_cast<TTdrCloverBgoHit*>(fBgoHits.at(i));
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" Hits are out of range: "<<oor.what()<<std::endl;
		if(!gInterpreter) {
			throw grsi::exit_exception(1);
		}
	}
	return nullptr;
}

