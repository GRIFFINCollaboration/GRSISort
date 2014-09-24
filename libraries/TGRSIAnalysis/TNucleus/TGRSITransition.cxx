
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
   if(this->intensity > ((TGRSITransition*)obj)->intensity) 
      return -1;  
   if(this->intensity == ((TGRSITransition*)obj)->intensity) 
      return  0;  
   if(this->intensity < ((TGRSITransition*)obj)->intensity) 
      return  1;  
}


