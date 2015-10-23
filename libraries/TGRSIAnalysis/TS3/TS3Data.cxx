



#include <cstdio>
#include <fstream>
#include <string>


#include <TS3Data.h>




TS3Data::TS3Data()  {  }

TS3Data::~TS3Data()  {  }
     
void TS3Data::Clear(Option_t *opt)  {

  fS3_RingDetector.clear();
  fS3_RingNumber.clear();
  fS3_RingFragment.clear();

  fS3_SectorDetector.clear();
  fS3_SectorNumber.clear();
  fS3_SectorFragment.clear();
}


void TS3Data::Print(Option_t *opt) const  {
  printf("==============================\n");
  printf("this has not yet been written.\n");
  printf("==============================\n");
}
