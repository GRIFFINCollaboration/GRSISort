#include "TDescantData.h"

bool TDescantData::fIsSet   = false;
bool TDescantData::fSetWave = false;

TDescantData::TDescantData()	{	
	Clear();
}

TDescantData::~TDescantData()	{	}


void TDescantData::Clear(Option_t *opt)	{
	fIsSet = false;

	fDet_Nbr.clear();
   fDet_Address.clear();
   fDet_Eng.clear();
	fDet_Chg.clear();
	fDet_CFD.clear();
	fDet_Time.clear();

	fDet_Wave.clear();
}

void TDescantData::Print(Option_t *opt)	{
	// not yet written.
	printf("not yet written.\n");
}
