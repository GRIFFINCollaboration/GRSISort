#ifndef __CINT__

#include "TTip.h"
#include "TTipData.h"

bool TTipData::fIsSet   = false;
bool TTipData::fSetWave = false;

TTipData::TTipData()	{	
	Clear();
}

TTipData::~TTipData()	{	}


void TTipData::Clear(Option_t *opt)	{
	fIsSet = false;

	fDet_Nbr.clear();
   fDet_Address.clear();
   fDet_Eng.clear();
	fDet_Chg.clear();
	fDet_CFD.clear();
	fDet_Time.clear();

	fDet_Wave.clear();
}

void TTipData::Print(Option_t *opt)	{
	// not yet written.
	printf("not yet written.\n");
}

Double_t TTipData::DoPID(std::vector<Short_t> waveform){
	// Space for PID function - needs importing from SFU software
}

#endif
