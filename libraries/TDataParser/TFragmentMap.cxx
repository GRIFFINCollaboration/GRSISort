#include "TFragmentMap.h"

#include "TFragmentQueue.h"

void TFragmentMap::Add(TFragment* frag, std::vector<Int_t> charge, std::vector<UShort_t> integrationLength) {
  // a single fragment with just one charge/integration length can be directly put into the queue
  if(charge.size() == 1 && fMap.count(frag->GetAddress()) == 0) {
	 frag->SetCharge(charge[0]);
	 frag->SetKValue(integrationLength[0]);
	 TFragmentQueue::GetQueue("GOOD")->Add(frag);
	 return;
  }
  // check if this is the last fragment needed
  int nofFrags = 1;
  int nofCharges = charge.size();
  auto range = fMap.equal_range(frag->GetAddress());
  for(auto it = range.first; it != range.second; ++it) {
	 ++nofFrags;
	 nofCharges += std::get<1>((*it).second).size();
  }
  // not the last fragment:
  if(nofCharges != 2*nofFrags - 1) {
	 //we need to insert this element into the map, and thanks to equal_range we already know where
	 fMap.insert(range.second, std::pair<UInt_t, std::tuple<TFragment*, std::vector<Int_t>, std::vector<UShort_t> > > (frag->GetAddress(), std::make_tuple(frag, charge, integrationLength)));
	 return;
  }
  // last fragment:
  // now we can loop over the stored fragments and the current fragment and calculate all charges
  switch(nofFrags) {
  case 2: // only one option: (2, 1)
	  break;
  case 3: // two options: (3, 1, 1), (2, 2, 1)
	  break;
  case 4: // five options: (4, 1, 1, 1), (3, 2, 1, 1), (3, 1, 2, 1), (2, 3, 1, 1), (2, 2, 2, 1)
	  break;
  default:
	  std::cerr<<"Deconvolution of "<<nofCharges<<" for "<<nofFrags<<" not implemented yet!"<<std::endl;
	  break;
  }
  // add all fragments to queue
  for(auto it = range.first; it != range.second; ++it) {
	 TFragmentQueue::GetQueue("GOOD")->Add(std::get<0>((*it).second));
  }
  TFragmentQueue::GetQueue("GOOD")->Add(frag);
  // remove these fragments from the map
  fMap.erase(range.first, range.second);
}
