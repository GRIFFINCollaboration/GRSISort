#ifndef TFRAGMENT_H
#define TFRAGMENT_H

#include "Globals.h"

#include<vector>
//#include<stdint.h>
#include<time.h>

#include<Rtypes.h>
#include<TObject.h>

//#ifndef __CINT__
//#include "Globals.h"
//#endif

//using namespace std;

class TFragment : public TObject	{
public:
   TFragment();
   ~TFragment(); 

   time_t   MidasTimeStamp;       //->
   Int_t	   MidasId;              //->
   Long_t   TriggerId;            //->          MasterFilterID in Griffin   
   Int_t    FragmentId;           //->
   Int_t    TriggerBitPattern;	 //->          MasterFilterPattern in Griffin

   Short_t ChannelNumber;         //->
   Int_t ChannelAddress;        //->
   std::vector<Int_t> Cfd;      //->
   std::vector<Int_t> Led;      //->
   std::vector<Int_t> Charge;	  //->

   //unsigned long TimeStamp;     //->
   Int_t TimeStampLow;          //->
   Int_t TimeStampHigh;         //->
   Int_t TimeToTrig;            //->

	/// Added to combine Grif Fragment  ////

   UInt_t PPG;                //->
   UShort_t DeadTime;	        //->
   UShort_t NumberOfFilters;    //->
   UShort_t NumberOfPileups;    //->
   UShort_t DataType;           //->
   UShort_t DetectorType;       //->
   UInt_t ChannelId;          //->

   std::vector<UShort_t>  KValue;    //->

   /// *****************************  ////

   std::vector<Short_t>  wavebuffer;	//->
  
   virtual void	Clear(Option_t *opt = ""); //!
   
   using TObject::Print; 
   virtual void Print(Option_t *opt = ""); //!
    
   ClassDef(TFragment,3);  // TFragment structure
};
#endif // TFRAGMENT_H
