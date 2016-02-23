#include "TNewFragment.h"
#include "TChannel.h"
#include "TROOT.h"
#include <iostream>

#include <TClass.h>

/// \cond CLASSIMP
ClassImp(TNewFragment)
/// \endcond

TNewFragment::TNewFragment(){
   // Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TNewFragment::TNewFragment(const TNewFragment& rhs) : TFragment() {
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
	fNumberOfWords = rhs.fNumberOfWords;
	fZc = rhs.fZc;
	fCcShort = rhs.fCcShort;
	fCcLong = rhs.fCcLong;

	fNumberOfPileups = rhs.fNumberOfPileups;
	fCfd = rhs.fCfd;
	fPulseHeight = rhs.fPulseHeight;
	fIntLength = rhs.fIntLength;
}

TNewFragment::~TNewFragment() {
	// Default destructor does nothing right now
}

TNewFragment& TNewFragment::operator=(const TNewFragment& rhs) {
	//we can simply copy all data members, so we don't need to worry about self-assignment
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
	fNumberOfWords = rhs.fNumberOfWords;
	fZc = rhs.fZc;
	fCcShort = rhs.fCcShort;
	fCcLong = rhs.fCcLong;

	fNumberOfPileups = rhs.fNumberOfPileups;
	fCfd = rhs.fCfd;
	fPulseHeight = rhs.fPulseHeight;
	fIntLength = rhs.fIntLength;

	return *this;
}

void TNewFragment::Clear(Option_t *opt){  
   // Clears all fields of the TNewFragment
	fMidasTimeStamp = 0;
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
	fNumberOfWords = 0;
	fZc = 0;
	fCcShort = 0;
	fCcLong = 0;

	fNumberOfPileups = 0;
	fCfd = 0;
	fPulseHeight = 0;
	fIntLength = 0;

	fWavebuffer.clear();
	fTriggerId.clear();
}

TNewFragment* TNewFragment::Clone(const char* name) {
	TNewFragment* result = new TNewFragment;
	*result = *this;
	return result;
}

long TNewFragment::GetTimeStamp() const {
   long time = fTimeStampHigh;
   time  = time << 28;
   time |= fTimeStampLow & 0x0fffffff;
   return time;
}

double TNewFragment::GetTime() const {
   double dtime = (double)(GetTimeStamp())+ gRandom->Uniform();
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan )
      return dtime;
   return dtime - chan->GetTZero(GetEnergy());
}

double TNewFragment::GetTZero() const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan )
      return 0.;
   return chan->GetTZero(GetEnergy());
}

long TNewFragment::GetTimeStamp_ns() const {
   long ns = 0;
   if(fDataType == 2) {
     ns = (fCfd >> 21) & 0xf;
   }
   return 10*GetTimeStamp() + ns;  
}

Int_t TNewFragment::Get4GCfd(Int_t i) const { // return a 4G cfd in terms of 1/256 ns since the trigger
  return fCfd & 0x001fffff;
}


const char* TNewFragment::GetName() const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan)
      return "";
   return chan->GetChannelName();
}

Short_t TNewFragment::GetChannelNumber() const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan ) return 0;
   return chan->GetNumber();
}

Float_t TNewFragment::GetCharge(Int_t i) const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan)
      return 0.00;
   if(chan->UseCalFileIntegration()) {
      return ((Float_t)fPulseHeight+gRandom->Uniform())/((Float_t)chan->GetIntegration());// this will use the integration value
   }                                                                                       // in the TChannel if it exists.
   if(fIntLength > 0){
      return ((Float_t)fPulseHeight+gRandom->Uniform())/((Float_t)fIntLength);// this will use the integration value
   }
   return ((Float_t)fPulseHeight+gRandom->Uniform());// this will use no integration value
}

double TNewFragment::GetEnergy(Int_t i) const {
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   if(!chan)
      return 0.00;
   if(chan->UseCalFileIntegration()) {
     return chan->CalibrateENG(fPulseHeight,0);  // this will use the integration value
                                                 // in the TChannel if it exists.
   }
   if(fIntLength>0)
     return chan->CalibrateENG(fPulseHeight,fIntLength);
   return chan->CalibrateENG(fPulseHeight);
}

ULong64_t TNewFragment::GetTimeInCycle() {
   if(fPPG == NULL) {
		fPPG = static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
   if(fPPG == NULL) {
      return 0;
   }
   return fPPG->GetTimeInCycle(GetTimeStamp());
}

ULong64_t TNewFragment::GetCycleNumber() {
   if(fPPG == NULL) {
		fPPG = static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
   if(fPPG == NULL) {
      return 0;
   }
   return fPPG->GetCycleNumber(GetTimeStamp());
}

TPPG* TNewFragment::GetPPG() {
   if(fPPG == NULL) {
		fPPG = static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
	return fPPG;
}

void TNewFragment::Print(Option_t* opt) const {
   //Prints out all fields of the TNewFragment
   TChannel* chan = TChannel::GetChannel(fChannelAddress);
   char buff[20];
   ctime(&fMidasTimeStamp);
   struct tm*  timeinfo = localtime(&fMidasTimeStamp);
   strftime(buff,20,"%b %d %H:%M:%S",timeinfo);
   printf("MidasTimeStamp: %s\n",buff);
   printf("MidasId    	%i\n", fMidasId);
   printf("\tTriggerId[%lu]	  ",fTriggerId.size()); for(size_t x=0;x<fTriggerId.size();x++){printf( "     0x%08lx", fTriggerId[x]);} printf("\n");
   printf("TriggerBit:	      0x%08x\n", fTriggerBitPattern);
   printf("NetworkPacketNumber: %i\n", fNetworkPacketNumber);
   if(chan)
	   printf("Channel: %i\tName: %s\n", chan->GetNumber(), chan->GetChannelName());
   printf("\tChannel Address: 0x%08x\n", fChannelAddress);
   printf("\tPulseHeight:     0x%08x\n", fPulseHeight);
   printf("\tCFD:             0x%08x\n", fCfd);
   printf("\tTimeStamp High:  0x%08x\n", fTimeStampHigh);
   printf("\tTimeStamp Low:   0x%08x\n", fTimeStampLow);
   if (!fWavebuffer.empty())
      printf("Has a wave form stored.\n");
   else
      printf("Does Not have a wave form stored.\n");
}

bool TNewFragment::IsDetector(const char*  prefix, Option_t* opt) const {
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

int TNewFragment::GetColor(Option_t* opt) const {
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
