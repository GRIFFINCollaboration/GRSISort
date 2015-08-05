
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
  fCSM_StripHorizontal_Fragment.clear();
  //fCSM_StripHorizontal_Energy.clear();
  //fCSM_StripHorizontal_Charge.clear();
  //fCSM_StripHorizontal_TimeCFD.clear();
  //fCSM_StripHorizontal_TimeLED.clear();
  //fCSM_StripHorizontal_Time.clear();

  //fCSM_StripHorizontal_Wave.clear();


  fCSM_StripVertical_DetectorNbr.clear();
  fCSM_StripVertical_DetectorPos.clear();
  fCSM_StripVertical_StripNbr.clear();
  fCSM_StripVertical_Fragment.clear();
  //fCSM_StripVertical_Energy.clear();
  //fCSM_StripVertical_Charge.clear();
  //fCSM_StripVertical_TimeCFD.clear();
  //fCSM_StripVertical_TimeLED.clear();
  //fCSM_StripVertical_Time.clear();

  //fCSM_StripVertical_Wave.clear();


}

/////////////////////////
void TCSMData::Print(Option_t* opt) 
{
  printf(DRED "\n Printing All TCSMData\n" RESET_COLOR);
  printf(GREEN "DetNbr  DetPos  StripNbr       Energy        Charge           CFD             LED            Time \n" RESET_COLOR);
  if(GetMultiplicityHorizontal()>0)
  {
    printf(BLUE "Horizontal:\n" RESET_COLOR);
    for(int i=0;i<GetMultiplicityHorizontal();i++)
    {
      printf(" %2i       %c        %2i     %11.2f  %12i  %12i  %14i  %14.2f\n",GetHorizontal_DetectorNbr(i), GetHorizontal_DetectorPos(i), GetHorizontal_StripNbr(i), GetHorizontal_Energy(i), GetHorizontal_Charge(i), GetHorizontal_TimeCFD(i), GetHorizontal_TimeLED(i), GetHorizontal_Time(i));
      //cout<<GetHorizontal_DetectorNbr(i)<<" "<<char(GetHorizontal_DetectorPos(i))<<" "<<GetHorizontal_StripNbr(i)<<" "<<GetHorizontal_Energy(i)<<" "<<GetHorizontal_Charge(i)<<" "<<GetHorizontal_TimeCFD(i)<<" "<<GetHorizontal_TimeLED(i)<<" "<<GetHorizontal_Time(i)<<endl;
    }
  }

  if(GetMultiplicityVertical()>0)
  {
    printf(RED "Vertical:\n" RESET_COLOR);
    for(int i=0;i<GetMultiplicityVertical();i++)
    {
      printf(" %2i       %c        %2i     %11.2f  %12i  %12i  %14i  %14.2f\n",GetVertical_DetectorNbr(i), GetVertical_DetectorPos(i), GetVertical_StripNbr(i), GetVertical_Energy(i), GetVertical_Charge(i), GetVertical_TimeCFD(i), GetVertical_TimeLED(i), GetVertical_Time(i));
    }
  }
}

void TCSMData::Print(int addr, bool horizontal)
{
  printf(DRED "\n \tPrinting a single ");
  if(horizontal)
    printf("horizontal");
  else
    printf("vertical");
  printf(" event at %2i\n" RESET_COLOR, addr);
  printf(GREEN "\tDetNbr  DetPos  StripNbr       Energy        Charge           CFD             LED            Time \n" RESET_COLOR);

  if(horizontal)
  {
    printf(BLUE "\tHorizontal:\n" RESET_COLOR);
    printf("\t %2i       %c        %2i     %11.2f  %12i  %12i  %14i  %14.2f\n",GetHorizontal_DetectorNbr(addr), GetHorizontal_DetectorPos(addr), GetHorizontal_StripNbr(addr), GetHorizontal_Energy(addr), GetHorizontal_Charge(addr), GetHorizontal_TimeCFD(addr), GetHorizontal_TimeLED(addr), GetHorizontal_Time(addr));
  }
  if(!horizontal)
  {
    printf(RED "\tVertical:\n" RESET_COLOR);
    printf("\t %2i       %c        %2i     %11.2f  %12i  %12i  %14i  %14.2f\n",GetVertical_DetectorNbr(addr), GetVertical_DetectorPos(addr), GetVertical_StripNbr(addr), GetVertical_Energy(addr), GetVertical_Charge(addr), GetVertical_TimeCFD(addr), GetVertical_TimeLED(addr), GetVertical_Time(addr));
  }
}
