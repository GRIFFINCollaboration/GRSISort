#include "TDataParser.h"

#include "TChannel.h"
#include "Globals.h"

#include "TFragmentQueue.h"
#include "TScalerQueue.h"
#include "TGRSILoop.h"

#include "TEpicsFrag.h"
#include "TGRSIRootIO.h"
#include "TDiagnostics.h"

#include "Rtypes.h"

TDataParser* TDataParser::fDataParser = 0;
bool TDataParser::fNoWaveforms = false;
bool TDataParser::fRecordDiag = true;

const unsigned long TDataParser::fMaxTriggerId = 1024 * 1024 * 16; // 24 bits internally

unsigned long TDataParser::fLastMidasId = 0;
unsigned long TDataParser::fLastTriggerId = 0;
unsigned long TDataParser::fLastNetworkPacket = 0;

TChannel* TDataParser::gChannel = new TChannel;

/// \cond CLASSIMP
//ClassImp(TDataParser)
/// \endcond

TDataParser* TDataParser::instance() {
	///Get the global instance of the TDataParser Class
	if(!fDataParser)
		fDataParser = new TDataParser();
	return fDataParser;
}

TDataParser::TDataParser() {}

TDataParser::~TDataParser() {}


int TDataParser::TigressDataToFragment(uint32_t* data,int size,unsigned int midasSerialNumber, time_t midasTime) {
	///Converts A MIDAS File from the Tigress DAQ into a TFragment.
	std::vector<TFragment*> FragsFound;
	int NumFragsFound = 0;
	TFragment* EventFrag = new TFragment();
	EventFrag->MidasTimeStamp = midasTime;
	EventFrag->MidasId = midasSerialNumber;    

	int x = 0;
	uint32_t dword = *(data+x);
	uint32_t type = (dword & 0xf0000000)>>28;
	uint32_t value = (dword & 0x0fffffff);

	if(!SetTIGTriggerID(dword,EventFrag)) {
		delete EventFrag;
		printf(RED "Setting TriggerId (0x%08x) failed on midas event: " DYELLOW "%i" RESET_COLOR "\n",dword,midasSerialNumber);
		return -x;  
	}
	x+=1;

  //There can be a tigger bit pattern between the header and the time !   pcb.

	if(!SetTIGTimeStamp((data+x),EventFrag)) { 
		delete EventFrag;	
		printf(RED "%i Setting TimeStamp failed on midas event: " DYELLOW "%i" RESET_COLOR "\n",x,midasSerialNumber);
		return -x;
	}
	//int temp_charge =  0;
	int temp_led    =  0;
	for(;x<size;x++) {
		dword = *(data+x);
		type = (dword & 0xf0000000)>>28;
		value = (dword & 0x0fffffff);
		switch(type) {
			case 0x0: // raw wave forms.
				{
					TChannel* chan = TChannel::GetChannel(EventFrag->ChannelAddress);
					if(!fNoWaveforms)
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
				//SetTIGCharge(temp_charge,EventFrag);
				SetTIGLed(temp_led,EventFrag);
				///check whether the fragment is 'good'
				//FragsFound.push_back(EventFrag);
				TFragmentQueue::GetQueue("GOOD")->Add(EventFrag);
				NumFragsFound++;
				if( (*(data+x+1) & 0xf0000000) != 0xe0000000) {
					TFragment* transferfrag = EventFrag;
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
				SetTIGCharge(value,EventFrag);
				break;
			case 0x6:
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

void TDataParser::SetTIGAddress(uint32_t value,TFragment* currentFrag) {
	///Sets the digitizer address of the 'currentFrag' TFragment
	currentFrag->ChannelAddress = (int32_t)(0x00ffffff&value);   ///the front end number is not in the tig odb!
	return;
}

void TDataParser::SetTIGWave(uint32_t value,TFragment* currentFrag) {
	///Sets the waveform for a Tigress event.

	//if(!currentFrag->wavebuffer)
	//   currentFrag->wavebuffer = new std::vector<short>;
	if(currentFrag->wavebuffer.size() > (100000) ) {printf("number of wave samples found is to great\n"); return;}       
	if (value & 0x00002000) {
		int temp =  value & 0x00003fff;
		temp = ~temp;
		temp = (temp & 0x00001fff) + 1;
		currentFrag->wavebuffer.push_back((int16_t)-temp); //eventfragment->SamplesFound++;
	} else {
		currentFrag->wavebuffer.push_back((int16_t)(value & 0x00001fff)); //eventfragment->SamplesFound++;
	}
	if ((value >> 14) & 0x00002000) {
		int temp =  (value >> 14) & 0x00003fff;
		temp = ~temp;
		temp = (temp & 0x00001fff) + 1;
		currentFrag->wavebuffer.push_back((int16_t)-temp); //eventfragment->SamplesFound++;
	} else {
		currentFrag->wavebuffer.push_back((int16_t)((value >> 14) & 0x00001fff) ); //eventfragment->SamplesFound++;
	}
	return;
}

void TDataParser::SetTIGCfd(uint32_t value,TFragment* currentFrag) {
	///Sets the CFD of a Tigress Event.

	//currentFragment->SlowRiseTime = value & 0x08000000;
	currentFrag->Cfd.push_back( int32_t(value & 0x07ffffff));
	//std::string dig_type = "";//"Tig64";
	TChannel* chan = TChannel::GetChannel(currentFrag->ChannelAddress);
	if(!chan)
		chan = gChannel;
	std::string dig_type = (chan)->GetDigitizerType();

	// remove vernier for now and calculate the time to the trigger
	int32_t tsBits  = 0;
	int32_t cfdBits = 0;
	if ( dig_type.compare(0,5,"Tig10")==0) {
		cfdBits = (currentFrag->Cfd.back() >> 4);
		tsBits  = currentFrag->TimeStampLow & 0x007fffff;
		// probably should check that there hasn't been any wrap around here
		//currentFrag->TimeToTrig = tsBits - cfdBits;
		currentFrag->Zc.push_back(tsBits - cfdBits);
	} else if ( dig_type.compare(0,5,"Tig64")==0 ) {
		//currentFrag->TimeToTrig = (currentFrag->Cfd.back() >> 5);
		cfdBits	= (currentFrag->Cfd.back() >> 4) & 0x003fffff;
		//tsBits  = currentFrag->TimeStampLow & 0x0000ffff; //0x003fffff;
		currentFrag->Zc.push_back(abs(cfdBits)&0x000fffff);

		//currentFrag->Print();
		//printf("\n------------------------------\n\n\n");
	} else {
		cfdBits = (currentFrag->Cfd.back() >> 4);
		tsBits  = currentFrag->TimeStampLow & 0x007fffff;
		currentFrag->Zc.push_back(tsBits - cfdBits);
		//printf(DYELLOW "Address: 0x%08x | " RESET_COLOR); (TChannel::GetChannel(currentFrag->ChannelAddress))->Print();
		//printf("CFD obtained without knowing digitizer type with midas Id = %d!\n",currentFrag->MidasId );
	}
	return;
}

void TDataParser::SetTIGLed(uint32_t value, TFragment* currentFrag) {
	///Sets the LED of a Tigress event.
	currentFrag->Led.push_back( int32_t(value & 0x07ffffff) );
	return;
}

void TDataParser::SetTIGCharge(uint32_t value, TFragment* currentFragment) {
	///Sets the integrated charge of a Tigress event.
	TChannel* chan = TChannel::GetChannel(currentFragment->ChannelAddress);
	if(!chan)
		chan = gChannel;
	std::string dig_type = chan->GetDigitizerType();
	currentFragment->ChannelNumber = chan->GetNumber();

	if((dig_type.compare(0,5,"Tig10") == 0) || (dig_type.compare(0,5,"TIG10") == 0))	{
		if(value & 0x02000000)	
			currentFragment->Charge.push_back( -( (~((int32_t)value & 0x01ffffff)) & 0x01ffffff)+1);
		else 
			currentFragment->Charge.push_back(value &  0x03ffffff);
	} else if((dig_type.compare(0,5,"Tig64") == 0) || (dig_type.compare(0,5,"TIG64") == 0))	{
		if(value & 0x00200000)	
			currentFragment->Charge.push_back( -( (~((int32_t)value & 0x001fffff)) & 0x001fffff)+1);
		else  
			currentFragment->Charge.push_back((value &	0x003fffff));
	} else {
		if(value & 0x02000000)	
			currentFragment->Charge.push_back( -( (~((int32_t)value & 0x01ffffff)) & 0x01ffffff)+1);
		else
			currentFragment->Charge.push_back( ((int32_t)value &	0x03ffffff));
	}
	/*
		if(currentFragment->ChannelNumber>1983 && 
		currentFragment->ChannelNumber<1988 &&
		currentFragment->Charge.back()<0    ) {
		printf("Name     = %s\n",currentFragment->GetName());
		printf("Address  = 0x%08x\n",currentFragment->ChannelAddress);
		printf("value    = 0x%08x\t%i\n",value,value);
		printf("dig_type = %s\n",dig_type.c_str());
		printf("Charge   = 0x%08x\t%i\n",currentFragment->Charge.back(),currentFragment->Charge.back()/125.0);
		chan->Print();
		printf("----------------------------------------\n");
		}
		*/
}

bool TDataParser::SetTIGTriggerID(uint32_t value, TFragment* currentFrag) {
	///Sets the Trigger ID of a Tigress event.
	if( (value&0xf0000000) != 0x80000000) {
		return false;         
	}
	value = value & 0x0fffffff;   
	unsigned int LastTriggerIdHiBits = fLastTriggerId & 0xFF000000; // highest 8 bits, remainder will be 
	unsigned int LastTriggerIdLoBits = fLastTriggerId & 0x00FFFFFF;   // determined by the reported value
	if ( value < fMaxTriggerId / 10 ) {  // the trigger id has wrapped around 
		if ( LastTriggerIdLoBits > fMaxTriggerId * 9 / 10 ) {
			currentFrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value + fMaxTriggerId);
			printf(DBLUE "We are looping new trigger id = %lu, last trigger hi bits = %d," 
					" last trigger lo bits = %d, value = %d,             midas = %d" 
					RESET_COLOR "\n", currentFrag->TriggerId, LastTriggerIdHiBits, 
					LastTriggerIdLoBits, value, 0);// midasSerialNumber);           
		} else {
			currentFrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value);
		}
	} else if ( value < fMaxTriggerId * 9 / 10 ) {
		currentFrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value);
	} else {
		if ( LastTriggerIdLoBits < fMaxTriggerId / 10 ) {
			currentFrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value - fMaxTriggerId);
			printf(DRED "We are backwards looping new trigger id = %lu, last trigger hi bits = %d,"
					" last trigger lo bits = %d, value = %d, midas = %d" 
					RESET_COLOR "\n", currentFrag->TriggerId, LastTriggerIdHiBits, 
					LastTriggerIdLoBits, value, 0);//midasSerialNumber);
		} else {
			currentFrag->TriggerId = (uint64_t)(LastTriggerIdHiBits + value);       
		}
	}  
	//fragment_id_map[value]++;
	//currentFrag->FragmentId = fragment_id_map[value];
	fLastTriggerId = (unsigned long)currentFrag->TriggerId;
	return true; 
}


bool TDataParser::SetTIGTimeStamp(uint32_t* data,TFragment* currentFrag ) {
	///Sets the Timestamp of a Tigress Event
	for(int x=0;x<10;x++) { //finds the timestamp.
		data = data + 1;
		if(((*data)>>28)==0xa) {
			break;
		}
	}
  
 //printf("\n\n\ndata = 0x%08x\n\n\n",*data);  fflush(stdout);

	if(!((*data&0xf0000000) == 0xa0000000)) { 
		printf("here 0?\t0x%08x\n",*data);
		return false;
	}
	//printf("data = 0x%08x\n",*data);

	unsigned int time[5] = {0};  // tigress can report up to 5 valid timestamp words
	int x = 0;

  while((*(data+x)&0xf0000000)==0xa0000000) {
    time[x] = *(data+x);
    x+=1;
    if(x==5)
       break;
  }

  switch(x) {
    case 1: //bad.
      break;
    case 2: //minimum number of good a's
      if(time[0]!=time[1]) { // tig64's only have two, both second hex's are 0s. also some times tig10s.
	      currentFrag->TimeStampLow = time[0] & 0x00ffffff;
        currentFrag->TimeStampHigh = time[1] & 0x00ffffff;
        return true;
      }
      break;
    case 3:
      if(time[0]==time[1] && time[0]!=time[2]) {
        if( ((time[0]&0x0f000000)!=0x00000000) && ((time[2]&0x0f000000)!=0x01000000) )
           break;
	      currentFrag->TimeStampLow = time[0] & 0x00ffffff;
        currentFrag->TimeStampHigh = time[2] & 0x00ffffff;
      } else if(time[0]!=time[1] && time[1]==time[2]) {
        if( ((time[0]&0x0f000000)!=0x00000000) && ((time[1]&0x0f000000)!=0x01000000) )
           break;
	      currentFrag->TimeStampLow = time[0] & 0x00ffffff;
        currentFrag->TimeStampHigh = time[1] & 0x00ffffff;
      } else { // assume the third if the counter.
        //if( ((time[0]&0x0f000000)!=0x00000000) && ((time[1]&0x0f000000)!=0x01000000) )
        //   break;
	      currentFrag->TimeStampLow = time[0] & 0x00ffffff;
        currentFrag->TimeStampHigh = time[1] & 0x00ffffff;
      }
      return true;
    case 4:
    case 5:  
      if(time[0]==time[1] && time[2]==time[3]) {
        if( ((time[0]&0x0f000000)!=0x00000000) && ((time[2]&0x0f000000)!=0x01000000) )
           break;
	      currentFrag->TimeStampLow = time[0] & 0x00ffffff;
        currentFrag->TimeStampHigh = time[1] & 0x00ffffff;
      } else {
        if( ((time[0]&0x0f000000)!=0x00000000) && ((time[1]&0x0f000000)!=0x01000000) )
           break;
	      currentFrag->TimeStampLow = time[0] & 0x00ffffff;
        currentFrag->TimeStampHigh = time[1] & 0x00ffffff;
      }
      return true; 
   };

	return false;
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

int TDataParser::GriffinDataToFragment(uint32_t* data, int size, int bank, unsigned int midasSerialNumber, time_t midasTime) {
	///Converts a Griffin flavoured MIDAS file into a TFragment and returns the number of words processed (or the negative index of the word it failed on)
	TFragment* EventFrag = new TFragment();

	EventFrag->MidasTimeStamp = midasTime;
	EventFrag->MidasId = midasSerialNumber; 

	int x = 0;  
	if(!SetGRIFHeader(data[x++],EventFrag,bank)) {
		printf(DYELLOW "data[0] = 0x%08x" RESET_COLOR "\n",data[0]);
		TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(-1); //we failed to get a good header, so we don't know which detector type this fragment would've belonged to
		delete EventFrag;
		return -x;
	}
	//Changed on 11 Aug 2015 by RD to include PPG events. If the event has DataType 4 and address 0xFFFF, it is a PPG event.
   if(EventFrag->DataType == 4 && EventFrag->ChannelAddress == 0xFFFF){
		delete EventFrag;
		return GriffinDataToPPGEvent(data,size,midasSerialNumber,midasTime);
	}
	//If the event has detector type 15 (0xf) it's a scaler event.
	if(EventFrag->DetectorType == 0xf) {
		//a scaler event (trigger or deadtime) has 8 words (including header and trailer), make sure we have at least that much left
		if(size < 8) {
			return -x;
		}
		x = GriffinDataToScalerEvent(data, EventFrag->ChannelAddress);
		delete EventFrag;
		return x;
	}

	//The Network packet number is for debugging and is not always written to
	//the midas file.
	if(SetGRIFNetworkPacket(data[x],EventFrag)) {
		x++;
	}

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
		TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(EventFrag->DetectorType);
		delete EventFrag;
		return -x;
	}

	//The Network packet number is for debugging and is not always written to
	//the midas file.
	if(SetGRIFNetworkPacket(data[x],EventFrag)) {
		x++;
	}

	if(!SetGRIFTimeStampLow(data[x++],EventFrag)) {
		TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(EventFrag->DetectorType);
		delete EventFrag;
		return -x;
	}

	for(;x<size;x++) {
		uint32_t dword  = *(data+x);
		uint32_t packet = dword & 0xf0000000;
		uint32_t value  = dword & 0x0fffffff; 

		switch(packet) {
			case 0x80000000: //The 8 packet type is for event headers
				//if this happens, we have "accidentally" found another event.
				//currently the GRIF-C only sets the master/slave port of the address for the first header (of the corrupt event)
				//so we want to ignore this corrupt event and the next event which has a wrong address
				TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(EventFrag->DetectorType);
				delete EventFrag;
				return -(x+1);//+1 to ensure we don't read this header as start of a good event
				break;
			case 0xc0000000: //The c packet type is for waveforms
				if(!fNoWaveforms) 
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
					if(!TGRSILoop::Get()->GetSuppressError() && EventFrag->DataType == 2) {
						// check whether the nios finished and if so whether it finished with an error
						if(((value>>14) & 0x1) == 0x1) {
							if(((value>>16) & 0xff) != 0) {
								printf( BLUE "0x%04x: NIOS code finished with error 0x%02x" RESET_COLOR "\n",EventFrag->ChannelAddress, (value>>16) & 0xff);
							}
						}
					}

					// Adding new data members to the fragment is not backwards 
					// compatable.  Also, an unsigned int to care a 1 is a pretty exssive.
					// If we really need this, I suggest moving to a TBits Objectt, which we 
					// can move all such flags too and expained with out breaking things backwards.
					//if(EventFrag->DataType == 1) {
					//   EventFrag->AcceptedChannelId = (value>>14) & 0x3fff;
					//printf("Set AcceptedChannelId to 0x%04x (from 0x%08x => 0x%04x)\n", EventFrag->AcceptedChannelId, value, value>>14);
					//} else {
					//   EventFrag->AcceptedChannelId = 0;
					//}

					if(fRecordDiag) TGRSIRootIO::Get()->GetDiagnostics()->GoodFragment(EventFrag);
					TFragmentQueue::GetQueue("GOOD")->Add(EventFrag);
					return x;
				} else  {
					if(fRecordDiag) TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(EventFrag->DetectorType);
					TFragmentQueue::GetQueue("BAD")->Add(EventFrag);
					return -x;
				}
				break;
			case 0xf0000000:
				switch(bank){
					case 1: // header format from before May 2015 experiments
						TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(EventFrag->DetectorType);
						delete EventFrag;
						return -x;
						break;
					case 2:
						if(x+1 < size) {
							SetGRIFCc(value, EventFrag);
							++x;
							dword = *(data+x);
							SetGRIFPsd(dword, EventFrag);
						} else {
							TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(EventFrag->DetectorType);
							delete EventFrag;
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
					//check that there is another word and that it is also a charge/cfd word
					if(x+1 < size &&  (*(data+x+1) & 0x80000000) == 0x0) {
						EventFrag->KValue.push_back( (*(data+x) & 0x7c000000) >> 21 );
						EventFrag->Charge.push_back((*(data+x) & 0x03ffffff));
						++x;
						EventFrag->KValue.back() |= (*(data+x) & 0x7c000000) >> 26;
						EventFrag->Cfd.push_back( (*(data+x) & 0x03ffffff));
					} else {
						//these types of corrupt events quite often end without a trailer which leads to the header of the next event missing the master/slave part of the address
						//so we look for the next trailer and stop there
						while((data[x] & 0xf0000000) != 0xe0000000) ++x;
						TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(EventFrag->DetectorType);
						delete EventFrag;
						return -x;
					}
				}
				break;
		};
	}

	TFragmentQueue::GetQueue("BAD")->Add(EventFrag);
	TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(EventFrag->DetectorType);
	return -x;
}

bool TDataParser::SetGRIFHeader(uint32_t value,TFragment* frag,int bank) {
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


	TChannel* chan = TChannel::GetChannel(frag->ChannelAddress);
	if(chan) {
		frag->ChannelNumber = chan->GetNumber();
	}

	return true;
}

bool TDataParser::SetGRIFMasterFilterId(uint32_t value,TFragment* frag) {
	///Sets the Griffin master filter ID and PPG
	if( (value &0x80000000) != 0x00000000) {
		return false;
	}

	frag->TriggerId = value & 0x7FFFFFFF;  //REAL
	//        frag->TriggerId = value & 0x7fffff00;  //Testing
	//        frag->TriggerBitPattern = value & 0x000000ff; //Testing
	return true;
}

bool TDataParser::SetGRIFMasterFilterPattern(uint32_t value, TFragment* frag) {
	///Sets the Griffin Master Filter Pattern
	if( (value &0xc0000000) != 0x00000000) {
		return false;
	}
	frag->TriggerBitPattern = (value & 0x3fff0000) >> 16; // bit shift included by JKS
	frag->PPG = value & 0x0000ffff;//This is due to new GRIFFIN data format
	return true;
}

bool TDataParser::SetGRIFChannelTriggerId(uint32_t value, TFragment* frag) {
	///Sets the Griffin Channel Trigger ID
	if( (value &0xf0000000) != 0x90000000) {
		return false;
	}
	frag->ChannelId = value & 0x0fffffff;
	return true;
}

bool TDataParser::SetGRIFNetworkPacket(uint32_t value, TFragment* frag) {
	///Ignores the network packet number (for now)
	//   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);
	if( (value &0xf0000000) != 0xd0000000) {
		return false;
	}
	if( (value&0x0f000000) == 0x0f000000 && frag->NetworkPacketNumber>0 ) {
		// descant zero crossing time.
		frag->Zc.push_back( value & 0x00ffffff);
	} else {
		frag->NetworkPacketNumber = value & 0x0fffffff;
		//   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);
	}
	return true;
}

bool TDataParser::SetGRIFTimeStampLow(uint32_t value, TFragment* frag) {
	///Sets the lower 28 bits of the griffin time stamp 
	if( (value &0xf0000000) != 0xa0000000) {
		return false;
	}
	frag->TimeStampLow = value & 0x0fffffff;
	return true;
}


bool TDataParser::SetGRIFWaveForm(uint32_t value,TFragment* currentFrag) {
	///Sets the Griffin waveform if record_waveform is set to true
	if(currentFrag->wavebuffer.size() > (100000) ) {printf("number of wave samples found is to great\n"); return false;}       
	if (value & 0x00002000) {
		int temp =  value & 0x00003fff;
		temp = ~temp;
		temp = (temp & 0x00001fff) + 1;
		currentFrag->wavebuffer.push_back((int16_t)-temp); //eventfragment->SamplesFound++;
	} else {
		currentFrag->wavebuffer.push_back((int16_t)(value & 0x00001fff)); //eventfragment->SamplesFound++;
	}
	if ((value >> 14) & 0x00002000) {
		int temp =  (value >> 14) & 0x00003fff;
		temp = ~temp;
		temp = (temp & 0x00001fff) + 1;
		currentFrag->wavebuffer.push_back((int16_t)-temp); //eventfragment->SamplesFound++;
	} else {
		currentFrag->wavebuffer.push_back((int16_t)((value >> 14) & 0x00001fff) ); //eventfragment->SamplesFound++;
	}
	return true;
}



bool TDataParser::SetGRIFDeadTime(uint32_t value, TFragment* frag) {
	///Sets the Griffin deadtime and the upper 14 bits of the timestamp
	frag->DeadTime      = (value & 0x0fffc000) >> 14;
	frag->TimeStampHigh = (value &  0x00003fff);
	return true;
}


bool TDataParser::SetGRIFCc(uint32_t value, TFragment* frag) {
	///set the short integration and the lower 9 bits of the long integration
	frag->ccShort.push_back(value & 0x7ffff);
	frag->ccLong.push_back(value >> 19);
	return true;
}


bool TDataParser::SetGRIFPsd(uint32_t value, TFragment* frag) {
	///set the zero crossing and the higher 10 bits of the long integration
	frag->Zc.push_back(value & 0x003fffff);
	frag->ccLong.back() |=  ((value & 0x7fe00000) >> 12);//21 bits from zero crossing minus 9 lower bits
	return true;
}

int TDataParser::GriffinDataToPPGEvent(uint32_t* data, int size, int bank, unsigned int midasSerialNumber, time_t midasTime) {
	TPPGData* ppgEvent = new TPPGData;
	int  x = 1; //We have already read the header so we can skip the 0th word.

	//The Network packet number is for debugging and is not always written to
	//the midas file.
	if(SetPPGNetworkPacket(data[x],ppgEvent)){ // The network packet placement is not yet stable.
		++x;
	}
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
				//if((value & 0xFFFF) == (ppgEvent->GetNewPPG())){
					TGRSIRootIO::Get()->FillPPG(ppgEvent);
					TGRSIRootIO::Get()->GetDiagnostics()->GoodFragment(-2); //use detector type -2 for PPG
					return x;
				//} else  {
				//	TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(-2); //use detector type -2 for PPG
				//	return -x;
				//}
				break;
		};
	}
	delete ppgEvent;
	//No trailer found
	TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(-2); //use detector type -2 for PPG
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

