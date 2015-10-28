

#include "TS3Hit.h"

ClassImp(TS3Hit)

TS3Hit::TS3Hit()	{
	Clear();
}
TS3Hit::TS3Hit(TFragment &frag)	: TGRSIDetectorHit(frag) {}

TS3Hit::~TS3Hit()	{	}

TS3Hit::TS3Hit(const TS3Hit &rhs) : TGRSIDetectorHit() {
   Clear();
   ((TS3Hit&)rhs).Copy(*this);
}

void TS3Hit::Copy(TObject &rhs) const {
   TGRSIDetectorHit::Copy(rhs);

	static_cast<TS3Hit&>(rhs).led = led;
	static_cast<TS3Hit&>(rhs).ring = ring;
	static_cast<TS3Hit&>(rhs).sector = sector;
   return;
}


void TS3Hit::Clear(Option_t *opt)	{
   TGRSIDetectorHit::Clear(opt);
   led            = -1;
   ring           = -1;
   sector         = -1;
}


void TS3Hit::Print(Option_t *opt) const	{
	printf("================\n");
	printf("not yet written.\n");
	printf("================\n");
}
