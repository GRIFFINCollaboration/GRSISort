#include "TEpicsFrag.h"

#include <iostream>
#include <iomanip>

#include <time.h>

#include <TChannel.h>

////////////////////////////////////////////////////////////////
//                                                            //
// TEpicsFrag   TSCLRFrag                                     //
//                                                            //
// This Class should contain all the information found in     //
// NOT typeid 1 midas events. aka Epics (scaler) Events.      //
//                                                            //
//                                                            //
////////////////////////////////////////////////////////////////

ClassImp(TEpicsFrag)

TEpicsFrag::TEpicsFrag()  {
  //Default Constructor.
  fMidasTimeStamp =  0;   
  fMidasId        = -1;         
}

TEpicsFrag::~TEpicsFrag() { }

void TEpicsFrag::Clear(Option_t *opt) {
   //Clears the TEpicsFrag.
   fMidasTimeStamp =  0;   
   fMidasId        = -1;         

   fData.clear();  
   fName.clear();
   fUnit.clear();
} 

void TEpicsFrag::Print(Option_t *opt) const { 
   //Prints the TEpicsFrag. This includes Midas information as well the data
   //kep inside of the scaler.
   size_t largest = fData.size();
   if(fName.size()>largest) largest = fName.size();
   if(fUnit.size()>largest) largest = fUnit.size();
   printf("------ EPICS %i Varibles Found ------\n",(int)largest);

   char buff[20];
   ctime(&fMidasTimeStamp);
   struct tm* timeInfo = localtime(&fMidasTimeStamp);
   strftime(buff,20,"%b %d %H:%M:%S",timeInfo);

   printf("  MidasTimeStamp: %s\n", buff);
   printf("  MidasId:    	  %i\n", fMidasId);
   for(size_t i=0;i<largest;i++) {
      if(i < fName.size())
			std::cout << std::setw(16) << fName[i];
      else 
			std::cout << std::setw(16) << "";
      if(i < fData.size()) {
			std::cout << fData[i]; 
			if(i < fUnit.size()) 
				std::cout << "\t" << fUnit[i]; 
      } else
			std::cout <<  "";
      std::cout << std::endl;
	}
}
