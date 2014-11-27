#include "TSceptarData.h"

//ClassImp(TSceptarData)

bool TSceptarData::fIsSet   = false;
bool TSceptarData::fSetWave = false;

TSceptarData::TSceptarData()	{	
	Clear();
}

TSceptarData::~TSceptarData()	{	}


void TSceptarData::Clear(Option_t *opt)	{
	fIsSet = false;

	fDet_Nbr.clear();
   fDet_Address.clear();
   fDet_Eng.clear();
	fDet_Chg.clear();
	fDet_CFD.clear();
	fDet_Time.clear();

	fDet_Wave.clear();
}

void TSceptarData::Print(Option_t *opt)	{
	// not yet written.
	printf("not yet written.\n");
}
