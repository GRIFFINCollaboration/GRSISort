

#include "TPacesData.h"


ClassImp(TPacesData)

bool TPacesData::fIsSet = false;

TPacesData::TPacesData()	{	
	Clear();
}

TPacesData::~TPacesData()	{	}


void TPacesData::Clear(Option_t *opt)	{

	fIsSet = false;

   fCore_MidasId.clear();
   fCore_NbrHits.clear(); 

	fCore_Nbr.clear();
   fCore_Address.clear();
   fCore_IsHighGain.clear();
   fCore_Eng.clear();
	fCore_Chg.clear();
	fCore_TimeCFD.clear();
	fCore_Time.clear();

   fPPG.clear();


	for(int x=0;x<fCore_Wave.size();x++)	{
		fCore_Wave[x].clear();
	}
	fCore_Wave.clear();

}

void TPacesData::Print(Option_t *opt) const	{
	// not yet written.
	printf("not yet written.\n");
}


