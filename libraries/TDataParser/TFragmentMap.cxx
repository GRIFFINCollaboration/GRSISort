#include "TFragmentMap.h"

#include <iterator>

#include "TFragmentQueue.h"

bool TFragmentMap::Add(TFragment* frag, std::vector<Int_t> charge, std::vector<UShort_t> integrationLength) {
  // a single fragment with just one charge/integration length can be directly put into the queue
  if(charge.size() == 1 && fMap.count(frag->GetAddress()) == 0) {
	 frag->SetCharge(charge[0]);
	 frag->SetKValue(integrationLength[0]);
	 TFragmentQueue::GetQueue("GOOD")->Add(frag);
	 return true;
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
	 return true;
  }
  // last fragment:
  // now we can loop over the stored fragments and the current fragment and calculate all charges
  std::vector<TFragment*> frags; // all fragments
  std::vector<UShort_t> k2; // all integration lengths squared
  std::vector<Float_t> c; // all charges (not integrated charges, but integrated charge divided by integration length!)
  int situation = -1; // flag to select different scenarios for the time sequence of multiple hits
  switch(nofFrags) {
  case 2: // only one option: (2, 1)
  {
	  if(charge.size() != 1) {
		  return false;
	  }
	  // create and fill the vector of fragments
	  frags.push_back(std::get<0>((*(range.first)).second));
	  frags.push_back(frag);
	  // create and fill the vector of all integration lengths
	  k2.insert(k2.begin(), std::get<2>((*(range.first)).second).begin(), std::get<2>((*(range.first)).second).end());
	  k2.push_back(integrationLength[0]);
	  // create and fill the vector of all charges
	  // we need the actual charges, not the integrated ones, so we calculate them now
	  std::vector<Float_t> c;
	  for(size_t i = 0; i < std::get<1>((*(range.first)).second).size(); ++i) { c.push_back((std::get<1>((*(range.first)).second)[i] + gRandom->Uniform())/k2[i]); }
	  c.push_back((charge[0] + gRandom->Uniform())/integrationLength[0]);
	  //all k's are needed squared so we square all elements of k
	  for(auto it = k2.begin(); it != k2.end(); ++it) { (*it) = (*it)*(*it); }
	  Solve(frags, c, k2);
  }
  case 3: // two options: (3, 1, 1), (2, 2, 1)
  {
	  if(charge.size() != 1) {
		  return false;
	  }
	  // create and fill the vector of fragments
	  frags.push_back(std::get<0>((*(range.first)).second));
	  frags.push_back(std::get<0>((*std::next(range.first)).second));
	  frags.push_back(frag);
	  // create and fill the vector of all integration lengths
	  k2.insert(k2.begin(), std::get<2>((*(range.first)).second).begin(), std::get<2>((*(range.first)).second).end());
	  if(k2.size() == 3) situation = 0;
	  else situation = 1;
	  k2.insert(k2.end(), std::get<2>((*std::next(range.first)).second).begin(), std::get<2>((*std::next(range.first)).second).end());
	  k2.push_back(integrationLength[0]);
	  // create and fill the vector of all charges
	  // we need the actual charges, not the integrated ones, so we calculate them now
	  std::vector<Float_t> c;
	  for(size_t i = 0; i < std::get<1>((*(range.first)).second).size(); ++i) { c.push_back((std::get<1>((*(range.first)).second)[i] + gRandom->Uniform())/k2[i]); }
	  for(size_t i = 0; i < std::get<1>((*std::next(range.first)).second).size(); ++i) { c.push_back((std::get<1>((*std::next(range.first)).second)[i] + gRandom->Uniform())/k2[i]); }
	  c.push_back((charge[0] + gRandom->Uniform())/integrationLength[0]);
	  //all k's are needed squared so we square all elements of k
	  for(auto it = k2.begin(); it != k2.end(); ++it) { (*it) = (*it)*(*it); }

	  Solve(frags, c, k2, situation);
  }
	  break;
  case 4: // five options: (4, 1, 1, 1), (3, 2, 1, 1), (3, 1, 2, 1), (2, 3, 1, 1), (2, 2, 2, 1)
	  break;
  default:
	  std::cerr<<"Deconvolution of "<<nofCharges<<" for "<<nofFrags<<" not implemented yet!"<<std::endl;
	  return false;
	  break;
  }
  // add all fragments to queue
  for(auto it = range.first; it != range.second; ++it) {
	 TFragmentQueue::GetQueue("GOOD")->Add(std::get<0>((*it).second));
  }
  TFragmentQueue::GetQueue("GOOD")->Add(frag);
  // remove these fragments from the map
  fMap.erase(range.first, range.second);

  return true;
}

