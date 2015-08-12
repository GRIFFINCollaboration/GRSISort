#include "TDataParser.h"
#include "TChannel.h"
#include "Globals.h"

#include "TFragmentQueue.h"
#include "TGRSIStats.h"

#include "TEpicsFrag.h"
#include "TGRSIRootIO.h"

#include "Rtypes.h"

////////////////////////////////////////////////////////////////
//                                                            //
// TDataParser                                                //
//                                                            //
// The TDataParser is the DAQ dependent part of GRSISort.     //
// It takes a "DAQ-dependent"-flavoured MIDAS file and        //
// converts it into a generic TFragment that the rest of      //
// GRSISort can deal with. This is where event word masks     //
// are applied, and any changes to the event format must      //
// be implemented.                                            //
//                                                            //
////////////////////////////////////////////////////////////////

TDataParser *TDataParser::fDataParser = 0;
bool TDataParser::no_waveforms = false;
bool TDataParser::record_stats = false;

const unsigned long TDataParser::fgMaxTriggerId = 1024 * 1024 * 16; // 24 bits internally

unsigned long TDataParser::fgLastMidasId = 0;
unsigned long TDataParser::fgLastTriggerId = 0;
unsigned long TDataParser::fgLastNetworkPacket = 0;

TChannel *TDataParser::gChannel = new TChannel;

ClassImp(TDataParser)

TDataParser *TDataParser::instance() {
//Get the global instance of the TDataParser Class
    if(!fDataParser)
   fDataParser = new TDataParser();
    return fDataParser;
}

TDataParser::TDataParser() {}

TDataParser::~TDataParser() {}


//std::vector<TFragment*> TDataParser::TigressDataToFragment(uint32_t *data, int size,unsigned int midasserialnumber, time_t midastime) {
int TDataParser::TigressDataToFragment(uint32_t *data, int size,unsigned int midasserialnumber, time_t midastime) {
//Converts A MIDAS File from the Tigress DAQ into a TFragment.
   std::vector<TFragment*> FragsFound;
   int NumFragsFound = 0;
   TFragment *EventFrag = new TFragment();
  //FragsFound.push_back(EventFrag);
   //EventFrag->Clear();   
   bool badtimestamp = false;
   bool needtodeletelast = false;
   EventFrag->MidasTimeStamp = midastime;
   EventFrag->MidasId = midasserialnumber;    
   
   int x = 0;
   uint32_t dword = *(data+x);
   uint32_t type = (dword & 0xf0000000)>>28;
   uint32_t value = (dword & 0x0fffffff);

   if(!SetTIGTriggerID(dword,EventFrag)) {
      delete EventFrag;
      printf(RED "Setting TriggerId (0x%08x) falied on midas event: " DYELLOW "%i" RESET_COLOR "\n",dword,midasserialnumber);
      return NumFragsFound;  
   }
   x+=1;
   if(!SetTIGTimeStamp((data+x),EventFrag)) { 
      delete EventFrag;
      printf(RED "Setting TimeStamp falied on midas event: " DYELLOW "%i" RESET_COLOR "\n",midasserialnumber);
      return NumFragsFound;
   }
   int temp_charge =  0;
   int temp_cfd    =  0;
   int temp_led    =  0;
   for(;x<size;x++) {
      dword = *(data+x);
      type = (dword & 0xf0000000)>>28;
      value = (dword & 0x0fffffff);
      switch(type) {
         case 0x0: // raw wave forms.
            {
               TChannel *chan = TChannel::GetChannel(EventFrag->ChannelAddress);
               if(!no_waveforms)
                  SetTIGWave(value,EventFrag);
               if(chan && strncmp("Tr",chan->GetChannelName(),2)==0) { 
                  SetTIGWave(value,EventFrag);
               } else if(chan && strncmp("RF",chan->GetChannelName(),2)==0) { 
                  SetTIGWave(value,EventFrag);
               }
            }  
            break;
         case 0x1: // trapizodal wave forms.
            break;
         case 0x4: // cfd values.  This also ends the the fragment!
            SetTIGCfd(value,EventFrag);
            SetTIGCharge(temp_charge,EventFrag);
            SetTIGLed(temp_led,EventFrag);
            ///check whether the fragment is 'good'
            //FragsFound.push_back(EventFrag);
            TFragmentQueue::GetQueue("GOOD")->Add(EventFrag);
            NumFragsFound++;
            if( (*(data+x+1) & 0xf0000000) != 0xe0000000) {
               TFragment *transferfrag = EventFrag;
               EventFrag = new TFragment;
               EventFrag->MidasTimeStamp = transferfrag->MidasTimeStamp;
               EventFrag->MidasId        = transferfrag->MidasId;           
               EventFrag->TriggerId      = transferfrag->TriggerId;     
               EventFrag->TimeStampLow   = transferfrag->TimeStampLow;  
               EventFrag->TimeStampHigh  = transferfrag->TimeStampHigh; 
            }
            else
               EventFrag = 0;
            break;
         case 0x5: // raw charge evaluation.
            //SetTIGCharge(value,EventFrag);
            {
               TChannel *chan = TChannel::GetChannel(EventFrag->ChannelAddress);
               if(!chan) {
                  temp_charge = (value &  0x03ffffff);
                  if(value & 0x02000000)  
                     temp_charge = -( (~(value & 0x01ffffff)) & 0x01ffffff)+1;
               } else if(strncmp(chan->GetChannelName(),"Tig10",5) == 0) {
                  //eventfragment->PileUp = (value &  0x04000000);
                  temp_charge   = (value &  0x03ffffff);
                  if(value & 0x02000000)  {
                     temp_charge = -( (~(value & 0x01ffffff)) & 0x01ffffff)+1;
                  }
               } else if(strncmp(chan->GetChannelName(),"Tig64",5) == 0) {
                  //eventfragment->PileUp = (value &  0x04000000);
                  temp_charge   = (value &  0x003fffff);
                  if(value & 0x00200000)  {
                     temp_charge = -( (~(value & 0x001fffff)) & 0x001fffff)+1;
                  }
               } else {
                  temp_charge = (value &  0x03ffffff);
                  if(value & 0x02000000)  
                     temp_charge = -( (~(value & 0x01ffffff)) & 0x01ffffff)+1;
               }
            }
            //temp_charge = value;
            break;
         case 0x6:
            //SetTIGLed(value,EventFrag);
            temp_led = value;
            break;
         case 0xb:
            //SetTIGBitPattern
            break;
         case 0xc:
            SetTIGAddress(value,EventFrag);
            break;
         case 0xe: // this ends the bank!
            if(EventFrag)
               delete EventFrag;
            break;
         case 0xf:
            break;
         default:
            break;
      }
   }
   return NumFragsFound;
}

