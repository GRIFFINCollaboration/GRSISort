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
#include "TBadFragment.h"
#include "ThreadsafeQueue.h"

class TFragmentMap {
public:
#ifndef __CINT__
   TFragmentMap(std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>>& goodOutputQueue,
                std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TBadFragment>>>&           badOutputQueue);
#endif

#ifndef __CINT__
   bool Add(const std::shared_ptr<TFragment>&, const std::vector<Int_t>&, const std::vector<Short_t>&);
#endif

private:
   static bool fDebug;
#ifndef __CINT__
   void Solve(std::vector<std::shared_ptr<TFragment>>, std::vector<Float_t>, std::vector<Long_t>, int situation = -1);
   void DropFragments(std::pair<
                      std::multimap<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t>>>::iterator,
                      std::multimap<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t>>>::iterator>& range);

   std::multimap<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t>>> fMap;
   std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>>&                        fGoodOutputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TBadFragment>>>&                                  fBadOutputQueue;
#endif
};
/*! @} */
#endif
