

#include "TBGOData.h"

//ClassImp(TBGOData)

bool TBGOData::fIsSet = false;

TBGOData::TBGOData()	{	
   Clear();
}

TBGOData::~TBGOData()	{	}


void TBGOData::Clear(Option_t *opt)	{

	fIsSet = false;

  fBGO_CloverNbr.clear();
  fBGO_CrystalNbr.clear();
  fBGO_PmNbr.clear();
  fBGO_Charge.clear();
  fBGO_Energy.clear();
  fBGO_TimeCFD.clear();
  fBGO_TimeLED.clear();
  fBGO_Time.clear();

  for(int x=0;x<fBGO_Wave.size();x++)	{
    fBGO_Wave.clear();
  }
}


void TBGOData::Print(Option_t *opt)	{
    //not yet written.
    printf("not yet written.\n");

}

