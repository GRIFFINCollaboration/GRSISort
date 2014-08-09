#ifndef TFRAGMENT_H
#define TFRAGMENT_H

#include "Globals.h"

#include<vector>
#include<stdint.h>
#include<time.h>

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
   uint32_t MidasId;              //->
   uint64_t TriggerId;            //->          MasterFilterID in Griffin   
   uint32_t FragmentId;           //->
   int32_t  TriggerBitPattern;	 //->          MasterFilterPattern in Griffin

   int32_t ChannelNumber;         //->
   int32_t ChannelAddress;        //->
   std::vector<int32_t> Cfd;      //->
   std::vector<int32_t> Led;      //->
   std::vector<int32_t> Charge;	  //->

   //unsigned long TimeStamp;     //->
   int32_t TimeStampLow;          //->
   int32_t TimeStampHigh;         //->
   int32_t TimeToTrig;            //->

	/// Added to combine Grif Fragment  ////

	uint32_t PPG;                //->
	uint16_t DeadTime;			  //->
	uint16_t NumberOfFilters;    //->
	uint16_t NumberOfPileups;    //->
	uint16_t DataType;           //->
	uint16_t DetectorType;       //->
	uint32_t ChannelId;          //->

	std::vector<uint16_t>  KValue;    //->

   /// *****************************  ////

   std::vector<int16_t>  wavebuffer;	//->
  
   virtual void	Clear(Option_t *opt = ""); //!
   virtual void 	Print(Option_t *opt = ""); //!
    
   ClassDef(TFragment,3);  // TFragment structure
};
#endif // TFRAGMENT_H
