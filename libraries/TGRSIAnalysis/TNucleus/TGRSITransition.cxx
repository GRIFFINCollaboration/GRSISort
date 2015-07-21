
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
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
  Clear();
}

TGRSITransition::~TGRSITransition() {
//Default Destructor
}

void TGRSITransition::Print(Option_t *opt) const {
//Prints information about the TGRSITransition
   printf("**************************\n");
   printf("TGRSITransition:\n");
   printf("Energy:    %lf\t+/-%lf\n",fenergy,fenergy_uncertainty);
   printf("Intensity: %lf\t+/-%lf\n",fintensity,fintensity_uncertainty);
   printf("**************************\n");

}

std::string TGRSITransition::PrintToString(){
//Writes transitions in a way that is nicer to ourput.
  std::string buffer;
   buffer.append(Form("%lf\t",fenergy)); 
   buffer.append(Form("%lf\t",fenergy_uncertainty)); 
   buffer.append(Form("%lf\t",fintensity)); 
   buffer.append(Form("%lf\t",fintensity_uncertainty)); 
  return buffer;

}

void TGRSITransition::Clear(Option_t *opt){
   //Clears TGRSITransition
  fenergy                 = 0.0; 
  fenergy_uncertainty    = 0.0; 
  fintensity              = 0.0; 
  fintensity_uncertainty = 0.0;  
}

int TGRSITransition::Compare(const TObject *obj) const { 
//Compares the intensities of the TGRSITransitions and returns
//-1 if this >  obj
//0 if  this == obj
//1 if  this <  obj
   if(this->fintensity > ((TGRSITransition*)obj)->fintensity) 
      return -1;  
   else if(this->fintensity == ((TGRSITransition*)obj)->fintensity) 
      return  0;  
   else//(this->fintensity < ((TGRSITransition*)obj)->fintensity) 
      return  1;  
   printf("%s: Error, intensity neither greater, nor equal, nor smaller than provided intensity!\n",__PRETTY_FUNCTION__);
   return -9;
}


