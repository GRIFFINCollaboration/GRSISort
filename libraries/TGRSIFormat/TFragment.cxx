#include"TFragment.h"
#include"TChannel.h"

#include <TClass.h>

ClassImp(TFragment)

TFragment::TFragment(){
   // Default Constructor
   Class()->IgnoreTObjectStreamer(true);
   Clear();
}

TFragment::~TFragment(){
  // Default destructor does nothing right now
  //Clear();
}

void TFragment::Clear(Option_t *opt){  
   // Clears all fields of the TFragment
   MidasTimeStamp    = 0;
   MidasId           = 0;

   TriggerId         = 0;
   FragmentId        = 0;
   TriggerBitPattern = 0;

   NetworkPacketNumber = 0;

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

long TFragment::GetTimeStamp() {
   long time = TimeStampHigh;
   time  = time << 28;
   time |= TimeStampLow & 0x0fffffff;
   return time;
}


const char *TFragment::GetName() {
   TChannel *chan = TChannel::GetChannel(ChannelAddress);
   if(!chan)
      return "";
   return chan->GetChannelName();
}


double TFragment::GetEnergy() {
   TChannel *chan = TChannel::GetChannel(ChannelAddress);
   if(!chan || Charge.size()<1)
      return 0.00;
   return chan->CalibrateENG(Charge.at(0));
}





void TFragment::Print(Option_t *opt)	{
   //Prints out all fields of the TFragment


   TChannel *chan = TChannel::GetChannel(this->ChannelAddress);
   //printf("%s Event at	%i:\n", chan->GetDigitizerType().c_str(), MidasId);
   char buff[20];
   time(&MidasTimeStamp);
   struct tm * timeinfo = localtime(&MidasTimeStamp);
   strftime(buff,20,"%b %d %H:%M:%S",timeinfo);
   printf("MidasTimeStamp: %s\n",buff);
   printf("MidasId    	%i\n", MidasId);
   printf("TriggerId: 	%lu\n", TriggerId);
   printf("FragmentId:   %i\n", FragmentId);
   printf("TriggerBit:	0x%08x\n", TriggerBitPattern);
   printf("NetworkPacketNumber: %i\n", NetworkPacketNumber);
   if(chan)
	   printf("Channel: %i\tName: %s\n", chan->GetNumber(), chan->GetChannelName());
   printf("\tChannel Address: 0x%08x\n", ChannelAddress);
   printf("\tChannel Num:      %i\n", ChannelNumber);
   printf("\tCharge[%lu]	  ",Charge.size());   for(int x=0;x<Charge.size();x++){printf( "     0x%08x", Charge.at(x));} printf("\n");
   printf("\tCFD[%lu]		  ",Cfd.size());      for(int x=0;x<Cfd.size();x++)   {printf( "     0x%08x", Cfd.at(x));} printf("\n");
   printf("\tLED[%lu]		  ",Led.size());      for(int x=0;x<Led.size();x++)   {printf( "     0x%08x", Led.at(x));} printf("\n");
   printf("\tTimeStamp High: 0x%08x\n", TimeStampHigh);
   printf("\tTimeStamp Low:    0x%08x\n", TimeStampLow);
   printf("\tTimeToTrig:  %i\n", TimeToTrig);
   //unsigned short temptime = (TimeStampLow & 0x0000ffff) - ((Cfd >> 4) & 0x0000ffff);   //TimeStampLow&0x0000ffff; 
   //printf("\ttime from timestamp(to the nearest 10ns):    0x%04x\t%ins\n", temptime, temptime * 10);
   if (!wavebuffer.empty())
      printf("Has a wave form stored.\n");
   else
      printf("Does Not have a wave form stored.\n");

}