void TDataParser::SetTIGAddress(uint32_t value,TFragment *currentfrag) {
//Sets the digitizer address of the 'currentfrag' TFragment
   currentfrag->ChannelAddress = (int32_t)(0x00ffffff&value);   ///the front end number is not in the tig odb!
   return;
}

void TDataParser::SetTIGWave(uint32_t value,TFragment *currentfrag) {
//Sets the waveform for a Tigress event.

   //if(!currentfrag->wavebuffer)
   //   currentfrag->wavebuffer = new std::vector<short>;
   if(currentfrag->wavebuffer.size() > (100000) ) {printf("number of wave samples found is to great\n"); return;}       
   if (value & 0x00002000) {
      int temp =  value & 0x00003fff;
      temp = ~temp;
      temp = (temp & 0x00001fff) + 1;
      currentfrag->wavebuffer.push_back((int16_t)-temp); //eventfragment->SamplesFound++;
   } else {
      currentfrag->wavebuffer.push_back((int16_t)(value & 0x00001fff)); //eventfragment->SamplesFound++;
   }
   if ((value >> 14) & 0x00002000) {
      int temp =  (value >> 14) & 0x00003fff;
      temp = ~temp;
      temp = (temp & 0x00001fff) + 1;
      currentfrag->wavebuffer.push_back((int16_t)-temp); //eventfragment->SamplesFound++;
   } else {
      currentfrag->wavebuffer.push_back((int16_t)((value >> 14) & 0x00001fff) ); //eventfragment->SamplesFound++;
   }
   return;
}

void TDataParser::SetTIGCfd(uint32_t value,TFragment *currentfrag) {
//Sets the CFD of a Tigress Event.

   //currentfragment->SlowRiseTime = value & 0x08000000;
   currentfrag->Cfd.push_back( int32_t(value & 0x07ffffff));
   //std::string dig_type = "";//"Tig64";
   TChannel *chan = TChannel::GetChannel(currentfrag->ChannelAddress);
   if(!chan)
      chan = gChannel;
   std::string dig_type = (chan)->GetDigitizerType();

   // remove vernier for now and calculate the time to the trigger
   int32_t tsBits;
   int32_t cfdBits;
   if ( dig_type.compare(0,5,"Tig10")==0) {
      cfdBits = (currentfrag->Cfd.back() >> 4);
      tsBits  = currentfrag->TimeStampLow & 0x007fffff;
      // probably should check that there hasn't been any wrap around here
      //currentfrag->TimeToTrig = tsBits - cfdBits;
      currentfrag->Zc.push_back(tsBits - cfdBits);
   } else if ( dig_type.compare(0,5,"Tig64")==0 ) {
      //currentfrag->TimeToTrig = (currentfrag->Cfd.back() >> 5);
      currentfrag->Zc.push_back(tsBits - cfdBits);
      // cfdBits  = (eventfragment->Cfd >> 5);
      // tsBits  = eventfragment->TimeStampLow & 0x003fffff;
   } else {
      cfdBits = (currentfrag->Cfd.back() >> 4);
      tsBits  = currentfrag->TimeStampLow & 0x007fffff;
      currentfrag->Zc.push_back(tsBits - cfdBits);
      //printf(DYELLOW "Address: 0x%08x | " RESET_COLOR); (TChannel::GetChannel(currentfrag->ChannelAddress))->Print();
      //printf("CFD obtained without knowing digitizer type with midas Id = %d!\n",currentfrag->MidasId );
   }
   return;
}

