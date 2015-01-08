#ifndef TGRSITRANSITION_H
#define TGRSITRANSITION_H

#include <cstdio>

#include "TClass.h"
#include "TObject.h"

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

   protected:
      double energy;
      double energy_uncertainty;
      double intensity;
      double intensity_uncertainty;

   ClassDef(TGRSITransition,1)
};

#endif

