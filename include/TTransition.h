#ifndef TTRANSITION_H
#define TTRANSITION_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <cstdio>

#include "TClass.h"
#include "TObject.h"

/////////////////////////////////////////////////////////////////
///
/// \class TTransition
///
/// This Class contains the information about a nuclear
/// transition. These transitions are a part of a TNucleus
/// and are typically set within the TNucleus framework
///
/////////////////////////////////////////////////////////////////

class TTransition : public TObject {
   friend class TNucleus;

public:
   TTransition();
   ~TTransition() override;

   bool IsSortable() const override { return true; }
   int Compare(const TObject* obj) const override;
   int CompareIntensity(const TObject* obj) const;

   void SetEnergy(double& tmpenergy) { fEnergy = tmpenergy; }
   void SetEnergyUncertainty(double& tmperror) { fEngUncertainty = tmperror; }
   void SetIntensity(double& tmpintens) { fIntensity = tmpintens; }
   void SetIntensityUncertainty(double& tmpinterror) { fIntUncertainty = tmpinterror; }

   double GetEnergy() const { return fEnergy; }
   double GetEnergyUncertainty() const { return fEngUncertainty; }
   double GetIntensity() const { return fIntensity; }
   double GetIntensityUncertainty() const { return fIntUncertainty; }

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   std::string PrintToString();

private:
   double fEnergy{0.};         // Energy of the transition
   double fEngUncertainty{0.}; // Uncertainty in the energy of the transition
   double fIntensity{0.};      // Intensity of the transition
   double fIntUncertainty{0.}; // Uncertainty in the intensity

   /// \cond CLASSIMP
   ClassDefOverride(TTransition, 0) // Information about a TNucleus transition
   /// \endcond
};
/*! @} */
#endif