void TDataParser::SetTIGLed(uint32_t value, TFragment *currentfrag) {
//Sets the LED of a Tigress event.
   currentfrag->Led.push_back( int32_t(value & 0x07ffffff) );
   return;
}

void TDataParser::SetTIGCharge(uint32_t value, TFragment *currentfragment) {
//Sets the integrated charge of a Tigress event.
   TChannel *chan = TChannel::GetChannel(currentfragment->ChannelAddress);
   if(!chan)
      chan = gChannel;
   std::string dig_type = chan->GetDigitizerType();
   currentfragment->ChannelNumber = chan->GetNumber();

   if( dig_type.compare(0,5,"Tig10") == 0)   {
     if(value & 0x02000000)   
         currentfragment->Charge.push_back( -( (~((int32_t)value & 0x01ffffff)) & 0x01ffffff)+1);
      else 
         currentfragment->Charge.push_back(value &  0x03ffffff);
   } else if( dig_type.compare(0,5,"Tig64") == 0) {
       if(value & 0x00200000) 
         currentfragment->Charge.push_back( -( (~((int32_t)value & 0x001fffff)) & 0x001fffff)+1);
      else  
         currentfragment->Charge.push_back((value &   0x003fffff));
   } else {
       if(value & 0x02000000) 
         currentfragment->Charge.push_back( -( (~((int32_t)value & 0x01ffffff)) & 0x01ffffff)+1);
       else
         currentfragment->Charge.push_back( ((int32_t)value &  0x03ffffff));
   }
}

bool TDataParser::SetTIGTriggerID(uint32_t value, TFragment *currentfrag) {
//Sets the Trigger ID of a Tigress event.
   if( (value&0xf0000000) != 0x80000000) {
      return false;         
   }
   value = value & 0x0fffffff;   
   unsigned int LastTriggerIdHiBits = fgLastTriggerId & 0xFF000000; // highest 8 bits, remainder will be 
   unsigned int LastTriggerIdLoBits = fgLastTriggerId & 0x00FFFFFF;   // determined by the reported value
   if ( value < fgMaxTriggerId / 10 ) {  // the trigger id has wrapped around 
      if ( LastTriggerIdLoBits > fgMaxTriggerId * 9 / 10 ) {
         currentfrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value + fgMaxTriggerId);
         printf(DBLUE "We are looping new trigger id = %lu, last trigger hi bits = %d," 
                      " last trigger lo bits = %d, value = %d,             midas = %d" 
                       RESET_COLOR "\n", currentfrag->TriggerId, LastTriggerIdHiBits, 
                       LastTriggerIdLoBits, value, 0);// midasserialnumber);           
      } else {
         currentfrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value);
      }
   } else if ( value < fgMaxTriggerId * 9 / 10 ) {
      currentfrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value);
   } else {
      if ( LastTriggerIdLoBits < fgMaxTriggerId / 10 ) {
         currentfrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value - fgMaxTriggerId);
         printf(DRED "We are backwards looping new trigger id = %lu, last trigger hi bits = %d,"
                     " last trigger lo bits = %d, value = %d, midas = %d" 
                      RESET_COLOR "\n", currentfrag->TriggerId, LastTriggerIdHiBits, 
                      LastTriggerIdLoBits, value, 0);//midasserialnumber);
      } else {
         currentfrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value);       
      }
   }  
   //fragment_id_map[value]++;
   //currentfrag->FragmentId = fragment_id_map[value];
   fgLastTriggerId = (unsigned long)currentfrag->TriggerId;
   return true; 
}


