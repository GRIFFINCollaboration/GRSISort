#ifndef TGRSITRANSITION_H
#define TGRSITRANSITION_H

#include <cstdio>

#include "TClass.h"
#include "TObject.h"

////////////////////////////////////////////////////////////////
//                                                            //
// TGRSITransition                                            //
//                                                            //
// This Class contains the information about a nuclear 
// transition. These transitions are a part of a TNucleus
// and are typically set within the TNucleus framework
//                                                            //
////////////////////////////////////////////////////////////////

class TGRSITransition : public TObject {
   friend class TNucleus;
   public:
      TGRSITransition();
      virtual ~TGRSITransition();

      bool IsSortable() const { return true; }
      int Compare(const TObject* obj) const;

      void SetEnergy(double &tmpenergy) {fEnergy = tmpenergy;}
      void SetEnergyUncertainty(double &tmperror){ fEnergyUncertainty = tmperror;}
      void SetIntensity(double &tmpintens){fIntensity = tmpintens;}
      void SetIntensityUncertainty(double &tmpinterror){ fIntensityUncertainty = tmpinterror;}

      double GetEnergy() const {return fEnergy;}
      double GetEnergyUncertainty() const {return fEnergyUncertainty;}
      double GetIntensity() const {return fIntensity;}
      double GetIntensityUncertainty() const {return fIntensityUncertainty;}

      void Clear(Option_t* opt = "");
      void Print(Option_t* opt = "") const;

      std::string PrintToString();

   protected:
      double fEnergy;                 //Energy of the transition
      double fEnergyUncertainty;      //Uncertainty in the energy of the transition
      double fIntensity;              //Intensity of the transition
      double fIntensityUncertainty;   //Uncertainty in the intensity

/// \cond CLASSIMP
   ClassDef(TGRSITransition,1) //Information about a TNucleus transition
/// \endcond
};

#endif
