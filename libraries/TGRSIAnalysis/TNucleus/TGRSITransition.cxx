
#include <TGRSITransition.h>

ClassImp(TGRSITransition)

////////////////////////////////////////////////////////////////
//                                                            //
// TGRSITransition                                            //
//                                                            //
// This Class contains the information about a nuclear 
// transition. These transitions are a part of a TNucleus
// and are typically set within the TNucleus framework
//                                                            //
////////////////////////////////////////////////////////////////

TGRSITransition::TGRSITransition() {
//Default constructor for TGRSITransition
  Class()->IgnoreTObjectStreamer(true);
  Clear();
}

TGRSITransition::~TGRSITransition() {
//Default Destructor
}

void TGRSITransition::Print(Option_t *opt) {
//Prints information about the TGRSITransition
   printf("**************************\n");
   printf("TGRSITransition:\n");
   printf("Energy:    %lf\t+/-%lf\n", energy, energy_uncertainty);
   printf("Intensity: %lf\t+/-%lf\n", intensity, intensity_uncertainty);
   printf("**************************\n");

}

void TGRSITransition::Clear(Option_t *opt){
   //Clears TGRSITransition
  energy                 = 0.0; 
  energy_uncertainty    = 0.0; 
  intensity              = 0.0; 
  intensity_uncertainty = 0.0;  
}

int TGRSITransition::Compare(const TObject *obj) const { 
//Compares the intensities of the TGRSITransitions and returns
//-1 if this >  obj
//0 if  this == obj
//1 if  this <  obj
   if(this->intensity > ((TGRSITransition*)obj)->intensity) 
      return -1;  
   if(this->intensity == ((TGRSITransition*)obj)->intensity) 
      return  0;  
   if(this->intensity < ((TGRSITransition*)obj)->intensity) 
      return  1;  
   printf("%s: Error, intensity neither greater, nor equal, nor smaller than provided intensity!\n",__PRETTY_FUNCTION__);
   return -9;
}