bool TDataParser::SetTIGTimeStamp(uint32_t *data,TFragment *currentfrag ) {
//Sets the Timestamp of a Tigress Event
   for(int x=0;x<10;x++) {
      data = data + 1;
      //printf(DBLUE "data for x = %i  |  0x%08x  |  0x%08x  |  0x%08x" RESET_COLOR "\n",x,*data,(*data)>>28,0xa);
      //if((*(data)&0xf0000000) == 0xa0000000){
      //if(((*data)&0xf0000000) == 0xa0000000){
      if(((*data)>>28)==0xa) {
         //printf(DRED "data for x = %i |  0x%08x" RESET_COLOR "\n",x,*data);
         break;
      }
   }
   if(!((*data&0xf0000000) == 0xa0000000)) { 
      printf("here 0?\t0x%08x\n",*data);
      return false;
   }
   //printf("data = 0x%08x\n",*data);

   int time[5];
   int x = 0;
   
   time[0] = *(data + x);  //printf("time[0] = 0x%08x\n",time[0]);
   x += 1;
   time[1] =   *(data + x);   //& 0x0fffffff;
   //printf("time[1] = 0x%08x\n",time[1]);
   if( (time[1] & 0xf0000000) != 0xa0000000) {
      printf("here 1?\tx = %i\t0x%08x\n",x,time[1]);
      return false;
      //break;
   } 
   x+=1;
   time[2] = *(data +x);
   if((time[2] & 0xf0000000) != 0xa0000000)   
          // this is ok, it always happens for tig64s.
     
   currentfrag->TimeStampLow = time[0] & 0x00ffffff;
   currentfrag->TimeStampHigh = time[1] & 0x00ffffff;
   //currentfrag->SetTimeStamp();
   return true;
   
   
   x+=1;
   time[3] = *(data +x);
   if((time[3] & 0xf0000000) != 0xa0000000) {  
      printf("here 2?\n");
      return false;
   }
   x+=1;
   time[4] = *(data +x);
   if((time[4] & 0xf0000000) != 0xa0000000) {  
      printf("here 3?\n");
      return false;
   }
   
   return true;
}





/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

int TDataParser::GriffinDataToFragment(uint32_t *data, int size, int bank, unsigned int midasserialnumber, time_t midastime) {
//Converts a Griffin flavoured MIDAS file into a TFragment and returns the number of words processed (or the negative index of the word it failed on)
   TFragment *EventFrag = new TFragment();

   EventFrag->MidasTimeStamp = midastime;
   EventFrag->MidasId = midasserialnumber; 

   int x = 0;  
   //int x = 6;
   if(!SetGRIFHeader(data[x++],EventFrag,bank)) {
      printf(DYELLOW "data[0] = 0x%08x" RESET_COLOR "\n",data[0]);
      delete EventFrag;
      return -x;
   }

   //Changed on 11 Aug 2015 by RD to include PPG events. If the event has DataType 3 and address 0xFFFF, it is a PPG event.
   if(EventFrag->DataType == 4 && EventFrag->ChannelAddress == 0xFFFF){
      delete EventFrag;
      return GriffinDataToPPGEvent(data,size,midasserialnumber,midastime);
   }

//   if(!SetGRIFPPG(data[x++],EventFrag)) {            //THIS FUNCTION SHOULD NOT BE USED
//      delete EventFrag;
//      return -x;
//   }
   //The master Filter Pattern is in an unstable state right now and is not
   //always written to the midas file
   if(SetGRIFMasterFilterPattern(data[x],EventFrag)) {
      x++;
   } 

   if(SetGRIFMasterFilterId(data[x],EventFrag)) {
      x++;
   }

   //The channel trigger ID is in an unstable state right now and is not
   //always written to the midas file
   if(!SetGRIFChannelTriggerId(data[x++],EventFrag)) {
      delete EventFrag;
      return -x;
   }

   //The Network packet number is for debugging and is not always written to
   //the midas file.
   if(SetGRIFNetworkPacket(data[x],EventFrag)) {
      x++;
   }

   if(!SetGRIFTimeStampLow(data[x++],EventFrag)) {
      delete EventFrag;
      return -x;
   }

   int  kwordcounter = 0;
   for(;x<size;x++) {
      uint32_t dword  = *(data+x);
      uint32_t packet = dword & 0xf0000000;
      uint32_t value  = dword & 0x0fffffff; 

      switch(packet) {
         case 0x80000000: //The 8 packet type is for event headers
               //if this happens, we have "accidentally" found another event.
               break;
         case 0xc0000000: //The c packet type is for waveforms
            if(!no_waveforms) 
               SetGRIFWaveForm(value,EventFrag);
            break;
         case 0xb0000000: //The b packet type contains the dead-time word
            SetGRIFDeadTime(value,EventFrag);
            break;
         case 0xd0000000: 
            SetGRIFNetworkPacket(dword,EventFrag); // The network packet placement is not yet stable.
            break;                                 
         case 0xe0000000:
            // changed on 21 Apr 2015 by JKS, when signal processing code from Chris changed the trailer.
            // change should be backward-compatible
            if((value & 0x3fff) == (EventFrag->ChannelId & 0x3fff)){
               if(record_stats)
                  FillStats(EventFrag); //we fill dead-time and run time stats from the fragment
               TFragmentQueue::GetQueue("GOOD")->Add(EventFrag);
               return x;
            } else  {
               TFragmentQueue::GetQueue("BAD")->Add(EventFrag);
               return -x;
            }
            break;
         case 0xf0000000:
            switch(bank){
               case 1: // header format from before May 2015 experiments
                  return -x;
                  break;
               case 2:
                  if(x+1 < size) {
                     SetGRIFCc(value, EventFrag);
                     ++x;
                     dword = *(data+x);
                     SetGRIFPsd(dword, EventFrag);
                  } else {
                     return -x;
                  }
                  break;
               default:
                  printf("This bank not yet defined.\n");
                  break;
            }
            break;
         
         default:
            if((packet & 0x80000000) == 0x00000000) {
               if(x+1 < size) {
                  EventFrag->KValue.push_back( (*(data+x) & 0x7c000000) >> 21 );
                  EventFrag->Charge.push_back((*(data+x) & 0x03ffffff));
                  ++x;
                  EventFrag->KValue.back() |= (*(data+x) & 0x7c000000) >> 26;
                  EventFrag->Cfd.push_back( (*(data+x) & 0x03ffffff));
               } else {
                  return -x;
               }
            }
         break;
      };
   }

   TFragmentQueue::GetQueue("BAD")->Add(EventFrag);
   return -x;
}





