

#include "TS3Hit.h"

ClassImp(TS3Hit)

TS3Hit::TS3Hit()	{
	Clear();
}

TS3Hit::~TS3Hit()	{	}

void TS3Hit::Clear(Option_t *opt)	{
	
   position.SetXYZ(0,0,1);
   ring           = -1;
   sector         = -1;
   detectornumber = -1;
   energy         = 0.0; 
   cfd            = 0.0;
   charge         = -1;
   time           = -1;

}


void TS3Hit::Print(Option_t *opt) const	{
	printf("================\n");
	printf("not yet written.\n");
	printf("================\n");
}
