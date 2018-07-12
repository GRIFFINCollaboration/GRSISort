#include "TDataParser.h"
#include "TDataParserException.h"

#include "TChannel.h"
#include "Globals.h"

#include "TScalerQueue.h"

#include "TEpicsFrag.h"
#include "TParsingDiagnostics.h"

#include "Rtypes.h"

#include "TFragment.h"
#include "TBadFragment.h"

TGRSIOptions* TDataParser::fOptions = nullptr;

TDataParser::TDataParser()
   : fBadOutputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TBadFragment>>>("bad_frag_queue")),
     fScalerOutputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TEpicsFrag>>>("scaler_queue")),
     fNoWaveforms(false), fRecordDiag(true), fMaxTriggerId(1024 * 1024 * 16), fLastMidasId(0), fLastTriggerId(0),
     fLastNetworkPacket(0), fFragmentHasWaveform(false), fFragmentMap(fGoodOutputQueues, fBadOutputQueue),
     fItemsPopped(nullptr), fInputSize(nullptr)
{
   gChannel = new TChannel;
}

TDataParser::~TDataParser()
{
   delete gChannel;
}

void TDataParser::ClearQueue()
{
   std::shared_ptr<const TFragment> frag;
   for(const auto& outQueue : fGoodOutputQueues) {
      while(outQueue->Size() != 0u) {
         outQueue->Pop(frag);
      }
   }
   std::shared_ptr<const TBadFragment> badFrag;
   while(fBadOutputQueue->Size() != 0u) {
      fBadOutputQueue->Pop(badFrag);
   }
   std::shared_ptr<TEpicsFrag> epicsFrag;
   while(fScalerOutputQueue->Size() != 0u) {
      fScalerOutputQueue->Pop(epicsFrag);
   }
}

void TDataParser::SetFinished()
{
   for(const auto& outQueue : fGoodOutputQueues) {
      outQueue->SetFinished();
   }
   fBadOutputQueue->SetFinished();
   fScalerOutputQueue->SetFinished();
}

int TDataParser::TigressDataToFragment(uint32_t* data, int size, unsigned int midasSerialNumber, time_t midasTime)
{
   /// Converts A MIDAS File from the Tigress DAQ into a TFragment.
   int                        NumFragsFound = 0;
   std::shared_ptr<TFragment> eventFrag     = std::make_shared<TFragment>();
   eventFrag->SetMidasTimeStamp(midasTime);
   eventFrag->SetMidasId(midasSerialNumber);

   int      x     = 0;
   uint32_t dword = *(data + x);

   uint32_t type;
   uint32_t value;

   if(!SetTIGTriggerID(dword, eventFrag)) {
      printf(RED "Setting TriggerId (0x%08x) failed on midas event: " DYELLOW "%i" RESET_COLOR "\n", dword,
             midasSerialNumber);
      return -x;
   }
   x += 1;

   // There can be a tigger bit pattern between the header and the time !   pcb.

   if(!SetTIGTimeStamp((data + x), eventFrag)) {
      printf(RED "%i Setting TimeStamp failed on midas event: " DYELLOW "%i" RESET_COLOR "\n", x, midasSerialNumber);
      return -x;
   }
   // int temp_charge =  0;
   int temp_led = 0;
   for(; x < size; x++) {
      dword = *(data + x);
      type  = (dword & 0xf0000000) >> 28;
      value = (dword & 0x0fffffff);
      switch(type) {
      case 0x0: // raw wave forms.
      {
         TChannel* chan = TChannel::GetChannel(eventFrag->GetAddress());
         if(!fNoWaveforms) {
            SetTIGWave(value, eventFrag);
         }
         if((chan != nullptr) && strncmp("Tr", chan->GetName(), 2) == 0) {
            SetTIGWave(value, eventFrag);
         } else if((chan != nullptr) && strncmp("RF", chan->GetName(), 2) == 0) {
            SetTIGWave(value, eventFrag);
         }
      } break;
      case 0x1: // trapizodal wave forms.
         break;
      case 0x4: // cfd values.  This also ends the the fragment!
         SetTIGCfd(value, eventFrag);
         SetTIGLed(temp_led, eventFrag);
         /// check whether the fragment is 'good'

         if(((*(data + x + 1)) & 0xf0000000) != 0xe0000000) {
            std::shared_ptr<TFragment> transferfrag = std::make_shared<TFragment>(*eventFrag);
            eventFrag                               = std::make_shared<TFragment>();
            eventFrag->SetMidasTimeStamp(transferfrag->GetMidasTimeStamp());
            eventFrag->SetMidasId(transferfrag->GetMidasId());
            eventFrag->SetTriggerId(transferfrag->GetTriggerId());
            eventFrag->SetTimeStamp(transferfrag->GetTimeStamp());

            Push(fGoodOutputQueues, transferfrag);
            NumFragsFound++;

            // printf("transferfrag = 0x%08x\n",transferfrag); fflush(stdout);
            // printf("transferfrag->GetTimeStamp() = %lu\n",transferfrag->GetTimeStamp()); fflush(stdout);
            // printf("eventFrag: = 0x%08x\n",eventFrag); fflush(stdout);
            // printf("eventFrag->GetTimeStamp() = %lu\n",eventFrag->GetTimeStamp()); fflush(stdout);
         } else {
            std::shared_ptr<TFragment> transferfrag = std::make_shared<TFragment>(*eventFrag);
            Push(fGoodOutputQueues, transferfrag);
            NumFragsFound++;
            eventFrag = nullptr;
            return NumFragsFound;
         }

         break;
      case 0x5: // raw charge evaluation.
         SetTIGCharge(value, eventFrag);
         break;
      case 0x6: temp_led = value; break;
      case 0xb:
         // SetTIGBitPattern
         break;
      case 0xc: SetTIGAddress(value, eventFrag); break;
      case 0xe: // this ends the bank!
         if(eventFrag) {
            // printf("this is never called\n"); fflush(stdout);
            return -x;
         }
         break;
      case 0xf: break;
      default: break;
      }
   }
   return NumFragsFound;
}

void TDataParser::SetTIGAddress(uint32_t value, const std::shared_ptr<TFragment>& currentFrag)
{
   /// Sets the digitizer address of the 'currentFrag' TFragment
   currentFrag->SetAddress(static_cast<int32_t>(0x00ffffff & value));
}

void TDataParser::SetTIGWave(uint32_t value, const std::shared_ptr<TFragment>& currentFrag)
{
   /// Sets the waveform for a Tigress event.

   if(currentFrag->GetWaveform()->size() > (100000)) {
      printf("number of wave samples found is to great\n");
      return;
   }

   if((value & 0x00002000) != 0u) {
      int temp = value & 0x00003fff;
      temp     = ~temp;
      temp     = (temp & 0x00001fff) + 1;
      currentFrag->AddWaveformSample(static_cast<Short_t>(-temp));
   } else {
      currentFrag->AddWaveformSample(static_cast<Short_t>(value & 0x00001fff));
   }
   if(((value >> 14) & 0x00002000) != 0u) {
      int temp = (value >> 14) & 0x00003fff;
      temp     = ~temp;
      temp     = (temp & 0x00001fff) + 1;
      currentFrag->AddWaveformSample(static_cast<Short_t>(-temp));
   } else {
      currentFrag->AddWaveformSample(static_cast<Short_t>((value >> 14) & 0x00001fff));
   }
   return;
}

void TDataParser::SetTIGCfd(uint32_t value, const std::shared_ptr<TFragment>& currentFrag)
{
   /// Sets the CFD of a Tigress Event.

   // currentFragment->SlowRiseTime = value & 0x08000000;
   currentFrag->SetCfd(int32_t(value & 0x07ffffff));
   // std::string dig_type = "";//"Tig64";
   TChannel* chan = TChannel::GetChannel(currentFrag->GetAddress());
   if(chan == nullptr) {
      chan = gChannel;
   }
   std::string dig_type = (chan)->GetDigitizerTypeString();

   // Zero-crossing now transient, why bother calculating it.
   // // remove vernier for now and calculate the time to the trigger
   // int32_t tsBits  = 0;
   // int32_t cfdBits = 0;
   // if ( dig_type.compare(0,5,"Tig10")==0) {
   //   cfdBits = (currentFrag->GetCfd() >> 4);
   //   tsBits  = currentFrag->GetTimeStamp() & 0x007fffff;
   //   // probably should check that there hasn't been any wrap around here
   //   //currentFrag->TimeToTrig = tsBits - cfdBits;
   //   currentFrag->Zc.push_back(tsBits - cfdBits);
   // } else if ( dig_type.compare(0,5,"Tig64")==0 ) {
   //   //currentFrag->TimeToTrig = (currentFrag->Cfd.back() >> 5);
   //   cfdBits	= (currentFrag->Cfd.back() >> 4) & 0x003fffff;
   //   //tsBits  = currentFrag->TimeStampLow & 0x0000ffff; //0x003fffff;
   //   currentFrag->Zc.push_back(abs(cfdBits)&0x000fffff);

   //   //currentFrag->Print();
   //   //printf("\n------------------------------\n\n\n");
   // } else {
   //   cfdBits = (currentFrag->Cfd.back() >> 4);
   //   tsBits  = currentFrag->TimeStampLow & 0x007fffff;
   //   currentFrag->Zc.push_back(tsBits - cfdBits);
   //   //printf(DYELLOW "Address: 0x%08x | " RESET_COLOR);
   //   (TChannel::GetChannel(currentFrag->ChannelAddress))->Print();
   //   //printf("CFD obtained without knowing digitizer type with midas Id = %d!\n",currentFrag->MidasId );
   // }
   return;
}

void TDataParser::SetTIGLed(uint32_t, const std::shared_ptr<TFragment>&)
{
   /// Sets the LED of a Tigress event.
   // No longer used anywhere
   //  currentFrag->SetLed( int32_t(value & 0x07ffffff) );
}

