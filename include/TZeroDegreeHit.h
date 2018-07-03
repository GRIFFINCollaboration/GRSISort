#ifndef ZERODEGREEHIT_H
#define ZERODEGREEHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// TZeroDegreeHit
///
/// This is class that contains the information about a ZDS
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"

class TZeroDegreeHit : public TGRSIDetectorHit {
public:
   TZeroDegreeHit();
   ~TZeroDegreeHit() override;
   TZeroDegreeHit(const TZeroDegreeHit&);
   TZeroDegreeHit(const TFragment& frag);

private:
   Int_t              fFilter{0};
   std::vector<short> fCfdMonitor;
   std::vector<int>   fPartialSum;

public:
   /////////////////////////		/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; } //!<!

   /////////////////////////		/////////////////////////////////////
   inline Int_t               GetFilterPattern() const { return fFilter; } //!<!
   inline std::vector<short>& GetCfdMonitor() { return fCfdMonitor; }
   inline std::vector<int>&   GetPartialSum() { return fPartialSum; }

   Int_t    GetCfd() const override;
   Int_t    GetRemainder() const;

   Int_t CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow,
                      unsigned int interpolationSteps); //!<!
   Int_t CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow,
                                unsigned int interpolationSteps, std::vector<Short_t>& monitor);     //!<!
   std::vector<Short_t> CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow); //!<!
   std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow);                 //!<!
   std::vector<Int_t> CalculatePartialSum();                                                         //!<!

   bool InFilter(Int_t); //!<!

   bool AnalyzeWaveform(); //!<!

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!
   void Copy(TObject&) const override;            //!<!
   void Copy(TObject&, bool) const override;      //!<!

   // Position Not written for ZeroDegree Yet

   /// \cond CLASSIMP
   ClassDefOverride(TZeroDegreeHit, 3) // Stores the information for a ZeroDegreeHit
   /// \endcond
};
/*! @} */
#endif
