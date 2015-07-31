#include "TEpicsFrag.h"

#include <iostream>
#include <iomanip>

#include <time.h>

#include <TChannel.h>

////////////////////////////////////////////////////////////////
//                                                            //
// TEpicsFrag   TSCLRFrag                                     //
//                                                            //
// These Classes should contain all the information found in  //
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

void TEpicsFrag::Print(Option_t *opt) const { 
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

ClassImp(TSCLRFrag) 

std::vector<Int_t > TSCLRFrag::AddressMap; 


TSCLRFrag::TSCLRFrag()  {
  Clear();
}

TSCLRFrag::~TSCLRFrag() { }



void TSCLRFrag::Clear(Option_t *opt) {
  MidasTimeStamp =  0;   
  MidasId        = -1;         

  if(!strcmp(opt,"ALL"))
     AddressMap.clear();

  Address.clear();
  Data1.clear();  
  Data2.clear();  
  Data3.clear();  
  Data4.clear();  
} 

void TSCLRFrag::Print(Option_t *opt) const { 
   printf("------ SCLR %i Unique Counters Found ------\n",Data1.size());

   char buff[20];
   ctime(&MidasTimeStamp);
   struct tm * timeinfo = localtime(&MidasTimeStamp);
   strftime(buff,20,"%b %d %H:%M:%S",timeinfo);

   printf("  MidasTimeStamp: %s\n",buff);
   printf("  MidasId:    	  %i\n", MidasId);

   if(!strcmp(opt,"LITE")) {
     printf("\tAddress[%i] at 0x%08x\n",Address.size(),&Address);
     printf("\tData1[%i] at 0x%08x\t%i\n",Data1.size(),&Data1,Data1.at(0));
     printf("\tData2[%i] at 0x%08x\n",Data2.size(),&Data2);
     printf("\tData3[%i] at 0x%08x\n",Data3.size(),&Data3);
     printf("\tData4[%i] at 0x%08x\n",Data4.size(),&Data4);
     return;
   }

   for(int i=0;i<Data1.size();i++) {
      TChannel *channel = 0;
      if(Address.size()>i) channel = TChannel::GetChannel(Address.at(i));
      printf("[%i]\t",i);
      if(channel) {
        printf("%s\t",channel->GetName());
      }
      printf("%lu\t",Data1.at(i));
      printf("%lu\t",Data2.at(i));
      printf("%lu\t",Data3.at(i));
      printf("%lu\n",Data4.at(i));

   }
} 

void TSCLRFrag::SetAddressMap(int *array, int size) { 
  //once we find where in the odb the order of the sclr is,
  //we need to write this function.  called in grsiloop so
  //we have access to the odb. 
  printf("TSCLRFrag::SetAddressMap() called,\n");
  return;
}


void TSCLRFrag::Copy(const TSCLRFrag &rhs) {
  Clear();
  ((TSCLRFrag&)(*this)).MidasTimeStamp = rhs.MidasTimeStamp;   
  ((TSCLRFrag&)(*this)).MidasId        = rhs.MidasId;         
  


  for(int x=0;x<rhs.Address.size();x++) { 
    (*((TSCLRFrag*)this)).Address.push_back(rhs.Address.at(x));
  }
  for(int x=0;x<rhs.Data1.size();x++) {
    ((TSCLRFrag&)(*this)).Data1.push_back(rhs.Data1.at(x));
    ((TSCLRFrag&)(*this)).Data2.push_back(rhs.Data1.at(x));
    ((TSCLRFrag&)(*this)).Data3.push_back(rhs.Data1.at(x));
    ((TSCLRFrag&)(*this)).Data4.push_back(rhs.Data1.at(x));
  }
}

