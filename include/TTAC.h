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
   virtual ~TTAC();
   TTAC(const TTAC& rhs);

public:
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0);
   void Copy(TObject& rhs) const;
   TTACHit* GetTACHit(const int& i);                           //!<!
   Short_t GetMultiplicity() const { return fTACHits.size(); } //!<!

#ifndef __CINT__
   void AddFragment(std::shared_ptr<const TFragment>, TChannel*); //!<!
#endif

   void ClearTransients()
   {
      for (auto hit : fTACHits) hit.ClearTransients();
   }

   TTAC& operator=(const TTAC&); //!<!

private:
   std::vector<TTACHit> fTACHits; //   The set of TAC hits

public:
   void Clear(Option_t* opt = "");       //!<!
   void Print(Option_t* opt = "") const; //!<!

   /// \cond CLASSIMP
   ClassDef(TTAC, 1) // TAC Physics structure
                     /// \endcond
};
/*! @} */
#endif
