#include "TGRSITransition.h"

#include <iostream>

TGRSITransition::TGRSITransition()
{
   // Default constructor for TGRSITransition
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

void TGRSITransition::Print(Option_t*) const
{
   // Prints information about the TGRSITransition
   std::cout << "**************************" << std::endl;
   std::cout << "TGRSITransition:" << std::endl;
   std::cout << "Energy:    " << fEnergy << " +/- " << fEnergyUncertainty << std::endl;
   std::cout << "Intensity: " << fIntensity << " +/- " << fIntensityUncertainty << std::endl;
   std::cout << "**************************" << std::endl;
}

std::string TGRSITransition::PrintToString() const
{
   // Writes transitions in a way that is nicer to ourput.
   std::string buffer;
   buffer.append(Form("%lf\t", fEnergy));
   buffer.append(Form("%lf\t", fEnergyUncertainty));
   buffer.append(Form("%lf\t", fIntensity));
   buffer.append(Form("%lf\t", fIntensityUncertainty));
   return buffer;
}

void TGRSITransition::Clear(Option_t*)
{
   // Clears TGRSITransition
   fEnergy               = 0.;
   fEnergyUncertainty    = 0.;
   fIntensity            = 0.;
   fIntensityUncertainty = 0.;
}

int TGRSITransition::Compare(const TObject* obj) const
{
   // Compares the intensities of the TGRSITransitions and returns
   //-1 if this >  obj
   // 0 if  this == obj
   // 1 if  this <  obj
   if(fIntensity > static_cast<const TGRSITransition*>(obj)->fIntensity) {
      return -1;
   }
   if(fIntensity == static_cast<const TGRSITransition*>(obj)->fIntensity) {
      return 0;
   }   //(fIntensity < static_cast<const TGRSITransition*>(obj)->fIntensity)
   return 1;

   std::cout << __PRETTY_FUNCTION__ << ": Error, intensity neither greater, nor equal, nor smaller than provided intensity!" << std::endl; // NOLINT
   return -9;
}
