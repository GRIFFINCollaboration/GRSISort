
#include "TGRSIDetectorHit.h"
#include <TClass.h>

ClassImp(TGRSIDetectorHit)

TGRSIDetectorHit::TGRSIDetectorHit()	{ 
  // Class()->IgnoreTObjectStreamer(true);
}

TGRSIDetectorHit::~TGRSIDetectorHit()	{	}

Bool_t TGRSIDetectorHit::BremSuppressed(TGRSIDetectorHit *bremorigin){
   
   return false;
}

void TGRSIDetectorHit::Print(Option_t *opt) { }

void TGRSIDetectorHit::Clear(Option_t *opt) { }