bool TDataParser::SetGRIFHeader(uint32_t value,TFragment *frag,int bank) {
   switch(bank){
      case 1: // header format from before May 2015 experiments
         //Sets: 
         //     The number of filters
         //     The Data Type
         //     Number of Pileups
         //     Channel Address
         //     Detector Type
         if( (value&0xf0000000) != 0x80000000) {
            return false;
         }
         frag->NumberOfFilters =  (value &0x0f000000)>> 24;
         frag->DataType        =  (value &0x00e00000)>> 21;
         frag->NumberOfPileups =  (value &0x001c0000)>> 18;
         frag->ChannelAddress  =  (value &0x0003fff0)>> 4;
         frag->DetectorType    =  (value &0x0000000f);
           
         // if(frag-DetectorType==2)
         //    frag->ChannelAddress += 0x8000;
         break;
      case 2:
         //Sets: 
         //     The number of filters
         //     The Data Type
         //     Number of Pileups
         //     Channel Address
         //     Detector Type
         if( (value&0xf0000000) != 0x80000000) {
            return false;
         }
         frag->NumberOfPileups =  (value &0x0c000000)>> 26;
         frag->DataType        =  (value &0x03800000)>> 23;
         frag->NumberOfFilters =  (value &0x00700000)>> 20;
         frag->ChannelAddress  =  (value &0x000ffff0)>> 4;
         frag->DetectorType    =  (value &0x0000000f);
           
         // if(frag-DetectorType==2)
         //    frag->ChannelAddress += 0x8000;
         break;
      default:
         printf("This bank not yet defined.\n");
         break;
   }
         
         
   TChannel *chan = TChannel::GetChannel(frag->ChannelAddress);
   if(chan) {
      frag->ChannelNumber = chan->GetNumber();
   }
   
   return true;
};


bool TDataParser::SetGRIFPPG(uint32_t value,TFragment *frag) {
//Sets the Programmable Pattern Generator words
   if( (value & 0xc0000000) != 0x00000000) {
      return false;
   }
   frag->PPG = value & 0x3fffffff;
   return true;
};

bool TDataParser::SetGRIFMasterFilterId(uint32_t value,TFragment *frag) {
//Sets the Griffin master filter ID and PPG
   if( (value &0x80000000) != 0x00000000) {
      return false;
   }
 
   frag->TriggerId = value & 0x7FFFFFFF;  //REAL
//        frag->TriggerId = value & 0x7fffff00;  //Testing
//        frag->TriggerBitPattern = value & 0x000000ff; //Testing
   return true;
}


bool TDataParser::SetGRIFMasterFilterPattern(uint32_t value, TFragment *frag) {
//Sets the Griffin Master Filter Pattern
   if( (value &0xc0000000) != 0x00000000) {
      return false;
   }
   frag->TriggerBitPattern = (value & 0x3fff0000) >> 16; // bit shift included by JKS
   frag->PPG = value & 0x0000ffff;//This is due to new GRIFFIN data format
   return true;
}


bool TDataParser::SetGRIFChannelTriggerId(uint32_t value, TFragment *frag) {
//Sets the Griffin Channel Trigger ID
   if( (value &0xf0000000) != 0x90000000) {
      return false;
   }
   frag->ChannelId = value & 0x0fffffff;
   return true;
}