void TDataParser::SetTIGCharge(uint32_t value, const std::shared_ptr<TFragment>& currentFragment)
{
   /// Sets the integrated charge of a Tigress event.
   TChannel* chan = currentFragment->GetChannel();
   if(chan == nullptr) {
      chan = gChannel;
   }
   std::string dig_type = chan->GetDigitizerTypeString();

   int charge;
   if((dig_type.compare(0, 5, "Tig10") == 0) || (dig_type.compare(0, 5, "TIG10") == 0)) {
      if((value & 0x02000000) != 0u) {
         charge = (-((~(static_cast<int32_t>(value) & 0x01ffffff)) & 0x01ffffff) + 1);
      } else {
         charge = (value & 0x03ffffff);
      }
   } else if((dig_type.compare(0, 5, "Tig64") == 0) || (dig_type.compare(0, 5, "TIG64") == 0)) {
      if((value & 0x00200000) != 0u) {
         charge = (-((~(static_cast<int32_t>(value) & 0x001fffff)) & 0x001fffff) + 1);
      } else {
         charge = ((value & 0x003fffff));
      }
   } else {
      if((value & 0x02000000) != 0u) {
         charge = (-((~(static_cast<int32_t>(value) & 0x01ffffff)) & 0x01ffffff) + 1);
      } else {
         charge = ((static_cast<int32_t>(value) & 0x03ffffff));
      }
   }
   currentFragment->SetCharge(charge);
}

bool TDataParser::SetTIGTriggerID(uint32_t value, const std::shared_ptr<TFragment>& currentFrag)
{
   /// Sets the Trigger ID of a Tigress event.
   if((value & 0xf0000000) != 0x80000000) {
      return false;
   }
   value                            = value & 0x0fffffff;
   unsigned int LastTriggerIdHiBits = fLastTriggerId & 0xFF000000; // highest 8 bits, remainder will be
   unsigned int LastTriggerIdLoBits = fLastTriggerId & 0x00FFFFFF; // determined by the reported value
   if(value < fMaxTriggerId / 10) {                                // the trigger id has wrapped around
      if(LastTriggerIdLoBits > fMaxTriggerId * 9 / 10) {
         currentFrag->SetTriggerId((uint64_t)(LastTriggerIdHiBits + value + fMaxTriggerId));
         printf(DBLUE "We are looping new trigger id = %lu, last trigger hi bits = %d,"
                      " last trigger lo bits = %d, value = %d,             midas = %d" RESET_COLOR "\n",
                currentFrag->GetTriggerId(), LastTriggerIdHiBits, LastTriggerIdLoBits, value, 0); // midasSerialNumber);
      } else {
         currentFrag->SetTriggerId(static_cast<uint64_t>(LastTriggerIdHiBits + value));
      }
   } else if(value < fMaxTriggerId * 9 / 10) {
      currentFrag->SetTriggerId(static_cast<uint64_t>(LastTriggerIdHiBits + value));
   } else {
      if(LastTriggerIdLoBits < fMaxTriggerId / 10) {
         currentFrag->SetTriggerId((uint64_t)(LastTriggerIdHiBits + value - fMaxTriggerId));
         printf(DRED "We are backwards looping new trigger id = %lu, last trigger hi bits = %d,"
                     " last trigger lo bits = %d, value = %d, midas = %d" RESET_COLOR "\n",
                currentFrag->GetTriggerId(), LastTriggerIdHiBits, LastTriggerIdLoBits, value, 0); // midasSerialNumber);
      } else {
         currentFrag->SetTriggerId(static_cast<uint64_t>(LastTriggerIdHiBits + value));
      }
   }
   // fragment_id_map[value]++;
   // currentFrag->FragmentId = fragment_id_map[value];
   fLastTriggerId = static_cast<unsigned long>(currentFrag->GetTriggerId());
   return true;
}

