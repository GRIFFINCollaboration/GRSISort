#include "TFragment.h"
#include "TChannel.h"
#include "TROOT.h"
#include <iostream>
#include <sstream>

#include <TClass.h>

/// \cond CLASSIMP
ClassImp(TFragment)
   /// \endcond

   Long64_t TFragment::fNumberOfFragments = 0;

TFragment::TFragment() : TGRSIDetectorHit()
{
/// Default constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TFragment::TFragment(const TFragment& rhs) : TGRSIDetectorHit(rhs)
{
   /// Copy constructor
   // first copy all "normal" data members
   fMidasTimeStamp      = rhs.fMidasTimeStamp;
   fMidasId             = rhs.fMidasId;
   fFragmentId          = rhs.fFragmentId;
   fTriggerBitPattern   = rhs.fTriggerBitPattern;
   fNetworkPacketNumber = rhs.fNetworkPacketNumber;
   fChannelId           = rhs.fChannelId;
   fAcceptedChannelId   = rhs.fAcceptedChannelId;

   fDeadTime        = rhs.fDeadTime;
   fModuleType      = rhs.fModuleType;
   fDetectorType    = rhs.fDetectorType;
   fNumberOfPileups = rhs.fNumberOfPileups;

   fTriggerId = rhs.fTriggerId;

   // copy transient data members
   fPPG           = rhs.fPPG;
   fZc            = rhs.fZc;
   fCcShort       = rhs.fCcShort;
   fCcLong        = rhs.fCcLong;
   fNumberOfWords = rhs.fNumberOfWords;
}

TFragment::~TFragment()
{
   /// Default destructor does nothing right now
}

void TFragment::Clear(Option_t* opt)
{
   /// Clears all fields of the TFragment
   TGRSIDetectorHit::Clear(opt);

   fMidasTimeStamp      = 0;
   fMidasId             = 0;
   fFragmentId          = 0;
   fTriggerBitPattern   = 0;
   fNetworkPacketNumber = 0;
   fChannelId           = 0;
   fAcceptedChannelId   = 0;

   fDeadTime        = 0;
   fModuleType      = 0;
   fDetectorType    = 0;
   fNumberOfPileups = 0;

   fTriggerId.clear();

   fPPG           = nullptr;
   fZc            = 0;
   fCcShort       = 0;
   fCcLong        = 0;
   fNumberOfWords = 0;
}

TObject* TFragment::Clone(const char*) const
{
   auto* result = new TFragment;
   *result           = *this;
   result->ClearTransients();
   return result;
}

double TFragment::GetTZero() const
{
   TChannel* chan = GetChannel();
   if(!chan) return 0.000;
   return chan->GetTZero(GetEnergy());
}

long TFragment::GetTimeStamp_ns() const
{
   long ns = 0;
   if(fModuleType == 2) {
      ns = (GetCfd() >> 21) & 0xf;
   }
   return 10 * GetTimeStamp() + ns;
}

Int_t TFragment::Get4GCfd() const
{ // return a 4G cfd in terms of 1/256 ns since the trigger
   return GetCfd() & 0x001fffff;
}

ULong64_t TFragment::GetTimeInCycle()
{

   if(fPPG == nullptr) {
      fPPG = TPPG::Get(); // static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
   if(fPPG == nullptr) {
      return 0;
   }
   return fPPG->GetTimeInCycle(GetTimeStamp());
}

ULong64_t TFragment::GetCycleNumber()
{
   if(fPPG == nullptr) {
      fPPG = TPPG::Get(); // static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
   if(fPPG == nullptr) {
      return 0;
   }
   return fPPG->GetCycleNumber(GetTimeStamp());
}

Short_t TFragment::GetChannelNumber() const
{
   TChannel* chan = TChannel::GetChannel(fAddress);
   if(!chan) return 0;
   return chan->GetNumber();
}

TPPG* TFragment::GetPPG()
{
   if(fPPG == nullptr) {
      fPPG = static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
   return fPPG;
}

void TFragment::Print(Option_t*) const
{
   /// Prints out all fields of the TFragment

   TChannel* chan = GetChannel();
   char      buff[20];
   ctime(&fMidasTimeStamp);
   struct tm* timeinfo = localtime(&fMidasTimeStamp);
   strftime(buff, 20, "%b %d %H:%M:%S", timeinfo);
   printf("MidasTimeStamp: %s\n", buff);
   printf("MidasId      %i\n", fMidasId);
   printf("\tTriggerId[%lu]	  ", fTriggerId.size());
   for(long x : fTriggerId) {
      printf("     0x%08lx", x);
   }
   printf("\n");
   printf("FragmentId:   %i\n", fFragmentId);
   printf("TriggerBit:  0x%08x\n", fTriggerBitPattern);
   printf("NetworkPacketNumber: %i\n", fNetworkPacketNumber);
   if(chan) {
      printf("Channel: %i\tName: %s\n", chan->GetNumber(), chan->GetName());
      printf("\tChannel Num:      %i\n", GetChannelNumber());
   }
   printf("\tChannel Address: 0x%08x\n", GetAddress());
   printf("\tCharge:          0x%08x\n ", static_cast<Int_t>(GetCharge()));
   printf("\tCFD:             0x%08x\n ", GetCfd());
   printf("\tZC:              0x%08x\n ", fZc);
   printf("\tTimeStamp:       %lld\n", GetTimeStamp());
   if(HasWave())
      printf("Has a wave form stored.\n");
   else
      printf("Does Not have a wave form stored.\n");
}

bool TFragment::IsDetector(const char* prefix, Option_t* opt) const
{
   // Checks to see if the current fragment constains the same "prefix", for example "GRG"
   // The option determines whether the channel should be:
   // - C : The core of a segmented detector
   // - S : The segments of a segmented detector
   // - A : Low gain output of a detector
   // - B : High gain output of a detectora
   // If C or S are not given, default to C
   // If A or B are not given, default to A
   // Note that multiple options add to the output, so "CAB" would return the core with both high and low gain
   // One should eventually add N,P,T options as well.
   std::string pre      = prefix;
   TString     option   = opt;
   std::string channame = this->GetName();
   if(channame.length() < 9) return false;

   option.ToUpper();
   // Could also do everything below with TMnemonic. This limits the amount of string processing that needs to be done
   // Because it returns false after every potential failure while the mnemonic class sets all of the strings, and then
   // checks
   // for conditions.
   if(!channame.compare(0, pre.length(), pre)) { // channame.BeginsWith(pre)){
      if(option.Length() < 1)                    // no option.
         return true;
      if(channame.length() > 8) {
         if(option.Contains("B") && (std::toupper(channame[9]) == std::toupper('B')))
            return true;
         else if(option.Contains("A") && (std::toupper(channame[9]) == std::toupper('A')))
            return true;
      }
      if(option.Contains("C") && !channame.compare(7, 2, "00"))
         return true;
      else if(option.Contains("S") && channame.compare(7, 2, "00"))
         return true;
   } else
      return false;

   return false;
}

Int_t TFragment::GetSharcMesyBoard() const
{
   int slave   = (fAddress & 0x00f00000) >> 20;
   int port    = (fAddress & 0x00000f00) >> 8;
   int channel = (fAddress & 0x000000ff);

   // printf("slave = 0x%08x    port = 0x%08x  channel = 0x%08x\n",slave,port,channel);

   if(slave != 0x1 && slave != 0x2) return -1;
   if(channel == 0x1f) {
      return (slave - 1) * 16 + (port - 1) * 2 + 1;
   } else if(channel == 0x3f) {
      return (slave - 1) * 16 + (port - 1) * 2 + 2;
   }
   return -1;
}
