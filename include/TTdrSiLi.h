#ifndef TTDRSILI_H
#define TTDRSILI_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TTdrSiLiHit.h"

class TTdrSiLi : public TGRSIDetector {

public:
   TTdrSiLi();
   TTdrSiLi(const TTdrSiLi&);
   ~TTdrSiLi() override;

public:
   TTdrSiLiHit* GetTdrSiLiHit(const int& i); //!<!
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override;
   Short_t GetMultiplicity() const override { return fTdrSiLiHits.size(); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;
#endif
   static TVector3 GetPosition(int DetNbr); //!<!

   void ClearTransients() override
   {
      for(const auto& hit : fTdrSiLiHits) {
         hit.ClearTransients();
      }
   }

   TTdrSiLi& operator=(const TTdrSiLi&); //!<!

private:
   std::vector<TTdrSiLiHit> fTdrSiLiHits; //  The set of crystal hits

   static bool fSetCoreWave; //!<!  Flag for Waveforms ON/OFF

public:
   static bool SetCoreWave() { return fSetCoreWave; } //!<!

   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "all") override;    //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTdrSiLi, 4) // TdrSiLi Physics structure
   /// \endcond
};
/*! @} */
#endif