bool TDataParser::SetTIGTimeStamp(uint32_t* data, const std::shared_ptr<TFragment>& currentFrag)
{
   /// Sets the Timestamp of a Tigress Event
   for(int x = 0; x < 10; x++) { // finds the timestamp.
      data = data + 1;
      if(((*data) >> 28) == 0xa) {
         break;
      }
   }
   long timestamplow  = -1;
   long timestamphigh = -1;

   // printf("\n\n\ndata = 0x%08x\n\n\n",*data);  fflush(stdout);

   if(!((*data & 0xf0000000) == 0xa0000000)) {
      printf("here 0?\t0x%08x\n", *data);
      return false;
   }
   // printf("data = 0x%08x\n",*data);

   unsigned int time[5] = {0}; // tigress can report up to 5 valid timestamp words
   int          x       = 0;

   while((*(data + x) & 0xf0000000) == 0xa0000000) {
      time[x] = *(data + x);
      x += 1;
      if(x == 5) {
         break;
      }
   }

   switch(x) {
   case 1: // bad.
      break;
   case 2:                     // minimum number of good a's
      if(time[0] != time[1]) { // tig64's only have two, both second hex's are 0s. also some times tig10s.
         timestamplow  = time[0] & 0x00ffffff;
         timestamphigh = time[1] & 0x00ffffff;
         // return true;
      }
      break;
   case 3:
      if(time[0] == time[1] && time[0] != time[2]) {
         if(((time[0] & 0x0f000000) != 0x00000000) && ((time[2] & 0x0f000000) != 0x01000000)) {
            break;
         }
         timestamplow  = time[0] & 0x00ffffff;
         timestamphigh = time[2] & 0x00ffffff;
      } else if(time[0] != time[1] && time[1] == time[2]) {
         if(((time[0] & 0x0f000000) != 0x00000000) && ((time[1] & 0x0f000000) != 0x01000000)) {
            break;
         }
         timestamplow  = time[0] & 0x00ffffff;
         timestamphigh = time[1] & 0x00ffffff;
      } else { // assume the third if the counter.
         // if( ((time[0]&0x0f000000)!=0x00000000) && ((time[1]&0x0f000000)!=0x01000000) )
         //   break;
         timestamplow  = time[0] & 0x00ffffff;
         timestamphigh = time[1] & 0x00ffffff;
      }
      break;
   // return true;
   case 4:
   case 5:
      if(time[0] == time[1] && time[2] == time[3]) {
         if(((time[0] & 0x0f000000) != 0x00000000) && ((time[2] & 0x0f000000) != 0x01000000)) {
            break;
         }
         timestamplow  = time[0] & 0x00ffffff;
         timestamphigh = time[1] & 0x00ffffff;
      } else {
         if(((time[0] & 0x0f000000) != 0x00000000) && ((time[1] & 0x0f000000) != 0x01000000)) {
            break;
         }
         timestamplow  = time[0] & 0x00ffffff;
         timestamphigh = time[1] & 0x00ffffff;
      }
      break;
      // return true;
   };
   if(timestamplow > -1 && timestamphigh > -1) {
      currentFrag->SetTimeStamp((timestamphigh<<24) + timestamplow);
      return true;
   }

   return false;
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

int TDataParser::GriffinDataToFragment(uint32_t* data, int size, EBank bank, unsigned int midasSerialNumber,
                                       time_t midasTime)
{
   /// Converts a Griffin flavoured MIDAS file into a TFragment and returns the number of words processed (or the
   /// negative index of the word it failed on)
   std::shared_ptr<TFragment> eventFrag = std::make_shared<TFragment>();
   // no need to delete eventFrag, it's a shared_ptr and gets deleted when it goes out of scope
   fFragmentHasWaveform = false;
   fState               = EDataParserState::kGood;
   int failedWord = -1; // Variable stores which word we failed on (if we fail). This is somewhat duplicate information
                        // to fState, but makes things easier to track.
   bool multipleErrors = false; // Variable to store if multiple errors occured parsing one fragment

   if(fOptions == nullptr) {
		fOptions = TGRSIOptions::Get();
	}

   int           x   = 0;
   if(!SetGRIFHeader(data[x++], eventFrag, bank)) {
      printf(DYELLOW "data[0] = 0x%08x" RESET_COLOR "\n", data[0]);
		// we failed to get a good header, so we don't know which detector type this fragment would've belonged to
      TParsingDiagnostics::Get()->BadFragment(-1); 
      // this is the first word, so no need to check is the state/failed word has been set before
      fState     = EDataParserState::kBadHeader;
      failedWord = 0;
   }

   eventFrag->SetMidasTimeStamp(midasTime);
   eventFrag->SetMidasId(midasSerialNumber);

   // Changed on 11 Aug 2015 by RD to include PPG events. If the event has ModuleType 4 and address 0xffff, it is a PPG
   // event.
   if((eventFrag->GetModuleType() == 1 || eventFrag->GetModuleType() == 4) && eventFrag->GetAddress() == 0xffff) {
      return GriffinDataToPPGEvent(data, size, midasSerialNumber, midasTime);
   }
   // If the event has detector type 15 (0xf) it's a scaler event.
   if(eventFrag->GetDetectorType() == 0xf) {
      // a scaler event (trigger or deadtime) has 8 words (including header and trailer), make sure we have at least
      // that much left
      if(size < 8) {
         if(fState == EDataParserState::kGood) {
            fState     = EDataParserState::kMissingWords;
            failedWord = x;
         } else {
            multipleErrors = true;
         }
         throw TDataParserException(fState, failedWord, multipleErrors);
      }
      return GriffinDataToScalerEvent(data, eventFrag->GetAddress());
   }

   // The Network packet number is for debugging and is not always written to the midas file.
   if(SetGRIFNetworkPacket(data[x], eventFrag)) {
      ++x;
   }

   // The master Filter Pattern is in an unstable state right now and is not
   // always written to the midas file
   if(SetGRIFMasterFilterPattern(data[x], eventFrag, bank)) {
      ++x;
   }

   // We can get multiple filter ids (one fragment can pass multiple filter conditions)
   // so we have to loop until we don't find one
   while(SetGRIFMasterFilterId(data[x], eventFrag)) {
      ++x;
   }

   // The channel trigger ID is in an unstable state right now and is not
   // always written to the midas file
   if(!SetGRIFChannelTriggerId(data[x++], eventFrag)) {
      TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
      if(fState == EDataParserState::kGood) {
         fState     = EDataParserState::kBadTriggerId;
         failedWord = x - 1; // -1 compensates the incrementation in the if-statement
      } else {
         multipleErrors = true;
      }
   }

   // The Network packet number is for debugging and is not always written to
   // the midas file.
   if(SetGRIFNetworkPacket(data[x], eventFrag)) {
      x++;
   }

   if(!SetGRIFTimeStampLow(data[x++], eventFrag)) {
      TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
      if(fState == EDataParserState::kGood) {
         fState     = EDataParserState::kBadLowTS;
         failedWord = x - 1; // -1 compensates the incrementation in the if-statement
      } else {
         multipleErrors = true;
      }
   }

   if(!SetGRIFDeadTime(data[x++], eventFrag)) {
      TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
      if(fState == EDataParserState::kGood) {
         fState     = EDataParserState::kBadHighTS;
         failedWord = x - 1; // -1 compensates the incrementation in the if-statement
      } else {
         multipleErrors = true;
      }
   }

   std::vector<Int_t>   tmpCharge;
   std::vector<Short_t> tmpIntLength;
   std::vector<Int_t>   tmpCfd;

   for(; x < size; x++) {
      uint32_t dword  = data[x];
      uint32_t packet = dword >> 28;
      uint32_t value  = dword & 0x0fffffff;

      switch(packet) {
      case 0x8: // The 8 packet type is for event headers
         // if this happens, we have "accidentally" found another event.
         // currently the GRIF-C only sets the master/slave port of the address for the first header (of the corrupt
         // event)
         // so we want to ignore this corrupt event and the next event which has a wrong address
         TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
         if(fState == EDataParserState::kGood) {
            fState     = EDataParserState::kSecondHeader;
            failedWord = x + 1; //+1 to ensure we don't read this header as start of a good event
         } else {
            multipleErrors = true;
         }
         Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
         throw TDataParserException(fState, failedWord, multipleErrors);
         break;
      case 0xc: // The c packet type is for waveforms
         if(!fNoWaveforms) {
            SetGRIFWaveForm(value, eventFrag);
         }
         break;
      case 0xd:
         SetGRIFNetworkPacket(dword, eventFrag); // The network packet placement is not yet stable.
         break;
      case 0xe:
         // changed on 21 Apr 2015 by JKS, when signal processing code from Chris changed the trailer.
         // change should be backward-compatible
         if((value & 0x3fff) == (eventFrag->GetChannelId() & 0x3fff)) {
            if(!fOptions->SuppressErrors() && (eventFrag->GetModuleType() == 2) && (bank < EBank::kGRF3)) {
               // check whether the nios finished and if so whether it finished with an error
               if(((value >> 14) & 0x1) == 0x1) {
                  if(((value >> 16) & 0xff) != 0) {
                     printf(BLUE "0x%04x: NIOS code finished with error 0x%02x" RESET_COLOR "\n",
                            eventFrag->GetAddress(), (value >> 16) & 0xff);
                  }
               }
            }

            if((eventFrag->GetModuleType() == 1) || (bank > EBank::kGRF2)) { // 4Gs have this only for banks newer than GRF2
               eventFrag->SetAcceptedChannelId((value >> 14) & 0x3fff);
            } else {
               eventFrag->SetAcceptedChannelId(0);
            }

            // check the number of words the header said we should have with the number of words we've read
            // the number of words is only set for bank >= GRF3
            // if the fragment has a waveform, we can't compare the number of words
            // the headers number of words includes the header (word 0) itself, so we need to compare to x plus one
            if(eventFrag->GetNumberOfWords() > 0 && !eventFrag->HasWave() && eventFrag->GetNumberOfWords() != x + fOptions->WordOffset()) {
               if(fState == EDataParserState::kGood) {
                  fState     = EDataParserState::kWrongNofWords;
                  failedWord = x;
               } else {
                  multipleErrors = true;
               }
               Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
               throw TDataParserException(fState, failedWord, multipleErrors);
            }

            // the way we insert the fragment(s) depends on the module type and bank:
            // 1 - for module type 1 & bank GRF4, we can't insert the fragments yet, we need to put them in a separate queue
            // 2 - for module type 2 (4G, all banks) and module type 1 & bank GRF3 we set the single charge, cfd, and
            //     IntLength, and insert the fragment
            // 3 - for module type 1 & banks GRF1/GRF2 we loop over the charge, cfd, and IntLengths, and insert the
            //     (multiple) fragment(s)
            // the last two cases can be treated the same since the second case will just have a single length charge,
            // cfd, and IntLengths

            // so we only need to check for the first case
            if(eventFrag->GetModuleType() == 1 && bank == EBank::kGRF4) {
               if(tmpCfd.size() != 1) {
                  if(fRecordDiag) {
                     TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
                  }
                  if(fState == EDataParserState::kGood) {
                     fState     = EDataParserState::kNotSingleCfd;
                     failedWord = x;
                  } else {
                     multipleErrors = true;
                  }
                  Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
                  throw TDataParserException(fState, failedWord, multipleErrors);
               }
               eventFrag->SetCfd(tmpCfd[0]);
               if(fRecordDiag) {
                  TParsingDiagnostics::Get()->GoodFragment(eventFrag);
               }
               fFragmentMap.Add(eventFrag, tmpCharge, tmpIntLength);
               return x;
            }
            if(tmpCharge.size() != tmpIntLength.size() || tmpCharge.size() != tmpCfd.size()) {
               if(fRecordDiag) {
                  TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
               }
               if(fState == EDataParserState::kGood) {
                  fState     = EDataParserState::kSizeMismatch;
                  failedWord = x;
               } else {
                  multipleErrors = true;
               }
               Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
               throw TDataParserException(fState, failedWord, multipleErrors);
            }
            for(size_t h = 0; h < tmpCharge.size(); ++h) {
               eventFrag->SetCharge(tmpCharge[h]);
               eventFrag->SetKValue(tmpIntLength[h]);
               eventFrag->SetCfd(tmpCfd[h]);
               if(fRecordDiag) {
                  TParsingDiagnostics::Get()->GoodFragment(eventFrag);
               }
               if(fState == EDataParserState::kGood) {
                  if(fOptions->ReconstructTimeStamp()) {
                     fLastTimeStampMap[eventFrag->GetAddress()] = eventFrag->GetTimeStamp();
                  }
                  Push(fGoodOutputQueues, std::make_shared<TFragment>(*eventFrag));
               } else {
                  if(fOptions->ReconstructTimeStamp() && fState == EDataParserState::kBadHighTS && !multipleErrors) {
                     // std::cout<<"reconstructing timestamp from 0x"<<std::hex<<eventFrag->GetTimeStamp()<<" using
                     // 0x"<<fLastTimeStampMap[eventFrag->GetAddress()];
                     // reconstruct the high bits of the timestamp from the high bits of the last time stamp of the
                     // same address
                     if((eventFrag->GetTimeStamp() & 0x0fffffff) <
                        (fLastTimeStampMap[eventFrag->GetAddress()] & 0x0fffffff)) {
                        // we had a wrap-around of the low time stamp, so we need to set the high bits to the old
                        // high bits plus one
                        eventFrag->AppendTimeStamp(((fLastTimeStampMap[eventFrag->GetAddress()] >> 28) + 1)<<28);
                     } else {
                        eventFrag->AppendTimeStamp(fLastTimeStampMap[eventFrag->GetAddress()] & 0x3fff0000000);
                     }
                     // std::cout<<" => 0x"<<eventFrag->GetTimeStamp()<<std::dec<<std::endl;
                     Push(fGoodOutputQueues, std::make_shared<TFragment>(*eventFrag));
                  } else {
                     // std::cout<<"Can't reconstruct time stamp, "<<fOptions->ReconstructTimeStamp()<<",
                     // state "<<fState<<" = "<<EDataParserState::kBadHighTS<<", "<<multipleErrors<<std::endl;
                     Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
                  }
               }
            }
            return x;
         } else {
            if(fRecordDiag) {
               TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
            }
            if(fState == EDataParserState::kGood) {
               fState     = EDataParserState::kBadFooter;
               failedWord = x;
            } else {
               multipleErrors = true;
            }
            Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
            throw TDataParserException(fState, failedWord, multipleErrors);
         }
         break;
      case 0xf:
         switch(bank) {
			case EBank::kGRF1: // format from before May 2015 experiments
            TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
            if(fState == EDataParserState::kGood) {
               fState     = EDataParserState::kFault;
               failedWord = x;
            } else {
               multipleErrors = true;
            }
            Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
            throw TDataParserException(fState, failedWord, multipleErrors);
            break;
			case EBank::kGRF2: // from May 2015 to the end of 2015 0xf denoted a psd-word from a 4G
            if(x + 1 < size) {
               SetGRIFCc(value, eventFrag);
               ++x;
               dword = data[x];
               SetGRIFPsd(dword, eventFrag);
            } else {
               TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
               if(fState == EDataParserState::kGood) {
                  fState     = EDataParserState::kMissingPsd;
                  failedWord = x;
               } else {
                  multipleErrors = true;
               }
               Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
               throw TDataParserException(fState, failedWord, multipleErrors);
            }
            break;
			case EBank::kGRF3: // from 2016 on we're back to reserving 0xf for faults
			case EBank::kGRF4:
            TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
            if(fState == EDataParserState::kGood) {
               fState     = EDataParserState::kFault;
               failedWord = x;
            } else {
               multipleErrors = true;
            }
            Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
            throw TDataParserException(fState, failedWord, multipleErrors);
            break;
         default: printf("This bank not yet defined.\n"); break;
         }
         break;

      default:
         // these are charge/cfd words which are different depending on module type, and bank number/detector type
         switch(eventFrag->GetModuleType()) {
         case 1:
            switch(bank) { // the GRIF-16 data format depends on the bank number
				case EBank::kGRF1:    // bank's 1&2 have n*2 words with (5 high bits IntLength, 26 Charge)(5 low bits IntLength, 26
                           // Cfd)
				case EBank::kGRF2:
               // read this pair of charge/cfd words, check if the next word is also a charge/cfd word
               if(((data[x] & 0x80000000) == 0x0) && x + 1 < size && (data[x + 1] & 0x80000000) == 0x0) {
                  Short_t tmp = (data[x] & 0x7c000000) >> 21; // 21 = 26 minus space for 5 low bits
                  tmpCharge.push_back((data[x] & 0x03ffffff) | (((data[x] & 0x02000000) == 0x02000000)
                                                                   ? 0xfc000000
                                                                   : 0x0)); // extend the sign bit of 26bit charge word
                  ++x;
                  tmpIntLength.push_back(tmp | ((data[x] & 0x7c000000) >> 26));
                  tmpCfd.push_back(data[x] & 0x03ffffff);
               } else {
                  TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
                  if(fState == EDataParserState::kGood) {
                     fState     = EDataParserState::kMissingCfd;
                     failedWord = x;
                  } else {
                     multipleErrors = true;
                  }
                  Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
                  throw TDataParserException(fState, failedWord, multipleErrors);
               }
               break;
				case EBank::kGRF3: // bank 3 has 2 words with (5 high bits IntLength, 26 Charge)(9 low bits IntLength, 22 Cfd)
               if(x + 1 < size &&
                  (data[x + 1] & 0x80000000) == 0x0) {        // check if the next word is also a charge/cfd word
                  Short_t tmp = (data[x] & 0x7c000000) >> 17; // 17 = 26 minus space for 9 low bits
                  tmpCharge.push_back((data[x] & 0x03ffffff) | (((data[x] & 0x02000000) == 0x02000000)
                                                                   ? 0xfc000000
                                                                   : 0x0)); // extend the sign bit of 26bit charge word
                  ++x;
                  tmpIntLength.push_back(tmp | ((data[x] & 0x7fc00000) >> 22));
                  tmpCfd.push_back(data[x] & 0x003fffff);
                  break;
               } else {
                  TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
                  if(fState == EDataParserState::kGood) {
                     fState     = EDataParserState::kMissingCfd;
                     failedWord = x;
                  } else {
                     multipleErrors = true;
                  }
                  Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
                  throw TDataParserException(fState, failedWord, multipleErrors);
               }
               break;
				case EBank::kGRF4: // bank 4 can have more than one integration (up to four), but these have to be combined with
                        // other fragments/hits!
               // we always have 2 words with (5 high bits IntLength, 26 Charge)(9 low bits IntLength, 22 Cfd)
               if((data[x] & 0x80000000) == 0x0 && x + 1 < size &&
                  (data[x + 1] & 0x80000000) == 0x0) { // check if the next word is also a charge/cfd word
                  Short_t tmp = ((data[x] & 0x7c000000) >> 17) |
                                (((data[x] & 0x40000000) == 0x40000000) ? 0xc000 : 0x0); // 17 = 26 minus space for 9
                                                                                         // low bits; signed, so we
                                                                                         // extend the sign bit from 14
                                                                                         // (31) to 16 bits
                  if(false && (data[x] & 0x02000000) == 0x02000000) {                             // overflow bit was set - disabled VB
                     tmpCharge.push_back(std::numeric_limits<int>::max());
                  } else {
                     tmpCharge.push_back((data[x] & 0x01ffffff) |
                                         (((data[x] & 0x01000000) == 0x01000000)
                                             ? 0xfe000000
                                             : 0x0)); // extend the sign bit of 25bit charge word
                  }
                  ++x;
                  tmpIntLength.push_back(tmp | ((data[x] & 0x7fc00000) >> 22));
                  tmpCfd.push_back(data[x] & 0x003fffff);
                  // check if we have two more words (X & XI) with (8 num hits, 2 reserved, 14 IntLength2)(31 Charge2);
                  // x has already been incremented once!
                  if(x + 2 < size && (data[x + 1] & 0x80000000) == 0x0 && (data[x + 2] & 0x80000000) == 0x0) {
                     ++x;
                     tmpIntLength.push_back((data[x] & 0x3fff) | (((data[x] & 0x2000) == 0x2000) ? 0xc000 : 0x0));
                     // eventFrag->SetNumberOfPileups((data[x] >> 16) & 0xff);
                     ++x;
                     if((data[x] & 0x02000000) == 0x02000000) { // overflow bit was set
                        tmpCharge.push_back(std::numeric_limits<int>::max());
                     } else {
                        tmpCharge.push_back((data[x] & 0x01ffffff) |
                                            (((data[x] & 0x01000000) == 0x01000000)
                                                ? 0xfe000000
                                                : 0x0)); // extend the sign bit of 25bit charge word
                     }
                     // check if we have two more words (XI & XIII) with (14 IntLength4, 2 reserved, 14 IntLength3)(31
                     // Charge3); x has already been incremented thrice!
                     if(x + 2 < size && (data[x + 1] & 0x80000000) == 0x0 && (data[x + 2] & 0x80000000) == 0x0) {
                        ++x;
                        tmpIntLength.push_back((data[x] & 0x3fff) | (((data[x] & 0x2000) == 0x2000) ? 0xc000 : 0x0));
                        tmpIntLength.push_back((data[x] >> 16) |
                                               (((data[x] & 0x20000000) == 0x20000000) ? 0xc000 : 0x0));
                        ++x;
                        if((data[x] & 0x02000000) == 0x02000000) { // overflow bit was set
                           tmpCharge.push_back(std::numeric_limits<int>::max());
                        } else {
                           tmpCharge.push_back((data[x] & 0x01ffffff) |
                                               (((data[x] & 0x01000000) == 0x01000000)
                                                   ? 0xfe000000
                                                   : 0x0)); // extend the sign bit of 25bit charge word
                        }
                        // check if we have one final word with (31 Charge4), otherwise remove the last integration
                        // length (IntLength4)
                        if(x + 1 < size && (data[x + 1] & 0x80000000) == 0x0) {
                           ++x;
                           if((data[x] & 0x02000000) == 0x02000000) { // overflow bit was set
                              tmpCharge.push_back(std::numeric_limits<int>::max());
                           } else {
                              tmpCharge.push_back((data[x] & 0x01ffffff) |
                                                  (((data[x] & 0x01000000) == 0x01000000)
                                                      ? 0xfe000000
                                                      : 0x0)); // extend the sign bit of 25bit charge word
                           }
                        } else {
                           tmpIntLength.pop_back();
                        }
                     } else if((data[x + 1] & 0x80000000) == 0x0) { // 5 words
                        ++x;
                     }
                  } else if((data[x + 1] & 0x80000000) == 0x0) { // 3 words
                     ++x;
                  }
               } else {
                  // these types of corrupt events quite often end without a trailer which leads to the header of the
                  // next event missing the master/slave part of the address
                  // so we look for the next trailer and stop there
                  while(x < size && (data[x] & 0xf0000000) != 0xe0000000) {
                     ++x;
                  }
                  TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
                  if(fState == EDataParserState::kGood) {
                     fState     = EDataParserState::kMissingCharge;
                     failedWord = x;
                  } else {
                     multipleErrors = true;
                  }
                  Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
                  throw TDataParserException(fState, failedWord, multipleErrors);
               }
               break;
            default:
               if(!fOptions->SuppressErrors()) {
                  printf(DRED "Error, bank type %d not implemented yet" RESET_COLOR "\n", static_cast<std::underlying_type<EBank>::type>(bank));
               }
               TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
               if(fState == EDataParserState::kGood) {
                  fState     = EDataParserState::kBadBank;
                  failedWord = x;
               } else {
                  multipleErrors = true;
               }
               Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
               throw TDataParserException(fState, failedWord, multipleErrors);
               break;
            }
            break;
         case 2:
            // the 4G data format depends on the detector type, but the first two words are always the same
            if(x + 1 < size && (data[x + 1] & 0x80000000) == 0x0) { // check if the next word is also a charge/cfd word
               Short_t tmp = (data[x] & 0x7c000000) >> 21;          // 21 = 26 minus space for 5 low bits
               tmpCharge.push_back((data[x] & 0x03ffffff) | (((data[x] & 0x02000000) == 0x02000000)
                                                                ? 0xfc000000
                                                                : 0x0)); // extend the sign bit of 26bit charge word
               ++x;
               tmpIntLength.push_back(tmp | ((data[x] & 0x7c000000) >> 26));
               tmpCfd.push_back(data[x] & 0x03ffffff);
            } else {
               TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
               if(fState == EDataParserState::kGood) {
                  fState     = EDataParserState::kMissingCfd;
                  failedWord = x;
               } else {
                  multipleErrors = true;
               }
               Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
               throw TDataParserException(fState, failedWord, multipleErrors);
            }
            // for descant types (6,10,11) there are two more words for banks > GRF2 (bank GRF2 used 0xf packet and bank
            // GRF1 never had descant)
            if(bank > EBank::kGRF2 && (eventFrag->GetDetectorType() == 6 || eventFrag->GetDetectorType() == 10 ||
                                eventFrag->GetDetectorType() == 11)) {
               ++x;
               if(x + 1 < size && (data[x + 1] & 0x80000000) == 0x0) {
                  SetGRIFCc(value, eventFrag);
                  ++x;
                  dword = data[x];
                  SetGRIFPsd(dword, eventFrag);
               } else {
                  TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
                  if(fState == EDataParserState::kGood) {
                     fState     = EDataParserState::kMissingPsd;
                     failedWord = x;
                  } else {
                     multipleErrors = true;
                  }
                  Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
                  throw TDataParserException(fState, failedWord, multipleErrors);
               }
            }
            break;
         default:
            if(!fOptions->SuppressErrors()) {
               printf(DRED "Error, module type %d not implemented yet" RESET_COLOR "\n", eventFrag->GetModuleType());
            }
            TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
            if(fState == EDataParserState::kGood) {
               fState     = EDataParserState::kBadModuleType;
               failedWord = x;
            } else {
               multipleErrors = true;
            }
            Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
            throw TDataParserException(fState, failedWord, multipleErrors);
         } // switch(eventFrag->GetModuleType())
         break;
      } // switch(packet)
   }    // for(;x<size;x++)

   TParsingDiagnostics::Get()->BadFragment(eventFrag->GetDetectorType());
   if(fState == EDataParserState::kGood) {
      fState     = EDataParserState::kEndOfData;
      failedWord = x;
   } else {
      multipleErrors = true;
   }
   Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, data, size, failedWord, multipleErrors));
   throw TDataParserException(fState, failedWord, multipleErrors);
   return -x;
}

