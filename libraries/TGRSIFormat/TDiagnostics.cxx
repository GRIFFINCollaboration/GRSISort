#include "TDiagnostics.h"

TDiagnostics::TDiagnostics() : TObject() {
	Clear();
}

TDiagnostics::TDiagnostics(const TDiagnostics& rhs) : TObject() {
	Clear();
}

TDiagnostics::~TDiagnostics() {
}

void TDiagnostics::Copy(TObject& obj) const {
	static_cast<TDiagnostics&>(obj).fPPGCycleLength = fPPGCycleLength;
	static_cast<TDiagnostics&>(obj).fNumberOfGoodFragments = fNumberOfGoodFragments;
	static_cast<TDiagnostics&>(obj).fNumberOfBadFragments = fNumberOfBadFragments;
}

void TDiagnostics::Clear(Option_t* opt) {
}

void TDiagnostics::Print(Option_t* opt) const {
}

void TDiagnostics::GoodFragment(TFragment* frag) {
	///increment the counter of good fragments for this detector type and check if any trigger ids have been lost
	fNumberOfGoodFragments[frag->DetectorType]++;

	Short_t channelNumber = frag->ChannelNumber;
	UInt_t channelId = frag->ChannelId;
	Long_t triggerId = frag->TriggerId;
	//check if this channel has been found before
	if(fMinChannelId.find(channelNumber) == fMinChannelId.end()) {
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
}

void TDiagnostics::Read(TPPG* ppg) {
	///store different TPPG diagnostics like cycle length, length of each state, offset, how often each state was found
	if(ppg == NULL) return;
	fPPGCycleLength = ppg->GetCycleLength();
	
}

