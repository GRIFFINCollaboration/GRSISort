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
#include <memory>
#endif
#include "TFragment.h"
#include "ThreadsafeQueue.h"

class TFragmentMap {
	public:
  //	TFragmentMap() {};

#ifndef __CINT__
	TFragmentMap(std::shared_ptr<ThreadsafeQueue<TFragment*> >& good_output_queue,
		     std::shared_ptr<ThreadsafeQueue<TFragment*> >& bad_output_queue);
#endif
	
	
	~TFragmentMap() {};
	bool Add(TFragment*, std::vector<Int_t>, std::vector<Short_t>);
	
	private:
	void Solve(std::vector<TFragment*>, std::vector<Float_t>, std::vector<Long_t>, int situation = -1);
#ifndef __CINT__
	std::multimap<UInt_t, std::tuple<TFragment*, std::vector<Int_t>, std::vector<Short_t> > > fMap;
	std::shared_ptr<ThreadsafeQueue<TFragment*> >& fGood_output_queue;
	std::shared_ptr<ThreadsafeQueue<TFragment*> >& fBad_output_queue;
#endif
};
/*! @} */
#endif
