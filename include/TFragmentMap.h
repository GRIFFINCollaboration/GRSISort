#ifndef TFRAGMENTMAP_H
#define TFRAGMENTMAP_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TDataParser
///
/// The TDataParser is the DAQ dependent part of GRSISort.
/// It takes a "DAQ-dependent"-flavoured MIDAS file and
/// converts it into a generic TFragment that the rest of
/// GRSISort can deal with. This is where event word masks
/// are applied, and any changes to the event format must
/// be implemented.
///
/////////////////////////////////////////////////////////////////

#include <map>
#include <vector>
#ifndef __CINT__
#include <tuple>
#endif
#include "TFragment.h"

class TFragmentMap {
	public:
	TFragmentMap() {};
	~TFragmentMap() {};
	bool Add(TFragment*, std::vector<Int_t>, std::vector<UShort_t>);
	
	private:
#ifndef __CINT__
	std::multimap<UInt_t, std::tuple<TFragment*, std::vector<Int_t>, std::vector<UShort_t> > > fMap;
#endif
};
/*! @} */
#endif
