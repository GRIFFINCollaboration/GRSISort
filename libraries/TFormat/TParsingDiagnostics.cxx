#include "TParsingDiagnostics.h"

#include <fstream>

#include "TChannel.h"

TParsingDiagnosticsData::TParsingDiagnosticsData()
{
   fMinChannelId = 0;
   fMaxChannelId = 0;

   fNumberOfHits = 0;

   fDeadTime = 0;
   fMinTimeStamp = 0;
   fMaxTimeStamp = 0;
}


TParsingDiagnosticsData::TParsingDiagnosticsData(const std::shared_ptr<const TFragment>& frag)
{
   fMinChannelId = frag->GetChannelId();
   fMaxChannelId = frag->GetChannelId();

   fNumberOfHits = 1;

   fDeadTime = frag->GetDeadTime();
   fMinTimeStamp = frag->GetTimeStampNs();
   fMaxTimeStamp = frag->GetTimeStampNs();
}

void TParsingDiagnosticsData::Update(const std::shared_ptr<const TFragment>& frag)
{
   UInt_t channelId      = frag->GetChannelId();
   long   timeStamp      = frag->GetTimeStampNs();
	
	// update minimum and maximum channel id if necessary
	if(channelId < fMinChannelId) {
		fMinChannelId = channelId;
	}
	if(channelId > fMaxChannelId) {
		fMaxChannelId = channelId;
	}

	++fNumberOfHits;

	// increment the dead time and set per channel min/max timestamps
	fDeadTime += frag->GetDeadTime();
	if(timeStamp < fMinTimeStamp) {
		fMinTimeStamp = timeStamp;
	}
	if(timeStamp > fMaxTimeStamp) {
		fMaxTimeStamp = timeStamp;
	}
}

void TParsingDiagnosticsData::Print(UInt_t address) const
{
	std::cout<<"channel 0x"<<std::hex<<std::setw(4)<<std::setfill('0')<<address<<std::dec
		<<std::setfill(' ')<<": "<<fDeadTime / 1e5<<" ms deadtime out of ";
	if(fMaxTimeStamp == fMinTimeStamp && fMaxChannelId == fMinChannelId) {
		std::cout<<"empty channel"<<std::endl;
	} else {
		std::cout<<std::setw(12)<<(fMaxTimeStamp - fMinTimeStamp) / 1e5
			<<"ms = "<<(100. * fDeadTime) / (fMaxTimeStamp - fMinTimeStamp)<<" %"
			<<std::endl;
	}
}

TParsingDiagnostics::TParsingDiagnostics() : TSingleton<TParsingDiagnostics>()
{
	fIdHist         = nullptr; // need to initial this
	Clear();
}

TParsingDiagnostics::TParsingDiagnostics(const TParsingDiagnostics&) : TSingleton<TParsingDiagnostics>()
{
	fIdHist         = nullptr; // need to initial this
	Clear();
}

TParsingDiagnostics::~TParsingDiagnostics()
{
	delete fIdHist;
}

void TParsingDiagnostics::Copy(TObject& obj) const
{
	static_cast<TParsingDiagnostics&>(obj).fPPGCycleLength         = fPPGCycleLength;
	static_cast<TParsingDiagnostics&>(obj).fNumberOfGoodFragments  = fNumberOfGoodFragments;
	static_cast<TParsingDiagnostics&>(obj).fNumberOfBadFragments   = fNumberOfBadFragments;
	static_cast<TParsingDiagnostics&>(obj).fChannelAddressData     = fChannelAddressData;
	static_cast<TParsingDiagnostics&>(obj).fMinNetworkPacketNumber = fMinNetworkPacketNumber;
	static_cast<TParsingDiagnostics&>(obj).fMaxNetworkPacketNumber = fMaxNetworkPacketNumber;
	static_cast<TParsingDiagnostics&>(obj).fNumberOfNetworkPackets = fNumberOfNetworkPackets;
}

void TParsingDiagnostics::Clear(Option_t*)
{
	delete fIdHist;
	fIdHist         = nullptr;
	fPPGCycleLength = 0;
	fNumberOfGoodFragments.clear();
	fNumberOfBadFragments.clear();
	fMinDaqTimeStamp      = 0;
	fMaxDaqTimeStamp      = 0;
	fMinNetworkPacketNumber = 0x7fffffff; // just a large number
	fMaxNetworkPacketNumber = 0;
	fNumberOfNetworkPackets = 0;
}

void TParsingDiagnostics::Print(Option_t*) const
{
	std::cout<<"Total run time of this (sub-)run is "<<fMaxDaqTimeStamp - fMinDaqTimeStamp<<" s"<<std::endl
		<<"PPG cycle is "<<fPPGCycleLength / 1e5<<" ms long."<<std::endl
		<<"Found "<<fNumberOfNetworkPackets<<" network packets in range "<<fMinNetworkPacketNumber<<" - "
		<<fMaxNetworkPacketNumber<<" => "
		<<100. * fNumberOfNetworkPackets / (fMaxNetworkPacketNumber - fMinNetworkPacketNumber + 1.)
		<<" % packet survival."<<std::endl;
	// loop over number of good fragments per detector type
	for(const auto& fNumberOfGoodFragment : fNumberOfGoodFragments) {
		std::cout<<"detector type "<<std::setw(2)<<fNumberOfGoodFragment.first<<": "<<std::setw(12)
			<<fNumberOfGoodFragment.second<<" good, ";
		// check if we have corresponding bad fragment for this detector type
		if(fNumberOfBadFragments.find(fNumberOfGoodFragment.first) == fNumberOfBadFragments.end()) {
			std::cout<<"          no";
		} else {
			std::cout<<std::setw(12)<<fNumberOfBadFragments.at(fNumberOfGoodFragment.first)<<" ("
				<<(100. * fNumberOfBadFragments.at(fNumberOfGoodFragment.first)) / fNumberOfGoodFragment.second
				<<" %)";
		}
		std::cout<<" bad fragments."<<std::endl;
	}
	for(const auto& it : fChannelAddressData) {
		it.second.Print(it.first);
	}
}

