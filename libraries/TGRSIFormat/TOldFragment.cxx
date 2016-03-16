#include "TOldFragment.h"
#include "TChannel.h"
#include "TROOT.h"
#include <iostream>

#include <TClass.h>

/// \cond CLASSIMP
ClassImp(TOldFragment)
/// \endcond

//long TOldFragment::nofFragments = 0;

TOldFragment::TOldFragment() {
   // Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	fCfd = NULL;
	fPulseHeight = NULL;
	fIntLength = NULL;
   Clear();
}

TOldFragment::TOldFragment(const TOldFragment& rhs) : TFragment() {
	//copy constructor
  
	//first copy all "normal" data members
	fMidasTimeStamp = rhs.fMidasTimeStamp;
	fTriggerId = rhs.fTriggerId;
	fMidasId = rhs.fMidasId;
	fTriggerBitPattern = rhs.fTriggerBitPattern;
	fNetworkPacketNumber = rhs.fNetworkPacketNumber;

	fTimeStampLow = rhs.fTimeStampLow;
	fTimeStampHigh = rhs.fTimeStampHigh;
	fChannelAddress = rhs.fChannelAddress;
	fChannelId = rhs.fChannelId;
	fAcceptedChannelId = rhs.fAcceptedChannelId;

	fDeadTime = rhs.fDeadTime;
	fDataType = rhs.fDataType;
	fDetectorType = rhs.fDetectorType;
	fWavebuffer = rhs.fWavebuffer;

	//copy transient data members
	fPPG = rhs.fPPG;
	fNumberOfFilters = rhs.fNumberOfFilters;
	fZc = rhs.fZc;
	fCcShort = rhs.fCcShort;
	fCcLong = rhs.fCcLong;

	fNumberOfPileups = rhs.fNumberOfPileups;
	fNumberOfHits = fNumberOfPileups+1;//number of pile-ups doesn't count the first hit
	if(fNumberOfHits <= 0) {
		printf("Error, got a fragment with no hits:\n");
		rhs.Print();
	} else {
		fCfd = new Int_t[fNumberOfHits];
		fPulseHeight = new Int_t[fNumberOfHits];
		fIntLength = new UShort_t[fNumberOfHits];
		for(int i = 0; i < fNumberOfHits; ++i) {
			fCfd[i] = rhs.fCfd[i];
			fPulseHeight[i] = rhs.fPulseHeight[i];
			fIntLength[i] = rhs.fIntLength[i];
		}
	}
}

TOldFragment::~TOldFragment() {
	// Default destructor does nothing right now
	if(fCfd != NULL) delete[] fCfd;
	if(fPulseHeight != NULL) delete[] fPulseHeight;
	if(fIntLength != NULL) delete[] fIntLength;
}

TOldFragment& TOldFragment::operator=(const TOldFragment& rhs) {
	//check for self-assignment, this could actually screw up the data because we first clear all data
	// and then copy the cleared data
	if(this != &rhs) {
		Clear();
		//first copy all "normal" data members
		fMidasTimeStamp = rhs.fMidasTimeStamp;
		fTriggerId = rhs.fTriggerId;
		fMidasId = rhs.fMidasId;
		fTriggerBitPattern = rhs.fTriggerBitPattern;
		fNetworkPacketNumber = rhs.fNetworkPacketNumber;

		fTimeStampLow = rhs.fTimeStampLow;
		fTimeStampHigh = rhs.fTimeStampHigh;
		fChannelAddress = rhs.fChannelAddress;
		fChannelId = rhs.fChannelId;
		fAcceptedChannelId = rhs.fAcceptedChannelId;

		fDeadTime = rhs.fDeadTime;
		fDataType = rhs.fDataType;
		fDetectorType = rhs.fDetectorType;
		fWavebuffer = rhs.fWavebuffer;

		//copy transient data members
		fPPG = rhs.fPPG;
		fNumberOfFilters = rhs.fNumberOfFilters;
		fZc = rhs.fZc;
		fCcShort = rhs.fCcShort;
		fCcLong = rhs.fCcLong;

		fNumberOfPileups = rhs.fNumberOfPileups;
		fNumberOfHits = fNumberOfPileups+1;//number of pile-ups doesn't count the first hit
		if(fNumberOfHits <= 0) {
			printf("Error, got a fragment with no hits:\n");
			rhs.Print();
		} else {
			fCfd = new Int_t[fNumberOfHits];
			fPulseHeight = new Int_t[fNumberOfHits];
			fIntLength = new UShort_t[fNumberOfHits];
			for(int i = 0; i < fNumberOfHits; ++i) {
				fCfd[i] = rhs.fCfd[i];
				fPulseHeight[i] = rhs.fPulseHeight[i];
				fIntLength[i] = rhs.fIntLength[i];
			}
		}
	}

	return *this;
}

