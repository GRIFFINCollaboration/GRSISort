

#include "TCrystalHit.h"
#include <TClass.h>


ClassImp(TCrystalHit)


TCrystalHit::TCrystalHit()	{
   Class()->IgnoreTObjectStreamer(true);
	Clear();
}

TCrystalHit::~TCrystalHit()	{	}

void TCrystalHit::Clear(Option_t *opt)	{
	segment = -1;
	charge  = 0xffffffff;
	
	energy = 0.0;
	time = 0.0;
	cfd = 0.0;

	wave.clear();
}


void TCrystalHit::Print(Option_t *opt)	{
	//not yet written.
	printf("TCrystalHit::Print() not yet written.\n");
}
