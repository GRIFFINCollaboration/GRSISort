#include "TLaBrBgo.h"
#include "TLaBrBgoHit.h"

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

void TLaBrBgo::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   // Builds the BGO Hits directly from the TFragment. Basically, loops through the hits for an event and sets
   // observables.
   if(frag == nullptr || chan == nullptr) {
      return;
   }

	TLaBrBgoHit hit(*frag);
	fBgoHits.push_back(std::move(hit));
}

TLaBrBgoHit* TLaBrBgo::GetLaBrBgoHit(const Int_t& i)
{
	try {
		return static_cast<TLaBrBgoHit*>(&(fBgoHits.at(i)));
	} catch(const std::out_of_range& oor) {
		std::cerr<<ClassName()<<" Hits are out of range: "<<oor.what()<<std::endl;
		if(!gInterpreter) {
			throw grsi::exit_exception(1);
		}
	}
	return nullptr;
}

