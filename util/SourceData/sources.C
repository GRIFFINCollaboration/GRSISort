#include <vector>
#include <string>
#include <map>
#include "TMath.h"

struct gamma_ray{
  
   Double_t energy; //in keV.
   Double_t intensity; //in %.
 
} ;
 
 
struct nucleus{
 
   std::string name;
   std::vector<gamma_ray> transition;

   void SetTransition(Double_t ener, Double_t intens) { 
      gamma_ray tmpgr;  
      tmpgr.energy = ener;
      tmpgr.intensity = intens;
      transition.push_back(tmpgr);
   } 

} Co60; 

int sources(){

 //Might add more transitions, but there are the important ones
 Co60.name = "60Co";
 Co60.AddTransition(1173.228,99.85);
 Co60.AddTransition(1332.492,99.9826);


}