bool TDataParser::SetGRIFNetworkPacket(uint32_t value, TFragment *frag) {
//Ignores the network packet number (for now)
//   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);
   if( (value &0xf0000000) != 0xd0000000) {
      return false;
   }
   if( (value&0x0f000000) == 0x0f000000) {
     // descant zero crossing time.
     frag->Zc.push_back( value & 0x00ffffff);
   }
   else {
     frag->NetworkPacketNumber = value & 0x00ffffff;
     //   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);
   }
   return true;
}

bool TDataParser::SetGRIFTimeStampLow(uint32_t value, TFragment *frag) {
//Sets the lower 28 bits of the griffin time stamp 
   if( (value &0xf0000000) != 0xa0000000) {
      return false;
   }
   frag->TimeStampLow = value & 0x0fffffff;
   return true;
}


bool TDataParser::SetGRIFWaveForm(uint32_t value,TFragment *currentfrag) {
//Sets the Griffin waveform if record_waveform is set to true
   if(currentfrag->wavebuffer.size() > (100000) ) {printf("number of wave samples found is to great\n"); return false;}       
   if (value & 0x00002000) {
      int temp =  value & 0x00003fff;
      temp = ~temp;
      temp = (temp & 0x00001fff) + 1;
      currentfrag->wavebuffer.push_back((int16_t)-temp); //eventfragment->SamplesFound++;
   } else {
      currentfrag->wavebuffer.push_back((int16_t)(value & 0x00001fff)); //eventfragment->SamplesFound++;
   }
   if ((value >> 14) & 0x00002000) {
      int temp =  (value >> 14) & 0x00003fff;
      temp = ~temp;
      temp = (temp & 0x00001fff) + 1;
      currentfrag->wavebuffer.push_back((int16_t)-temp); //eventfragment->SamplesFound++;
   } else {
      currentfrag->wavebuffer.push_back((int16_t)((value >> 14) & 0x00001fff) ); //eventfragment->SamplesFound++;
   }
   return true;
}



bool TDataParser::SetGRIFDeadTime(uint32_t value, TFragment *frag) {
//Sets the Griffin deadtime and the upper 14 bits of the timestamp
   frag->DeadTime      = (value & 0x0fffc000) >> 14;
   frag->TimeStampHigh = (value &  0x00003fff);
   return true;
}


bool TDataParser::SetGRIFCc(uint32_t value, TFragment* frag) {
   //set the short integration and the lower 9 bits of the long integration
   frag->ccShort.push_back(value & 0x7ffff);
   frag->ccLong.push_back(value >> 19);
   return true;
}


bool TDataParser::SetGRIFPsd(uint32_t value, TFragment* frag) {
   //set the zero crossing and the higher 10 bits of the long integration
   frag->Zc.push_back(value & 0x003fffff);
   frag->ccLong.back() |=  ((value & 0x7fe00000) >> 12);//21 bits from zero crossing minus 9 lower bits
   return true;
}

void TDataParser::FillStats(TFragment *frag) {
//Takes a TFragment and records statistics for it's channel address.
//The statistics recorded currently include:
//          The total deadtime for the channel
//          The lowest MIDAS Time stamp
//          The highest MIDAS Time stamp
   TGRSIStats *stat = TGRSIStats::GetStats(frag->ChannelAddress); //get the stats for the specific channel
   stat->IncDeadTime(frag->DeadTime); //Increments the deadtime for the specific channel.

   TGRSIStats::IncGoodEvents(); //Since we made it to this stage, we have a good event and should increment the good event counter
   //If this event contains either the lowest (or highest) time stamp recorded so far we should increment the 
   //lowest (or highest) time stamp in the stats. This allows us to determine the total run length.
   if( (frag->MidasTimeStamp < TGRSIStats::GetLowestMidasTimeStamp()) ||
       (TGRSIStats::GetLowestMidasTimeStamp() == 0)) {
      TGRSIStats::SetLowestMidasTimeStamp(frag->MidasTimeStamp);
   } else if (frag->MidasTimeStamp > TGRSIStats::GetHighestMidasTimeStamp()) {
      TGRSIStats::SetHighestMidasTimeStamp(frag->MidasTimeStamp);
   }
   //If this event contains the lowest (or highest) recorded packet number recorded so far we should increment the
   // lowest (or highest) network packet in the stats. This allows us to determine whe total number of network 
   //packets created.
   if( (frag->NetworkPacketNumber < TGRSIStats::GetLowestNetworkPacket()) ||
       (TGRSIStats::GetLowestNetworkPacket() == 0)) {
      TGRSIStats::SetLowestNetworkPacket(frag->NetworkPacketNumber);
   } else if (frag->NetworkPacketNumber > TGRSIStats::GetHighestNetworkPacket()) {
      TGRSIStats::SetHighestNetworkPacket(frag->NetworkPacketNumber);
   }

   return;
}

