
#include "TGRSIDetectorHit.h"
#include <TClass.h>

ClassImp(TGRSIDetectorHit)

TGRSIDetectorHit::TGRSIDetectorHit()	{ 
   Class()->IgnoreTObjectStreamer(true);
}

TGRSIDetectorHit::~TGRSIDetectorHit()	{	}


void TGRSIDetectorHit::Print(Option_t *opt) { }

void TGRSIDetectorHit::Clear(Option_t *opt) { }

