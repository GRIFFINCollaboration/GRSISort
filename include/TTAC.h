#ifndef TTAC_H
#define TTAC_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TTAC
///
/// The TTAC class defines the observables and algorithms used
/// when analyzing TAC data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TTACHit.h"

class TTAC : public TGRSIDetector {
public:
   TTAC();
   ~TTAC() override;
   TTAC(const TTAC& rhs);

public:
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0);
   void Copy(TObject& rhs) const override;
   TTACHit* GetTACHit(const int& i);                           //!<!
   Short_t GetMultiplicity() const { return fTACHits.size(); } //!<!

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif

   void ClearTransients() override
   {
      for(const auto& hit : fTACHits) {
         hit.ClearTransients();
      }
   }

   TTAC& operator=(const TTAC&); //!<!

private:
   std::vector<TTACHit> fTACHits; //   The set of TAC hits

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTAC, 1) // TAC Physics structure
                             /// \endcond
};
/*! @} */
#endif
