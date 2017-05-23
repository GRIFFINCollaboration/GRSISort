#include "TParsingDiagnostics.h"

#include <fstream>

#include "TChannel.h"

TParsingDiagnostics* TParsingDiagnostics::fParsingDiagnostics = nullptr;

TParsingDiagnostics::TParsingDiagnostics() : TObject() {
	fIdHist = nullptr;
	Clear();
}

TParsingDiagnostics::TParsingDiagnostics(const TParsingDiagnostics&) : TObject() {
	fIdHist = nullptr;
	Clear();
}

TParsingDiagnostics::~TParsingDiagnostics() {
	if(fIdHist != nullptr) delete fIdHist;
}

void TParsingDiagnostics::Copy(TObject& obj) const {
	static_cast<TParsingDiagnostics&>(obj).fPPGCycleLength = fPPGCycleLength;
	static_cast<TParsingDiagnostics&>(obj).fNumberOfGoodFragments = fNumberOfGoodFragments;
	static_cast<TParsingDiagnostics&>(obj).fNumberOfBadFragments = fNumberOfBadFragments;
	static_cast<TParsingDiagnostics&>(obj).fMinChannelId = fMinChannelId;
	static_cast<TParsingDiagnostics&>(obj).fMaxChannelId = fMaxChannelId;
	static_cast<TParsingDiagnostics&>(obj).fDeadTime = fDeadTime;
	static_cast<TParsingDiagnostics&>(obj).fMinTimeStamp = fMinTimeStamp;
	static_cast<TParsingDiagnostics&>(obj).fMaxTimeStamp = fMaxTimeStamp;
	static_cast<TParsingDiagnostics&>(obj).fMinMidasTimeStamp = fMinMidasTimeStamp;
	static_cast<TParsingDiagnostics&>(obj).fMaxMidasTimeStamp = fMaxMidasTimeStamp;
	static_cast<TParsingDiagnostics&>(obj).fMinNetworkPacketNumber = fMinNetworkPacketNumber;
	static_cast<TParsingDiagnostics&>(obj).fMaxNetworkPacketNumber = fMaxNetworkPacketNumber;
	static_cast<TParsingDiagnostics&>(obj).fNumberOfNetworkPackets = fNumberOfNetworkPackets;
	static_cast<TParsingDiagnostics&>(obj).fNumberOfHits = fNumberOfHits;
}

void TParsingDiagnostics::Clear(Option_t*) {
	if(fIdHist != nullptr) delete fIdHist;
	fIdHist = nullptr;
	fPPGCycleLength = 0;
	fNumberOfGoodFragments.clear();
	fNumberOfBadFragments.clear();
	fMinChannelId.clear();
	fMaxChannelId.clear();
	fDeadTime.clear();
	fMinTimeStamp.clear();
	fMaxTimeStamp.clear();
	fMinMidasTimeStamp = 0;
	fMaxMidasTimeStamp = 0;
	fMinNetworkPacketNumber = 0x7fffffff; // just a large number
	fMaxNetworkPacketNumber = 0;
	fNumberOfNetworkPackets = 0;
	fNumberOfHits.clear();
}

void TParsingDiagnostics::Print(Option_t*) const {
	std::cout<<"Total run time of this (sub-)run is "<<fMaxMidasTimeStamp-fMinMidasTimeStamp<<" s"<<std::endl
				<<"PPG cycle is "<<fPPGCycleLength/1e5<<" ms long."<<std::endl
				<<"Found "<<fNumberOfNetworkPackets<<" network packets in range "<<fMinNetworkPacketNumber<<" - "<<fMaxNetworkPacketNumber<<" => "<<100.*fNumberOfNetworkPackets/(fMaxNetworkPacketNumber-fMinNetworkPacketNumber+1.)<<" % packet survival."<<std::endl;
	for(auto it = fNumberOfGoodFragments.begin(); it != fNumberOfGoodFragments.end(); ++it) {
		std::cout<<"detector type "<<std::setw(2)<<it->first<<": "<<std::setw(12)<<it->second<<" good, ";
		if(fNumberOfBadFragments.find(it->first) == fNumberOfBadFragments.end()) {
			std::cout<<"          no";
		} else {
			std::cout<<std::setw(12)<<fNumberOfBadFragments.at(it->first)<<" ("<<(100.*fNumberOfBadFragments.at(it->first))/it->second<<" %)";
		}
		std::cout<<" bad fragments."<<std::endl;
	}
	for(auto it = fDeadTime.begin(); it != fDeadTime.end(); ++it) {
	  std::cout<<"channel 0x"<<std::hex<<std::setw(4)<<std::setfill('0')<<it->first<<std::dec<<std::setfill(' ')<<": "<<it->second/1e5<<" ms deadtime out of ";
		if(fMinTimeStamp.find(it->first) == fMinTimeStamp.end() || fMaxTimeStamp.find(it->first) == fMaxTimeStamp.end()) {
		  std::cout<<"nonexisting channel???"<<std::endl;
		} else {
		  std::cout<<std::setw(12)<<(fMaxTimeStamp.at(it->first)-fMinTimeStamp.at(it->first))/1e5<<" ms = "<<(100.*it->second)/(fMaxTimeStamp.at(it->first)-fMinTimeStamp.at(it->first))<<" %"<<std::endl;
		}
	}
}

