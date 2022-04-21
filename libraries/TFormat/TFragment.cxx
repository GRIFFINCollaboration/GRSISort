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

TFragment::TFragment() : TDetectorHit()
{
/// Default constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TFragment::TFragment(const TFragment& rhs) : TDetectorHit(rhs)
{
   /// Copy constructor
   // first copy all "normal" data members
   fDaqTimeStamp      = rhs.fDaqTimeStamp;
   fDaqId             = rhs.fDaqId;
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
   TDetectorHit::Clear(opt);

   fDaqTimeStamp      = 0;
   fDaqId             = 0;
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
	fEntryNumber   = 0;
   fZc            = 0;
   fCcShort       = 0;
   fCcLong        = 0;
   fNumberOfWords = 0;
}

TObject* TFragment::Clone(const char*) const
{
   auto* result = new TFragment;
   *result      = *this;
   result->ClearTransients();
   return result;
}

double TFragment::GetTZero() const
{
   TChannel* chan = GetChannel();
   if(chan == nullptr) {
      return 0.000;
   }
   return chan->GetTZero(GetEnergy());
}

Int_t TFragment::Get4GCfd() const
{ // return a 4G cfd in terms of 1/256 ns since the trigger
   return static_cast<Int_t>(GetCfd()) & 0x001fffff;
}

ULong64_t TFragment::GetTimeInCycle()
{

   if(fPPG == nullptr) {
      fPPG = TPPG::Get(); // static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
   if(fPPG == nullptr) {
      return 0;
   }
   return fPPG->GetTimeInCycle(GetTimeStampNs());
}

ULong64_t TFragment::GetCycleNumber()
{
   if(fPPG == nullptr) {
      fPPG = TPPG::Get(); // static_cast<TPPG*>(gROOT->FindObject("TPPG"));
   }
   if(fPPG == nullptr) {
      return 0;
   }
   return fPPG->GetCycleNumber(GetTimeStampNs());
}

Short_t TFragment::GetChannelNumber() const
{
   TChannel* chan = TChannel::GetChannel(fAddress, true);
   if(chan == nullptr) {
      return 0;
   }
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
	Print(std::cout);
}

void TFragment::Print(std::ostream& out) const
{
	/// Print fragment to stream out in a thread-safe way
	std::ostringstream str;
	str<<"DaqTimeStamp:        "<<ctime(&fDaqTimeStamp)<<std::endl
		<<"DaqId:               "<<fDaqId<<std::endl
		<<"\tTriggerId["<<fTriggerId.size()<<"]";
	for(auto id : fTriggerId) {
		str<<"    0x"<<std::hex<<id<<std::dec;
	}
	str<<std::endl
		<<"FragmentId:           "<<fFragmentId<<std::endl
		<<"TriggerBit:           0x"<<std::hex<<fTriggerBitPattern<<std::dec<<std::endl
		<<"NetworkPacketNumber:  "<<fNetworkPacketNumber<<std::endl
		<<"Channel Address:      0x"<<std::hex<<GetAddress()<<std::dec<<std::endl;
	TChannel* channel = GetChannel();
	if(channel != nullptr) {
		str<<"Channel: "<<channel->GetNumber()<<"\t Name: "<<channel->GetName()<<std::endl;
	}
	str<<"Charge:               0x"<<std::hex<<static_cast<Int_t>(GetCharge())<<std::dec<<std::endl
		<<"Cfd:                  0x"<<std::hex<<static_cast<Int_t>(GetCfd())<<std::dec<<std::endl
		<<"ZC:                   0x"<<std::hex<<fZc<<std::dec<<std::endl
		<<"TimeStamp:            "<<GetTimeStamp()<<std::endl
		<<(HasWave()?"Has a wave form stored":"Doesn't have a wave form stored")<<std::endl;
	out<<str.str();
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
   std::string channame = GetName();
   if(channame.length() < 9) {
      return false;
   }

   option.ToUpper();
   // Could also do everything below with TMnemonic. This limits the amount of string processing that needs to be done
   // Because it returns false after every potential failure while the mnemonic class sets all of the strings, and then
   // checks
   // for conditions.
   if(channame.compare(0, pre.length(), pre) == 0) { // channame.BeginsWith(pre)){
      if(option.Length() < 1) {                      // no option.
         return true;
      }
      if(channame.length() > 8) {
         if(option.Contains("B") && (std::toupper(channame[9]) == std::toupper('B'))) {
            return true;
         }
         if(option.Contains("A") && (std::toupper(channame[9]) == std::toupper('A'))) {
            return true;
         }
      }
      if(option.Contains("C") && (channame.compare(7, 2, "00") == 0)) {
         return true;
      }
      if(option.Contains("S") && (channame.compare(7, 2, "00") != 0)) {
         return true;
      }
   } else {
      return false;
   }

   return false;
}

Int_t TFragment::GetSharcMesyBoard() const
{
   int slave   = (fAddress & 0x00f00000) >> 20;
   int port    = (fAddress & 0x00000f00) >> 8;
   int channel = (fAddress & 0x000000ff);

   if(slave != 0x1 && slave != 0x2) {
      return -1;
   }
   if(channel == 0x1f) {
      return (slave - 1) * 16 + (port - 1) * 2 + 1;
   }
   if(channel == 0x3f) {
      return (slave - 1) * 16 + (port - 1) * 2 + 2;
   }
   return -1;
}
