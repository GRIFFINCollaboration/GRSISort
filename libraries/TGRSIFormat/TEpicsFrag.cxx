#include "TEpicsFrag.h"

#include <iostream>
#include <iomanip>

#include <time.h>

////////////////////////////////////////////////////////////////
//                                                            //
// TEpicsFrag                                                 //
//                                                            //
// This Class should contain all the information found in     //
// NOT typeid 1 midas events.                                 //
//                                                            //
//                                                            //
////////////////////////////////////////////////////////////////

ClassImp(TEpicsFrag)

TEpicsFrag::TEpicsFrag()  {
  MidasTimeStamp =  0;   
  MidasId        = -1;         
}

TEpicsFrag::~TEpicsFrag() { }

void TEpicsFrag::Clear(Option_t *opt) {
  MidasTimeStamp =  0;   
  MidasId        = -1;         

  Data.clear();  
  Name.clear();
  Unit.clear();
} 

void TEpicsFrag::Print(Option_t *opt) { 
   int largest = Data.size();
   if(Name.size()>largest) largest = Name.size();
   if(Unit.size()>largest) largest = Unit.size();
   printf("------ EPICS %i Varibles Found ------\n",largest);

   char buff[20];
   ctime(&MidasTimeStamp);
   struct tm * timeinfo = localtime(&MidasTimeStamp);
   strftime(buff,20,"%b %d %H:%M:%S",timeinfo);

   printf("  MidasTimeStamp: %s\n",buff);
   printf("  MidasId:    	  %i\n", MidasId);
   for(int i=0;i<largest;i++) {
      if(Name.size()>i) std::cout << std::setw(16) << Name.at(i);
      else std::cout << std::setw(16) << "";
      if(Data.size()>i) {
        std::cout << Data.at(i); 
        if(Unit.size()>i) std::cout << "\t" << Unit.at(i); 
      } else std::cout <<  "";
      std::cout << "\n";
    }
} 

