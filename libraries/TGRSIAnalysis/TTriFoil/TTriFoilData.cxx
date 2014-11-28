
//
//	 pcb.
//

#include <iostream>
#include <fstream>
#include <string>

#include "TTriFoilData.h"

//ClassImp(TTriFoilData)

bool TTriFoilData::fIsSet = false;

/////////////////////////
TTriFoilData::TTriFoilData()	{	}

/////////////////////////
TTriFoilData::~TTriFoilData()	{	}

/////////////////////////

void TTriFoilData::Clear(Option_t *opt)
{
 	fIsSet = false;

	TriFoil_TimeStampHigh = 0;
	TriFoil_TimeStampLow = 0;
	TriFoil_Time = 0;

	TriFoil_WaveBuffer.clear();

}

/////////////////////////
void TTriFoilData::Print(Option_t *opt) 
{
	printf("No print for you!\n");

}