void TFragmentMap::Solve(std::vector<TFragment*> frag, std::vector<Float_t> c, std::vector<UShort_t> k2, int situation) {
	switch(frag.size()) {
		case 2:
			frag[0]->SetCharge((c[0]*(k2[0]*k2[1]+k2[0]*k2[2])+(c[1]-c[2])*k2[1]*k2[2])/(k2[0]*k2[1]+k2[0]*k2[1]+k2[1]*k2[2]));
			frag[1]->SetCharge((c[2]*(k2[0]*k2[2]+k2[1]*k2[2])+(c[1]-c[0])*k2[0]*k2[1])/(k2[0]*k2[1]+k2[0]*k2[1]+k2[1]*k2[2]));
			frag[0]->SetKValue(1);
			frag[1]->SetKValue(1);
			break;
		case 3:
			if(situation == 0) {
				frag[0]->SetCharge((c[0]*(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4])+(c[1]+c[4])*k2[1]*k2[3]*k2[4]+c[2]*(k2[1]*k2[2]*k2[3]+k2[2]*k2[3]*k2[4])-c[3]*(k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]))/(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]));
				frag[1]->SetCharge((c[0]*(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4])-c[1]*(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[1]*k2[2]*k2[3])+c[2]*(k2[1]*k2[2]*k2[3]-k2[0]*k2[2]*k2[4])-c[3]*(k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4])+c[4]*(k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]))/(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]));
				frag[2]->SetCharge(((c[0]+c[3])*k2[0]*k2[1]*k2[3]+c[1]*(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[1]*k2[2]*k2[3])+c[2]*(k2[0]*k2[1]*k2[2]+k2[1]*k2[2]*k2[3])+c[4]*(k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]))/(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]));
			} else {
				frag[0]->SetCharge((c[0]*(k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[3]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4])+c[1]*(k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4]+k2[1]*k2[3]*k2[4])-c[2]*(k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4])-(c[3]-c[4])*k2[1]*k2[3]*k2[4])/(k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[3]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4]+k2[1]*k2[3]*k2[4]));
				frag[1]->SetCharge(-(c[0]*k2[0]*k2[1]*k2[3]+c[0]*k2[0]*k2[1]*k2[4]-c[1]*k2[0]*k2[1]*k2[3]-c[1]*k2[0]*k2[1]*k2[4]-c[2]*k2[0]*k2[2]*k2[3]-c[2]*k2[0]*k2[2]*k2[4]-c[2]*k2[1]*k2[2]*k2[3]-c[2]*k2[1]*k2[2]*k2[4]-c[3]*k2[0]*k2[3]*k2[4]-c[3]*k2[1]*k2[3]*k2[4]+c[4]*k2[0]*k2[3]*k2[4]+c[4]*k2[1]*k2[3]*k2[4])/(k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[3]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4]+k2[1]*k2[3]*k2[4]));
				frag[2]->SetCharge((c[0]*k2[0]*k2[1]*k2[3]-c[1]*k2[0]*k2[1]*k2[3]-c[2]*k2[0]*k2[2]*k2[3]-c[2]*k2[1]*k2[2]*k2[3]+c[3]*k2[0]*k2[1]*k2[3]+c[3]*k2[0]*k2[2]*k2[3]+c[3]*k2[1]*k2[2]*k2[3]+c[4]*k2[0]*k2[1]*k2[4]+c[4]*k2[0]*k2[2]*k2[4]+c[4]*k2[0]*k2[3]*k2[4]+c[4]*k2[1]*k2[2]*k2[4]+c[4]*k2[1]*k2[3]*k2[4])/(k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[3]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4]+k2[1]*k2[3]*k2[4]));
			}
			frag[0]->SetKValue(1);
			frag[1]->SetKValue(1);
			frag[2]->SetKValue(1);
			break;
	}
}
