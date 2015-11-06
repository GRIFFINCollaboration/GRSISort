#include "TS3Hit.h"

/// \cond CLASSIMP
ClassImp(TS3Hit)
/// \endcond

TS3Hit::TS3Hit()	{
	Clear();
}

TS3Hit::~TS3Hit()	{	}

void TS3Hit::Clear(Option_t *opt)	{
   TGRSIDetectorHit::Clear(opt);
   fLed            = -1;
   fRing           = -1;
   fSector         = -1;
}


void TS3Hit::Print(Option_t *opt) const	{
	printf("================\n");
	printf("not yet written.\n");
	printf("================\n");
}
