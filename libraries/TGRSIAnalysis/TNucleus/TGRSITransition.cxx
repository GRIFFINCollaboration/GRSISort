
#include <TGRSITransition.h>


ClassImp(TGRSITransition)

TGRSITransition::TGRSITransition() {
  Class()->IgnoreTObjectStreamer(true);
  energy                 = 0.0; 
  energy_uncertainity    = 0.0; 
  intensity              = 0.0; 
  intensity_uncertainity = 0.0;  

}

TGRSITransition::~TGRSITransition() { }




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
}


