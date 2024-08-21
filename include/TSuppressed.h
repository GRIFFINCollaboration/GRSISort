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
   TSuppressed()                                  = default;
   TSuppressed(const TSuppressed&)                = default;
   TSuppressed(TSuppressed&&) noexcept            = default;
   TSuppressed& operator=(const TSuppressed&)     = default;
   TSuppressed& operator=(TSuppressed&&) noexcept = default;
   ~TSuppressed()                                 = default;

   virtual bool AddbackCriterion(const TDetectorHit*, const TDetectorHit*) { return false; }
   virtual bool SuppressionCriterion(const TDetectorHit*, const TDetectorHit*) { return false; }

   void Copy(TObject&) const override;           //!<!
   void Clear(Option_t* opt = "all") override;   //!<!

protected:
   template <class T>
   void CreateAddback(const std::vector<T*>& hits, std::vector<T*>& addbacks, std::vector<UShort_t>& nofFragments)
   {
      /// This funxtion always(!) re-creates the vectors of addback hits and number of fragments per addback hit based on the provided vector of hits
      addbacks.clear();
      nofFragments.clear();
      size_t j = 0;
      for(auto hit : hits) {
         // check for each existing addback hit if this hit should be added to it
         for(j = 0; j < addbacks.size(); ++j) {
            if(AddbackCriterion(addbacks[j], hit)) {
               addbacks[j]->Add(hit);
               // copy constructor does not copy the bit field, so we need to set it
               addbacks[j]->SetHitBit(TDetectorHit::EBitFlag::kIsEnergySet);   // this must be set for summed hits
               addbacks[j]->SetHitBit(TDetectorHit::EBitFlag::kIsTimeSet);     // this must be set for summed hits
               ++(nofFragments.at(j));
               break;
            }
         }
         // if we haven't found an addback hit to add this hit to, or if there are no addback hits yet we create a new addback hit
         if(j == addbacks.size()) {
            /// Because the functions to return hit vectors etc. are almost always returning vectors of TDetectorHits, T is most likely TDetectorHit.
            /// This means we can't use T directly to create a new hit, we need to use TClass::New().
            T* tmpT = static_cast<T*>(hit->IsA()->New());
            *tmpT   = *hit;
            addbacks.push_back(tmpT);
            nofFragments.push_back(1);
         }
      }
   }

   template <class T>
   void CreateSuppressed(const TBgo* bgo, const std::vector<T*>& hits, std::vector<T*>& suppressedHits)
   {
      /// This function always(!) re-creates the vector of suppressed hits based on the provided TBgo and vector of hits
      suppressedHits.clear();
      for(auto hit : hits) {
         bool suppress = false;
         if(bgo != nullptr) {
            for(auto* b : bgo->GetHitVector()) {
               if(SuppressionCriterion(hit, b)) {
                  suppress = true;
                  break;
               }
            }
         }
         /// Because the functions to return hit vectors etc. are almost always returning vectors of TDetectorHits, T is most likely TDetectorHit.
         /// This means we can't use T directly to create a new hit, we need to use TClass::New().
         if(!suppress) {
            T* tmpT = static_cast<T*>(hit->IsA()->New());
            *tmpT   = *hit;
            suppressedHits.push_back(tmpT);
         }
      }
   }

   template <class T>
   void CreateSuppressedAddback(const TBgo* bgo, const std::vector<T*>& hits, std::vector<T*>& addbacks, std::vector<UShort_t>& nofFragments)
   {
      /// This funxtion always(!) re-creates the vectors of suppressed addback hits and number of fragments per suppressed addback hit based on the provided TBgo and vector of hits
      addbacks.clear();
      nofFragments.clear();
      size_t            j = 0;
      std::vector<bool> suppressed;
      for(auto hit : hits) {
         // check if this hit is suppressed
         bool suppress = false;
         if(bgo != nullptr) {
            for(auto* b : bgo->GetHitVector()) {
               if(SuppressionCriterion(hit, b)) {
                  suppress = true;
                  break;
               }
            }
         }
         // check for each existing addback hit if this hit should be added to it
         for(j = 0; j < addbacks.size(); ++j) {
            if(AddbackCriterion(addbacks[j], hit)) {
               addbacks[j]->Add(hit);
               // copy constructor does not copy the bit field, so we need to set it
               addbacks[j]->SetHitBit(TDetectorHit::EBitFlag::kIsEnergySet);   // this must be set for summed hits
               addbacks[j]->SetHitBit(TDetectorHit::EBitFlag::kIsTimeSet);     // this must be set for summed hits
               ++(nofFragments.at(j));
               if(suppress) {
                  suppressed[j] = true;
               }
               break;
            }
         }
         // if we haven't found an addback hit to add this hit to, or if there are no addback hits yet we create a new addback hit
         if(j == addbacks.size()) {
            /// Because the functions to return hit vectors etc. are almost always returning vectors of TDetectorHits, T is most likely TDetectorHit.
            /// This means we can't use T directly to create a new hit, we need to use TClass::New().
            T* tmpT = static_cast<T*>(hit->IsA()->New());
            *tmpT   = *hit;
            addbacks.push_back(tmpT);
            nofFragments.push_back(1);
            suppressed.push_back(suppress);
         }
      }
      // loop over all created addback hits and check if they contain a suppressed hit
      for(j = 0; j < addbacks.size(); ++j) {
         // if this hit is suppressed we need to suppress the whole addback event
         if(suppressed[j]) {
            addbacks.erase(addbacks.begin() + j);
            nofFragments.erase(nofFragments.begin() + j);
            suppressed.erase(suppressed.begin() + j);
            --j;
         }
      }
   }

   /// \cond CLASSIMP
   ClassDefOverride(TSuppressed, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
