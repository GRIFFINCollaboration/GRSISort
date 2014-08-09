#include"TFragment.h"

#include <TClass.h>

ClassImp(TFragment)


TFragment::TFragment(){
   Class()->IgnoreTObjectStreamer(true);
   Clear();
}

TFragment::~TFragment(){
	//Clear();
}

void TFragment::Clear(Option_t *opt){  
   MidasTimeStamp    = 0;
   MidasId           = 0;

   TriggerId         = 0;
   FragmentId        = 0;
   TriggerBitPattern = 0;

   ChannelAddress    = -1;
   Cfd.clear();//              = -1;
   Led.clear();//               = -1;
   Charge.clear();//            = -1;
  
   //TimeStamp         = -1;
   TimeStampLow  = 0;
   TimeStampHigh = 0;




   PPG                = 0; 
   DeadTime           = 0; 
   NumberOfFilters    = 0; 
   NumberOfPileups    = 0; 
   DataType           = 0; 
   DetectorType       = 0; 
   ChannelId          = 0; 



   if(!wavebuffer.empty())
      wavebuffer.clear();

   if(!KValue.empty())     //->
   	KValue.clear();      //->

}

void TFragment::Print(Option_t *opt)	{
	printf("not written yet.\n");
}