int TDataParser::GriffinDataToPPGEvent(uint32_t *data, int size, int bank, unsigned int midasserialnumber, time_t midastime) {
   TPPGData *ppgEvent = new TPPGData;
 
   int  kwordcounter = 0;
   int  x = 1; //We have already read the header so we can skip the 0th word.
   
   if(SetNewPPGPattern(data[x],ppgEvent)) {
      ++x;
   } 

   for(;x<size;x++) {
      uint32_t dword  = *(data+x);
      uint32_t packet = dword & 0xf0000000;
      uint32_t value  = dword & 0x0fffffff; 

      switch(packet) {
         case 0x80000000: //The 8 packet type is for event headers
               //if this happens, we have "accidentally" found another event.
               return -x;
         case 0x90000000: //The b packet type contains the dead-time word
            SetOldPPGPattern(value,ppgEvent);
            break;
         case 0xd0000000: 
            SetPPGNetworkPacket(dword,ppgEvent); // The network packet placement is not yet stable.
            break;              
         case 0xa0000000:
            SetPPGLowTimeStamp(value,ppgEvent);
            break;
         case 0xb0000000:
            SetPPGHighTimeStamp(value,ppgEvent);
            break;
         case 0xe0000000:
            if((value) == (ppgEvent->GetNewPPG())){
               TGRSIRootIO::Get()->FillPPG(ppgEvent);
               return x;
            } else  {
               return -x;
            }
            break;
      };
   }
   delete ppgEvent;
   //No trailer found
   return -x;
}

