#include "TFragmentMap.h"

#include <iterator>

bool TFragmentMap::fDebug = false;

TFragmentMap::TFragmentMap(std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > > >& good_output_queue,
      			            std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > >& bad_output_queue)
  : fGoodOutputQueue(good_output_queue), fBadOutputQueue(bad_output_queue) { }

bool TFragmentMap::Add(std::shared_ptr<TFragment> frag, std::vector<Int_t> charge, std::vector<Short_t> integrationLength) {
	if(fDebug) {
		std::cout<<"Adding fragment "<<frag<<" (address "<<frag->GetAddress()<<" , # pileups "<<frag->GetNumberOfPileups()<<") with "<<charge.size()<<" charges and "<<integrationLength.size()<<" k-values:"<<std::endl;
		for(size_t i = 0; i < charge.size() && i < integrationLength.size(); ++i) {
			std::cout<<"\t"<<charge[i]<<",\t"<<integrationLength[i]<<std::endl;
		}
	}
	// a single fragment with just one charge/integration length can be directly put into the queue
	if(charge.size() == 1 && fMap.count(frag->GetAddress()) == 0) {
		frag->SetCharge(charge[0]);
		frag->SetKValue(integrationLength[0]);
		if(fDebug) std::cout<<"address "<<frag->GetAddress()<<": added single fragment "<<frag<<std::endl;
		if(fDebug && integrationLength[0] != 700) {
			std::cout<<"single fragment (address 0x"<<std::hex<<frag->GetAddress()<<std::dec<<") with integration length "<<integrationLength[0]<<", # pileups "<<frag->GetNumberOfPileups()<<std::endl;
			if(frag->GetNumberOfPileups() > 1) {
				std::cout<<"have fragments:"<<std::endl;
				for(auto it = fMap.begin(); it != fMap.end(); ++it) {
					std::cout<<"\t0x"<<std::hex<<std::get<0>((*it).second)->GetAddress()<<std::dec<<": "<<std::get<0>((*it).second)->GetNumberOfPileups()<<std::endl;
				}
			}
		}
		frag->SetEntryNumber();
      for(auto outputQueue : fGoodOutputQueue) {
		   outputQueue->Push(frag);
      }
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
		if(fDebug) {
			std::cout<<"address "<<frag->GetAddress()<<": inserting fragment "<<frag<<" with "<<charge.size()<<" charges"<<std::endl;
		}
		if(range.first != range.second) {
			fMap.insert(range.second, std::pair<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t> > > (frag->GetAddress(), std::make_tuple(frag, charge, integrationLength)));
		} else {
			fMap.insert(std::pair<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t> > > (frag->GetAddress(), std::make_tuple(frag, charge, integrationLength)));
		}
		if(fDebug) std::cout<<"done"<<std::endl;
		return true;
	}
	if(fDebug) std::cout<<"address "<<frag->GetAddress()<<": last fragment found, calculating charges for "<<nofFrags<<" fragments"<<std::endl;
	// last fragment:
	// now we can loop over the stored fragments and the current fragment and calculate all charges
	std::vector<std::shared_ptr<TFragment> > frags; // all fragments
	std::vector<Long_t> k2; // all integration lengths squared
	std::vector<Float_t> c; // all charges (not integrated charges, but integrated charge divided by integration length!)
	int situation = -1; // flag to select different scenarios for the time sequence of multiple hits
	switch(nofFrags) {
		case 2: // only one option: (2, 1)
			{
				if(charge.size() != 1) {
					DropFragments(range);
					if(fDebug) std::cout<<"2 w/o single charge"<<std::endl;
					return false;
				}
				// create and fill the vector of fragments
				frags.push_back(std::get<0>((*(range.first)).second));
				frags.push_back(frag);
				// create and fill the vector of all integration lengths
				if(fDebug) std::cout<<"inserting ... "<<std::endl;
				k2.insert(k2.begin(), std::get<2>((*(range.first)).second).begin(), std::get<2>((*(range.first)).second).end());
				if(fDebug) std::cout<<"done"<<std::endl;
				k2.push_back(integrationLength[0]);
				// create and fill the vector of all charges
				// we need the actual charges, not the integrated ones, so we calculate them now
				int dropped = -1;
				for(size_t i = 0; i < std::get<1>((*(range.first)).second).size(); ++i) { 
					if(k2[i] > 0) {
						c.push_back((std::get<1>((*(range.first)).second)[i] + gRandom->Uniform())/k2[i]);
						if(fDebug) std::cout<<"2, "<<i<<std::hex<<": 0x"<<std::get<1>((*(range.first)).second)[i]<<"/0x"<<k2[i]<<std::dec<<" = "<<(std::get<1>((*(range.first)).second)[i] + gRandom->Uniform())<<"/"<<k2[i]<<" = "<<c.back()<<std::endl;
					} else {
						//drop this charge, it's no good
						if(dropped >= 0) { // we've already dropped one, so we don't have enough left
							DropFragments(range);
							if(fDebug) std::cout<<"2 too much dropped"<<std::endl;
							return false;
						}
						if(fDebug) std::cout<<"2, dropping "<<i<<std::endl;
						dropped = i;
					}
				}
				if(dropped >= 0 && integrationLength[0] <= 0) { // we've already dropped one, so we don't have enough left
					DropFragments(range);
					if(fDebug) std::cout<<"2 too much dropped (end)"<<std::endl;
					return false;
				}
				if(integrationLength[0] <= 0) { dropped = 2; }
				//we've dropped one, so we use the other two charges to set the fragment charges directly
				//this should never happen, if the two hits are too close to get an integration of their individual charges, we don't see them as two hits (and we would miss both of them)
				//if they are too far apart to get an integration of their sum, they're not piled up
				if(fDebug) std::cout<<"dropped = "<<dropped<<", c.size() = "<<c.size()<<std::endl;
				switch(dropped) {
					case 0: // dropped e0, so only e0+e1 and e1 are left
						frags[0]->SetCharge(c[0]-charge[0]/integrationLength[0]);
						frags[1]->SetCharge(charge[0]);
						frags[0]->SetKValue(1);
						frags[1]->SetKValue(1);
						frags[0]->SetNumberOfPileups(-200);
						frags[1]->SetNumberOfPileups(-201);
						break;
					case 1: // dropped e0+e1, so only e0 and e1 are left
						frags[0]->SetCharge(c[0]);
						frags[1]->SetCharge(charge[0]/integrationLength[0]);
						frags[0]->SetKValue(1);
						frags[1]->SetKValue(1);
						frags[0]->SetNumberOfPileups(-200);
						frags[1]->SetNumberOfPileups(-201);
						break;
					case 2: // dropped e1, so only e0 and e0+e1 are left
						frags[0]->SetCharge(c[0]);
						frags[1]->SetCharge(c[1]-c[0]);
						frags[0]->SetKValue(1);
						frags[1]->SetKValue(1);
						frags[0]->SetNumberOfPileups(-200);
						frags[1]->SetNumberOfPileups(-201);
						break;
					default: // dropped none
						c.push_back((charge[0] + gRandom->Uniform())/integrationLength[0]);
						if(fDebug) std::cout<<std::hex<<"2, -: 0x"<<charge[0]<<"/0x"<<integrationLength[0]<<std::dec<<" = "<<(charge[0] + gRandom->Uniform())<<"/"<<integrationLength[0]<<" = "<<c.back()<<std::endl;
						//all k's are needed squared so we square all elements of k
						for(auto it = k2.begin(); it != k2.end(); ++it) { (*it) = (*it)*(*it); }
						Solve(frags, c, k2);
						break;
				}
			}
			break;
		case 3: // two options: (3, 1, 1), (2, 2, 1)
			{
				if(charge.size() != 1) {
					DropFragments(range);
					if(fDebug) std::cout<<"3 w/o single charge"<<std::endl;
					return false;
				}
				// create and fill the vector of fragments
				frags.push_back(std::get<0>((*(range.first)).second));
				frags.push_back(std::get<0>((*std::next(range.first)).second));
				frags.push_back(frag);
				// create and fill the vector of all integration lengths
				if(fDebug) std::cout<<"inserting first ... "<<std::endl;
				k2.insert(k2.begin(), std::get<2>((*(range.first)).second).begin(), std::get<2>((*(range.first)).second).end());
				if(fDebug) std::cout<<"done"<<std::endl;
				situation = k2.size();
				if(fDebug) std::cout<<"inserting second ... "<<std::endl;
				k2.insert(k2.end(), std::get<2>((*std::next(range.first)).second).begin(), std::get<2>((*std::next(range.first)).second).end());
				if(fDebug) std::cout<<"done"<<std::endl;
				k2.push_back(integrationLength[0]);
				// create and fill the vector of all charges
				// we need the actual charges, not the integrated ones, so we calculate them now
				std::vector<int> dropped;
				for(size_t i = 0; i < std::get<1>((*(range.first)).second).size(); ++i) { 
					if(k2[i] > 0) {
						c.push_back((std::get<1>((*(range.first)).second)[i] + gRandom->Uniform())/k2[i]); 
						if(fDebug) std::cout<<"3, "<<i<<std::hex<<": 0x"<<std::get<1>((*(range.first)).second)[i]<<"/0x"<<k2[i]<<std::dec<<" = "<<(std::get<1>((*(range.first)).second)[i] + gRandom->Uniform())<<"/"<<k2[i]<<" = "<<c.back()<<std::endl;
					} else {
						dropped.push_back(i);
						if(fDebug) std::cout<<"3, dropping "<<i<<std::endl;
					}
				}
				for(size_t i = 0; i < std::get<1>((*std::next(range.first)).second).size(); ++i) {
					if(k2[i+situation] > 0) {
						c.push_back((std::get<1>((*std::next(range.first)).second)[i] + gRandom->Uniform())/k2[i+situation]); 
						if(fDebug) std::cout<<"3, "<<i+situation<<std::hex<<": 0x"<<std::get<1>((*std::next(range.first)).second)[i]<<"/0x"<<k2[i+situation]<<std::dec<<" = "<<(std::get<1>((*std::next(range.first)).second)[i] + gRandom->Uniform())<<"/"<<k2[i+situation]<<" = "<<c.back()<<std::endl;
					} else {
						dropped.push_back(i+situation);
						if(fDebug) std::cout<<"dropping "<<i+situation<<std::endl;
					}
				}
				if(integrationLength[0] <= 0) { dropped.push_back(4); }
				switch(dropped.size()) {
					case 0: // dropped none
						c.push_back((charge[0] + gRandom->Uniform())/integrationLength[0]);
						if(fDebug) std::cout<<std::hex<<"3, -: 0x"<<charge[0]<<"/0x"<<integrationLength[0]<<std::dec<<" = "<<(charge[0] + gRandom->Uniform())<<"/"<<integrationLength[0]<<" = "<<c.back()<<std::endl;
						//all k's are needed squared so we square all elements of k
						for(auto it = k2.begin(); it != k2.end(); ++it) { (*it) = (*it)*(*it); }
						Solve(frags, c, k2, situation);
						break;
					case 1: // dropped one
						//don't know how to handle these right now
						DropFragments(range);
						return false;
						break;
					case 2: // dropped two => as many left as there are fragments
						DropFragments(range);
						if(fDebug) std::cout<<"3, double drop"<<std::endl;
						return false;
						switch(dropped[0]) {
							case 0:
								switch(dropped[1]) {
									case 1: // e0+e1+e2, e2, e3
										break;
									case 2:
										break;
									case 3:
										break;
									case 4:
										break;
								}
								break;
						}
						break;
					default: // dropped too many
						DropFragments(range);
						if(fDebug) std::cout<<"3, dropped too many"<<std::endl;
						return false;
				}
			}
			break;
		case 4: // five options: (4, 1, 1, 1), (3, 2, 1, 1), (3, 1, 2, 1), (2, 3, 1, 1), (2, 2, 2, 1)
			//break;
		default:
			//std::cerr<<"address "<<frag->GetAddress()<<": deconvolution of "<<nofCharges<<" charges for "<<nofFrags<<" fragments not implemented yet!"<<std::endl;
			DropFragments(range);
			return false;
			break;
	} // switch(nofFrags)
	// add all fragments to queue
	int i = 0;
	for(auto it = range.first; it != range.second; ++it) {
		frag->SetEntryNumber();
      for(auto outputQueue : fGoodOutputQueue) {
         outputQueue->Push(std::get<0>((*it).second));
      }
		if(fDebug) std::cout<<"Added "<<++i<<". fragment "<<std::get<0>((*it).second)<<std::endl;
	}
	frag->SetEntryNumber();
   for(auto outputQueue : fGoodOutputQueue) {
      outputQueue->Push(frag);
   }
	if(fDebug) std::cout<<"address "<<frag->GetAddress()<<": added last fragment "<<frag<<std::endl;
	// remove these fragments from the map
	fMap.erase(range.first, range.second);

	return true;
}

