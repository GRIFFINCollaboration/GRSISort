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

      void SetEnergy(double &tmpenergy){fenergy = tmpenergy;}
      void SetEnergyUncertainty(double &tmperror){ fenergy_uncertainty = tmperror;}
      void SetIntensity(double &tmpintens){fintensity = tmpintens;}
      void SetIntensityUncertainty(double &tmpinterror){ fintensity_uncertainty = tmpinterror;}

      double GetEnergy() const {return fenergy;}
      double GetEnergyUncertainty() const {return fenergy_uncertainty;}
      double GetIntensity() const {return fintensity;}
      double GetIntensityUncertainty() const {return fintensity_uncertainty;}

      void Clear(Option_t *opt = "");
      void Print(Option_t *opt = "") const;

      std::string PrintToString();

   protected:
      double fenergy;                  //Energy of the transition
      double fenergy_uncertainty;      //Uncertainty in the energy of the transition
      double fintensity;               //Intensity of the transition
      double fintensity_uncertainty;   //Uncertainty in the intensity

   ClassDef(TGRSITransition,1) //Information about a TNucleus transition
};

#endif

