
//   A class used to hold the raw sharc data before sharc_hits are built
//
//   The major idea here is we stick all the data from an array of TFragments
//   and put it into these storage containers.  Then we turn this data in an
//   array of TSharcHits and promtly 'throw this out'  (i.e don't write it to 
//   the Analysis Tree.
//
//	 pcb.
//

#include <iostream>
#include <fstream>
#include <string>

#include "TSharcData.h"

ClassImp(TSharcData)

bool TSharcData::fIsSet = false;

/////////////////////////
TSharcData::TSharcData()	{	}

/////////////////////////
TSharcData::~TSharcData()	{	}

/////////////////////////
void TSharcData::Clear(Option_t *opt)
{
 	fIsSet = false;

  fSharc_StripFront_DetectorNbr.clear();
  fSharc_StripFront_ChannelAddress.clear();
  fSharc_StripFront_StripNbr.clear();
  fSharc_StripFront_Energy.clear();
  fSharc_StripFront_EngChi2.clear();
  fSharc_StripFront_Charge.clear();
  fSharc_StripFront_TimeCFD.clear();
  fSharc_StripFront_TimeLED.clear();
  fSharc_StripFront_Time.clear();

  fSharc_StripFront_Wave.clear();


  fSharc_StripBack_DetectorNbr.clear();
  fSharc_StripBack_ChannelAddress.clear();
  fSharc_StripBack_StripNbr.clear();
  fSharc_StripBack_Energy.clear();
  fSharc_StripBack_EngChi2.clear();
  fSharc_StripBack_Charge.clear();
  fSharc_StripBack_TimeCFD.clear();
  fSharc_StripBack_TimeLED.clear();
  fSharc_StripBack_Time.clear();

  fSharc_StripBack_Wave.clear();

  fSharc_PAD_ChannelAddress.clear();
  fSharc_PAD_DetectorNbr.clear();
  fSharc_PAD_Energy.clear();
  fSharc_PAD_Charge.clear();
  fSharc_PAD_TimeCFD.clear(); 
  fSharc_PAD_TimeLED.clear();
  fSharc_PAD_Time.clear();

  fSharc_PAD_Wave.clear();

}

/////////////////////////
void TSharcData::Print(Option_t *opt) 
{
  // Energy
  std::cout << "Sharc_StripFront_Mult = " << fSharc_StripFront_DetectorNbr.size() << std::endl;
  
  // Front
  for (UShort_t i = 0; i < fSharc_StripFront_DetectorNbr.size(); i++){
    std::cout << "DetNbr: " << fSharc_StripFront_DetectorNbr[i]
         << " Strip: " << fSharc_StripFront_StripNbr[i]
         << " Energy: " << fSharc_StripFront_Energy[i]
         << " Time CFD: " << fSharc_StripFront_TimeCFD[i]
         << " Time LED: " << fSharc_StripFront_TimeLED[i]
         << " Time    : " << fSharc_StripFront_Time[i] << std::endl;

  }
  
  // Back
  for (UShort_t i = 0; i < fSharc_StripFront_DetectorNbr.size(); i++){
    std::cout << "DetNbr: " << fSharc_StripFront_DetectorNbr[i]
    << " Strip: " << fSharc_StripFront_StripNbr[i]
    << " Energy: " << fSharc_StripFront_Energy[i]
    << " Time CFD: " << fSharc_StripBack_TimeCFD[i]
    << " Time LED: " << fSharc_StripBack_TimeLED[i]
    << " Time    : " << fSharc_StripBack_Time[i] << std::endl;

  }
  
  // PAD
  for (UShort_t i = 0; i < fSharc_PAD_DetectorNbr.size(); i++){
    std::cout << "DetNbr: " << fSharc_PAD_DetectorNbr[i]
    << " Energy: " << fSharc_PAD_Energy[i]
    << " Time CFD: " << fSharc_PAD_TimeCFD[i]
    << " Time LED: " << fSharc_PAD_TimeLED[i]
    << " Time    : " << fSharc_PAD_Time[i] << std::endl;
  }
}

