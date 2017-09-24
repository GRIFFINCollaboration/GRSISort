#ifndef TPACES_H
#define TPACES_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TPacesHit.h"

class TPaces : public TGRSIDetector {

public:
   TPaces();
   TPaces(const TPaces&);
   ~TPaces() override;

public:
   TPacesHit* GetPacesHit(const int& i); //!<!
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override;
   Short_t GetMultiplicity() const override { return fPacesHits.size(); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;
#endif
   static TVector3 GetPosition(int DetNbr); //!<!

   void ClearTransients() override
   {
      for(const auto& hit : fPacesHits) {
         hit.ClearTransients();
      }
   }

   TPaces& operator=(const TPaces&); //!<!

private:
   std::vector<TPacesHit> fPacesHits; //  The set of crystal hits

   static bool fSetCoreWave; //!<!  Flag for Waveforms ON/OFF

public:
   static bool SetCoreWave() { return fSetCoreWave; } //!<!

   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "all") override;    //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TPaces, 4) // Paces Physics structure
   /// \endcond
};
/*! @} */
#endif
