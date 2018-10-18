#ifndef TSUPPRESSED_H
#define TSUPPRESSED_H

#include "TDetector.h"
#include "TDetectorHit.h"
#include "TBgo.h"

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TSuppressed
///
/// This is an abstract class that adds basic functionality for
/// Compton suppressed detectors like GRIFFIN.
///
/////////////////////////////////////////////////////////////////

class TSuppressed : public TDetector {
public:
	TSuppressed() : TDetector() {}
	~TSuppressed() {}

	virtual bool AddbackCriterion(const TDetectorHit*, const TDetectorHit*) { return false; }
	virtual bool SuppressionCriterion(const TDetectorHit*, const TDetectorHit*) { return false; }

   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "all") override;    //!<!
   
protected:
	template<class T>
	void CreateAddback(const std::vector<T>& hits, std::vector<T>& addbacks, std::vector<UShort_t>& nofFragments)
	{
		/// This funxtion always(!) re-creates the vectors of addback hits and number of fragments per addback hit based on the provided vector of hits
		addbacks.clear();
		nofFragments.clear();
		size_t j;
		for(auto hit : hits) {
			//check for each existing addback hit if this hit should be added to it
			for(j = 0; j < addbacks.size(); ++j) {
				if(AddbackCriterion(addbacks[j], hit)) {
					addbacks[j]->Add(hit);
					// copy constructor does not copy the bit field, so we need to set it
					addbacks[j]->SetHitBit(TDetectorHit::EBitFlag::kIsEnergySet); // this must be set for summed hits
					addbacks[j]->SetHitBit(TDetectorHit::EBitFlag::kIsTimeSet);   // this must be set for summed hits
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

	template<class T>
	void CreateSuppressed(const TBgo* bgo, const std::vector<T>& hits, std::vector<T>& suppressedHits)
	{
		/// This function always(!) re-creates the vector of suppressed hits based on the provided TBgo and vector of hits
		suppressedHits.clear();
		for(auto hit : hits) {
			bool suppress = false;
         if(bgo != nullptr) {
            for(auto b : bgo->GetHitVector()) {
               if(SuppressionCriterion(hit, b)) {
                  suppress = true;
                  break;
               }
            }
         }
			if(!suppress) suppressedHits.push_back(hit);
		}
	}

	template<class T>
	void CreateSuppressedAddback(const TBgo* bgo, const std::vector<T>& hits, std::vector<T>& addbacks, std::vector<UShort_t>& nofFragments)
	{
		/// This funxtion always(!) re-creates the vectors of suppressed addback hits and number of fragments per suppressed addback hit based on the provided TBgo and vector of hits
		addbacks.clear();
		nofFragments.clear();
		size_t j;
		for(auto hit : hits) {
			// check if this hit is suppressed
			bool suppress = false;
         if(bgo != nullptr){
			   for(auto b : bgo->GetHitVector()) {
				   if(SuppressionCriterion(hit, b)) {
					   suppress = true;
					   break;
				   }
			   }
         }
			//check for each existing addback hit if this hit should be added to it
			for(j = 0; j < addbacks.size(); ++j) {
				if(AddbackCriterion(addbacks[j], hit)) {
					// if this his is suppressed we need to suppress the whole addback event
					if(suppress) {
						addbacks.erase(addbacks.begin()+j);
						nofFragments.erase(nofFragments.begin()+j);
						break;
					}
					addbacks[j]->Add(hit);
					// copy constructor does not copy the bit field, so we need to set it
					addbacks[j]->SetHitBit(TDetectorHit::EBitFlag::kIsEnergySet); // this must be set for summed hits
					addbacks[j]->SetHitBit(TDetectorHit::EBitFlag::kIsTimeSet);   // this must be set for summed hits
					++(nofFragments.at(j));
					break;
				}
			}
			// if we haven't found an addback hit to add this hit to, or if there are no addback hits yet we create a new addback hit
			// unless this hit was suppressed in which case we don't want to create a new addback
			// this also covers the case where the last addback hit was just removed
			if(j == addbacks.size() && !suppress) {
				addbacks.push_back(hit);
				nofFragments.push_back(1);
			}
		}
	}

	/// \cond CLASSIMP
	ClassDefOverride(TSuppressed, 1)
	/// \endcond
};
/*! @} */
#endif
