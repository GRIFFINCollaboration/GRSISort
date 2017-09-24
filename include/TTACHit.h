#ifndef TACHIT_H
#define TACHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TTACHit
///
/// This is class that contains the information about a TAC
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
#include "TGraph.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"

class TTACHit : public TGRSIDetectorHit {
public:
   TTACHit();
   ~TTACHit() override;
   TTACHit(const TTACHit&);
   TTACHit(const TFragment& frag) : TGRSIDetectorHit(frag) {}

private:
   Int_t fFilter{0};

public:
   /////////////////////////		/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; } //!<!

   /////////////////////////		/////////////////////////////////////
   inline Int_t GetFilterPattern() const { return fFilter; } //!<!

   bool InFilter(Int_t); //!<!

   Double_t GetTempCorrectedCharge(TGraph* correction_graph) const;
   Double_t TempCorrectedCharge(TGraph* correction_graph) const;
   Double_t GetTempCorrectedEnergy(TGraph* correction_graph) const;

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!
   void Copy(TObject&) const override;            //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTACHit, 1) // Stores the information for a TACrHit
   /// \endcond
};
/*! @} */
#endif
