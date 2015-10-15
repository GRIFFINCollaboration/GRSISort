

#include "TTigressData.h"


//ClassImp(TTigressData)

bool TTigressData::fIsSet = false;

TTigressData::TTigressData()	{	
	Clear();
}

TTigressData::~TTigressData()	{	}


void TTigressData::Clear(Option_t *opt)	{

	fCoreSet.clear();

	fIsSet = false;

	fClover_Nbr.clear();
	fCore_Nbr.clear();
	fCore_Eng.clear();
	fCore_Chg.clear();
	fCore_TimeCFD.clear();
	fCore_TimeLED.clear();
	fCore_Time.clear();
	fCore_TimeStamp.clear();

	fCore_Wave.clear();

	fSeg_Clover_Nbr.clear();
	fSeg_Core_Nbr.clear();
	fSegment_Nbr.clear();	
	fSegment_Eng.clear();
	fSegment_Chg.clear();
	fSegment_TimeCFD.clear();
	fSegment_TimeLED.clear();
	fSegment_Time.clear();

	fSegment_Wave.clear();
}

void TTigressData::Print(Option_t *opt) const	{
	// not yet written.
	printf("not yet written.\n");
}


