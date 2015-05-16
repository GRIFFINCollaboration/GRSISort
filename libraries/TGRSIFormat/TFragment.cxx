#include"TFragment.h"
#include"TChannel.h"
#include <iostream>

#include <TClass.h>

ClassImp(TFragment)

////////////////////////////////////////////////////////////////
//                                                            //
// TFragment                                                  //
//                                                            //
// This Class contains all of the information in an event     //
// fragment                                                   //
//                                                            //
////////////////////////////////////////////////////////////////

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
   Zc.clear();//              = -1;
   ccShort.clear();//
   ccLong.clear();//
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

double TFragment::GetTimeStamp() const {
   long time = TimeStampHigh;
   time  = time << 28;
   time |= TimeStampLow & 0x0fffffff;
   double dtime = double(time)+ gRandom->Uniform();
   TChannel *chan = TChannel::GetChannel(ChannelAddress);
   if(!chan )//|| Charge.size()<1)
      return dtime;
   return dtime - chan->GetTZero(GetEnergy());
}

double TFragment::GetTZero() const {
   TChannel *chan = TChannel::GetChannel(ChannelAddress);
   if(!chan )//|| Charge.size()<1)
      return 0.000;
   return chan->GetTZero(GetEnergy());
}




long TFragment::GetTimeStamp_ns() {
   long ns = 0;
   if(DataType==2 && Cfd.size()>0) {
     ns = (Cfd.at(0) >> 21) & 0xf;
   }
   return 10*GetTimeStamp() + ns;  
}

Int_t TFragment::Get4GCfd(int i) { // return a 4G cfd in terms 
  if(Cfd.size()==0)                // of 1/256 ns since the trigger
     return -1;
  if(Cfd.size()<i)
     i = Cfd.size()-1;
  return  Cfd.at(i)&0x001fffff;

}


const char *TFragment::GetName() const {
   TChannel *chan = TChannel::GetChannel(ChannelAddress);
   if(!chan)
      return "";
   return chan->GetChannelName();
}

/*
double TFragment::GetEnergy() const {
   TChannel *chan = TChannel::GetChannel(ChannelAddress);
   if(!chan || Charge.size()<1)
      return 0.00;
   return chan->CalibrateENG((int)(Charge.at(0)));
}
*/

double TFragment::GetEnergy(int i) const {
   TChannel *chan = TChannel::GetChannel(ChannelAddress);
   if(!chan || !(Charge.size()>i))
      return 0.00;
   if(chan->UseCalFileIntegration()) {
      //printf("I am here\n");
     return chan->CalibrateENG((int)(Charge.at(i)),0);  // this will use the integration value
                                                        // in the tchannel if it exists.
   }
   if(KValue.size()>i && KValue.at(i)>0)
     return chan->CalibrateENG((int)(Charge.at(i)),(int)KValue.at(i));
   return chan->CalibrateENG((int)(Charge.at(i)));
}

double TFragment::GetCharge(int i) const {
   TChannel *chan = TChannel::GetChannel(ChannelAddress);
   if(!chan || !(Charge.size()>i))
      return 0.00;
   if(chan->UseCalFileIntegration()) {
      return ((double)Charge.at(i)+gRandom->Uniform())/((double)chan->GetIntegration());// this will use the integration value
   }                                                                       // in the tchannel if it exists.
   if(KValue.size()>i && KValue.at(i)>0){
      return ((double)Charge.at(i)+gRandom->Uniform())/((double)KValue.at(i));// this will use the integration value
   }
   return ((double)Charge.at(i)+gRandom->Uniform());// this will use no integration value
}

void TFragment::Print(Option_t *opt)	{
   //Prints out all fields of the TFragment


   TChannel *chan = TChannel::GetChannel(this->ChannelAddress);
   //printf("%s Event at	%i:\n", chan->GetDigitizerType().c_str(), MidasId);
   char buff[20];
   ctime(&MidasTimeStamp);
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
   printf("\tZC[%lu]		  ",Zc.size());      for(int x=0;x<Zc.size();x++)   {printf( "     0x%08x", Zc.at(x));} printf("\n");
   printf("\tLED[%lu]		  ",Led.size());      for(int x=0;x<Led.size();x++)   {printf( "     0x%08x", Led.at(x));} printf("\n");
   printf("\tTimeStamp High: 0x%08x\n", TimeStampHigh);
   printf("\tTimeStamp Low:    0x%08x\n", TimeStampLow);
   //unsigned short temptime = (TimeStampLow & 0x0000ffff) - ((Cfd >> 4) & 0x0000ffff);   //TimeStampLow&0x0000ffff; 
   //printf("\ttime from timestamp(to the nearest 10ns):    0x%04x\t%ins\n", temptime, temptime * 10);
   if (!wavebuffer.empty())
      printf("Has a wave form stored.\n");
   else
      printf("Does Not have a wave form stored.\n");

}

bool TFragment::IsTigCore() {
  // If a tigress fragment a signal from the core
  // return true, else false.

  std::string channame = this->GetName();
  if(channame.length()<9) //not a good mnemonic
     return false;
  if(!channame.compare(0,3,"TIG"))
    if(!channame.compare(6,3,"N00"))
      return true;
  return false;

};


bool TFragment::IsGriffCore() {
  // If a tigress fragment a signal from the core
  // return true, else false.

  std::string channame = this->GetName();
  if(channame.length()<9) //not a good mnemonic
     return false;
  if(!channame.compare(0,3,"GRG"))
      return true;
    //if(!channame.compare(6,3,"N00"))
      return true;
  return false;

};

bool TFragment::IsDetector(std::string prefix, Option_t *opt) const {
   //Checks to see if the current fragment constains the same "prefix", for example "GRG"
   //The option determines whether the channel should be:
   // - C : The core of a segmented detector
   // - S : The segments of a segmented detector
   // - A : Low gain output of a detector
   // - B : High gain output of a detectora
   // If C or S are not given, default to C
   // If A or B are not given, default to A
   //Note that multiple options add to the output, so "CAB" would return the core with both high and low gain
   //One should eventually add N,P,T options as well.
   TString pre = prefix;
   TString option = opt;
   TString channame = this->GetName();
   option.ToUpper();
   //Could also do everything below with MNEMONIC Struct. This limits the amount of string processing that needs to be done
   //Because it returns false after every potential failure while the mnemonic class sets all of the strings, and then checks
   //for conditions.
   if(channame.BeginsWith(pre)){
      bool high_gain = option.Contains("B");
      bool low_gain = option.Contains("A");
      bool core = option.Contains("C");
      bool segments = option.Contains("S");
   
      if(!high_gain && !low_gain)
         low_gain = true;

      if(!core && !segments)
         core = true;

      if(!high_gain && channame.EndsWith("B")){
         return false;
      }
      if(!low_gain && channame.EndsWith("A")){
         return false;
      }
      //Now remove the last letter for comparing segments/core.
      channame = channame.Chop();

      if(!core && channame.EndsWith("00"))
         return false;
      if(!segments && !channame.EndsWith("00"))
         return false;

      channame = channame.Chop();
      channame = channame.Chop();

      return true;

   }
   else return false;

}
