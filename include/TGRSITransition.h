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
      ~TGRSITransition();

      bool IsSortable() const { return true; }
      int Compare(const TObject *obj) const;

      void SetEnergy(double &tmpenergy){energy = tmpenergy;}
      void SetEnergyUncertainty(double &tmperror){ energy_uncertainty = tmperror;}
      void SetIntensity(double &tmpintens){intensity = tmpintens;}
      void SetIntensityUncertainty(double &tmpinterror){ intensity_uncertainty = tmpinterror;}

      double GetEnergy() const {return energy;}
      double GetEnergyUncertainty() const {return energy_uncertainty;}
      double GetIntensity() const {return intensity;}
      double GetIntensityUncertainty() const {return intensity_uncertainty;}

      void Clear(Option_t *opt = "");
      void Print(Option_t *opt = "");

   protected:
      double energy;                  //Energy of the transition
      double energy_uncertainty;      //Uncertainty in the energy of the transition
      double intensity;               //Intensity of the transition
      double intensity_uncertainty;   //Uncertainty in the intensity

   ClassDef(TGRSITransition,1) //Information about a TNucleus transition
};

#endif

