#include "TDiagnostics.h"

TDiagnostics::TDiagnostics() : TObject() {
	fIdHist = NULL;
	Clear();
}

TDiagnostics::TDiagnostics(const TDiagnostics& rhs) : TObject() {
	fIdHist = NULL;
	Clear();
}

TDiagnostics::~TDiagnostics() {
	if(fIdHist != NULL) delete fIdHist;
}

void TDiagnostics::Copy(TObject& obj) const {
	static_cast<TDiagnostics&>(obj).fPPGCycleLength = fPPGCycleLength;
	static_cast<TDiagnostics&>(obj).fNumberOfGoodFragments = fNumberOfGoodFragments;
	static_cast<TDiagnostics&>(obj).fNumberOfBadFragments = fNumberOfBadFragments;
	static_cast<TDiagnostics&>(obj).fMinChannelId = fMinChannelId;
	static_cast<TDiagnostics&>(obj).fMaxChannelId = fMaxChannelId;
	static_cast<TDiagnostics&>(obj).fMinNetworkPacketNumber = fMinNetworkPacketNumber;
	static_cast<TDiagnostics&>(obj).fMaxNetworkPacketNumber = fMaxNetworkPacketNumber;
	static_cast<TDiagnostics&>(obj).fNumberOfNetworkPackets = fNumberOfNetworkPackets;
	static_cast<TDiagnostics&>(obj).fNumberOfHits = fNumberOfHits;
}

void TDiagnostics::Clear(Option_t* opt) {
	if(fIdHist != NULL) delete fIdHist;
	fIdHist = NULL;
	fPPGCycleLength = 0;
	fNumberOfGoodFragments.clear();
	fNumberOfBadFragments.clear();
	fMinChannelId.clear();
	fMaxChannelId.clear();
	fMinNetworkPacketNumber = 0x7fffffff;
	fMaxNetworkPacketNumber = 0;
	fNumberOfNetworkPackets = 0;
	fNumberOfHits.clear();
}

void TDiagnostics::Print(Option_t* opt) const {
	std::cout<<"PPG cycle is "<<fPPGCycleLength/1e5<<" ms long."<<std::endl
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
}

void TDiagnostics::GoodFragment(TFragment* frag) {
	///increment the counter of good fragments for this detector type and check if any trigger ids have been lost
	fNumberOfGoodFragments[frag->DetectorType]++;

	Short_t channelNumber = frag->ChannelNumber;
	UInt_t channelId = frag->ChannelId;
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
	if(frag->NetworkPacketNumber > 0) {
		++fNumberOfNetworkPackets;
		if(frag->NetworkPacketNumber < fMinNetworkPacketNumber) {
			fMinNetworkPacketNumber = frag->NetworkPacketNumber;
		}
		if(frag->NetworkPacketNumber > fMaxNetworkPacketNumber) {
			fMaxNetworkPacketNumber = frag->NetworkPacketNumber;
		}
	}
}

void TDiagnostics::Read(TPPG* ppg) {
	///store different TPPG diagnostics like cycle length, length of each state, offset, how often each state was found
	if(ppg == NULL) return;
	fPPGCycleLength = ppg->GetCycleLength();
	
}

void TDiagnostics::Draw(Option_t* opt) {
	Short_t minChannel = fNumberOfHits.begin()->first;
	Short_t maxChannel = std::prev(fNumberOfHits.end())->first;

	//check that the histogram (if it already exists) has the right number of bins
	if(fIdHist != NULL && fIdHist->GetNbinsX() != maxChannel-minChannel+1) {
		delete fIdHist;
		fIdHist = NULL;
	}
	if(fIdHist == NULL) {
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