void TOldFragment::Clear(Option_t *opt) {
   // Clears all fields of the TOldFragment
	fMidasTimeStamp = 0;
	fTriggerId = 0;
	fMidasId = 0;
	fTriggerBitPattern = 0;
	fNetworkPacketNumber = 0;

	fTimeStampLow = 0;
	fTimeStampHigh = 0;
	fChannelAddress = 0;
	fChannelId = 0;
	fAcceptedChannelId = 0;

	fDeadTime = 0;
	fDataType = 0;
	fDetectorType = 0;

	//copy transient data members
	fPPG = NULL;
	fNumberOfFilters = 0;
	fZc = 0;
	fCcShort = 0;
	fCcLong = 0;

	fNumberOfPileups = 0;
	fNumberOfHits = 0;
	if(fCfd != NULL) {
		delete[] fCfd;
		fCfd = NULL;
	}
	if(fPulseHeight != NULL) {
		delete[] fPulseHeight;
		fPulseHeight = NULL;
	}
	if(fIntLength != NULL) {
		delete[] fIntLength;
		fIntLength = NULL;
	}

	fWavebuffer.clear();
}

TOldFragment* TOldFragment::Clone(const char* name) {
	TOldFragment* result = new TOldFragment;
	*result = *this;
	return result;
}

long TOldFragment::GetTimeStamp() const {
   long time = fTimeStampHigh;
   time  = time << 28;
   time |= fTimeStampLow & 0x0fffffff;
   return time;
}

double TOldFragment::GetTime() const {
   double dtime = (double)(GetTimeStamp())+ gRandom->Uniform();
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan )
      return dtime;
   return dtime - chan->GetTZero(GetEnergy());
}

double TOldFragment::GetTZero() const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan )
      return 0.000;
   return chan->GetTZero(GetEnergy());
}

long TOldFragment::GetTimeStamp_ns() const {
   long ns = 0;
   if(fDataType == 2 && fNumberOfHits > 0) {
     ns = (fCfd[0] >> 21) & 0xf;
   }
   return 10*GetTimeStamp() + ns;  
}

Int_t TOldFragment::Get4GCfd(Int_t i) const { // return a 4G cfd in terms 
	if(fNumberOfHits <= 0)                     // of 1/256 ns since the trigger
     return -1;
  if(fNumberOfHits <= i)
     i = fNumberOfHits-1;
  return fCfd[i]&0x001fffff;
}


const char* TOldFragment::GetName() const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan)
      return "";
   return chan->GetChannelName();
}

Short_t TOldFragment::GetChannelNumber() const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan ) return 0;
   return chan->GetNumber();
}

Float_t TOldFragment::GetCharge(Int_t i) const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan || !(fNumberOfHits > i))
      return 0.00;
   if(chan->UseCalFileIntegration()) {
      return ((Float_t)fPulseHeight[i]+gRandom->Uniform())/((Float_t)chan->GetIntegration());// this will use the integration value
   }                                                                                         // in the TChannel if it exists.
   if(fIntLength[i] > 0) {
      return ((Float_t)fPulseHeight[i]+gRandom->Uniform())/((Float_t)fIntLength[i]);// this will use the integration value
   }
   return ((Float_t)fPulseHeight[i]+gRandom->Uniform());// this will use no integration value
}

double TOldFragment::GetEnergy(Int_t i) const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan || !(fNumberOfHits > i))
      return 0.00;
   if(chan->UseCalFileIntegration()) {
		return chan->CalibrateENG((int)(fPulseHeight[i]),0);  // this will use the integration value
                                                        // in the TChannel if it exists.
   }
   if(fIntLength[i]>0)
     return chan->CalibrateENG((int)(fPulseHeight[i]),(int)fIntLength[i]);
   return chan->CalibrateENG((int)(fPulseHeight[i]));
}

