#ifndef TSCEPTAR_H
#define TSCEPTAR_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TSceptar
///
/// The TSceptar class defines the observables and algorithms used
/// when analyzing GRIFFIN data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TSceptarHit.h"

class TSceptar : public TGRSIDetector {
public:
   TSceptar();
   ~TSceptar() override;
   TSceptar(const TSceptar& rhs);

public:
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override;
   void Copy(TObject& rhs) const override;
   TSceptarHit* GetSceptarHit(const int& i);                       //!<!
   Short_t GetMultiplicity() const override { return fSceptarHits.size(); } //!<!
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif

   static TVector3 GetPosition(int DetNbr) { return gPaddlePosition[DetNbr]; } //!<!

   void ClearTransients() override
   {
      for(const auto& hit : fSceptarHits) {
         hit.ClearTransients();
      }
   }

   TSceptar& operator=(const TSceptar&); //!<!

private:
   std::vector<TSceptarHit> fSceptarHits; //   The set of sceptar hits

   static bool fSetWave; //  Flag for Waveforms ON/OFF

public:
   static bool SetWave() { return fSetWave; } //!<!

private:
   static TVector3 gPaddlePosition[21]; //!<!  Position of each Paddle

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TSceptar, 2) // Sceptar Physics structure
                                 /// \endcond
};
/*! @} */
#endif
