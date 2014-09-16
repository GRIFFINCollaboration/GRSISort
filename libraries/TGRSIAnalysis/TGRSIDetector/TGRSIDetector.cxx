

#include "TGRSIDetector.h"
#include <TClass.h>

ClassImp(TGRSIDetector)

TGRSIDetector::TGRSIDetector() : data(0) {
   Class()->IgnoreTObjectStreamer(true);

}


TGRSIDetector::~TGRSIDetector()	{
	if(data) delete data;
}


void TGRSIDetector::Print(Option_t *opt) { }


void TGRSIDetector::Clear(Option_t *opt) { }


