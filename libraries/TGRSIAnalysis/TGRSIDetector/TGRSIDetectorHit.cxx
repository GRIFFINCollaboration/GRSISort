#include "TGRSIDetectorHit.h"

#include "TClass.h"

/// \cond CLASSIMP
ClassImp(TGRSIDetectorHit)
/// \endcond

TPPG* TGRSIDetectorHit::fPPG = 0;

TVector3 TGRSIDetectorHit::fBeamDirection(0,0,1);

TGRSIDetectorHit::TGRSIDetectorHit(const int& Address) : TObject() { 
	///Default constructor
	Clear();
	fAddress = Address;
	if(!fPPG)
		fPPG = static_cast<TPPG*>(gDirectory->Get("TPPG")); //There Might be a better way to do this

#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::TGRSIDetectorHit(const TGRSIDetectorHit& rhs) : TObject() { 
	///Default Copy constructor
	rhs.Copy(*this);
	fPPG = static_cast<TPPG*>(gDirectory->Get("TPPG")); //There Might be a better way to do this
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::~TGRSIDetectorHit() {
	//Default destructor
}

void TGRSIDetectorHit::Streamer(TBuffer& R__b) {
	/// Stream an object of class TGRSIDetectorHit.
	if (R__b.IsReading()) {
		R__b.ReadClassBuffer(TGRSIDetectorHit::Class(),this);
	} else {
		fBitflags = 0;
		R__b.WriteClassBuffer(TGRSIDetectorHit::Class(),this);
	}
}

Double_t TGRSIDetectorHit::GetTime(Option_t* opt) const {
	if(IsTimeSet())
		return fTime;

	Double_t dTime = static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform());
	TChannel* chan = GetChannel();
	if(!chan) {
		Error("GetTime","No TChannel exists for address 0x%08x",GetAddress());
		return 10.*dTime;
	}

	return 10.*(dTime - chan->GetTZero(GetEnergy()));
}

Double_t TGRSIDetectorHit::GetTime(Option_t* opt) {
	if(IsTimeSet())
		return fTime;

	Double_t dTime = static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform());
	TChannel* chan = GetChannel();
	if(!chan) {
		Error("GetTime","No TChannel exists for address 0x%08x",GetAddress());
		return 10.*dTime;
	}

	SetTime(10.*(dTime - chan->GetTZero(GetEnergy())));

	return fTime;
}


double TGRSIDetectorHit::GetEnergy(Option_t* opt) const {
	if(IsEnergySet())
		return fEnergy;

	TChannel* chan = GetChannel();
	if(!chan) {
		Error("GetEnergy","No TChannel exists for address 0x%08x",GetAddress());
		return 0.;
	}
	return chan->CalibrateENG(GetCharge());
}

double TGRSIDetectorHit::GetEnergy(Option_t* opt) {
	if(IsEnergySet()) {
		return fEnergy;
	}

	TChannel* chan = GetChannel();
	if(chan == NULL) {
		Error("GetEnergy","No TChannel exists for address 0x%08x",GetAddress());
		return 0.;
	}
	SetEnergy(chan->CalibrateENG(GetCharge()));
	return fEnergy;
}

void TGRSIDetectorHit::Copy(TObject& rhs) const {
	TObject::Copy(rhs);
	static_cast<TGRSIDetectorHit&>(rhs).fAddress        = fAddress;
	static_cast<TGRSIDetectorHit&>(rhs).fPosition       = fPosition;
	static_cast<TGRSIDetectorHit&>(rhs).fWaveform       = fWaveform;
	static_cast<TGRSIDetectorHit&>(rhs).fCfd            = fCfd;
	static_cast<TGRSIDetectorHit&>(rhs).fTimeStamp      = fTimeStamp;
	static_cast<TGRSIDetectorHit&>(rhs).fCharge         = fCharge;
	//static_cast<TGRSIDetectorHit&>(rhs).fDetector       = fDetector;
  //static_cast<TGRSIDetectorHit&>(rhs).fSegment        = fSegment;
	static_cast<TGRSIDetectorHit&>(rhs).fEnergy         = fEnergy;
	static_cast<TGRSIDetectorHit&>(rhs).fTime           = fTime;

	static_cast<TGRSIDetectorHit&>(rhs).fBitflags       = 0;
	static_cast<TGRSIDetectorHit&>(rhs).fPPGStatus      = fPPGStatus;
	static_cast<TGRSIDetectorHit&>(rhs).fCycleTimeStamp = fCycleTimeStamp;
}

