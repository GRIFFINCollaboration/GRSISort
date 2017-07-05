#ifndef TZERODEGREE_H
#define TZERODEGREE_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TZeroDegree
///
/// The TZeroDegree class defines the observables and algorithms used
/// when analyzing GRIFFIN data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TZeroDegreeHit.h"

class TZeroDegree : public TGRSIDetector {
public:
   TZeroDegree();
   ~TZeroDegree() override;
   TZeroDegree(const TZeroDegree& rhs);

public:
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override;
   void Copy(TObject& rhs) const override;
   TZeroDegreeHit* GetZeroDegreeHit(const int& i);                    //!<!
   Short_t GetMultiplicity() const override { return fZeroDegreeHits.size(); } //!<!

   static TVector3 GetPosition(double dist) { return TVector3(0, 0, dist); } //!<!

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif

   void ClearTransients() override
   {
      for(const auto& hit : fZeroDegreeHits) {
         hit.ClearTransients();
      }
   }

   TZeroDegree& operator=(const TZeroDegree&); //!<!

private:
   std::vector<TZeroDegreeHit> fZeroDegreeHits; ///<   The set of zeroDegree hits

   static bool fSetWave; ///<  Flag for Waveforms ON/OFF

public:
   static bool SetWave() { return fSetWave; } //!<!

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TZeroDegree, 2) // ZeroDegree Physics structure
                                    /// \endcond
};
/*! @} */
#endif