void TParsingDiagnostics::GoodFragment(const std::shared_ptr<const TFragment>& frag)
{
	/// increment the counter of good fragments for this detector type and check if any trigger ids have been lost
	fNumberOfGoodFragments[frag->GetDetectorType()]++;

	UInt_t channelAddress = frag->GetAddress();
	if(fChannelAddressData.find(channelAddress) == fChannelAddressData.end()) {
		fChannelAddressData[channelAddress] = TParsingDiagnosticsData(frag);
	} else {
		fChannelAddressData[channelAddress].Update(frag);
	}

	// check if this is a new minimum/maximum network packet id
	if(frag->GetNetworkPacketNumber() > 0) {
		++fNumberOfNetworkPackets;
		if(frag->GetNetworkPacketNumber() < fMinNetworkPacketNumber) {
			fMinNetworkPacketNumber = frag->GetNetworkPacketNumber();
		}
		if(frag->GetNetworkPacketNumber() > fMaxNetworkPacketNumber) {
			fMaxNetworkPacketNumber = frag->GetNetworkPacketNumber();
		}
	}

	if(fMinDaqTimeStamp == 0 || frag->GetDaqTimeStamp() < fMinDaqTimeStamp) {
		fMinDaqTimeStamp = frag->GetDaqTimeStamp();
	}
	if(fMaxDaqTimeStamp == 0 || frag->GetDaqTimeStamp() > fMaxDaqTimeStamp) {
		fMaxDaqTimeStamp = frag->GetDaqTimeStamp();
	}
}

void TParsingDiagnostics::ReadPPG(TPPG* ppg)
{
	/// store different TPPG diagnostics like cycle length, length of each state, offset, how often each state was found
	if(ppg == nullptr) {
		return;
	}
	fPPGCycleLength = ppg->GetCycleLength();
}

void TParsingDiagnostics::Draw(Option_t* opt)
{
	UInt_t minChannel = fChannelAddressData.begin()->first;
	UInt_t maxChannel = fChannelAddressData.begin()->first;

	for(auto it : fChannelAddressData) {
		if(it.first < minChannel) minChannel = it.first;
		if(it.first > maxChannel) maxChannel = it.first;
	}

	// check that the histogram (if it already exists) has the right number of bins
	if(fIdHist != nullptr && fIdHist->GetNbinsX() != static_cast<Int_t>(maxChannel - minChannel + 1)) {
		delete fIdHist;
		fIdHist = nullptr;
	}
	if(fIdHist == nullptr) {
		fIdHist = new TH1F("IdHist", "Event survival;channel number;survival rate [%]", maxChannel - minChannel + 1,
				minChannel, maxChannel + 1);
	} else {
		// the histogram already had the right number of bins, but to be save we set the range
		fIdHist->SetAxisRange(minChannel, maxChannel + 1);
	}

	for(auto it : fChannelAddressData) {
		if(it.second.MinChannelId() != 0 || it.second.MinChannelId() != it.second.MaxChannelId()) {
			fIdHist->SetBinContent(fIdHist->GetXaxis()->FindBin(it.first),
					(100. * it.second.NumberOfHits()) / (it.second.MaxChannelId() - it.second.MinChannelId() + 1.));
		}
	}

	fIdHist->Draw(opt);
}

void TParsingDiagnostics::WriteToFile(const char* fileName) const
{
	std::ofstream statsOut(fileName);
	statsOut<<std::endl
		<<"Run time to the nearest second = "<<fMaxDaqTimeStamp - fMinDaqTimeStamp<<std::endl
		<<std::endl;

	statsOut<<"Good fragments:";
	for(auto it : fNumberOfGoodFragments) {
		statsOut<<" "<<it.second<<" of type "<<it.first;
	}
	statsOut<<std::endl;

	statsOut<<"Bad fragments:";
	for(auto it : fNumberOfBadFragments) {
		statsOut<<" "<<it.second<<" of type "<<it.first;
	}
	statsOut<<std::endl;

	for(auto it : fChannelAddressData) {
		TChannel* chan = TChannel::GetChannel(it.first, false);
		if(chan == nullptr) {
			continue;
		}
		statsOut<<"0x"<<std::hex<<it.first<<std::dec<<":\t"<<chan->GetName()
			<<"\tdead time: "<<static_cast<float>(it.second.DeadTime()) / 1e9<<" seconds."<<std::endl;
	}
	statsOut<<std::endl;

	statsOut.close();
}