void TGRSIDetectorHit::Print(Option_t* opt) const {
	///General print statement for a TGRSIDetectorHit.
	///Currently prints nothing.
	fPosition.Print();
}

const char *TGRSIDetectorHit::GetName() const { 
  TChannel *channel = TChannel::GetChannel(fAddress);
  if(!channel)
     return Class()->ClassName();
  else
     return channel->GetChannelName();
}


void TGRSIDetectorHit::Clear(Option_t* opt) {
	///General clear statement for a TGRSIDetectorHit.
	fAddress = 0xffffffff;    // -1
	fPosition.SetXYZ(0,0,1);  // unit vector along the beam.
	fWaveform.clear();        // reset size to zero.
	fCharge         = 0;
	fCfd            = -1;
	fTimeStamp      = -1;
	//fDetector       = -1;
  // fSegment		    = -1;
	fEnergy         = 0.;
	fBitflags       = 0;
	fPPGStatus      = TPPG::kJunk;
	fCycleTimeStamp = 0;
}

Int_t TGRSIDetectorHit::GetDetector() const {
  
	//if(IsDetSet())
	//	return fDetector;

	//MNEMONIC mnemonic;
	TChannel* channel = GetChannel();
	if(!channel) {
		Error("GetDetector","No TChannel exists for address 0x%08x",GetAddress());
		return -1;
	}
	//ClearMNEMONIC(&mnemonic);
	//ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
  return channel->GetDetectorNumber(); //mnemonic.arrayposition;
  
}
/*
UInt_t TGRSIDetectorHit::GetDetector() {
	if(IsDetSet())
		return fDetector;

	MNEMONIC mnemonic;
	TChannel* channel = GetChannel();
	if(!channel) {
		Error("GetDetector","No TChannel exists for address 0x%08x",GetAddress());
		return -1;
	}
	ClearMNEMONIC(&mnemonic);
	ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
	return SetDetector(mnemonic.arrayposition);
}
*/
Int_t TGRSIDetectorHit::GetSegment() const {
   //if(IsSegSet())
   //  return fSegment;

   //MNEMONIC mnemonic;
   TChannel *channel = GetChannel();
   if(!channel){
      Error("GetSegment","No TChannel exists for address %08x",GetAddress());
      return -1;
   }
   //ClearMNEMONIC(&mnemonic);
   //ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
   //std::string name = channel->GetChannelName();
   //TString str = name[9];
   //if(str.IsDigit()){
   //	 std::string buf;
   //	 buf.clear(); buf.assign(name,7,3);
   //	 return (uint16_t)atoi(buf.c_str());
   //}
   //else{   
   //	 return mnemonic.segment;
   //}
   //return -1;
   return channel->GetSegmentNumber(); //mnemonic.arrayposition;
}
/*
Short_t TGRSIDetectorHit::GetSegment() {
   if(IsSegSet()){
     printf("Segment set:\t%i\n",fSegment); 
	 return fSegment;
   }

   MNEMONIC mnemonic;
   TChannel *channel = GetChannel();
   if(!channel){
      Error("GetSegment","No TChannel exists for address %u",GetAddress());
      return -1;
   }
   ClearMNEMONIC(&mnemonic);
   ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
   std::string name = channel->GetChannelName();
   TString str = name[9];
   if(str.IsDigit()){
   	 std::string buf;
   	 buf.clear(); buf.assign(name,7,3);
   	 return SetSegment((uint16_t)atoi(buf.c_str()));
   }
   else{   
   	 return SetSegment(mnemonic.segment);
   }
   return -1;
}
*/
/*
UInt_t TGRSIDetectorHit::SetDetector(const UInt_t& det) {
	fDetector = det;
	SetFlag(kIsDetSet,true);
	return fDetector;
}

Short_t TGRSIDetectorHit::SetSegment(const Short_t &seg) {
   fSegment = seg;
   SetFlag(kIsSegSet,true);
   return fSegment;
}
*/
TVector3 TGRSIDetectorHit::SetPosition(Double_t dist) {
	///This should not be overridden. It's job is to call the correct 
	///position for the derived TGRSIDetector object.
	SetFlag(kIsPositionSet,true);
	fPosition = GetChannelPosition(dist); //Calls a general Hit GetPosition function
	return fPosition;
}

TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) const{
	///This should not be overridden and instead GetChannelPosition should
	///be used in the derived class.
	if(IsPosSet())
		return fPosition;

	return GetChannelPosition(dist); //Calls the derivative GetPosition function
	//We must do a check in here to make sure it is returning something reasonable
}

TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) {
	///This should not be overridden and instead GetChannelPosition should
	///be used in the derived class.
	if(IsPosSet())
		return fPosition;

  if(GetDetector()>0)
		return TGRSIDetectorHit::SetPosition(dist); 

	//GetDetector();
	//if(IsDetSet())
	//	return TGRSIDetectorHit::SetPosition(dist); 

	printf("no position found for current hit\n");
	return *GetBeamDirection();  //TVector3(0,0,1);
}

bool TGRSIDetectorHit::CompareEnergy(TGRSIDetectorHit* lhs, TGRSIDetectorHit* rhs) {
	return (lhs->GetEnergy() > rhs->GetEnergy());
}

uint16_t TGRSIDetectorHit::GetPPGStatus() const {
	if(IsPPGSet())
		return fPPGStatus;

	return TPPG::kJunk;
}

uint16_t TGRSIDetectorHit::GetPPGStatus() {
	if(IsPPGSet())
		return fPPGStatus;

	if(!fPPG)
		return TPPG::kJunk;

	fPPGStatus = fPPG->GetStatus(this->GetTime());
	fCycleTimeStamp = GetTime() - fPPG->GetLastStatusTime(GetTime());
	SetFlag(kIsPPGSet,true);
	return fPPGStatus;
}

uint16_t TGRSIDetectorHit::GetCycleTimeStamp() const {
	if(IsPPGSet())
		return fCycleTimeStamp;

	return 0;
}

uint16_t TGRSIDetectorHit::GetCycleTimeStamp() {
	if(IsPPGSet())
		return fCycleTimeStamp;

	if(!fPPG)
		return 0;

	fPPGStatus = fPPG->GetStatus(this->GetTime());
	fCycleTimeStamp = GetTime() - fPPG->GetLastStatusTime(GetTime());
	SetFlag(kIsPPGSet,true);
	return fCycleTimeStamp;
}

void TGRSIDetectorHit::CopyFragment(const TFragment& frag) {
	this->fAddress   = frag.ChannelAddress;  
	this->fCharge    = frag.GetCharge();
	this->fCfd       = frag.GetCfd();
	this->fTimeStamp = frag.GetTimeStamp();
	this->fPosition  = TVector3(0,0,1); 
	this->fEnergy    = frag.GetEnergy();
}

void TGRSIDetectorHit::CopyWaveform(const TFragment &frag) {
  if(frag.HasWave())
    SetWaveform(frag.wavebuffer); 
}

void TGRSIDetectorHit::SetFlag(enum Ebitflag flag, Bool_t set){
	if(set)
		fBitflags |= flag;
	else
		fBitflags &= (~flag);
}