bool TDataParser::SetPPGNetworkPacket(uint32_t value, TPPGData* ppgevent) {
	//Ignores the network packet number (for now)
	//   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);
	if( (value &0xf0000000) != 0xd0000000) {
		return false;
	} else {
		ppgevent->SetNetworkPacketId(value & 0x00ffffff);
		//   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);
	}
	return true;
}

bool TDataParser::SetPPGLowTimeStamp(uint32_t value, TPPGData* ppgevent){
	ppgevent->SetLowTimeStamp(value & 0x0fffffff);
	return true;
}

bool TDataParser::SetPPGHighTimeStamp(uint32_t value, TPPGData* ppgevent){
	ppgevent->SetHighTimeStamp(value & 0x0fffffff);
	return true;
}

int TDataParser::GriffinDataToScalerEvent(uint32_t* data, int address) {
	TScalerData* scalerEvent = new TScalerData;
	scalerEvent->SetAddress(address);
	int  x = 1; //We have already read the header so we can skip the 0th word.

	//we expect a word starting with 0xd containing the network packet id
	//this is a different format than the others because it will not always be in the scaler word
	if(SetScalerNetworkPacket(data[x],scalerEvent)) {
		x++;
	}

	//we expect a word starting with 0xa containing the 28 lowest bits of the timestamp
	if(!SetScalerLowTimeStamp(data[x++],scalerEvent)) {
		TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(-3); //use detector type -3 for scaler data
		return -x;
	}
	//followed by four scaler words (32 bits each)
	for(int i = 0; i < 4; ++i) {
		if(!SetScalerValue(i, data[x++], scalerEvent)) {
			TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(-3); //use detector type -3 for scaler data
			return -x;
		}
	}
	//and finally the trailer word with the highest 24 bits of the timestamp
	int scalerType = 0;
	if(!SetScalerHighTimeStamp(data[x++],scalerEvent,scalerType)) {
		TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(-3); //use detector type -3 for scaler data
		return -x;
	}

	if(scalerType == 0) { //deadtime scaler
		TDeadtimeScalerQueue::Get()->Add(scalerEvent);
	} else if(scalerType == 1) { //rate scaler
		//the rate scaler has only one real value, the rate
		scalerEvent->ResizeScaler();
		TRateScalerQueue::Get()->Add(scalerEvent);
	} else { //unknown scaler type
		TGRSIRootIO::Get()->GetDiagnostics()->BadFragment(-3); //use detector type -3 for scaler data
		return -x;
	}

	TGRSIRootIO::Get()->GetDiagnostics()->GoodFragment(-3); //use detector type -3 for scaler data
	return x;
}

