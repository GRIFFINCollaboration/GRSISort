#ifndef TFRAGMENTMAP_H
#define TFRAGMENTMAP_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TFragmentMap
///
/// The TFragmentMap calculates the charges of piled-up hits.
/// In the newest GRIFFIN data (starting with tests in 2016), piled-up hits
/// have 2*n-1 integrated charges reported, for the different integration areas.
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
	static bool fDebug;
#ifndef __CINT__
	std::multimap<UInt_t, std::tuple<TFragment*, std::vector<Int_t>, std::vector<Short_t> > > fMap;
	std::shared_ptr<ThreadsafeQueue<TFragment*> >& fGood_output_queue;
	std::shared_ptr<ThreadsafeQueue<TFragment*> >& fBad_output_queue;
#endif
};
/*! @} */
#endif
