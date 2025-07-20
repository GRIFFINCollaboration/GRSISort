#ifndef TDETECTOR_H
#define TDETECTOR_H

/** \addtogroup Detectors
 *  @{
 */

#include <stdexcept>
#include <cstdio>
#include <vector>
#ifndef __CINT__
#include <memory>
#endif

#include "TVector3.h"
#include "TObject.h"

#include "Globals.h"
#include "TFragment.h"
#include "TDetectorHit.h"
#include "TChannel.h"

/////////////////////////////////////////////////////////////////
///
/// \class TDetector
///
/// This is an abstract class that contains the basic info
/// about a detector. This is where the hits are built and
/// the data is filled. It's main role is to act as a wrapper
/// for every other type of detector system.
///
/////////////////////////////////////////////////////////////////

class TDetector : public TObject {
public:
   TDetector() = default;
   TDetector(const TDetector&);
   TDetector(TDetector&&) noexcept;
   ~TDetector();
   TDetector& operator=(const TDetector& other)
   {
      if(this != &other) {
         other.Copy(*this);
      }
      return *this;
   }
   TDetector& operator=(TDetector&& other) noexcept
   {
      if(this != &other) {
         other.Copy(*this);
      }
      return *this;
   }

   virtual void BuildHits() { AbstractMethod("BuildHits()"); }   //!<!
#ifndef __CINT__
   virtual void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*)
   {
      AbstractMethod("AddFragment()");
   }   //!<!
#endif

   virtual void AddHit(TDetectorHit* hit)
   {
      fHits.push_back(hit);
   }
   void         Copy(TObject&) const override;              //!<!
   void         Clear(Option_t* = "") override;             //!<!
   virtual void ClearTransients();                          //!<!
   void         Print(Option_t* opt = "") const override;   //!<!
   virtual void Print(std::ostream& out) const;

   virtual Short_t                           GetMultiplicity() const { return static_cast<Short_t>(fHits.size()); }
   virtual TDetectorHit*                     GetHit(const int& index) const;
   virtual const std::vector<TDetectorHit*>& GetHitVector() const { return fHits; }
   virtual bool                              NoHits() const { return fHits.empty(); }

   std::vector<TDetectorHit*>&       Hits() { return fHits; }
   const std::vector<TDetectorHit*>& Hits() const { return fHits; }

   friend std::ostream& operator<<(std::ostream& out, const TDetector& det)
   {
      det.Print(out);
      return out;
   }

private:
   std::vector<TDetectorHit*> fHits;

   /// \cond CLASSIMP
   ClassDefOverride(TDetector, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
