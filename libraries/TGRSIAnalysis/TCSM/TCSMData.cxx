
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

//ClassImp(TCSMData)

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
  /*
  printf(DGREEN "\n\t\t****Printing TCSMHit****" RESET_COLOR "\n");
  printf(DGREEN "\t****Delta Info****" RESET_COLOR "\n");
  printf(DGREEN "\t\t Vert    Horiz" RESET_COLOR "\n");
  printf(DBLUE "\tStrip:\t" RESET_COLOR "   %i       %i" RESET_COLOR "\n", ver_d_strip,hor_d_strip);
  printf(DBLUE "\tCharge:\t" RESET_COLOR "   %d       %d" RESET_COLOR "\n", ver_d_charge,hor_d_charge);
  printf(DBLUE "\tTime:\t" RESET_COLOR "   %d       %d" RESET_COLOR "\n", ver_d_time,hor_d_time);
  printf(DBLUE "\tCFD:\t" RESET_COLOR "   %d       %d" RESET_COLOR "\n", ver_d_cfd,hor_d_cfd);
  */
  //printf("                 DetNbr  DetPos  StripNbr  En  Char  CFD  LED  Time \n");
  //printf("Horizontal hits:   %2i      %2i       %2i     %2i   %2i    %2i   %2i   %2i \n",fCSM_StripHorizontal_DetectorNbr.size(),fCSM_StripHorizontal_DetectorPos.size(),fCSM_StripHorizontal_StripNbr.size(),fCSM_StripHorizontal_Energy.size(),fCSM_StripHorizontal_Charge.size(),fCSM_StripHorizontal_TimeCFD.size(),fCSM_StripHorizontal_TimeLED.size(),fCSM_StripHorizontal_Time.size());

  printf(GREEN "DetNbr  DetPos  StripNbr   Energy        Charge           CFD             LED            Time \n" RESET_COLOR);
  if(fCSM_StripHorizontal_DetectorNbr.size()>0)
  {
    printf(BLUE "Horizontal:\n" RESET_COLOR);
    for(int i=0;i<fCSM_StripHorizontal_DetectorNbr.size();i++)
    {
      printf(" %2i       %c        %2i     %7d  %12d  %12d  %14d  %14d\n",fCSM_StripHorizontal_DetectorNbr.at(i), fCSM_StripHorizontal_DetectorPos.at(i), fCSM_StripHorizontal_StripNbr.at(i), fCSM_StripHorizontal_Energy.at(i), fCSM_StripHorizontal_Charge.at(i), fCSM_StripHorizontal_TimeCFD.at(i), fCSM_StripHorizontal_TimeLED.at(i), fCSM_StripHorizontal_Time.at(i));
    }
  }

  if(fCSM_StripVertical_DetectorNbr.size()>0)
  {
    printf(RED "Vertical:\n" RESET_COLOR);
    for(int i=0;i<fCSM_StripVertical_DetectorNbr.size();i++)
    {
      printf(" %2i       %c        %2i     %7d  %12i  %12i  %14i  %14d\n",fCSM_StripVertical_DetectorNbr.at(i), fCSM_StripVertical_DetectorPos.at(i), fCSM_StripVertical_StripNbr.at(i), fCSM_StripVertical_Energy.at(i), fCSM_StripVertical_Charge.at(i), fCSM_StripVertical_TimeCFD.at(i), fCSM_StripVertical_TimeLED.at(i), fCSM_StripVertical_Time.at(i));
    }
  }

  /*
  vector<UShort_t>   fCSM_StripHorizontal_DetectorNbr;		//!
  vector<Char_t>   fCSM_StripHorizontal_DetectorPos;		//!
  vector<Short_t>   fCSM_StripHorizontal_StripNbr;		//!
  vector<Double_t>   fCSM_StripHorizontal_Energy;			//!
  vector<Int_t>	   fCSM_StripHorizontal_Charge;			//!
  vector<Int_t>      fCSM_StripHorizontal_TimeCFD;			//!
  vector<Int_t>      fCSM_StripHorizontal_TimeLED;			//!
  vector<Double_t>   fCSM_StripHorizontal_Time;			//!
  
  
  vector<UShort_t>   fCSM_StripVertical_DetectorNbr;		//!
  vector<Char_t>   fCSM_StripVertical_DetectorPos;		//!
  vector<Short_t>   fCSM_StripVertical_StripNbr;			//!
  vector<Double_t>   fCSM_StripVertical_Energy;			//!
  vector<Int_t>	   fCSM_StripVertical_Charge;			//!
  vector<Int_t>   fCSM_StripVertical_TimeCFD;			//!
  vector<Int_t>   fCSM_StripVertical_TimeLED;			//!
  vector<Double_t>   fCSM_StripVertical_Time;				//!
  */
  
}

