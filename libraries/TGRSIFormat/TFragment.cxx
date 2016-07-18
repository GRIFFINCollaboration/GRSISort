#include "TFragment.h"
#include "TChannel.h"
#include "TROOT.h"
#include <iostream>

#include <TClass.h>

/// \cond CLASSIMP
ClassImp(TFragment)
  /// \endcond

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
#if MAJOR_ROOT_VERSION < 6
    Class()->IgnoreTObjectStreamer(kTRUE);
#endif
    Clear();
  }

TFragment::TFragment(const TFragment& rhs) : TGRSIDetectorHit(rhs) {

  //first copy all "normal" data members
  MidasTimeStamp = rhs.MidasTimeStamp;
  MidasId = rhs.MidasId;
  TriggerId = rhs.TriggerId;
  FragmentId = rhs.FragmentId;
  TriggerBitPattern = rhs.TriggerBitPattern;

  NetworkPacketNumber = rhs.NetworkPacketNumber;

  PPG = rhs.PPG;
  DeadTime = rhs.DeadTime;
  NumberOfFilters = rhs.NumberOfFilters;
  NumberOfPileups = rhs.NumberOfPileups;
  DataType = rhs.DataType;
  DetectorType  = rhs.DetectorType;
  ChannelId     = rhs.ChannelId;

  //if(hit < 0 || hit >= static_cast<int>(Cfd.size())) {
  Zc = rhs.Zc;
  ccShort = rhs.ccShort;
  ccLong = rhs.ccLong;
  Led = rhs.Led;

  //NumberOfHits = Cfd.size();
  //HitIndex = hit;
}

TFragment::~TFragment(){
  // Default destructor does nothing right now
  //Clear();
}

void TFragment::Clear(Option_t *opt){  
  // Clears all fields of the TFragment
  TGRSIDetectorHit::Clear(opt);
  MidasTimeStamp    = 0;
  MidasId           = 0;

  TriggerId         = 0;
  FragmentId        = 0;
  TriggerBitPattern = 0;

  NetworkPacketNumber = 0;

  Zc        = -1;
  ccShort   = -1;
  ccLong    = -1;
  Led       = -1;

  PPG                = 0; 
  DeadTime           = 0; 
  NumberOfFilters    = 0; 
  NumberOfPileups    = 0; 
  DataType           = 0; 
  DetectorType       = 0; 
  ChannelId          = 0; 

  fPPG = NULL;
}


double TFragment::GetTZero() const {
  TChannel *chan = GetChannel();
  if(!chan )//|| Charge.size()<1)
    return 0.000;
  return chan->GetTZero(GetEnergy());
}

long TFragment::GetTimeStamp_ns() const {
  long ns = 0;
  if(DataType==2) { //&& Cfd.size()>0) {
    ns = (GetCfd() >> 21) & 0xf;
  }
  return 10*GetTimeStamp() + ns;  
}

Int_t TFragment::Get4GCfd(size_t i) const { // return a 4G cfd in terms 
  //if(Cfd.size()==0)                // of 1/256 ns since the trigger
  //   return -1;
  //if(Cfd.size()<=i)
  //   i = Cfd.size()-1;
  return GetCfd()&0x001fffff;
}


ULong64_t TFragment::GetTimeInCycle() {
  if(fPPG == NULL) {
    fPPG = static_cast<TPPG*>(gROOT->FindObject("TPPG"));
  }
  if(fPPG == NULL) {
    return 0;
  }
  return fPPG->GetTimeInCycle(GetTimeStamp());
}

ULong64_t TFragment::GetCycleNumber() {
  if(fPPG == NULL) {
    fPPG = static_cast<TPPG*>(gROOT->FindObject("TPPG"));
  }
  if(fPPG == NULL) {
    return 0;
  }
  return fPPG->GetCycleNumber(GetTimeStamp());
}

void TFragment::Print(Option_t *opt) const {
  //Prints out all fields of the TFragment


  TChannel *chan = GetChannel();
  //printf("%s Event at  %i:\n", chan->GetDigitizerType().c_str(), MidasId);
  char buff[20];
  ctime(&MidasTimeStamp);
  struct tm * timeinfo = localtime(&MidasTimeStamp);
  strftime(buff,20,"%b %d %H:%M:%S",timeinfo);
  printf("MidasTimeStamp: %s\n",buff);
  printf("MidasId      %i\n", MidasId);
  printf("TriggerId:   %lu\n", TriggerId);
  printf("FragmentId:   %i\n", FragmentId);
  printf("TriggerBit:  0x%08x\n", TriggerBitPattern);
  printf("NetworkPacketNumber: %i\n", NetworkPacketNumber);
  if(chan) {
    printf("Channel: %i\tName: %s\n", chan->GetNumber(), chan->GetChannelName());
    printf("\tChannel Num:      %i\n",GetChannelNumber());
  }
  printf("\tChannel Address: 0x%08x\n", GetAddress());
  printf("\tCharge:          0x%08x\n ",   GetCharge());
  printf("\tCFD:             0x%08x\n ",   GetCfd());
  printf("\tZC:              0x%08x\n ",   Zc);   
  printf("\tLED:             0x%08x\n ",   Led);   
  printf("\tTimeStamp:       %lu\n", GetTimeStamp());
  //unsigned short temptime = (TimeStampLow & 0x0000ffff) - ((Cfd >> 4) & 0x0000ffff);   //TimeStampLow&0x0000ffff; 
  //printf("\ttime from timestamp(to the nearest 10ns):    0x%04x\t%ins\n", temptime, temptime * 10);
  if(HasWave())
    printf("Has a wave form stored.\n");
  else
    printf("Does Not have a wave form stored.\n");

}




bool TFragment::IsDetector(const char * prefix, Option_t *opt) const {
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
  std::string channame = this->GetName();
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










