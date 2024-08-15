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
   ~TTransition();
   TTransition(const TTransition&)     = default;
   TTransition(TTransition&&) noexcept = default;

   TTransition& operator=(const TTransition&)     = default;
   TTransition& operator=(TTransition&&) noexcept = default;

   bool IsSortable() const override { return true; }
   int  Compare(const TObject* obj) const override;
   int  CompareIntensity(const TObject* obj) const;
   int  CompareEnergy(const TObject* obj) const;

   void SetEnergy(const double& tmpenergy) { fEnergy = tmpenergy; }
   void SetEnergyUncertainty(const double& tmperror) { fEngUncertainty = tmperror; }
   void SetIntensity(const double& tmpintens) { fIntensity = tmpintens; }
   void SetIntensityUncertainty(const double& tmpinterror) { fIntUncertainty = tmpinterror; }
   void SetCompareIntensity(const bool& val) { fCompareIntensity = val; }

   double GetEnergy() const { return fEnergy; }
   double GetEnergyUncertainty() const { return fEngUncertainty; }
   double GetIntensity() const { return fIntensity; }
   double GetIntensityUncertainty() const { return fIntUncertainty; }

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   std::string PrintToString() const;

   bool operator>(const TTransition& rhs) const { return GetEnergy() > rhs.GetEnergy(); }
   bool operator<(const TTransition& rhs) const { return GetEnergy() < rhs.GetEnergy(); }

private:
   double fEnergy{0.};               ///< Energy of the transition
   double fEngUncertainty{0.};       ///< Uncertainty in the energy of the transition
   double fIntensity{0.};            ///< Intensity of the transition
   double fIntUncertainty{0.};       ///< Uncertainty in the intensity
   bool   fCompareIntensity{true};   ///< Whether to sort by intensity or energy

   /// \cond CLASSIMP
   ClassDefOverride(TTransition, 0) // NOLINT
   /// \endcond
};
/*! @} */
#endif
