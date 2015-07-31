
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

//ClassImp(TSharcData)

bool TSharcData::fIsSet = false;

/////////////////////////
TSharcData::TSharcData()	{	}

/////////////////////////
TSharcData::~TSharcData()	{	}

/////////////////////////
void TSharcData::Clear(Option_t *opt)
{
 	fIsSet = false;

  fSharc_Front_DetectorNbr.clear();
  fSharc_Front_StripNbr.clear();
  fSharc_FrontFragment.clear();
  
  fSharc_Back_DetectorNbr.clear();
  fSharc_Back_StripNbr.clear();
  fSharc_BackFragment.clear();

  fSharc_Pad_DetectorNbr.clear();
  fSharc_PadFragment.clear();

}

/////////////////////////
void TSharcData::Print(Option_t *opt) const  
{
  // Energy
  
  // Back
  
  // PAD
}

