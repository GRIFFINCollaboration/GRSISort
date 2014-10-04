

#include "TGriffinData.h"


ClassImp(TGriffinData)

bool TGriffinData::fIsSet = false;

TGriffinData::TGriffinData()	{	
	Clear();
}

TGriffinData::~TGriffinData()	{	}


void TGriffinData::Clear(Option_t *opt)	{

	fIsSet = false;

   fCore_MidasId.clear();
   fCore_NbrHits.clear(); 


	fClover_Nbr.clear();
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

void TGriffinData::Print(Option_t *opt) const	{
	// not yet written.
	printf("not yet written.\n");
}


