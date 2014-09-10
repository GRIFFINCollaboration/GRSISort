
//   A class used to hold the raw sharc data before sharc_hits are built
//   This class is mostly taken from the NPTOOLS foramt.
//
//   The major idea here is we stick all the data from an array of TFragments
//   and put it into these storage containers.  Then we turn this data in an
//   array of TCSMHits and promtly 'throw this out'  (i.e don't write it to 
//   the Analysis Tree.
//
//	 pcb.
//



#include <iostream>
#include <fstream>
#include <string>
using namespace std; 

#include "TCSMData.h"

ClassImp(TCSMData)

bool TCSMData::fIsSet = false;

/////////////////////////
TCSMData::TCSMData()	{	}

/////////////////////////
TCSMData::~TCSMData()	{	}

/////////////////////////
void TCSMData::Clear(Option_t *opt)
{
  fCSM_StripHorizontal_DetectorNbr.clear();
  fCSM_StripHorizontal_DetectorPos.clear();
  fCSM_StripHorizontal_StripNbr.clear();
  fCSM_StripHorizontal_Energy.clear();
  fCSM_StripHorizontal_Charge.clear();
  fCSM_StripHorizontal_TimeCFD.clear();
  fCSM_StripHorizontal_TimeLED.clear();
  fCSM_StripHorizontal_Time.clear();

  fCSM_StripHorizontal_Wave.clear();


  fCSM_StripVertical_DetectorNbr.clear();
  fCSM_StripVertical_DetectorPos.clear();
  fCSM_StripVertical_StripNbr.clear();
  fCSM_StripVertical_Energy.clear();
  fCSM_StripVertical_Charge.clear();
  fCSM_StripVertical_TimeCFD.clear();
  fCSM_StripVertical_TimeLED.clear();
  fCSM_StripVertical_Time.clear();

  fCSM_StripVertical_Wave.clear();


}

/////////////////////////
void TCSMData::Print(Option_t *opt) 
{
  // Energy
  cout << "CSM_StripHorizontal_Mult = " << fCSM_StripHorizontal_DetectorNbr.size() << endl;
  /*
  // Horizontal
  for (UShort_t i = 0; i < fCSM_StripHorizontal_DetectorNbr.size(); i++){
    cout << "DetNbr: " << fCSM_StripHorizontal_DetectorNbr[i]
         << " Strip: " << fCSM_StripHorizontal_StripNbr[i]
         << " Energy: " << fCSM_StripHorizontal_Energy[i]
         << " Time CFD: " << fCSM_StripHorizontal_TimeCFD[i]
         << " Time LED: " << fCSM_StripHorizontal_TimeLED[i]
         << " Time    : " << fCSM_StripHorizontal_Time[i] << endl;

  }
  
  // Vertical
  for (UShort_t i = 0; i < fCSM_StripHorizontal_DetectorNbr.size(); i++){
    cout << "DetNbr: " << fCSM_StripHorizontal_DetectorNbr[i]
    << " Strip: " << fCSM_StripHorizontal_StripNbr[i]
    << " Energy: " << fCSM_StripHorizontal_Energy[i]
    << " Time CFD: " << fCSM_StripVertical_TimeCFD[i]
    << " Time LED: " << fCSM_StripVertical_TimeLED[i]
    << " Time    : " << fCSM_StripVertical_Time[i] << endl;

  }
  
  // PAD
  for (UShort_t i = 0; i < fCSM_PAD_DetectorNbr.size(); i++){
    cout << "DetNbr: " << fCSM_PAD_DetectorNbr[i]
    << " Energy: " << fCSM_PAD_Energy[i]
    << " Time CFD: " << fCSM_PAD_TimeCFD[i]
    << " Time LED: " << fCSM_PAD_TimeLED[i]
    << " Time    : " << fCSM_PAD_Time[i] << endl;
  }
  */
}

