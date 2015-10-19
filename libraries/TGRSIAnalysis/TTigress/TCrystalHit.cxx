

#include "TCrystalHit.h"
#include <TClass.h>


ClassImp(TCrystalHit)


TCrystalHit::TCrystalHit(): suppress(false)	{
   Class()->IgnoreTObjectStreamer(true);
	Clear();
}


TCrystalHit::TCrystalHit(const TCrystalHit &rhs)
  : TGRSIDetectorHit() {
   //Class()->IgnoreTObjectStreamer(true);
   ((TCrystalHit&)rhs).Copy(*this);

}

TCrystalHit::~TCrystalHit()	{	}

void TCrystalHit::Clear(Option_t *opt)	{
	segment = -1;
   TGRSIDetectorHit::Clear();
	//charge  = 0xffffffff;
	
	//energy = 0.0;
	//time = 0.0;
	//cfd = 0.0;

	//wave.clear();
}


void TCrystalHit::Print(Option_t *opt)	const {
	//not yet written.
	printf("TCrystalHit::Print() not yet written.\n");
}

void TCrystalHit::Copy(TObject &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
  ((TCrystalHit&)rhs).segment = segment;
}
