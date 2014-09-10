

#include "TGRSIDetector.h"


ClassImp(TGRSIDetector)

TGRSIDetector::TGRSIDetector() : data(0) {

}


TGRSIDetector::~TGRSIDetector()	{
	if(data) delete data;
}


void TGRSIDetector::Print(Option_t *opt) { }


void TGRSIDetector::Clear(Option_t *opt) { }


