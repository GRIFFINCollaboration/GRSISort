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
      virtual ~TTransition();

      bool IsSortable() const { return true; }
      int Compare(const TObject* obj) const;
      int CompareIntensity(const TObject* obj) const;

      void SetEnergy(double &tmpenergy) {fEnergy = tmpenergy;}
      void SetEnergyUncertainty(double &tmperror){ fEngUncertainty = tmperror;}
      void SetIntensity(double &tmpintens){fIntensity = tmpintens;}
      void SetIntensityUncertainty(double &tmpinterror){ fIntUncertainty = tmpinterror;}

      double GetEnergy() const {return fEnergy;}
      double GetEnergyUncertainty() const {return fEngUncertainty;}
      double GetIntensity() const {return fIntensity;}
      double GetIntensityUncertainty() const {return fIntUncertainty;}

      void Clear(Option_t* opt = "");
      void Print(Option_t* opt = "") const;

      std::string PrintToString();

   private:
      double fEnergy;           //Energy of the transition
      double fEngUncertainty;   //Uncertainty in the energy of the transition
      double fIntensity;        //Intensity of the transition
      double fIntUncertainty;   //Uncertainty in the intensity

/// \cond CLASSIMP
   ClassDef(TTransition,0) //Information about a TNucleus transition
/// \endcond
};
/*! @} */
#endif
