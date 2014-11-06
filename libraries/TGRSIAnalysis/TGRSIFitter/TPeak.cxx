#include "TPeak.h"

ClassImp(TPeak)


void TPeak::SetType(Option_t * type){
// This sets the style of gaussian fit function to use for the fitted peak.

   if(strchr(type,'g') != NULL){
      //Gaussian
   }
   if(strchr(type,'s') != NULL){
      //skewed gaussian
   }
   if(strchr(type,'c') != NULL){
      //include a step function to the background
   }

}