bool TDataParser::SetNewPPGPattern(uint32_t value, TPPGData* ppgevent){
   if( (value &0xf0000000) != 0x00000000) {
      return false;
   }
   ppgevent->SetNewPPG(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetOldPPGPattern(uint32_t value, TPPGData* ppgevent){
   ppgevent->SetOldPPG(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetPPGNetworkPacket(uint32_t value, TPPGData *ppgevent) {
//Ignores the network packet number (for now)
//   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);
   if( (value &0xf0000000) != 0xd0000000) {
      return false;
   }
   else {
     ppgevent->SetNetworkPacketId(value & 0x00ffffff);
     //   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);
   }
   return true;
}

bool TDataParser::SetPPGLowTimeStamp(uint32_t value, TPPGData *ppgevent){
   ppgevent->SetLowTimeStamp(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetPPGHighTimeStamp(uint32_t value, TPPGData *ppgevent){
   ppgevent->SetHighTimeStamp(value & 0x0fffffff);
   return true;
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

int TDataParser::EightPIDataToFragment(uint32_t stream,uint32_t* data,
                                     int size,unsigned int midasserialnumber, time_t midastime) {

   int NumFragsFound = 0;
   //TFragment *EventFrag = new TFragment();
   //EventFrag->MidasTimeStamp = midastime;
   //EventFrag->MidasId = midasserialnumber;  


   for(int i=0;i<size;i++) {
     if( (data[i]==0xff06) || (data[i]==0xff16) ) { //found a fifo...
        i+=1; //lets get the next int.
        int fifowords =  data[i] & 0x00001fff;
        i+=1;
        int fifoserial = data[i] & 0x000000ff;
        i+=1;
        if(stream == 0)
          printf(DBLUE "%i  GOOD FIFO,      fifowords = %i      fifoserial = %i   " RESET_COLOR  "\n",stream,fifowords,fifoserial);
        else if(stream == 1)
          printf(DGREEN "%i  GOOD FIFO,      fifowords = %i      fifoserial = %i   " RESET_COLOR  "\n",stream,fifowords,fifoserial);
        else if(stream == 2)
          printf(DRED "%i  GOOD FIFO,      fifowords = %i      fifoserial = %i   " RESET_COLOR  "\n",stream,fifowords,fifoserial);
        else if(stream == 3)
          printf(DYELLOW "%i  GOOD FIFO,      fifowords = %i      fifoserial = %i   " RESET_COLOR  "\n",stream,fifowords,fifoserial);
        //at this point am looking at the actual fera bank.
        //   ->convert to an arry of shorts.
        //   ->iterate
        bool extrafifo = false;
        unsigned short *words = (unsigned short*)(data+i);
        if(fifowords%2!=0) {
           fifowords += 1;
           extrafifo = true;
        }
        printf(DMAGENTA "i = %i    2*size =  %i   fifowords  = %i  " RESET_COLOR  "\n",i,2*size,fifowords);
        
        for(int j=0;j<fifowords;j+=2) {
           unsigned short temp = words[j];
           words[j] = words[j+1];
           words[j+1] = temp;
        }


        NumFragsFound += FifoToFragment(words,fifowords,extrafifo,midasserialnumber,midastime); 
        i+= (fifowords/2);
     }
   }



   //TFragmentQueue::GetQueue("GOOD")->Add(EventFrag);
   //return NumFragsFound;
   return 1;
}

int TDataParser::FifoToFragment(unsigned short *data,int size,bool zerobuffer,
                                unsigned int midasserialnumber, time_t midastime) {

   if(size<10) //this is too short to be anything useful
     return 0;

   TFragment *EventFrag = new TFragment();
   EventFrag->MidasTimeStamp = midastime;
   EventFrag->MidasId = midasserialnumber;    


   printf("\t");
   for(int j=0;j<size;j++) {
     if(j!=0 && (j%8==0))
       printf("\n\t");
     printf("0x%04x ",data[j]);
   }
   printf("\n\n");

   unsigned short type;
   unsigned short value,value2;
   
   int ulm = 0;
   if(zerobuffer)
      ulm = size-9;
   else 
      ulm = size-8;
   type = data[ulm];
   if((type &0xfff0) != 0xff20) { //not a ulm, bad things are happening.
     if(EventFrag) delete EventFrag;
     printf("here??   ulm = %i   0x%04x  \n",ulm,type);
     return 0;
   } else {
     EventFrag->DetectorType = (type&0x000f);
     value = data[ulm+1];
     EventFrag->PPG = (value&0x3ff);
     EventFrag->TriggerBitPattern = (value&0xf000)>>11;
     EventFrag->TimeStampLow  = data[ulm+2]*65536 +data[ulm+3];
     EventFrag->TimeStampHigh = data[ulm+4]*65536 +data[ulm+5];
     EventFrag->TriggerId     = data[ulm+6]*65536 +data[ulm+7];
   }
   size = ulm;  //only look at what is left.
   for(int x=0;x<size;x++) {
     type  = data[x]; // & 0x80f0;
     printf("type = 0x%04x  ||  type&0x80f0 = 0x%04x\n",type,(type&0x80f0));
     switch(type & 0x80f0) {
        case 0x8010:  // Lecroy 3377
          value = *(data+x+1);  //this is just so I can start the loop...
          x++;
          while(((value&0x8000)==0) && x<size) {
            value  = data[x]; x+=1;
            value2 = data[x]; x+=1; 
            if((value&0x7c00) != (value2&0x7c00)) 
               printf("TIME MISMATCH!  bad things are happening.\n");
            int temp = ((value&0x7c00)>>10) << 16;
            temp += ((value&0x00ff) << 8) + (value2&0x00ff);
            //printf("size = %i | ulm = %i | x = %i  | temp = 0x%08x \n",size,ulm,x,temp);
            EventFrag->Cfd.push_back(temp);
          }
          printf(DGREEN "ENDED TDC LOOP  data[%i] = 0x%04x" RESET_COLOR "\n",x,data[x]);
          break;
        case 0x8040:
        case 0x8050: // Ortect AD114 single channel ?
        case 0x8060:
          value = data[++x];
          if(EventFrag->DetectorType==0) {
             int temp = (type&0x001f) << 16;
             temp += value&0x3fff;
             //printf("temp = %08x\n",temp);
             EventFrag->Charge.push_back(temp);
             EventFrag->ChannelNumber = 0;
             //EventFrag->ChannelNumber  = (type&0x001f); //clear as mud.
             //EventFrag->ChannelAddress = 0x0000 + EventFrag->ChannelNumber;
             //EventFrag->Charge.push_back(value&0x3fff);
          } else if (EventFrag->DetectorType==3){
             int temp = (type&0x001f) << 16;
             temp += value&0x3fff;
             //printf("temp = %08x\n",temp);
             EventFrag->Charge.push_back(temp);
             EventFrag->ChannelNumber = 3;
             //EventFrag->ChannelNumber  = (type&0x001f); //clear as mud.
             //EventFrag->ChannelAddress = 0x3000 + EventFrag->ChannelNumber;
             //EventFrag->Charge.push_back(value&0x3fff);
          }
          //printf("value = 0x%04x\n",value);
          break;


     };
   }
   EventFrag->Print();


   TFragmentQueue::GetQueue("GOOD")->Add(EventFrag);

  return 1;
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////


int TDataParser::EPIXToScalar(float *data,int size,unsigned int midasserialnumber,time_t midastime) {

   int NumFragsFound = 1;
   TEpicsFrag *EXfrag = new TEpicsFrag;

   EXfrag->MidasTimeStamp = midastime;
   EXfrag->MidasId        = midasserialnumber;   


   for(int x=0;x<size;x++) {
      EXfrag->Data.push_back(*(data+x));
   }

   //EXfrag->Print();
   TGRSIRootIO::Get()->FillEpicsTree(EXfrag);
   delete EXfrag;
   return NumFragsFound;
}







