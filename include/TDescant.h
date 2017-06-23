#ifndef TDESCANT_H
#define TDESCANT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TDescant
///
/// The TDescant class defines the observables and algorithms used
/// when analyzing GRIFFIN data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TDescantHit.h"

class TDescant : public TGRSIDetector {
public:
   TDescant();
   TDescant(const TDescant&);
   ~TDescant() override;

public:
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0);
   TDescantHit* GetDescantHit(const Int_t& idx = 0);

   Short_t GetMultiplicity() const { return fDescantHits.size(); } //!<!

   static TVector3 GetPosition(int DetNbr, double dist = 222); //!<!

#ifndef __CINT__
   void              AddFragment(std::shared_ptr<const TFragment>, TChannel*) override; //!<!
   TGRSIDetectorHit* CreateHit(std::shared_ptr<const TFragment> frag, TChannel*) { return new TDescantHit(*frag); }
#endif

   void ClearTransients() override
   {
      for (auto hit : fDescantHits) hit.ClearTransients();
   }

   TDescant& operator=(const TDescant&); //

private:
   std::vector<TDescantHit> fDescantHits; ///<  The set of crystal hits
   static bool              fSetWave;     ///<  Flag for Waveforms ON/OFF
   bool                     fHitFlag;     ///<   Is there a Descant hit?

public:
   static bool SetWave() { return fSetWave; }         //!<!
   void SetWave(bool flag) { fSetWave = flag; }       //!<!
   bool              Hit() { return fHitFlag; }       //!<!
   void SetHit(bool flag = true) { fHitFlag = flag; } //!<!
private:
   static TVector3 gPosition[71];         //!<!
   static TVector3 gAncillaryPosition[9]; //!<!
public:
   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TDescant, 1) // Descant Physics structure
                         /// \endcond
};
/*! @} */
#endif