bool TDataParser::SetGRIFHeader(uint32_t value, const std::shared_ptr<TFragment>& frag, EBank bank)
{
   if((value & 0xf0000000) != 0x80000000) {
      return false;
   }
   switch(bank) {
	case EBank::kGRF1: // header format from before May 2015 experiments
      // Sets:
      //     The number of filters
      //     The Data Type
      //     Number of Pileups
      //     Channel Address
      //     Detector Type
      // frag->SetNumberOfFilters((value &0x0f000000)>> 24);
      frag->SetModuleType((value & 0x00e00000) >> 21);
      frag->SetNumberOfPileups((value & 0x001c0000) >> 18);
      frag->SetAddress((value & 0x0003fff0) >> 4);
      frag->SetDetectorType((value & 0x0000000f));

      // if(frag-DetectorType==2)
      //    frag->ChannelAddress += 0x8000;
      break;
	case EBank::kGRF2:
      // Sets:
      //     The number of filters
      //     The Data Type
      //     Number of Pileups
      //     Channel Address
      //     Detector Type
      frag->SetNumberOfPileups((value & 0x0c000000) >> 26);
      frag->SetModuleType((value & 0x03800000) >> 23);
      // frag->SetNumberOfFilters((value &0x00700000)>> 20);
      frag->SetAddress((value & 0x000ffff0) >> 4);
      frag->SetDetectorType((value & 0x0000000f));

      break;
	case EBank::kGRF3:
	case EBank::kGRF4:
      frag->SetModuleType((value & 0x0e000000) >> 25);
      frag->SetNumberOfWords((value & 0x01f00000) >> 20);
      frag->SetAddress((value & 0x000ffff0) >> 4);
      frag->SetDetectorType((value & 0x0000000f));

      break;
   default: printf("This bank is not yet defined.\n"); return false;
   }

   return true;
}