void TParsingDiagnostics::GoodFragment(std::shared_ptr<const TFragment> frag) {
	///increment the counter of good fragments for this detector type and check if any trigger ids have been lost
	fNumberOfGoodFragments[frag->GetDetectorType()]++;

	Short_t channelNumber = frag->GetChannelNumber();
	UInt_t channelId = frag->GetChannelId();
	long timeStamp = frag->GetTimeStamp();
	//Long_t triggerId = frag->TriggerId;
	//check if this is a new minimum/maximum of the channel id
	if(fMinChannelId.find(channelNumber) == fMinChannelId.end()) { //check if this channel has been found before
		fMinChannelId[channelNumber] = channelId;
		fMaxChannelId[channelNumber] = channelId;
	} else {
		if(channelId < fMinChannelId[channelNumber]) {
			fMinChannelId[channelNumber] = channelId;
		}
		if(channelId > fMaxChannelId[channelNumber]) {
			fMaxChannelId[channelNumber] = channelId;
		}
	}

	//count the number of hits for this channel
	if(fNumberOfHits.find(channelNumber) == fNumberOfHits.end()) {
		fNumberOfHits[channelNumber] = 0;
	}
	++fNumberOfHits[channelNumber];

	//check if this is a new minimum/maximum network packet id
	if(frag->GetNetworkPacketNumber() > 0) {
		++fNumberOfNetworkPackets;
		if(frag->GetNetworkPacketNumber() < fMinNetworkPacketNumber) {
			fMinNetworkPacketNumber = frag->GetNetworkPacketNumber();
		}
		if(frag->GetNetworkPacketNumber() > fMaxNetworkPacketNumber) {
			fMaxNetworkPacketNumber = frag->GetNetworkPacketNumber();
		}
	}
	//increment the dead time and set per channel min/max timestamps
	if(fDeadTime.find(channelNumber) == fDeadTime.end()) {
	  fDeadTime[channelNumber] = frag->GetDeadTime();
	  fMinTimeStamp[channelNumber] = timeStamp;
	  fMaxTimeStamp[channelNumber] = timeStamp;
	} else {
	  fDeadTime[channelNumber] += frag->GetDeadTime();
	  if(timeStamp < fMinTimeStamp[channelNumber]) {
			fMinTimeStamp[channelNumber] = timeStamp;
		}
		if(timeStamp > fMaxTimeStamp[channelNumber]) {
			fMaxTimeStamp[channelNumber] = timeStamp;
		}
	}
	
	if(fMinMidasTimeStamp == 0 || frag->GetMidasTimeStamp() < fMinMidasTimeStamp) {
	  fMinMidasTimeStamp = frag->GetMidasTimeStamp();
	}
	if(fMaxMidasTimeStamp == 0 || frag->GetMidasTimeStamp() > fMaxMidasTimeStamp) {
	  fMaxMidasTimeStamp = frag->GetMidasTimeStamp();
	}
}

void TParsingDiagnostics::ReadPPG(TPPG* ppg) {
	///store different TPPG diagnostics like cycle length, length of each state, offset, how often each state was found
	if(ppg == nullptr) return;
	fPPGCycleLength = ppg->GetCycleLength();
	
}

void TParsingDiagnostics::Draw(Option_t* opt) {
	Short_t minChannel = fNumberOfHits.begin()->first;
	Short_t maxChannel = std::prev(fNumberOfHits.end())->first;

	//check that the histogram (if it already exists) has the right number of bins
	if(fIdHist != nullptr && fIdHist->GetNbinsX() != maxChannel-minChannel+1) {
		delete fIdHist;
		fIdHist = nullptr;
	}
	if(fIdHist == nullptr) {
		fIdHist = new TH1F("IdHist","Event survival;channel number;survival rate [%]", maxChannel-minChannel+1, minChannel, maxChannel+1);
	} else {
		//the histogram already had the right number of bins, but to be save we set the range
		fIdHist->SetAxisRange(minChannel, maxChannel+1);
	}

	for(auto it = fNumberOfHits.begin(); it != fNumberOfHits.end(); ++it) {
		if(fMinChannelId.find(it->first) != fMinChannelId.end() && fMaxChannelId.find(it->first) != fMaxChannelId.end()) {
			fIdHist->SetBinContent(fIdHist->GetXaxis()->FindBin(it->first), (100.*fNumberOfHits.at(it->first))/(fMaxChannelId.at(it->first) - fMinChannelId.at(it->first) + 1.));
		}
	}

	fIdHist->Draw(opt);
}

void TParsingDiagnostics::WriteToFile(const char* fileName) const {
	std::ofstream statsOut(fileName);
	statsOut<<std::endl
			  <<"Run time to the nearest second = "<<fMaxMidasTimeStamp-fMinMidasTimeStamp<<std::endl
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

	for(auto it : fDeadTime) {
		TChannel* chan = TChannel::GetChannel(it.first);
		if(!chan)
			continue;
		statsOut<<"0x"<<std::hex<<it.first<<std::dec<<":\t"<<chan->GetName()<<"\tdead time: "<<static_cast<float>(it.second)/1e8<<" seconds."<<std::endl;
	}
	statsOut<<std::endl;

	statsOut.close();
}