bool TDataParser::SetScalerNetworkPacket(uint32_t value, TScalerData* scalerEvent) {
	if((value>>28) != 0xd) {
		return false;
	}
	scalerEvent->SetNetworkPacketId(value & 0x0fffffff);
	return true;
}

bool TDataParser::SetScalerLowTimeStamp(uint32_t value, TScalerData* scalerEvent) {
	if((value>>28) != 0xa) {
		return false;
	}
	scalerEvent->SetLowTimeStamp(value & 0x0fffffff);
	return true;
}

bool TDataParser::SetScalerHighTimeStamp(uint32_t value, TScalerData* scalerEvent, int& type) {
	if((value>>28) != 0xe || (value&0xff) != (scalerEvent->GetLowTimeStamp()>>20)) {
		return false;
	}
	scalerEvent->SetHighTimeStamp((value>>8) & 0x0000ffff);
	type = (value>>24 & 0xf);
	return true;
}

bool TDataParser::SetScalerValue(int index, uint32_t value, TScalerData* scalerEvent) {
	scalerEvent->SetScaler(index, value);
	return true;
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

int TDataParser::EightPIDataToFragment(uint32_t stream,uint32_t* data,
		int size,unsigned int midasSerialNumber, time_t midasTime) {

	int NumFragsFound = 0;
	//TFragment* EventFrag = new TFragment();
	//EventFrag->MidasTimeStamp = midasTime;
	//EventFrag->MidasId = midasSerialNumber;  


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
			unsigned short* words = reinterpret_cast<unsigned short*>(data+i);
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


			NumFragsFound += FifoToFragment(words,fifowords,extrafifo,midasSerialNumber,midasTime); 
			i+= (fifowords/2);
		}
	}



	//TFragmentQueue::GetQueue("GOOD")->Add(EventFrag);
	//return NumFragsFound;
	return 1;
}

int TDataParser::FifoToFragment(unsigned short* data,int size,bool zerobuffer,
		unsigned int midasSerialNumber, time_t midasTime) {

	if(size<10) //this is too short to be anything useful
		return 0;

	TFragment* EventFrag = new TFragment();
	EventFrag->MidasTimeStamp = midasTime;
	EventFrag->MidasId = midasSerialNumber;    


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


int TDataParser::EPIXToScalar(float* data,int size,unsigned int midasSerialNumber,time_t midasTime) {
	int NumFragsFound = 1;
	TEpicsFrag* EXfrag = new TEpicsFrag;

	EXfrag->fMidasTimeStamp = midasTime;
	EXfrag->fMidasId        = midasSerialNumber;   


	for(int x=0;x<size;x++) {
		EXfrag->fData.push_back(*(data+x));
	}

	TGRSIRootIO::Get()->FillEpicsTree(EXfrag);
	delete EXfrag;
	return NumFragsFound;
}