bool TDataParser::SetGRIFMasterFilterPattern(uint32_t value, const std::shared_ptr<TFragment>& frag, EBank bank)
{
   /// Sets the Griffin Master Filter Pattern
   if((value & 0xc0000000) != 0x00000000) {
      return false;
   }
   switch(bank) {
	case EBank::kGRF1:
	case EBank::kGRF2:
      frag->SetTriggerBitPattern(value >> 16);
      // frag->SetPPGWord(value & 0x0000ffff);//This is due to new GRIFFIN data format
      break;
	case EBank::kGRF3:
	case EBank::kGRF4:
      frag->SetTriggerBitPattern(value >> 16);
      frag->SetNumberOfPileups(value & 0x1f);
      fFragmentHasWaveform = ((value & 0x8000) == 0x8000);
      break;
   default: return false;
   }
   return true;
}

bool TDataParser::SetGRIFMasterFilterId(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
   /// Sets the Griffin master filter ID and PPG
   if((value & 0x80000000) != 0x00000000) {
      return false;
   }

   frag->SetTriggerId(value & 0x7FFFFFFF); // REAL
   return true;
}

bool TDataParser::SetGRIFChannelTriggerId(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
   /// Sets the Griffin Channel Trigger ID
   if((value & 0xf0000000) != 0x90000000) {
      return false;
   }
   frag->SetChannelId(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetGRIFNetworkPacket(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
   /// Ignores the network packet number (for now)
   if((value & 0xf0000000) != 0xd0000000) {
      return false;
   }
   if((value & 0x0f000000) == 0x0f000000 && frag->GetNetworkPacketNumber() > 0) {
      if(frag->GetZc() != 0) {
         return false;
      }
      // descant zero crossing time.
      frag->SetZc(value & 0x00ffffff);
   } else {
      frag->SetNetworkPacketNumber(value & 0x0fffffff);
   }
   return true;
}

bool TDataParser::SetGRIFTimeStampLow(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
   /// Sets the lower 28 bits of the griffin time stamp
   if((value & 0xf0000000) != 0xa0000000) {
      return false;
   }
   // we always get the lower 28 bits first
   frag->SetTimeStamp(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetGRIFWaveForm(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
   /// Sets the Griffin waveform if record_waveform is set to true
   if(frag->GetWaveform()->size() > (100000)) {
      printf("number of wave samples found is to great\n");
      return false;
   }

   // to go from a 14-bit signed number to a 16-bit signed number, we simply set the two highest bits if the sign bit is
   // set
   frag->AddWaveformSample((value & 0x2000) != 0u ? static_cast<Short_t>((value & 0x3fff) | 0xc000)
                                                  : static_cast<Short_t>(value & 0x3fff));
   value = value >> 14;
   frag->AddWaveformSample((value & 0x2000) != 0u ? static_cast<Short_t>((value & 0x3fff) | 0xc000)
                                                  : static_cast<Short_t>(value & 0x3fff));

   return true;
}

bool TDataParser::SetGRIFDeadTime(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
   /// Sets the Griffin deadtime and the upper 14 bits of the timestamp
   if((value & 0xf0000000) != 0xb0000000) {
      return false;
   }
   frag->SetDeadTime((value & 0x0fffc000) >> 14);
   frag->AppendTimeStamp(static_cast<Long64_t>(value & 0x00003fff)<<28);
   return true;
}

bool TDataParser::SetGRIFCc(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
   /// set the short integration and the lower 9 bits of the long integration
   if(frag->GetCcShort() != 0 || frag->GetCcLong() != 0) {
      return false;
   }
   frag->SetCcShort(value & 0x7ffff);
   frag->SetCcLong(value >> 19);
   return true;
}

bool TDataParser::SetGRIFPsd(uint32_t value, const std::shared_ptr<TFragment>& frag)
{
   /// set the zero crossing and the higher 10 bits of the long integration
   if(frag->GetZc() != 0) { // low bits of ccLong have already been set
      return false;
   }
   frag->SetZc(value & 0x003fffff);
   frag->SetCcLong(frag->GetCcLong() | ((value & 0x7fe00000) >> 12)); // 21 bits from zero crossing minus 9 lower bits
   return true;
}

int TDataParser::GriffinDataToPPGEvent(uint32_t* data, int size, unsigned int, time_t)
{
   auto* ppgEvent = new TPPGData;
   int   x        = 1; // We have already read the header so we can skip the 0th word.

   // The Network packet number is for debugging and is not always written to
   // the midas file.
   if(SetPPGNetworkPacket(data[x], ppgEvent)) { // The network packet placement is not yet stable.
      ++x;
   }
   if(SetNewPPGPattern(data[x], ppgEvent)) {
      ++x;
   }

   for(; x < size; x++) {
      uint32_t dword  = data[x];
      uint32_t packet = dword & 0xf0000000;
      uint32_t value  = dword & 0x0fffffff;

      switch(packet) {
      case 0x80000000: // The 8 packet type is for event headers
         // if this happens, we have "accidentally" found another event.
         return -x;
      case 0x90000000: // The b packet type contains the dead-time word
         SetOldPPGPattern(value, ppgEvent);
         break;
      case 0xd0000000:
         SetPPGNetworkPacket(dword, ppgEvent); // The network packet placement is not yet stable.
         break;
      case 0xa0000000: SetPPGLowTimeStamp(value, ppgEvent); break;
      case 0xb0000000: SetPPGHighTimeStamp(value, ppgEvent); break;
      case 0xe0000000:
         // if((value & 0xFFFF) == (ppgEvent->GetNewPPG())){
         TPPG::Get()->AddData(ppgEvent);
         TParsingDiagnostics::Get()->GoodFragment(-2); // use detector type -2 for PPG
         return x;
         //} else  {
         //	TParsingDiagnostics::Get()->BadFragment(-2); //use detector type -2 for PPG
         //	return -x;
         //}
         break;
      };
   }
   delete ppgEvent;
   // No trailer found
   TParsingDiagnostics::Get()->BadFragment(-2); // use detector type -2 for PPG
   return -x;
}

bool TDataParser::SetNewPPGPattern(uint32_t value, TPPGData* ppgevent)
{
   if((value & 0xf0000000) != 0x00000000) {
      return false;
   }
   ppgevent->SetNewPPG(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetOldPPGPattern(uint32_t value, TPPGData* ppgevent)
{
   ppgevent->SetOldPPG(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetPPGNetworkPacket(uint32_t value, TPPGData* ppgevent)
{
   // Ignores the network packet number (for now)
   //   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);
   if((value & 0xf0000000) != 0xd0000000) {
      return false;
   }
   ppgevent->SetNetworkPacketId(value & 0x00ffffff);
   //   printf("value = 0x%08x    |   frag->NetworkPacketNumber = %i   \n",value,frag->NetworkPacketNumber);

   return true;
}

bool TDataParser::SetPPGLowTimeStamp(uint32_t value, TPPGData* ppgevent)
{
   ppgevent->SetLowTimeStamp(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetPPGHighTimeStamp(uint32_t value, TPPGData* ppgevent)
{
   ppgevent->SetHighTimeStamp(value & 0x0fffffff);
   return true;
}

int TDataParser::GriffinDataToScalerEvent(uint32_t* data, int address)
{
   auto* scalerEvent = new TScalerData;
   scalerEvent->SetAddress(address);
   int x          = 1; // We have already read the header so we can skip the 0th word.
   int failedWord = -1;

   // we expect a word starting with 0xd containing the network packet id
   // this is a different format than the others because it will not always be in the scaler word
   if(SetScalerNetworkPacket(data[x], scalerEvent)) {
      x++;
   }

   // we expect a word starting with 0xa containing the 28 lowest bits of the timestamp
   if(!SetScalerLowTimeStamp(data[x++], scalerEvent)) {
      TParsingDiagnostics::Get()->BadFragment(-3); // use detector type -3 for scaler data
      fState     = EDataParserState::kBadScalerLowTS;
      failedWord = x;
      throw TDataParserException(fState, failedWord, false);
   }
   // followed by four scaler words (32 bits each)
   for(int i = 0; i < 4; ++i) {
      if(!SetScalerValue(i, data[x++], scalerEvent)) {
         TParsingDiagnostics::Get()->BadFragment(-3); // use detector type -3 for scaler data
         fState     = EDataParserState::kBadScalerValue;
         failedWord = x;
         throw TDataParserException(fState, failedWord, false);
      }
   }
   // and finally the trailer word with the highest 24 bits of the timestamp
   int scalerType = 0;
   if(!SetScalerHighTimeStamp(data[x++], scalerEvent, scalerType)) {
      TParsingDiagnostics::Get()->BadFragment(-3); // use detector type -3 for scaler data
      fState     = EDataParserState::kBadScalerHighTS;
      failedWord = x;
      throw TDataParserException(fState, failedWord, false);
   }

   if(scalerType == 0) { // deadtime scaler
      TDeadtimeScalerQueue::Get()->Add(scalerEvent);
   } else if(scalerType == 1) { // rate scaler
      // the rate scaler has only one real value, the rate
      scalerEvent->ResizeScaler();
      TRateScalerQueue::Get()->Add(scalerEvent);
   } else {                                        // unknown scaler type
      TParsingDiagnostics::Get()->BadFragment(-3); // use detector type -3 for scaler data
      fState     = EDataParserState::kBadScalerType;
      failedWord = x;
      throw TDataParserException(fState, failedWord, false);
   }

   TParsingDiagnostics::Get()->GoodFragment(-3); // use detector type -3 for scaler data
   return x;
}

bool TDataParser::SetScalerNetworkPacket(uint32_t value, TScalerData* scalerEvent)
{
   if((value >> 28) != 0xd) {
      return false;
   }
   scalerEvent->SetNetworkPacketId(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetScalerLowTimeStamp(uint32_t value, TScalerData* scalerEvent)
{
   if((value >> 28) != 0xa) {
      return false;
   }
   scalerEvent->SetLowTimeStamp(value & 0x0fffffff);
   return true;
}

bool TDataParser::SetScalerHighTimeStamp(uint32_t value, TScalerData* scalerEvent, int& type)
{
   if((value >> 28) != 0xe || (value & 0xff) != (scalerEvent->GetLowTimeStamp() >> 20)) {
      return false;
   }
   scalerEvent->SetHighTimeStamp((value >> 8) & 0x0000ffff);
   type = (value >> 24 & 0xf);
   return true;
}

bool TDataParser::SetScalerValue(int index, uint32_t value, TScalerData* scalerEvent)
{
   scalerEvent->SetScaler(index, value);
   return true;
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

int TDataParser::EightPIDataToFragment(uint32_t stream, uint32_t* data, int size, unsigned int midasSerialNumber,
                                       time_t midasTime)
{

   int NumFragsFound = 0;
   // TFragment* eventFrag = new TFragment();
   // eventFrag->MidasTimeStamp = midasTime;
   // eventFrag->MidasId = midasSerialNumber;

   for(int i = 0; i < size; i++) {
      if((data[i] == 0xff06) || (data[i] == 0xff16)) { // found a fifo...
         i++;                                          // lets get the next int.
         int fifowords = data[i] & 0x00001fff;
         i++;
         int fifoserial = data[i] & 0x000000ff;
         i++;
         if(stream == 0) {
            printf(DBLUE "%i  GOOD FIFO,      fifowords = %i      fifoserial = %i   " RESET_COLOR "\n", stream,
                   fifowords, fifoserial);
         } else if(stream == 1) {
            printf(DGREEN "%i  GOOD FIFO,      fifowords = %i      fifoserial = %i   " RESET_COLOR "\n", stream,
                   fifowords, fifoserial);
         } else if(stream == 2) {
            printf(DRED "%i  GOOD FIFO,      fifowords = %i      fifoserial = %i   " RESET_COLOR "\n", stream,
                   fifowords, fifoserial);
         } else if(stream == 3) {
            printf(DYELLOW "%i  GOOD FIFO,      fifowords = %i      fifoserial = %i   " RESET_COLOR "\n", stream,
                   fifowords, fifoserial);
         }
         // at this point am looking at the actual fera bank.
         //   ->convert to an arry of shorts.
         //   ->iterate
         bool            extrafifo = false;
         unsigned short* words     = reinterpret_cast<unsigned short*>(data + i);
         if(fifowords % 2 != 0) {
            fifowords++;
            extrafifo = true;
         }
         printf(DMAGENTA "i = %i    2*size =  %i   fifowords  = %i  " RESET_COLOR "\n", i, 2 * size, fifowords);

         for(int j = 0; j < fifowords; j += 2) {
            unsigned short temp = words[j];
            words[j]            = words[j + 1];
            words[j + 1]        = temp;
         }

         NumFragsFound += FifoToFragment(words, fifowords, extrafifo, midasSerialNumber, midasTime);
         i += (fifowords / 2);
      }
   }

   return 1;
}

int TDataParser::FifoToFragment(unsigned short*, int, bool, unsigned int, time_t)
{
   //
   //	if(size<10) //this is too short to be anything useful
   //		return 0;
   //
   //	TFragment* eventFrag = new TFragment();
   //	eventFrag->MidasTimeStamp = midasTime;
   //	eventFrag->MidasId = midasSerialNumber;
   //
   //
   //	printf("\t");
   //	for(int j=0;j<size;j++) {
   //		if(j!=0 && (j%8==0))
   //			printf("\n\t");
   //		printf("0x%04x ",data[j]);
   //	}
   //	printf("\n\n");
   //
   //	unsigned short type;
   //	unsigned short value,value2;
   //
   //	int ulm = 0;
   //	if(zerobuffer)
   //		ulm = size-9;
   //	else
   //		ulm = size-8;
   //	type = data[ulm];
   //	if((type &0xfff0) != 0xff20) { //not a ulm, bad things are happening.
   //		if(eventFrag) delete eventFrag;
   //		printf("here??   ulm = %i   0x%04x  \n",ulm,type);
   //		return 0;
   //	} else {
   //		eventFrag->DetectorType = (type&0x000f);
   //		value = data[ulm+1];
   //		eventFrag->PPG = (value&0x3ff);
   //		eventFrag->TriggerBitPattern = (value&0xf000)>>11;
   //		eventFrag->TimeStampLow  = data[ulm+2]*65536 +data[ulm+3];
   //		eventFrag->TimeStampHigh = data[ulm+4]*65536 +data[ulm+5];
   //		eventFrag->TriggerId     = data[ulm+6]*65536 +data[ulm+7];
   //	}
   //	size = ulm;  //only look at what is left.
   //	for(int x=0;x<size;x++) {
   //		type  = data[x]; // & 0x80f0;
   //		printf("type = 0x%04x  ||  type&0x80f0 = 0x%04x\n",type,(type&0x80f0));
   //		switch(type & 0x80f0) {
   //			case 0x8010:  // Lecroy 3377
   //				value = data[x+1];  //this is just so I can start the loop...
   //				x++;
   //				while(((value&0x8000)==0) && x<size) {
   //					value  = data[x]; x+=1;
   //					value2 = data[x]; x+=1;
   //					if((value&0x7c00) != (value2&0x7c00))
   //						printf("TIME MISMATCH!  bad things are happening.\n");
   //					int temp = ((value&0x7c00)>>10)<<16;
   //					temp += ((value&0x00ff)<<8) + (value2&0x00ff);
   //					//printf("size = %i | ulm = %i | x = %i  | temp = 0x%08x \n",size,ulm,x,temp);
   //					eventFrag->Cfd.push_back(temp);
   //				}
   //				printf(DGREEN "ENDED TDC LOOP  data[%i] = 0x%04x" RESET_COLOR "\n",x,data[x]);
   //				break;
   //			case 0x8040:
   //			case 0x8050: // Ortect AD114 single channel ?
   //			case 0x8060:
   //				value = data[++x];
   //				if(eventFrag->DetectorType==0) {
   //					int temp = (type&0x001f)<<16;
   //					temp += value&0x3fff;
   //					//printf("temp = %08x\n",temp);
   //					eventFrag->PulseHeight.push_back(temp);
   //					eventFrag->ChannelNumber = 0;
   //					//eventFrag->ChannelNumber  = (type&0x001f); //clear as mud.
   //					//eventFrag->ChannelAddress = 0x0000 + eventFrag->ChannelNumber;
   //					//eventFrag->PulseHeight.push_back(value&0x3fff);
   //				} else if (eventFrag->DetectorType==3){
   //					int temp = (type&0x001f)<<16;
   //					temp += value&0x3fff;
   //					//printf("temp = %08x\n",temp);
   //					eventFrag->PulseHeight.push_back(temp);
   //					eventFrag->ChannelNumber = 3;
   //					//eventFrag->ChannelNumber  = (type&0x001f); //clear as mud.
   //					//eventFrag->ChannelAddress = 0x3000 + eventFrag->ChannelNumber;
   //					//eventFrag->PulseHeight.push_back(value&0x3fff);
   //				}
   //				//printf("value = 0x%04x\n",value);
   //				break;
   //
   //
   //		};
   //	}
   //	eventFrag->Print();
   //
   //
   //	fGoodOutputQueue->Add(eventFrag);
   //
   return 1;
}

int TDataParser::FippsToFragment(std::vector<char> data)
{
   uint32_t* ptr = reinterpret_cast<uint32_t*>(data.data());

   int                        totalEventsRead = 0;
   std::shared_ptr<TFragment> eventFrag       = std::make_shared<TFragment>();
   Long64_t                   tmpTimestamp;
   if(fItemsPopped != nullptr && fInputSize != nullptr) {
      *fItemsPopped = 0;
      *fInputSize   = data.size()/16; //4 words of 4 bytes for each event
   }

	// we read 4 words for each event, and data.size() is in bytes, so we need to divide it by 4 (size of uint32_t)
   for(size_t i = 0; i + 3 < data.size() / 4; i += 4) {
      if(fItemsPopped != nullptr && fInputSize != nullptr) {
         ++(*fItemsPopped);
         --(*fInputSize);
      }
      eventFrag->SetAddress(ptr[i] >> 16);
      tmpTimestamp = ptr[i] & 0xffff;
      tmpTimestamp = tmpTimestamp<<30;
      tmpTimestamp |= ptr[i + 1] & 0x3fffffff;
      eventFrag->SetTimeStamp(tmpTimestamp);
      ++totalEventsRead;
      if((ptr[i + 2] & 0x7fff) == 0) {
         if(fRecordDiag) {
            TParsingDiagnostics::Get()->BadFragment(99);
         }
         // Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, ptr, data.size() / 4, i + 2, false));
         continue;
      }
      eventFrag->SetCharge(static_cast<int32_t>(ptr[i + 2] & 0x7fff));
      if(fRecordDiag) {
         TParsingDiagnostics::Get()->GoodFragment(eventFrag);
      }
      Push(fGoodOutputQueues, std::make_shared<TFragment>(*eventFrag));
      // std::cout<<totalEventsRead<<": "<<eventFrag->Charge()<<", "<<eventFrag->GetTimeStamp()<<std::endl;
   }

   return totalEventsRead;
}

int TDataParser::CaenToFragment(uint32_t* data, int size)
{
   /// Converts a Caen flavoured MIDAS events into TFragments and returns the number of events processed
   std::shared_ptr<TFragment> eventFrag = std::make_shared<TFragment>();
	int w = 0;
	int nofFragments = 0;
   short timestampRemainder;
   uint64_t timestamp;
	for(int board = 0; w < size; ++board) {
		// read board aggregate header
		if(data[w]>>28 != 0xa) {
			if(data[w] == 0x0) {
				while(w < size) {
					if(data[w++] != 0x0) {
                  if(!fOptions->SuppressErrors()) {
                     std::cerr<<board<<". board - failed on first word, found empty word, but not all following words were empty: "<<w-1<<" 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w-1]<<std::dec<<std::setfill(' ')<<std::endl;
                  }
						return -w;
					}
				}
				return nofFragments;
			}
         if(!fOptions->SuppressErrors()) {
            std::cerr<<board<<". board - failed on first word 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<", highest nibble should have been 0xa!"<<std::endl;
         }
			return -w;
		}
		int32_t numWordsBoard = data[w++]&0xfffffff; // this is the number of 32-bit words from this board
		if(w - 1 + numWordsBoard > size) { 
         if(!fOptions->SuppressErrors()) {
            std::cerr<<"0 - Missing words, at word "<<w-1<<", expecting "<<numWordsBoard<<" more words for board "<<board<<" (bank size "<<size<<")"<<std::endl;
         }
			return -w;
		}
      //std::cout<<w-1<<": 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w-1]<<std::dec<<std::setfill(' ')<<" - "<<numWordsBoard<<" words"<<std::endl;
		uint8_t boardId = data[w]>>27; // GEO address of board (can be set via register 0xef08 for VME)
		uint16_t pattern = (data[w]>>8) & 0x7fff; // value read from LVDS I/O (VME only)
		uint8_t channelMask = data[w++]&0xff; // which channels are in this board aggregate
      //std::cout<<w-1<<": 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w-1]<<std::dec<<std::setfill(' ')<<" - boardId "<<boardId<<", pattern "<<pattern<<", channelMask "<<channelMask<<std::endl;
		uint32_t boardCounter = data[w++]&0x7fffff; // ??? "counts the board aggregate"
      //std::cout<<w-1<<": 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w-1]<<std::dec<<std::setfill(' ')<<" - boardCounter "<<boardCounter<<std::endl;
		uint32_t boardTime = data[w++]; // time of creation of aggregate (does not correspond to a physical quantity)
      //std::cout<<w-1<<": 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w-1]<<std::dec<<std::setfill(' ')<<" - boardTime "<<boardTime<<std::endl;
		//if(boardCounter < gBoardCounter) {
		//	std::cerr<<"current board counter "<<boardCounter<<" is less than previous one "<<gBoardCounter<<", skipping this data"<<std::endl;
		//	return nofFragments;
		//}
		//gBoardCounter = boardCounter;

		for(uint8_t channel = 0; channel < 16; channel += 2) {
			if(((channelMask>>(channel/2)) & 0x1) == 0x0) {
				continue;
			}
			// read channel aggregate header
			if(data[w]>>31 != 0x1) {
            if(!fOptions->SuppressErrors()) {
               std::cerr<<"Failed on first word 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<", highest bit should have been set!"<<std::endl;
            }
				return -w;
			}
			int32_t numWords = data[w++]&0x3fffff;//per channel
			if(w >= size) {
            if(!fOptions->SuppressErrors()) {
               std::cerr<<"1 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
            }
				return -w;
			}
			if(((data[w]>>29) & 0x3) != 0x3) {
            if(!fOptions->SuppressErrors()) {
               std::cerr<<"Failed on second word 0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<", bits 29 and 30 should have been set!"<<std::endl;
            }
				return -w;
			}
			bool dualTrace = ((data[w]>>31) == 0x1);
			bool extras    = (((data[w]>>28) & 0x1) == 0x1);
			bool waveform  = (((data[w]>>27) & 0x1) == 0x1);
			uint8_t extraFormat = ((data[w]>>24) & 0x7);
			//for now we ignore the information which traces are stored:
			//bits 22,23: if(dualTrace) 00 = "Input and baseline", 01 = "CFD and Baseline", 10 = "Input and CFD"
			//            else          00 = "Input", 01 = "CFD"
			//bits 19,20,21: 000 = "Long gate",  001 = "over thres.", 010 = "shaped TRG", 011 = "TRG Val. Accept. Win.", 100 = "Pile Up", 101 = "Coincidence", 110 = reserved, 111 = "Trigger"
			//bits 16,17,18: 000 = "Short gate", 001 = "over thres.", 010 = "TRG valid.", 011 = "TRG HoldOff",           100 = "Pile Up", 101 = "Coincidence", 110 = reserved, 111 = "Trigger"
			int numSampleWords = 4*(data[w++]&0xffff);// this is actually the number of samples divided by eight, 2 sample per word => 4*
			if(w >= size) {
            if(!fOptions->SuppressErrors()) {
               std::cerr<<"2 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
            }
				return -w;
			}
			int eventSize = numSampleWords+2; // +2 = trigger time words and charge word
			if(extras) ++eventSize;
			if(numWords%eventSize != 2) {
            if(!fOptions->SuppressErrors()) {
               std::cerr<<numWords<<" words in channel aggregate, event size is "<<eventSize<<" => "<<static_cast<double>(numWords-2.)/static_cast<double>(eventSize)<<" events?"<<std::endl;
            }
				return -w;
			}

			// read channel data
			for(int ev = 0; ev < (numWords-2)/eventSize; ++ev) { // -2 = 2 header words for channel aggregate
				eventFrag->SetMidasTimeStamp(boardTime);
				eventFrag->SetAddress(0x8000 + channel + (data[w]>>31)); // highest bit indicates odd channel
            if(eventFrag->GetAddress() == 0x8000) eventFrag->SetDetectorType(9); //ZDS will always be in channel 0
            else                                  eventFrag->SetDetectorType(6);
            // these timestamps are in 2ns units, but the "normal" timestamps are in 10ns units
            // so we store the remainder and set our timestamp in 10ns units
            // the remainder will later be used to modify the CFD value to automatically correct for it
            timestamp = data[w] & 0x7fffffff;
            timestampRemainder = timestamp%5;
				eventFrag->SetTimeStamp(timestamp/5);
            eventFrag->SetCfd(timestampRemainder<<10); //this is in case we do not get a CFD word
            ++w;
				if(waveform) {
					if(w + numSampleWords >= size) { // need to read at least the sample words plus the charge/extra word
                  if(!fOptions->SuppressErrors()) {
                     std::cerr<<"3 - Missing "<<numSampleWords<<" waveform words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
                  }
						return -w;
					}
					for(int s = 0; s < numSampleWords && w < size; ++s, ++w) {
						//eventFrag->AddDigitalWaveformSample(0, (data[w]>>14)&0x1);
						//eventFrag->AddDigitalWaveformSample(1, (data[w]>>15)&0x1);
						if(dualTrace) {
							// all even samples are from the first trace, all odd ones from the second trace
							//eventFrag->AddWaveformSample(data[w]&0x3fff);
							//eventFrag->AddWaveformSample((data[w]>>16)&0x3fff);
							eventFrag->AddWaveformSample(data[w]&0xffff);
							eventFrag->AddWaveformSample((data[w]>>16)&0xffff);
						} else {
							// both samples are from the first trace
							//eventFrag->AddWaveformSample(data[w]&0x3fff);
							//eventFrag->AddWaveformSample((data[w]>>16)&0x3fff);
							eventFrag->AddWaveformSample(data[w]&0xffff);
							eventFrag->AddWaveformSample((data[w]>>16)&0xffff);
						}
						//eventFrag->AddDigitalWaveformSample(0, (data[w]>>30)&0x1);
						//eventFrag->AddDigitalWaveformSample(1, (data[w]>>31)&0x1);
					}
				} else {
					if(w >= size) { // need to read at least the sample words plus the charge/extra word
                  if(!fOptions->SuppressErrors()) {
                     std::cerr<<"3 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
                  }
						return -w;
					}
				}
				if(extras) {
               //std::cout<<eventFrag->GetAddress()<<" - extra word format "<<static_cast<int>(extraFormat)<<": 0x"<<std::hex<<data[w]<<std::dec<<std::endl;
					switch(extraFormat) {
						case 0: // [31:16] extended time stamp, [15:0] baseline*4
							//eventFrag->Baseline(data[w]&0xffff);
							eventFrag->SetTimeStamp(eventFrag->GetTimeStamp() | static_cast<uint64_t>(data[w])<<15);
							break;
						case 1: // [31:16] extended time stamp, 15 trigger lost, 14 over range, 13 1024 triggers, 12 n lost triggers
							eventFrag->SetNetworkPacketNumber((data[w]>>12)&0xf);
							//eventFrag->NLostCount(((data[w]>>12)&0x1) == 0x1);
							//eventFrag->KiloCount(((data[w]>>13)&0x1) == 0x1);
							//eventFrag->OverRange(((data[w]>>14)&0x1) == 0x1);
							//eventFrag->LostTrigger(((data[w]>>15)&0x1) == 0x1);
							eventFrag->SetTimeStamp(eventFrag->GetTimeStamp() | static_cast<uint64_t>(data[w])<<15);
							break;
						case 2: // [31:16] extended time stamp,  15 trigger lost, 14 over range, 13 1024 triggers, 12 n lost triggers, [9:0] fine time stamp
                     //std::cout<<"timestamp "<<timestamp<<" (0x"<<std::hex<<timestamp<<" | 0x"<<(static_cast<uint64_t>(data[w]&0xffff0000)<<15)<<" => 0x";
                     timestamp |= static_cast<uint64_t>(data[w]&0xffff0000)<<15;
                     //std::cout<<timestamp<<std::dec<<"), "<<timestamp<<", setting to ";
                     timestampRemainder = timestamp%5;
                     eventFrag->SetTimeStamp(timestamp/5);
                     //std::cout<<eventFrag->GetTimeStamp()<<std::endl;
							eventFrag->SetCfd((timestampRemainder<<10) | (data[w]&0x3ff));
							eventFrag->SetNetworkPacketNumber((data[w]>>12)&0xf);
							//eventFrag->NLostCount(((data[w]>>12)&0x1) == 0x1);
							//eventFrag->KiloCount(((data[w]>>13)&0x1) == 0x1);
							//eventFrag->OverRange(((data[w]>>14)&0x1) == 0x1);
							//eventFrag->LostTrigger(((data[w]>>15)&0x1) == 0x1);
							break;
						case 4: // [31:16] lost trigger counter, [15:0] total trigger counter
							eventFrag->SetAcceptedChannelId(data[w]&0xffff); // this is actually the lost trigger counter!
							eventFrag->SetChannelId(data[w]>>16);
							break;
						case 5: // [31:16] CFD sample after zero cross., [15:0] CFD sample before zero cross.
							//eventFrag->CfdAfterZC(data[w]&0xffff);
							//eventFrag->CfdBeforeZC(data[w]>>16);
							w++;
							break;
						case 7: // fixed value of 0x12345678
							if(data[w] != 0x12345678) {
                        if(!fOptions->SuppressErrors()) {
                           std::cerr<<"Failed to get debug data word 0x12345678, got "<<std::hex<<std::setw(8)<<std::setfill('0')<<data[w]<<std::dec<<std::setfill(' ')<<std::endl;
                        }
								break;
							}
							break;
						default:
							break;
					}
               ++w;
				}
				//if(w >= size) {
				//	std::cerr<<"4 - Missing words, got only "<<w-1<<" words for channel "<<channel<<" (bank size "<<size<<")"<<std::endl;
				//}
				eventFrag->SetCcShort(data[w]&0x7fff);
				eventFrag->SetCcLong((data[w]>>15) & 0x1);//this is actually the over-range bit!
				eventFrag->SetCharge(static_cast<Int_t>(data[w++]>>16));
				Push(fGoodOutputQueues, std::make_shared<TFragment>(*eventFrag));
            eventFrag->Clear();
            ++nofFragments;
			} // while(w < size)
		} // for(uint8_t channel = 0; channel < 16; channel += 2)
	} // for(int board = 0; w < size; ++board)

	return nofFragments;
}

int TDataParser::TdrToFragment(std::vector<char> data)
{
	uint64_t* ptr = reinterpret_cast<uint64_t*>(data.data());

	int totalEventsRead = 0;
	static uint64_t timeStampHighBits = 0;
   std::shared_ptr<TFragment> eventFrag       = std::make_shared<TFragment>();
   if(fItemsPopped != nullptr && fInputSize != nullptr) {
      *fInputSize   += data.size()/8; // words of 8 bytes each
   }

	for(size_t i = 0; i < data.size()/8; ++i) {
		eventFrag->Clear();
		if(fItemsPopped != nullptr && fInputSize != nullptr) {
			++(*fItemsPopped);
			--(*fInputSize);
		}
		//std::cout<<"0x"<<std::setw(16)<<ptr[i]<<std::endl;
		switch(ptr[i]>>62) {
			case 0:
				std::cout<<"unknown word"<<std::endl;
				break;
			case 1:
				std::cout<<"trace word"<<std::endl;
				break;
			case 2:
				// this word gives us the extended timestamp (and a moduleNumber and channelNumber)
				// the only thing we really care about is the high timestamp bits
				{
					//short moduleNumber = (ptr[i]>>56)&0x3f;
					short infoCode = (ptr[i]>>52)&0xf;
					if(((ptr[i]>>28)&0xf) != 0x0) {
						std::cout<<"not a proper word!"<<std::endl;
					}
					//uint64_t timeStamp = ptr[i]&0xfffffff;
					//uint32_t channelNumber = 0;
					switch(infoCode) {
						case 0:
							std::cout<<"undefined infoCode "<<infoCode<<std::endl;
							break;
						case 1:
							//channelNumber = (ptr[i]>>32)&0xfffff;
							break;
						case 2:
						case 3:
						case 4:
						case 7:
							//timeStamp |= ptr[i]&0xfffff00000000>>4;
							timeStampHighBits = (ptr[i]>>32)&0xfffff;
							break;
						case 5:
							//timeStamp |= (ptr[i]&0xfffff00000000)<<20;
							timeStampHighBits = (ptr[i]>>12)&0xfffff00000;
							break;
						default:
							std::cout<<"infoCode "<<infoCode<<std::endl;
							break;
					}
					//std::cout<<"moduleNumber "<<moduleNumber<<", channelNumber "<<channelNumber<<", timeStamp "<<timeStamp<<std::endl;
					break;
				}
			case 3:
				{
					// data word with channelNumber, adcData, and 28 low bits of timestamp
					// check if this is the tape-move event
					auto channel = TChannel::GetChannel((ptr[i]>>48)&0xfff);
					if(channel != nullptr && channel->GetDigitizerType() == TMnemonic::EDigitizer::kPixieTapeMove) {
						//PPG
						auto* ppgEvent = new TPPGData;
						switch(channel->GetDetectorNumber()) {
							case 1:
								// tape move for "normal" experiments
								// target finished moving in front of detector for PR294
								ppgEvent->SetNewPPG(EPpgPattern::kTapeMove);
								break;
							case 2:
								// unused for "normal" experiments
								// target moving away from detector for PR294
								ppgEvent->SetNewPPG(EPpgPattern::kBackground);
								break;
							default:
								ppgEvent->SetNewPPG(EPpgPattern::kJunk);
								break;
						}
						ppgEvent->SetLowTimeStamp(ptr[i]&0xfffffff);
						ppgEvent->SetHighTimeStamp(timeStampHighBits);
						ppgEvent->SetNetworkPacketId(static_cast<Int_t>((ptr[i]>>32)&0xffff));
						TPPG::Get()->AddData(ppgEvent);
						continue;
					}
					eventFrag->SetAddress((ptr[i]>>48)&0xfff);
					eventFrag->SetTimeStamp((ptr[i]&0xfffffff) | (timeStampHighBits<<28));
					++totalEventsRead;
					// charge is a 14bit signed integer (despite being reported as 16 bits) so we extend the sign bit for an Int_t (4 bytes)
					//eventFrag->SetCharge(static_cast<Int_t>(((ptr[i]>>32)&0xffff) | ((((ptr[i]>>32)&0x2000) == 0x2000) ? 0xffffc000 : 0x0)));
					eventFrag->SetCharge(static_cast<Int_t>((ptr[i]>>32)&0xffff));
					//std::cout<<std::hex<<std::setfill('0');
					//std::cout<<std::setw(16)<<ptr[i]<<": addr "<<eventFrag->GetAddress()<<", ts "<<eventFrag->GetTimeStamp()<<", charge "<<eventFrag->Charge()<<std::endl;
					//std::cout<<std::dec<<std::setfill(' ');
					// we expect a second word from the channel+16 with the same timestamp
					++i;
					if(i >= data.size()/8) {
						if(fRecordDiag) {
							TParsingDiagnostics::Get()->BadFragment(66);
						}
						Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, reinterpret_cast<uint32_t*>(data.data()), data.size()/4, i, false));
						continue;
					}
					// check address
					if(((ptr[i]>>48)&0xfff) != eventFrag->GetAddress() + 16) {
						//std::cout<<"Address mismatch "<<((ptr[i]>>48)&0xfff)<<" != "<<eventFrag->GetAddress()<<std::endl;
						if(fRecordDiag) {
							TParsingDiagnostics::Get()->BadFragment(67);
						}
						Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, reinterpret_cast<uint32_t*>(data.data()), data.size()/4, i, false));
						// re-try this word
						--i;
						continue;
					}
					// check timestamp
					if(static_cast<Long64_t>((ptr[i]&0xfffffff) | (timeStampHighBits<<28)) != eventFrag->GetTimeStamp()) {
						//std::cout<<"Timestamp mismatch "<<((ptr[i]&0xfffffff) | (timeStampHighBits<<28))<<" != "<<eventFrag->GetTimeStamp()<<std::endl;
						if(fRecordDiag) {
							TParsingDiagnostics::Get()->BadFragment(68);
						}
						Push(*fBadOutputQueue, std::make_shared<TBadFragment>(*eventFrag, reinterpret_cast<uint32_t*>(data.data()), data.size()/4, i, false));
						// re-try this word
						--i;
						continue;
					}
					if(fItemsPopped != nullptr && fInputSize != nullptr) {
						--(*fInputSize); // we just read another word from the input
					}
					// set Cfd ???
					eventFrag->SetCfd(static_cast<Int_t>((ptr[i]>>32)&0xffff));
					//std::cout<<std::hex<<std::setfill('0');
					//std::cout<<std::setw(16)<<ptr[i]<<": addr "<<eventFrag->GetAddress()<<", ts "<<eventFrag->GetTimeStamp()<<", cfd "<<eventFrag->GetCfd()<<std::endl;
					//std::cout<<std::dec<<std::setfill(' ');
					if(fRecordDiag) {
						TParsingDiagnostics::Get()->GoodFragment(eventFrag);
					}
					Push(fGoodOutputQueues, std::make_shared<TFragment>(*eventFrag));
					break;
				}
		}
	}

	return totalEventsRead;
}

void TDataParser::Push(std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>>& queues,
		const std::shared_ptr<TFragment>&                                                frag)
{
	frag->SetFragmentId(fFragmentIdMap[frag->GetTriggerId()]);
	fFragmentIdMap[frag->GetTriggerId()]++;
	frag->SetEntryNumber();
	for(const auto& queue : queues) {
		queue->Push(frag);
	}
}

void TDataParser::Push(ThreadsafeQueue<std::shared_ptr<const TBadFragment>>& queue, const std::shared_ptr<TBadFragment>& frag)
{
	frag->SetFragmentId(fFragmentIdMap[frag->GetTriggerId()]);
	fFragmentIdMap[frag->GetTriggerId()]++;
	frag->SetEntryNumber();
	queue.Push(frag);
}

std::string TDataParser::OutputQueueStatus()
{
	std::stringstream ss;
	ss<<"********************************************************************************"<<std::endl;
	for(const auto& queue : fGoodOutputQueues) {
		ss<<queue->Name()<<": "<<queue->ItemsPushed()<<" pushed, "<<queue->ItemsPopped()<<" popped, "
			<<queue->Size()<<" left"<<std::endl;
	}
	ss<<"********************************************************************************"<<std::endl;
	return ss.str();
}

/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////
/////////////***************************************************************/////////////

int TDataParser::EPIXToScalar(float* data, int size, unsigned int midasSerialNumber, time_t midasTime)
{
	int                         NumFragsFound = 1;
	std::shared_ptr<TEpicsFrag> EXfrag        = std::make_shared<TEpicsFrag>();

	EXfrag->fMidasTimeStamp = midasTime;
	EXfrag->fMidasId        = midasSerialNumber;

	for(int x = 0; x < size; x++) {
		EXfrag->fData.push_back(data[x]);
		EXfrag->fName.push_back(TEpicsFrag::GetEpicsVariableName(x));
	}

	fScalerOutputQueue->Push(EXfrag);
	return NumFragsFound;
}
