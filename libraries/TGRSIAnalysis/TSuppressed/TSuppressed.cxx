#include "TSuppressed.h"

ClassImp(TSuppressed)

void TSuppressed::CreateAddback(const std::vector<TGRSIDetectorHit>& hits, std::vector<TGRSIDetectorHit>& addbacks, std::vector<UShort_t>& nofFragments)
{
	/// This funxtion always(!) re-creates the vectors of addback hits and number of fragments per addback hit based on the provided vector of hits
	addbacks.clear();
	nofFragments.clear();
	size_t j;
	for(auto hit : hits) {
		//check for each existing addback hit if this hit should be added to it
		for(j = 0; j < addbacks.size(); ++j) {
			if(fAddbackCriterion(addbacks[j], hit) {
				addbacks[j].Add(&hit);
				// copy constructor does not copy the bit field, so we need to set it
				addbacks[j].SetHitBit(TGRSIDetectorHit::EBitFlag::kIsEnergySet); // this must be set for summed hits
				addbacks[j].SetHitBit(TGRSIDetectorHit::EBitFlag::kIsTimeSet);   // this must be set for summed hits
				++(nofFragments.at(j));
				break;
			}
		}
		// if we haven't found an addback hit to add this hit to, or if there are no addback hits yet we create a new addback hit
		if(j == addbacks.size()) {
			addbacks.push_back(hit);
			nofFragments.push_back(1);
		}
	}
}

void TSuppressed::CreateSuppressed(const TBgo* bgo, const std::vector<TGRSIDetectorHit>& hits, std::vector<TGRSIDetectorHit>& suppressedHits)
{
	/// This function always(!) re-creates the vector of suppressed hits based on the provided TBgo and vector of hits
	suppressedHits.clear();
	for(auto hit : hits) {
		bool suppress = false;
		for(auto b : bgo->GetHitVector()) {
			if(fSuppressionCriterion(hit, b)) {
				suppress = true;
				break;
			}
		}
		if(!suppress) suppressedHits.push_back(hit);
	}
}

void TSuppressed::CreateSuppressedAddback(const TBgo* bgo, const std::vector<TGRSIDetectorHit>& hits, std::vector<TGRSIDetectorHit>& addbacks, std::vector<UShort_t>& nofFragments)
{
	/// This funxtion always(!) re-creates the vectors of suppressed addback hits and number of fragments per suppressed addback hit based on the provided TBgo and vector of hits
	addbacks.clear();
	nofFragments.clear();
	size_t j;
	for(auto hit : hits) {
		// check if this hit is suppressed
		bool suppress = false;
		for(auto b : bgo->GetHitVector()) {
			if(fSuppressionCriterion(hit, b)) {
				suppress = true;
				break;
			}
		}
		//check for each existing addback hit if this hit should be added to it
		for(j = 0; j < addbacks.size(); ++j) {
			if(fAddbackCriterion(addbacks[j], hit) {
				// if this his is suppressed we need to suppress the whole addback event
				if(suppress) {
					addbacks.erase(addbacks.begin()+j);
					nofFragments.erase(nofFragments.begin()+j);
					break;
				}
				addbacks[j].Add(&hit);
				// copy constructor does not copy the bit field, so we need to set it
				addbacks[j].SetHitBit(TGRSIDetectorHit::EBitFlag::kIsEnergySet); // this must be set for summed hits
				addbacks[j].SetHitBit(TGRSIDetectorHit::EBitFlag::kIsTimeSet);   // this must be set for summed hits
				++(nofFragments.at(j));
				break;
			}
		}
		// if we haven't found an addback hit to add this hit to, or if there are no addback hits yet we create a new addback hit
		// unless this hit was suppressed in which case we don't want to create a new addback
		// this also covers the case where the last addback hit was just removed
		if(j == addbacks.size() && !supress) {
			addbacks.push_back(hit);
			nofFragments.push_back(1);
		}
	}
}