void TFragmentMap::Solve(std::vector<std::shared_ptr<TFragment> > frag, std::vector<Float_t> c, std::vector<Long_t> k2, int situation) {
	switch(frag.size()) {
		case 2:
			frag[0]->SetCharge((c[0]*(k2[0]*k2[1]+k2[0]*k2[2])+(c[1]-c[2])*k2[1]*k2[2])/(k2[0]*k2[1]+k2[0]*k2[2]+k2[1]*k2[2]));
			frag[1]->SetCharge((c[2]*(k2[0]*k2[2]+k2[1]*k2[2])+(c[1]-c[0])*k2[0]*k2[1])/(k2[0]*k2[1]+k2[0]*k2[2]+k2[1]*k2[2]));
			frag[0]->SetKValue(1);
			frag[1]->SetKValue(1);
			frag[0]->SetNumberOfPileups(-2);
			frag[1]->SetNumberOfPileups(-20);
			if(fDebug) {
				std::cout<<"2: charges "<<c[0]<<", "<<c[1]<<", "<<c[2]<<" and squared int. lengths "<<k2[0]<<", "<<k2[1]<<", "<<k2[2]<<" => "<<frag[0]->GetCharge()<<", "<<frag[1]->GetCharge()<<std::endl
				         <<"\t("<<(c[0]*(k2[0]*k2[1]+k2[0]*k2[2])+(c[1]-c[2])*k2[1]*k2[2])/(k2[0]*k2[1]+k2[0]*k2[2]+k2[1]*k2[2])<<", "<<(c[2]*(k2[0]*k2[2]+k2[1]*k2[2])+(c[1]-c[0])*k2[0]*k2[1])/(k2[0]*k2[1]+k2[0]*k2[2]+k2[1]*k2[2])<<" = "<<(c[0]*(k2[0]*k2[1]+k2[0]*k2[2])+(c[1]-c[2])*k2[1]*k2[2])<<"/"<<(k2[0]*k2[1]+k2[0]*k2[2]+k2[1]*k2[2])<<" = ("<<c[0]*(k2[0]*k2[1]+k2[0]*k2[2])<<"+"<<(c[1]-c[2])*k2[1]*k2[2]<<")/("<<k2[0]*k2[1]<<"+"<<k2[0]*k2[2]<<"+"<<k2[1]*k2[2]<<"))"<<std::endl
				         <<"pileups = "<<frag[0]->GetNumberOfPileups()<<", "<<frag[1]->GetNumberOfPileups()<<std::endl;
			}
			break;
		case 3:
			if(situation == 3) { //(3, 1, 1)
				frag[0]->SetCharge((c[0]*(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4])+(c[1]+c[4])*k2[1]*k2[3]*k2[4]+c[2]*(k2[1]*k2[2]*k2[3]+k2[2]*k2[3]*k2[4])-c[3]*(k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]))/(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]));
				frag[1]->SetCharge((c[0]*(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4])-c[1]*(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[1]*k2[2]*k2[3])+c[2]*(k2[1]*k2[2]*k2[3]-k2[0]*k2[2]*k2[4])-c[3]*(k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4])+c[4]*(k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]))/(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]));
				frag[2]->SetCharge(((c[0]+c[3])*k2[0]*k2[1]*k2[3]+c[1]*(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[1]*k2[2]*k2[3])+c[2]*(k2[0]*k2[1]*k2[2]+k2[1]*k2[2]*k2[3])+c[4]*(k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]))/(k2[0]*k2[1]*k2[2]+k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[3]*k2[4]+k2[2]*k2[3]*k2[4]));
			} else { //(2, 2, 1)
				frag[0]->SetCharge((c[0]*(k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[3]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4])+c[1]*(k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4]+k2[1]*k2[3]*k2[4])-c[2]*(k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4])-(c[3]-c[4])*k2[1]*k2[3]*k2[4])/(k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[3]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4]+k2[1]*k2[3]*k2[4]));
				frag[1]->SetCharge(-(c[0]*k2[0]*k2[1]*k2[3]+c[0]*k2[0]*k2[1]*k2[4]-c[1]*k2[0]*k2[1]*k2[3]-c[1]*k2[0]*k2[1]*k2[4]-c[2]*k2[0]*k2[2]*k2[3]-c[2]*k2[0]*k2[2]*k2[4]-c[2]*k2[1]*k2[2]*k2[3]-c[2]*k2[1]*k2[2]*k2[4]-c[3]*k2[0]*k2[3]*k2[4]-c[3]*k2[1]*k2[3]*k2[4]+c[4]*k2[0]*k2[3]*k2[4]+c[4]*k2[1]*k2[3]*k2[4])/(k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[3]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4]+k2[1]*k2[3]*k2[4]));
				frag[2]->SetCharge((c[0]*k2[0]*k2[1]*k2[3]-c[1]*k2[0]*k2[1]*k2[3]-c[2]*k2[0]*k2[2]*k2[3]-c[2]*k2[1]*k2[2]*k2[3]+c[3]*k2[0]*k2[1]*k2[3]+c[3]*k2[0]*k2[2]*k2[3]+c[3]*k2[1]*k2[2]*k2[3]+c[4]*k2[0]*k2[1]*k2[4]+c[4]*k2[0]*k2[2]*k2[4]+c[4]*k2[0]*k2[3]*k2[4]+c[4]*k2[1]*k2[2]*k2[4]+c[4]*k2[1]*k2[3]*k2[4])/(k2[0]*k2[1]*k2[3]+k2[0]*k2[1]*k2[4]+k2[0]*k2[2]*k2[3]+k2[0]*k2[2]*k2[4]+k2[0]*k2[3]*k2[4]+k2[1]*k2[2]*k2[3]+k2[1]*k2[2]*k2[4]+k2[1]*k2[3]*k2[4]));
			}
			frag[0]->SetKValue(1);
			frag[1]->SetKValue(1);
			frag[2]->SetKValue(1);
			frag[0]->SetNumberOfPileups(-3);
			frag[1]->SetNumberOfPileups(-30);
			frag[2]->SetNumberOfPileups(-31);
			if(fDebug) std::cout<<"3, situation "<<situation<<": charges "<<c[0]<<", "<<c[1]<<", "<<c[2]<<", "<<c[3]<<", "<<c[4]<<" and squared int. lengths "<<k2[0]<<", "<<k2[1]<<", "<<k2[2]<<", "<<k2[3]<<", "<<k2[4]<<" => "<<frag[0]->GetCharge()<<", "<<frag[1]->GetCharge()<<", "<<frag[2]->GetCharge()<<std::endl;
			break;
	}
}

void TFragmentMap::DropFragments(std::pair<std::multimap<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t> > >::iterator, 
		                                     std::multimap<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t> > >::iterator>& range) {
	// put the fragments into the bad output queue
	for(auto it = range.first; it != range.second; ++it) {
		for(auto outputQueue : fGoodOutputQueue) {
			fBadOutputQueue->Push(std::get<0>((*it).second));
		}
		if(fDebug) std::cout<<"Added bad fragment "<<std::get<0>((*it).second)<<std::endl;
	}
}