ULong64_t TOldFragment::GetTimeInCycle() {
   if(fPPG == NULL) {
		fPPG = static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
   if(fPPG == NULL) {
      return 0;
   }
   return fPPG->GetTimeInCycle(GetTimeStamp());
}

ULong64_t TOldFragment::GetCycleNumber() {
   if(fPPG == NULL) {
		fPPG = static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
   if(fPPG == NULL) {
      return 0;
   }
   return fPPG->GetCycleNumber(GetTimeStamp());
}

TPPG* TOldFragment::GetPPG() {
   if(fPPG == NULL) {
		fPPG = static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
	return fPPG;
}

void TOldFragment::Print(Option_t* opt) const {
   //Prints out all fields of the TOldFragment

   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   //printf("%s Event at   %i:\n", chan->GetDigitizerType().c_str(), MidasId);
   char buff[20];
   ctime(&fMidasTimeStamp);
   struct tm*  timeinfo = localtime(&fMidasTimeStamp);
   strftime(buff,20,"%b %d %H:%M:%S",timeinfo);
   printf("MidasId:             %8i, MidasTimeStamp:    %s\n",fMidasId, buff);
   printf("TriggerId:           %8lu, TriggerBit:     0x%08x\n", fTriggerId, fTriggerBitPattern);
   printf("DataType:            %8d, DetectorType:      %8d, DeadTime:  %8d\n", fDataType, fDetectorType, fDeadTime);
   printf("NetworkPacketNumber: %8i\n", fNetworkPacketNumber);
   if(chan)
      printf("Channel:             %8i\tName:              %s\n", chan->GetNumber(), chan->GetChannelName());
   printf("\tChannel Address: 0x%08x\n", fChannelAddress);
   printf("\tPulseHeight[%d]",fNumberOfHits); for(Int_t x=0;x<fNumberOfHits;x++){printf( "   0x%08x", fPulseHeight[x]);} printf("\n");
   printf("\tIntLength[%d]  ",fNumberOfHits); for(Int_t x=0;x<fNumberOfHits;x++){printf( "   0x%08x", fIntLength[x]);} printf("\n");
   printf("\tCFD[%d]        ",fNumberOfHits); for(Int_t x=0;x<fNumberOfHits;x++){printf( "   0x%08x", fCfd[x]);} printf("\n");
   printf("\tTimeStamp High:  0x%08x\n", fTimeStampHigh);
   printf("\tTimeStamp Low:   0x%08x\n", fTimeStampLow);
   printf("\tChannelId:         %8d, AcceptedChannelId: %8d\n", fChannelId, fAcceptedChannelId);
   if (!fWavebuffer.empty())
      printf("Has a wave form stored.\n");
   else
      printf("Does Not have a wave form stored.\n");
}

bool TOldFragment::IsDetector(const char*  prefix, Option_t* opt) const {
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
   std::string pre = prefix;
   TString option = opt;
   std::string channame = GetName();
   if(channame.length()<9)
      return false;

   option.ToUpper();
   //Could also do everything below with MNEMONIC Struct. This limits the amount of string processing that needs to be done
   //Because it returns false after every potential failure while the mnemonic class sets all of the strings, and then checks
   //for conditions.
   if(!channame.compare(0,pre.length(),pre)) {     //channame.BeginsWith(pre)){
      if(option.Length()<1) //no option.
         return true;
      if(channame.length()>8) {
        if(option.Contains("B") && (std::toupper(channame[9])==std::toupper('B')))
          return true;
        else if(option.Contains("A") && (std::toupper(channame[9])==std::toupper('A')))
          return true;
      }
      if(option.Contains("C") && !channame.compare(7,2,"00"))
        return true;
      else if(option.Contains("S") && channame.compare(7,2,"00"))
         return true;
   } else 
     return false;
   
   return false;
}

int TOldFragment::GetColor(Option_t* opt) const {
   std::string channame = GetName();
   if(channame.length()<9)
      return false;
   char color = channame[5];
   switch(color) {
      case 'B':
        return 0;
      case 'G':
        return 1;
      case 'R':
        return 2;
      case 'W':
        return 3;
   };
   return -1;
}
