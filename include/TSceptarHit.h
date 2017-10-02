#ifndef SCEPTARHIT_H
#define SCEPTARHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TSceptarHit
///
/// This is class that contains the information about a sceptar
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>
#include <vector>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"

class TSceptarHit : public TGRSIDetectorHit {
public:
   TSceptarHit();
   ~TSceptarHit() override;
   TSceptarHit(const TSceptarHit&);
   TSceptarHit(const TFragment& frag);

private:
   Int_t fFilter{0};

public:
   /////////////////////////		/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; } //!<!

   /////////////////////////		/////////////////////////////////////
   inline Int_t GetFilterPattern() const { return fFilter; } //!<!

   Int_t CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow,
                      unsigned int interpolationSteps); //!<!
   Int_t CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow,
                                unsigned int interpolationSteps, std::vector<Short_t>& monitor);     //!<!
   std::vector<Short_t> CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow); //!<!
   std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow);                 //!<!

   bool InFilter(Int_t); //!<!

   bool     AnalyzeWaveform();                         //!<!
   TVector3 GetPosition(Double_t dist) const override; //!<!
   TVector3 GetPosition() const override;              //!<!

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!
   void Copy(TObject&) const override;            //!<!
   void Copy(TObject&, bool) const override;      //!<!

private:
   Double_t GetDefaultDistance() const { return 0.0; }

   /// \cond CLASSIMP
   ClassDefOverride(TSceptarHit, 2) // Stores the information for a SceptarHit
   /// \endcond
};
/*! @} */
#endif
